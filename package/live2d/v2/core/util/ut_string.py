from typing import Union, List


class UtString:
    @staticmethod
    def startswith(s: bytes, offset: int, pat: str) -> bool:
        end_pos = offset + len(pat)
        if end_pos >= len(s):
            return False

        for i in range(offset, end_pos, 1):
            if chr(s[i]) != pat[i - offset]:
                return False

        return True

    @staticmethod
    def createString(buf: bytes, offset: int, size: int) -> str:
        return buf[offset:offset + size].decode("utf-8")

    @staticmethod
    def strToFloat(s: bytes, length: int, offset: int, ret: List[bool]) -> float:
        result = 0
        _n = 10
        _p = False
        neg = chr(s[offset]) == '-'
        if neg:
            offset += 1
        while offset < length:
            c = chr(s[offset])
            if '0' <= c <= '9':
                if _p:
                    result += float(c) / _n
                    _n *= 10
                else:
                    result = result * 10 + float(c)
            elif c == '.':
                _p = True
            else:
                break

            offset += 1
        if neg:
            result = -result
        ret[0] = offset
        return result
