import json
import os
import sys
from typing import Any

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
from shared.core import Settings, response

try:
    import boto3
except ImportError:
    boto3 = None

settings = Settings()
dynamodb = boto3.resource("dynamodb", region_name=os.getenv("AWS_REGION", "us-east-1")) if boto3 else None


def handler(event: dict[str, Any], context: Any) -> dict[str, Any]:
    job_id = (event.get("pathParameters") or {}).get("jobId")
    if not job_id:
        return response(400, {"error": "jobId is required"})
    table = (event.get("_table") or dynamodb.Table(settings.table_name))
    item = table.get_item(Key={"jobId": job_id}).get("Item")
    if not item:
        return response(404, {"error": "job not found"})
    return response(200, item)
