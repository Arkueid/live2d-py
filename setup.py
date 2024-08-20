import configparser
import os
import platform
import re
import subprocess
import sys

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext

NAME = "live2d-py"
VERSION = "0.2.1"
DESCRIPTION = "Live2D Python SDK"
LONG_DESCRIPTION = open("README.md", "r", encoding="utf-8").read()
AUTHOR = "Arkueid"
AUTHOR_EMAIL = "thetardis@qq.com"
URL = "https://github.com/Arkueid/live2d-py"

INSTALL_REQUIRES = ["numpy"]


class CMakeExtension(Extension):

    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


def is_virtualenv():
    return 'VIRTUAL_ENV' in os.environ


def get_base_python_path(venv_path):
    return re.search("home = (.*)\n", open(os.path.join(venv_path, "pyvenv.cfg"), 'r').read()).group(1)


class CMakeBuild(build_ext):

    def get_cmake_version(self):
        try:
            out = subprocess.check_output(["cmake", "--version"])
        except:
            sys.stderr.write("CMake must be installed to build the following extensions: " + ", ".join(self.extensions))
            sys.exit(1)
        return re.search(r"cmake version ([0-9.]+)", out.decode()).group(1)

    def run(self):
        cmake_version = self.get_cmake_version()
        if platform.system() == "Windows":
            if cmake_version < "3.12":
                sys.stderr.write("CMake >= 3.12 is required on Windows")
        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = [
            "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=" + extdir,
            "-DPYTHON_EXECUTABLE=" + sys.executable
        ]
        build_args = ["--config", "Release"]

        if platform.system() == "Windows":
            cmake_args += [
                "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}".format("Release", extdir)
            ]
            if sys.maxsize > 2 ** 32:
                cmake_args += ["-A", "x64"]
            build_args += ["--", "/m"]
        else:
            cmake_args += ["-DCMAKE_BUILD_TYPE=" + "Release"]
            build_args += ["--", "-j2"]
            # raise Exception("Building on Windows is not supported yet")
        build_folder = os.path.abspath(self.build_temp)

        if not os.path.exists(build_folder):
            os.makedirs(build_folder)

        if is_virtualenv():
            python_installation_path = get_base_python_path(os.environ["VIRTUAL_ENV"])
        else:
            python_installation_path = os.path.split(sys.executable)[0]
        print("Python installation path: " + python_installation_path)
        sys.stdout.flush()

        cmake_args += ["-DPYTHON_INSTALLATION_PATH=" + python_installation_path]

        cmake_setup = ["cmake", ext.sourcedir] + cmake_args
        cmake_build = ["cmake", "--build", "."] + build_args

        print("Building extension for Python {}".format(sys.version.split('\n', 1)[0]))
        print("Invoking CMake setup: '{}'".format(' '.join(cmake_setup)))
        sys.stdout.flush()
        subprocess.check_call(cmake_setup, cwd=build_folder)
        print("Invoking CMake build: '{}'".format(' '.join(cmake_build)))
        sys.stdout.flush()
        subprocess.check_call(cmake_build, cwd=build_folder)


setup(
    name=NAME,
    version=VERSION,
    description=DESCRIPTION,
    long_description=LONG_DESCRIPTION,
    long_description_content_type="text/markdown",
    author=AUTHOR,
    author_email=AUTHOR_EMAIL,
    license="MIT",
    url=URL,
    install_requires=INSTALL_REQUIRES,
    ext_modules=[CMakeExtension("LAppModelWrapper", ".")],
    cmdclass=dict(build_ext=CMakeBuild),
    packages=find_packages("package"),
    package_data={"": ["*.pyd", "*.so"]},
    include_package_data=True,
    package_dir={"": "package"},
    keywords=["Live2D", "Cubism Live2D", "Cubism SDK", "Cubism SDK for Python"]
)
