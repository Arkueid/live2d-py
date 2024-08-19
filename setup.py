import os
import sys
import subprocess
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeBuild(build_ext):

    def run(self):
        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name))
        )

        cmake_args = [

        ]


setup(
    name="live2d-py"
)