import json
from pathlib import Path
from typing import Literal
from uuid import UUID

from pydantic import BaseModel, ConfigDict

from src_py.environment.paths import path_output
from src_py.utils.log import logger


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


def block_graph_read(path: Path) -> BlockGraph:
    """Read and validate a block graph JSON file."""
    data = json.loads(Path(path).read_text(encoding="utf-8"))
    return BlockGraph.model_validate(data)


read_block_graph = block_graph_read


def block_graph_get_block(block_graph: BlockGraph, block_id: UUID) -> Block:
    for block in block_graph.blockGraph:
        if block.blockId == block_id:
            return block

    raise ValueError(f"Block {block_id} not found in block graph")


def block_graph_get_obj_at_port(
    execution_id: UUID, block_graph: BlockGraph, this_block: Block, port_id: str
) -> tuple[str, str]:

    # Get the port from this block
    this_port = None
    for port in this_block.ports:
        if port.portId == port_id and port.direction == "input":
            this_port = port
            break

    if this_port is None:
        raise ValueError(
            f"Input port {port_id} not found in block {this_block.blockId}"
        )

    # There can only ever be one connection to an input port
    if len(this_port.connections) != 1:
        raise ValueError(
            f"Input port {port_id} has {len(this_port.connections)} connections in block {this_block.blockId}"
        )
    connection = this_port.connections[0]

    # Find the block connected to this port
    try:
        that_block = block_graph_get_block(block_graph, connection.blockId)
    except ValueError:
        logger.error("Failed to find block %s in graph", connection.blockId)
        raise

    # Get the type of the product being exchanged on these ports. Get this from
    # "that" block because "this" block may be of type "any", whereas "that"
    # block must have a specific type.
    that_port = next(
        (
            port
            for port in that_block.ports
            if port.portId == connection.portId and port.direction == "output"
        ),
        None,
    )

    if that_port is None:
        raise ValueError(
            f"Output port {connection.portId} not found in block {that_block.blockId}"
        )

    product_type = that_port.type

    # Read the output file of that block
    try:
        that_output_path = path_output(
            execution_id, that_block.block, that_block.blockId
        )
        with open(that_output_path, "r") as f:
            that_output = json.load(f)
    except Exception:
        logger.error("Failed to find output file for block %s", connection.blockId)
        raise

    # Return the file name of the object connected at that port
    for port in that_output.get("ports", []):
        if port.get("portId", "") == that_port.portId:
            return product_type, port.get("file", "")

    raise RuntimeError(
        f"Port {that_port.portId} not found in output of block {connection.blockId}"
    )
