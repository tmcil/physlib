import os
from pathlib import Path
from uuid import UUID

WORKING_DIR = Path(os.getenv("WORKING_DIR", ""))


def path_output(execution_id: UUID, block_name: str, block_id: UUID) -> Path:
    return (
        WORKING_DIR
        / str(execution_id)
        / "outputs"
        / f"{block_name}_{str(block_id).split('-')[-1]}.json"
    )
