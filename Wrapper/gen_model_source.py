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


with open("PyModel.cpp", "w") as f:
    f.write(includes)
    f.write(defines)