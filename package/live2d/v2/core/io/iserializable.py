from abc import ABC, abstractmethod

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .binary_reader import BinaryReader


class ISerializable(ABC):

    @abstractmethod
    def read(self, br: 'BinaryReader'):
        pass
