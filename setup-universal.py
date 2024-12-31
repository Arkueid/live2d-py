import os
from setuptools import setup, Extension, find_packages

NAME = "live2d-py"
VERSION = "0.3.2"  # TODO: edit before push
DESCRIPTION = "Live2D Python SDK"
LONG_DESCRIPTION = open("README.md", "r", encoding="utf-8").read()
AUTHOR = "Arkueid"
AUTHOR_EMAIL = "thetardis@qq.com"
URL = "https://github.com/Arkueid/live2d-py"
REQUIRES_PYTHON = [">=3.2"]
INSTALL_REQUIRES = ["numpy", "pyopengl", "pillow"]


class CMakeExtension(Extension):

    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[], py_limited_api=True)
        self.sourcedir = os.path.abspath(sourcedir)

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
    packages=find_packages("package"),
    package_data={"": ["*.pyd", "*.so"]},
    package_dir={"": "package"},
    keywords=["Live2D", "Cubism Live2D", "Cubism SDK", "Cubism SDK for Python"],
    python_requires=">=3.2,<=3.12",
    classifiers=[
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
    ]
)
