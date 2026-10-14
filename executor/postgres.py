import psycopg

from .log import logger

def test_db_connection():
    with psycopg.connect() as conn:
        with conn.cursor() as cur:
            cur.execute(
                "SELECT current_database(), current_user"
            )
            print(f"Message from postgres: {cur.fetchone()}")