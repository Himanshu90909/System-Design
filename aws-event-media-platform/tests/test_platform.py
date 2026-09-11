import json
import os
import sys
import unittest
from unittest.mock import Mock

ROOT = os.path.dirname(os.path.dirname(__file__))
sys.path.insert(0, os.path.join(ROOT, "src"))
from ingest.handler import handler as ingest_handler
from status.handler import handler as status_handler
from worker.handler import handler as worker_handler


class FakeTable:
    def __init__(self):
        self.items = {}

    def put_item(self, Item, ConditionExpression=None):
        if Item["jobId"] in self.items:
            raise RuntimeError("ConditionalCheckFailed")
        self.items[Item["jobId"]] = dict(Item)

    def get_item(self, Key):
        item = self.items.get(Key["jobId"])
        return {"Item": dict(item)} if item else {}

    def update_item(self, Key, UpdateExpression, ExpressionAttributeNames, ExpressionAttributeValues):
        item = self.items[Key["jobId"]]
        item["status"] = ExpressionAttributeValues.get(":processing", ExpressionAttributeValues.get(":completed"))
        if ":now" in ExpressionAttributeValues:
            field = "startedAt" if item["status"] == "PROCESSING" else "completedAt"
            item[field] = ExpressionAttributeValues[":now"]
        if ":output" in ExpressionAttributeValues:
            item["outputKey"] = ExpressionAttributeValues[":output"]


class FakeDynamo:
    def __init__(self, table): self.table = table
    def Table(self, name): return self.table


class FakeQueue:
    def __init__(self): self.messages = []
    def send_message(self, QueueUrl, MessageBody): self.messages.append({"body": MessageBody})


class PlatformTests(unittest.TestCase):
    def test_ingest_worker_and_status_flow(self):
        table, queue = FakeTable(), FakeQueue()
        event = {"body": json.dumps({"ownerId": "user-1", "filename": "clip.mp4", "contentType": "video/mp4"}), "_clients": (FakeDynamo(table), queue)}
        created = ingest_handler(event, None)
        self.assertEqual(created["statusCode"], 202)
        job_id = json.loads(created["body"])["jobId"]
        self.assertEqual(len(queue.messages), 1)

        worker_handler({"Records": queue.messages, "_table": table}, None)
        status = status_handler({"pathParameters": {"jobId": job_id}, "_table": table}, None)
        self.assertEqual(status["statusCode"], 200)
        self.assertEqual(json.loads(status["body"])["status"], "COMPLETED")

    def test_invalid_content_type(self):
        event = {"body": json.dumps({"ownerId": "user-1", "filename": "x.txt", "contentType": "text/plain"}), "_clients": (FakeDynamo(FakeTable()), FakeQueue())}
        self.assertEqual(ingest_handler(event, None)["statusCode"], 400)


if __name__ == "__main__":
    unittest.main()
