import sys
from uuid import UUID

from pydantic import BaseModel

from src_py.utils.log import logger


class CmdlineArgs(BaseModel):
    program_name: str
    execution_id: UUID
    block_id: UUID


def parse_cmdline() -> CmdlineArgs:

    try:
        args = CmdlineArgs(
            program_name=sys.argv[0],
            execution_id=UUID(sys.argv[1]),
            block_id=UUID(sys.argv[2]),
        )

    except Exception:
        logger.error("Invlid command line arguments")
        raise

    return args
