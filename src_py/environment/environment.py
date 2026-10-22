import os
from pathlib import Path

from src_py.environment.block_graph import BlockGraph, read_block_graph
from src_py.environment.cmdline import CmdlineArgs, parse_cmdline
from src_py.utils.log import logger

WORKING_DIR = Path(os.getenv("WORKING_DIR", ""))


def environment_init() -> tuple[CmdlineArgs, BlockGraph]:

    # Check working directory
    if not WORKING_DIR.exists():
        logger.error("Working directory does not exist: %s", WORKING_DIR)
        raise RuntimeError("Working directory does not exist: %s", WORKING_DIR)

    # Parse command line arguments
    try:
        args = parse_cmdline()
    except Exception:
        logger.error("Failed to parse command line arguments")
        raise

    # Read block graph
    try:
        block_graph = read_block_graph(
            WORKING_DIR / str(args.execution_id) / "inputs" / "block_graph.json"
        )
    except Exception:
        logger.error("Failed to read block graph")
        raise

    return args, block_graph
