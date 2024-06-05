from setuptools import setup, find_packages

setup(
    name='live2d-py',
    version='0.1',
    packages=find_packages(),
    package_data={
        '': ['*.pyd', '*.so', '*.pyi'],
    },
    author='Arkueid'
)