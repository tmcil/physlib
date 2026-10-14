import uuid

import psycopg

from .log import logger


def test_db_connection():
    with psycopg.connect() as conn, conn.cursor() as cur:
        cur.execute("SELECT current_database(), current_user")
        logger.info(f"Message from postgres: {cur.fetchone()}")


def pg_update_status(id: uuid.UUID, status: str):
    with psycopg.connect() as conn, conn.cursor() as cur:
        cur.execute(
            """
            UPDATE executions
            SET status = %s
            WHERE id = %s
            """,
            (status, str(id)),
        )

    if cur.rowcount == 0:
        raise ValueError(f"No execution found with id {id!s}")

    logger.info(f"Updated status ({status}): {id!s}")
