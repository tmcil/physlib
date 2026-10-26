import json
import uuid
from datetime import UTC, datetime, timedelta

import psycopg

from .log import logger


def pg_publish_product(execution_id: uuid.UUID, type: str, product: str):

    expires_at = (datetime.now(UTC) + timedelta(hours=1)).strftime(
        "%Y-%m-%dT%H:%M:%S.%f"
    )[:-3] + "Z"

    message = {
        "type": type,
        "status": "AVAILABLE",
        "claimed": False,
        "expires_at": expires_at,
        "file": product,
    }

    with psycopg.connect() as conn, conn.cursor() as cur:
        cur.execute(
            """
            UPDATE executions
            SET results = results || %s::jsonb,
                updated_at = NOW()
            WHERE id = %s
            """,
            (json.dumps([message]), str(execution_id)),
        )

        if cur.rowcount == 0:
            raise ValueError(f"No execution found with id {execution_id!s}")

    logger.info(f"Output product is available: {product}")
