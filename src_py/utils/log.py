import logging
import sys
from datetime import UTC, datetime


class ISO8601Formatter(logging.Formatter):
    def formatTime(self, record, datefmt=None):
        return datetime.fromtimestamp(record.created, tz=UTC).strftime(
            "%Y-%m-%dT%H:%M:%S"
        )


logging.basicConfig(level=logging.INFO)

handler = logging.StreamHandler(sys.stdout)
handler.addFilter(
    lambda record: record.name != "pika" and not record.name.startswith("pika.")
)
handler.setFormatter(ISO8601Formatter("%(asctime)s %(levelname)s (_PY) %(message)s"))
root = logging.getLogger()
root.handlers.clear()
root.addHandler(handler)
root.setLevel(logging.INFO)
logger = logging.getLogger(__name__)
