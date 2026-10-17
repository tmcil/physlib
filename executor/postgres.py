import uuid

import psycopg
from psycopg import sql

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
            SET status = %s,
                updated_at = NOW()
            WHERE id = %s
            """,
            (status, str(id)),
        )

    if cur.rowcount == 0:
        raise ValueError(f"No execution found with id {id!s}")

    logger.info(f"Updated status ({status}): {id!s}")


def pg_select_json(id: uuid.UUID, column: str) -> dict:

    with psycopg.connect() as conn, conn.cursor() as cur:
        cur.execute(
            sql.SQL("""
                SELECT {} FROM executions
                WHERE id = %s
            """).format(sql.Identifier(column)),
            (str(id),),
        )

        row = cur.fetchone()

        if row is None:
            raise ValueError(f"No execution found with id {id!s}")
        if row[0] == {}:
            raise ValueError(f"No JSON content found in {column} for id {id!s}")

    logger.info(f"Retrieved {column} from db")
    return row[0]
