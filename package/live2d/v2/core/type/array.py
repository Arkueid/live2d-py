def Float32Array(size: int):
    if not isinstance(size, int):
        raise Exception("invalid param")

    return [0.0] * size


def Float64Array(size: int):
    return Float32Array(size)


def Int8Array(size: int):
    return Array(size)


def Int16Array(size: int):
    return Array(size)


def Int32Array(size: int = None):
    if size is None:
        return []
    if not isinstance(size, int):
        raise Exception("invalid param")

    return [0] * size


def Array(size: int = None):
    if size is None:
        return []
    if not isinstance(size, int):
        raise Exception("invalid param")

    return [None] * size
