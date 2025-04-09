includes = """#include "PyModel.hpp"
"""

methods = [
    "Init",
    "Dealloc",
    "LoadModelJson",
    "GetModelHomeDir",
    "UpdateMotion",
    "UpdateDrag",
    "UpdateBreath",
    "UpdateBlink",
    "UpdateExpression",
    "UpdatePhysics",
    "UpdatePose",
    "GetParameterIds",
    "GetParameterValue",
    "GetParameterMaximumValue",
    "GetParameterMinimumValue",
    "GetParameterDefaultValue",
    "SetParameterValue",
    "SetParameterValueById",
    "AddParameterValue",
    "AddParameterValueById",
    "LoadParameters",
    "SaveParameters",
    "Resize",
    "SetOffset",
    "Rotate",
    "SetScale",
    "GetMvp",
    "StartMotion",
    "StartRandomMotion",
    "IsMotionFinished",
    "LoadExtraMotion",
    "GetMotions",
    "HitPart",
    "HitDrawable",
    "Drag",
    "IsAreaHit",
    "IsPartHit",
    "IsDrawableHit",
    "CreateRenderer",
    "Draw",
    "GetPartIds",
    "SetPartOpacity",
    "SetPartScreenColor",
    "SetPartMultiplyColor",
    "GetDrawableIds",
    "SetExpression",
    "GetExpressions",
    "SetRandomExpression",
    "ResetExpression",
    "SetDefaultExpression",
    "SetFadeOutExpression",
    "StopAllMotions",
    "ResetAllParameters",
    "ResetPose",
]

defines = ""
for i in methods:
    defines += "static PyObject* PyModel_" + i + "(PyModelObject* self, PyObject* args, PyObject* kwargs)\n{\n}\n"

PyModel_Methods = "static PyMethodDef PyModel_Methods[] = {\n"
for i in methods:
    PyModel_Methods += "\t{ \"" + i + "\", (PyCFunction)PyModel_" + i + ", METH_VARARGS | METH_KEYWORDS, nullptr },\n"

PyModel_Methods += "\t{ NULL }\n};\n"

ends = """static PyObject* PyModel_New(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    PyObject* self = (PyObject*)PyObject_Malloc(sizeof(PyModelObject));
    PyObject_Init(self, type);
    return self;
}

static PyType_Slot PyModel_slots[] = {
    {Py_tp_new, PyModel_New},
    {Py_tp_init, PyModel_Init},
    {Py_tp_dealloc, PyModel_Dealloc},
    {Py_tp_methods, PyModel_Methods},
    {0, 0},
};

PyType_Spec PyModel_Spec = {
    "live2d.Model",
    sizeof(PyModelObject),
    0,
    Py_TPFLAGS_DEFAULT,
    PyModel_slots,
};
"""

with open("PyModel.cpp", "w") as f:
    f.write(includes)
    f.write(defines)
    f.write(PyModel_Methods)
    f.write(ends)