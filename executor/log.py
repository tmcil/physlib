import logging
from datetime import UTC, datetime

logging.basicConfig(
    level=logging.DEBUG, format="%(asctime)s %(levelname)s (SYS) %(message)s"
)


class ISO8601Formatter(logging.Formatter):
    def formatTime(self, record, datefmt=None):
        return datetime.fromtimestamp(record.created, tz=UTC).strftime(
            "%Y-%m-%dT%H:%M:%S"
        )


logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
)

handler = logging.StreamHandler()
handler.setFormatter(ISO8601Formatter("%(asctime)s %(levelname)s %(message)s"))
root = logging.getLogger()
root.handlers.clear()
root.addHandler(handler)
root.setLevel(logging.INFO)
logger = logging.getLogger(__name__)
