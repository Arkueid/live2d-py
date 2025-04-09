#pragma once

#include <fine-grained/Model.hpp>

#define Py_LIMITED_API
#include <Python.h>

struct PyModelObject {
    PyObject_HEAD
    Model* model;
};

extern PyType_Spec PyModel_Spec;