import json
from pathlib import Path
from typing import Literal
from uuid import UUID

from pydantic import BaseModel, ConfigDict


class BlockGraphModel(BaseModel):
    model_config = ConfigDict(extra="ignore")


class PortConnection(BlockGraphModel):
    portId: str
    blockId: UUID


class Port(BlockGraphModel):
    type: str
    array: bool
    portId: str
    optional: bool
    direction: Literal["input", "output"]
    connections: list[PortConnection]


class Block(BlockGraphModel):
    block: str
    ports: list[Port]
    blockId: UUID


class BlockGraph(BlockGraphModel):
    blockGraph: list[Block]


def read_block_graph(path: Path) -> BlockGraph:
    """Read and validate a block graph JSON file."""
    data = json.loads(Path(path).read_text(encoding="utf-8"))
    return BlockGraph.model_validate(data)
