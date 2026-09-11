import os
import sys
from typing import Any

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from shared.core import Settings, ValidationError, new_id, now_epoch, parse_body, response

try:
    import boto3
except ImportError:  # local unit tests use injected clients
    boto3 = None


settings = Settings()
dynamodb = boto3.resource("dynamodb", region_name=os.getenv("AWS_REGION", "us-east-1")) if boto3 else None
sqs = boto3.client("sqs", region_name=os.getenv("AWS_REGION", "us-east-1")) if boto3 else None


def _clients(event: dict[str, Any]):
    return event.get("_clients", dynamodb, sqs)


def handler(event: dict[str, Any], context: Any) -> dict[str, Any]:
    try:
        body = parse_body(event)
        owner_id = body.get("ownerId")
        filename = body.get("filename")
        content_type = body.get("contentType", "application/octet-stream")
        if not owner_id or not filename or len(filename) > 256:
            raise ValidationError("ownerId and a filename up to 256 characters are required")
        if not content_type.startswith(("image/", "video/", "audio/")):
            raise ValidationError("contentType must be image, video, or audio")

        job_id = new_id("job")
        key = f"uploads/{owner_id}/{job_id}/{filename}"
        item = {"jobId": job_id, "ownerId": owner_id, "objectKey": key, "filename": filename,
                "contentType": content_type, "status": "QUEUED", "createdAt": now_epoch()}
        clients = event.get("_clients")
        table = clients[0].Table(settings.table_name) if clients else dynamodb.Table(settings.table_name)
        queue = clients[1] if clients else sqs
        table.put_item(Item=item, ConditionExpression="attribute_not_exists(jobId)")
        queue.send_message(QueueUrl=settings.queue_url, MessageBody=__import__("json").dumps(item))
        return response(202, {"jobId": job_id, "status": "QUEUED", "objectKey": key})
    except ValidationError as exc:
        return response(400, {"error": str(exc)})
    except Exception as exc:
        print(f"ingest failure: {exc}")
        return response(500, {"error": "unable to queue media job"})
