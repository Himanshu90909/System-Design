import json
import os
import sys
from typing import Any

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from shared.core import Settings, now_epoch

try:
    import boto3
except ImportError:
    boto3 = None

settings = Settings()
dynamodb = boto3.resource("dynamodb", region_name=os.getenv("AWS_REGION", "us-east-1")) if boto3 else None


def handler(event: dict[str, Any], context: Any) -> dict[str, Any]:
    table = event.get("_table") or dynamodb.Table(settings.table_name)
    processed = 0
    for record in event.get("Records", []):
        payload = json.loads(record["body"])
        job_id = payload["jobId"]
        current = table.get_item(Key={"jobId": job_id}).get("Item", {})
        if current.get("status") in {"PROCESSING", "COMPLETED"}:
            continue
        table.update_item(
            Key={"jobId": job_id},
            UpdateExpression="SET #s = :processing, startedAt = :now",
            ExpressionAttributeNames={"#s": "status"},
            ExpressionAttributeValues={":processing": "PROCESSING", ":now": now_epoch()},
        )
        # Production hook: invoke MediaConvert or an ffmpeg container here.
        table.update_item(
            Key={"jobId": job_id},
            UpdateExpression="SET #s = :completed, completedAt = :now, outputKey = :output",
            ExpressionAttributeNames={"#s": "status"},
            ExpressionAttributeValues={":completed": "COMPLETED", ":now": now_epoch(),
                                        ":output": f"processed/{job_id}/manifest.json"},
        )
        processed += 1
    return {"processed": processed}
