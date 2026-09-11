import json
import os
import time
import uuid
from dataclasses import dataclass
from typing import Any


class ValidationError(ValueError):
    pass


def response(status_code: int, body: dict[str, Any]) -> dict[str, Any]:
    return {"statusCode": status_code, "headers": {"Content-Type": "application/json"}, "body": json.dumps(body)}


def parse_body(event: dict[str, Any]) -> dict[str, Any]:
    raw = event.get("body") or "{}"
    try:
        body = json.loads(raw) if isinstance(raw, str) else raw
    except json.JSONDecodeError as exc:
        raise ValidationError("body must be valid JSON") from exc
    if not isinstance(body, dict):
        raise ValidationError("body must be a JSON object")
    return body


def new_id(prefix: str) -> str:
    return f"{prefix}_{uuid.uuid4().hex}"


def now_epoch() -> int:
    return int(time.time())


@dataclass(frozen=True)
class Settings:
    table_name: str = os.getenv("MEDIA_TABLE", "MediaJobs")
    bucket_name: str = os.getenv("MEDIA_BUCKET", "local-media-bucket")
    queue_url: str = os.getenv("MEDIA_QUEUE_URL", "local-queue")
