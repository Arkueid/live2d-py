from typing import List, Optional, Any


def Float32Array(size: int) -> List[float]:
    if not isinstance(size, int):
        raise Exception("invalid param")

    return [0.0] * size


def Float64Array(size: int) -> List[float]:
    return Float32Array(size)


def Int8Array(size: int) -> List[int]:
    return Int32Array(size)


def Int16Array(size: int) -> List[int]:
    return Int32Array(size)


def Int32Array(size: Optional[int] = None) -> List[int]:
    if size is None:
        return []
    if not isinstance(size, int):
        raise Exception("invalid param")

    return [0] * size


def Array(size: Optional[int] = None) -> List[Any]:
    if size is None:
        return []
    if not isinstance(size, int):
        raise Exception("invalid param")

    return [None] * size
