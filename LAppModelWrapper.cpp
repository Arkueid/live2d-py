#include <LAppModel.hpp>
#include <CubismFramework.hpp>
#include <LAppPal.hpp>
#include <LAppAllocator.hpp>
#include <Log.hpp>
#include <unordered_map>
#include <mutex>
#include <MatrixManager.hpp>
#include <Default.hpp>

#include <Python.h>

#ifdef WIN32
#include <Windows.h>
#endif

static LAppAllocator _cubismAllocator;
static Csm::CubismFramework::Option _cubismOption;

struct PyLAppModelObject
{
    PyObject_HEAD LAppModel *model;
    MatrixManager matrixManager;
    size_t key;
};

static std::unordered_map<size_t, LAppModel *> g_model;

// LAppModel()
static int PyLAppModel_init(PyLAppModelObject *self, PyObject *args, PyObject *kwds)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    self->model = new LAppModel();
    self->key = (size_t)self->model;
    g_model[self->key] = self->model;

    self->matrixManager.Initialize();
    Info("[M] allocate model: %p", self->key);

    PyGILState_Release(gstate);
    return 0;
}

static void PyLAppModel_dealloc(PyLAppModelObject *self)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    if (g_model.find(self->key) != g_model.end())
    {
        g_model.erase(self->key);
        delete self->model;
        Info("[M] release: LAppModel(at=%p)", self->model);
    }

    Info("[M] deallocate: PyLAppModelObject(at=%p)", self);

    Py_TYPE(self)->tp_free((PyObject *)self);
    PyGILState_Release(gstate);
}

// LAppModel->LoadAssets
static PyObject *PyLAppModel_LoadModelJson(PyLAppModelObject *self, PyObject *args)
{
    const char *fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName))
    {
        return NULL;
    }

    self->model->LoadAssets(fileName);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_Resize(PyLAppModelObject *self, PyObject *args)
{
    int ww, wh;
    if (!PyArg_ParseTuple(args, "ii", &ww, &wh))
    {
        PyErr_SetString(PyExc_TypeError, "invalid params.");
        return NULL;
    }

    self->matrixManager.UpdateScreenToScene(ww, wh);

    Py_RETURN_NONE;
}

// LAppModel->Update
static PyObject *PyLAppModel_Draw(PyLAppModelObject *self, PyObject *args)
{
    LAppPal::UpdateTime();

    self->model->Draw(self->matrixManager.GetProjection(self->model));
    Py_RETURN_NONE;
}

typedef Live2D::Cubism::Framework::ACubismMotion::FinishedMotionCallback FinishedMotionCallback;
typedef Live2D::Cubism::Framework::ACubismMotion ACubismMotion;

static PyObject *PyLAppModel_StartMotion(PyLAppModelObject *self, PyObject *args, PyObject *kwargs)
{
    const char *group;
    int no, priority;
    PyObject *onStartHandler = nullptr;
    PyObject *onFinishHandler = nullptr;

    static char *kwlist[] = {(char *)"group", (char *)"no", (char *)"priority", (char *)"onStartMotionHandler", (char *)"onFinishMotionHandler", NULL};
    if (!(PyArg_ParseTupleAndKeywords(args, kwargs, "sii|OO", kwlist, &group, &no, &priority, &onStartHandler, &onFinishHandler)))
    {
        return NULL;
    }

    bool isStartNull = true, isFinishNull = true;

    if (onStartHandler != nullptr)
    {
        isStartNull = Py_IsNone(onStartHandler);
        if (!isStartNull && !PyCallable_Check(onStartHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 4 must be callable.");
            return NULL;
        }
        Py_XINCREF(onStartHandler);
    }

    if (onFinishHandler != nullptr)
    {
        isFinishNull = Py_IsNone(onFinishHandler);
        if (!isFinishNull && !PyCallable_Check(onFinishHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 5 must be callable.");
            return NULL;
        }
        Py_XINCREF(onFinishHandler);
    }

    auto onStartCallback = [=](const char *group, int no)
    {
        if (isStartNull)
            return;
        PyObject *result = PyObject_CallFunction(onStartHandler, "si", group, no);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onStartHandler);
    };

    auto onFinishCallback = [=](Csm::ACubismMotion *)
    {
        if (isFinishNull)
            return;
        PyObject *result = PyObject_CallFunction(onFinishHandler, NULL);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onFinishHandler);
    };

    Csm::CubismMotionQueueEntryHandle handle = self->model->StartMotion(group, no, priority,
                                                                        onStartCallback,
                                                                        onFinishCallback);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_StartRandomMotion(PyLAppModelObject *self, PyObject *args, PyObject *kwargs)
{
    const char *group;
    int priority;

    PyObject *onStartHandler = nullptr;
    PyObject *onFinishHandler = nullptr;

    static char *kwlist[] = {(char *)"group", (char *)"priority", (char *)"onStartMotionHandler", (char *)"onFinishMotionHandler", NULL};
    if (!(PyArg_ParseTupleAndKeywords(args, kwargs, "si|OO", kwlist, &group, &priority, &onStartHandler, &onFinishHandler)))
    {
        return NULL;
    }
    bool isStartNull = true, isFinishNull = true;

    if (onStartHandler != nullptr)
    {
        isStartNull = Py_IsNone(onStartHandler);
        if (!isStartNull && !PyCallable_Check(onStartHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 4 must be callable.");
            return NULL;
        }
        Py_XINCREF(onStartHandler);
    }

    if (onFinishHandler != nullptr)
    {
        isFinishNull = Py_IsNone(onFinishHandler);
        if (!isFinishNull && !PyCallable_Check(onFinishHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 5 must be callable.");
            return NULL;
        }
        Py_XINCREF(onFinishHandler);
    }

    auto onStartCallback = [=](const char *group, int no)
    {
        if (isStartNull)
            return;
        PyObject *result = PyObject_CallFunction(onStartHandler, "si", group, no);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onStartHandler);
    };

    auto onFinishCallback = [=](Csm::ACubismMotion *)
    {
        if (isFinishNull)
            return;
        PyObject *result = PyObject_CallFunction(onFinishHandler, NULL);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onFinishHandler);
    };

    self->model->StartRandomMotion(group, priority, onStartCallback, onFinishCallback);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetExpression(PyLAppModelObject *self, PyObject *args)
{
    const char *expressionID;

    if (!(PyArg_ParseTuple(args, "s", &expressionID)))
    {
        return NULL;
    }

    self->model->SetExpression(expressionID);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetRandomExpression(PyLAppModelObject *self, PyObject *args)
{
    self->model->SetRandomExpression();
    Py_RETURN_NONE;
}

typedef Live2D::Cubism::Framework::csmString csmString;

static PyObject *PyLAppModel_HitTest(PyLAppModelObject *self, PyObject *args)
{
    float x, y;
    if (!(PyArg_ParseTuple(args, "ff", &x, &y)))
    {
        return NULL;
    }

    csmString area = self->model->HitTest(x, y);

    return Py_BuildValue("s", area.GetRawString());
}

static PyObject *PyLAppModel_HasMocConsistencyFromFile(PyLAppModelObject *self, PyObject *args)
{
    const char *mocFileName;
    if (!(PyArg_ParseTuple(args, "s", &mocFileName)))
    {
        return NULL;
    }

    bool result = self->model->HasMocConsistencyFromFile(mocFileName);

    if (result)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *PyLAppModel_Touch(PyLAppModelObject *self, PyObject *args, PyObject *kwargs)
{
    float mx, my;
    PyObject *onStartHandler = nullptr;
    PyObject *onFinishHandler = nullptr;

    static char *kwlist[] = {(char *)"mx", (char *)"my", (char *)"onStartMotionHandler", (char *)"onFinishMotionHandler", NULL};
    if (!(PyArg_ParseTupleAndKeywords(args, kwargs, "ff|OO", kwlist, &mx, &my, &onStartHandler, &onFinishHandler)))
    {
        return NULL;
    }

    bool isStartNull = true, isFinishNull = true;

    if (onStartHandler != nullptr)
    {
        isStartNull = Py_IsNone(onStartHandler);
        if (!isStartNull && !PyCallable_Check(onStartHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 4 must be callable.");
            return NULL;
        }
        Py_XINCREF(onStartHandler);
    }

    if (onFinishHandler != nullptr)
    {
        isFinishNull = Py_IsNone(onFinishHandler);
        if (!isFinishNull && !PyCallable_Check(onFinishHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 5 must be callable.");
            return NULL;
        }
        Py_XINCREF(onFinishHandler);
    }

    auto onStartCallback = [=](const char *group, int no)
    {
        if (isStartNull)
            return;
        PyObject *result = PyObject_CallFunction(onStartHandler, "si", group, no);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onStartHandler);
    };

    auto onFinishCallback = [=](Csm::ACubismMotion *)
    {
        if (isFinishNull)
            return;
        PyObject *result = PyObject_CallFunction(onFinishHandler, NULL);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onFinishHandler);
    };

    self->matrixManager.ScreenToScene(&mx, &my);

    csmString hitArea = self->model->HitTest(mx, my);
    if (strlen(hitArea.GetRawString()) != 0)
    {
        Info("hit area: [%s]", hitArea.GetRawString());
        if (strcmp(hitArea.GetRawString(), HIT_AREA_HEAD) == 0)
            self->model->SetRandomExpression();
        self->model->StartRandomMotion(hitArea.GetRawString(), MOTION_PRIORITY_FORCE, onStartCallback, onFinishCallback);
    }

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_Drag(PyLAppModelObject *self, PyObject *args)
{
    float mx, my;
    if (!(PyArg_ParseTuple(args, "ff", &mx, &my)))
    {
        return NULL;
    }

    self->matrixManager.ScreenToScene(&mx, &my);

    self->model->SetDragging(mx, my);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_IsMotionFinished(PyLAppModelObject *self, PyObject *args)
{

    if (self->model->IsMotionFinished())
    {
        Py_RETURN_TRUE;
    }
    else
    {
        Py_RETURN_FALSE;
    }
}

static PyObject *PyLAppModel_SetOffset(PyLAppModelObject *self, PyObject *args)
{
    float dx, dy;

    if (PyArg_ParseTuple(args, "ff", &dx, &dy) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Missing param 'float dx, float dy'");
        return NULL;
    }

    self->matrixManager.SetOffset(dx, dy);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetScale(PyLAppModelObject *self, PyObject *args)
{
    float scale;

    if (PyArg_ParseTuple(args, "f", &scale) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Missing param 'float scale'");
        return NULL;
    }

    self->matrixManager.SetScale(scale);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetParameterValue(PyLAppModelObject *self, PyObject *args)
{
    const char *paramId;
    float value, weight;

    if (PyArg_ParseTuple(args, "sff", &paramId, &value, &weight) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Invalid params (str, float, float)");
        return NULL;
    }

    self->model->SetParameterValue(paramId, value, weight);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_AddParameterValue(PyLAppModelObject *self, PyObject *args)
{
    const char *paramId;
    float value;

    if (PyArg_ParseTuple(args, "sf", &paramId, &value) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Invalid params (str, float)");
        return NULL;
    }

    self->model->AddParameterValue(paramId, value);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_Update(PyLAppModelObject *self, PyObject *args)
{

    self->model->Update();

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetAutoBreathEnable(PyLAppModelObject *self, PyObject *args)
{
    bool enable;

    if (PyArg_ParseTuple(args, "p", &enable) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Invalid param");
        return NULL;
    }

    self->model->SetAutoBreathEnable(enable);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetAutoBlinkEnable(PyLAppModelObject *self, PyObject *args)
{
    bool enable;

    if (PyArg_ParseTuple(args, "p", &enable) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Invalid param");
        return NULL;
    }

    self->model->SetAutoBlinkEnable(enable);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_GetParameterCount(PyLAppModelObject *self, PyObject *args)
{
    return PyLong_FromLong(self->model->GetParameterCount());
}

static PyObject *module_live2d_v3_params = nullptr;
static PyObject *class_parameter = nullptr;

static PyObject *PyLAppModel_GetParameter(PyLAppModelObject *self, PyObject *args)
{
    int index;
    if (PyArg_ParseTuple(args, "i", &index) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Invalid param");
        return NULL;
    }

    Parameter param = self->model->GetParameter(index);

    PyObject *instance = PyObject_CallObject(class_parameter, NULL);
    if (instance == NULL)
    {
        PyErr_Print();
        return NULL;
    }

    PyObject_SetAttrString(instance, "id", PyUnicode_FromString(param.id.c_str()));
    PyObject_SetAttrString(instance, "type", PyLong_FromLong(param.type));
    PyObject_SetAttrString(instance, "value", PyFloat_FromDouble(param.value));
    PyObject_SetAttrString(instance, "max", PyFloat_FromDouble(param.maxValue));
    PyObject_SetAttrString(instance, "min", PyFloat_FromDouble(param.minValue));
    PyObject_SetAttrString(instance, "default", PyFloat_FromDouble(param.defaultValue));

    return instance;
}

// 包装模块方法的方法列表
static PyMethodDef PyLAppModel_methods[] = {
    {"LoadModelJson", (PyCFunction)PyLAppModel_LoadModelJson, METH_VARARGS, ""},
    {"Resize", (PyCFunction)PyLAppModel_Resize, METH_VARARGS, ""},
    {"Draw", (PyCFunction)PyLAppModel_Draw, METH_VARARGS, ""},
    {"StartMotion", (PyCFunction)PyLAppModel_StartMotion, METH_VARARGS | METH_KEYWORDS, ""},
    {"StartRandomMotion", (PyCFunction)PyLAppModel_StartRandomMotion, METH_VARARGS | METH_KEYWORDS, ""},
    {"SetExpression", (PyCFunction)PyLAppModel_SetExpression, METH_VARARGS, ""},
    {"SetRandomExpression", (PyCFunction)PyLAppModel_SetRandomExpression, METH_VARARGS, ""},
    {"HitTest", (PyCFunction)PyLAppModel_HitTest, METH_VARARGS, "Get the name of the area being hit."},
    {"HasMocConsistencyFromFile", (PyCFunction)PyLAppModel_HasMocConsistencyFromFile, METH_VARARGS, ""},
    {"Touch", (PyCFunction)PyLAppModel_Touch, METH_VARARGS | METH_KEYWORDS, ""},
    {"Drag", (PyCFunction)PyLAppModel_Drag, METH_VARARGS, ""},
    {"IsMotionFinished", (PyCFunction)PyLAppModel_IsMotionFinished, METH_VARARGS, ""},
    {"SetOffset", (PyCFunction)PyLAppModel_SetOffset, METH_VARARGS, ""},
    {"SetScale", (PyCFunction)PyLAppModel_SetScale, METH_VARARGS, ""},
    {"SetParameterValue", (PyCFunction)PyLAppModel_SetParameterValue, METH_VARARGS, ""},
    {"AddParameterValue", (PyCFunction)PyLAppModel_AddParameterValue, METH_VARARGS, ""},
    {"Update", (PyCFunction)PyLAppModel_Update, METH_VARARGS, ""},
    {"SetAutoBreathEnable", (PyCFunction)PyLAppModel_SetAutoBreathEnable, METH_VARARGS, ""},
    {"SetAutoBlinkEnable", (PyCFunction)PyLAppModel_SetAutoBlinkEnable, METH_VARARGS, ""},
    {"GetParameterCount", (PyCFunction)PyLAppModel_GetParameterCount, METH_VARARGS, ""},
    {"GetParameter", (PyCFunction)PyLAppModel_GetParameter, METH_VARARGS, ""},
    {NULL} // 方法列表结束的标志
};

// 定义LAppModel类的类型对象
static PyTypeObject PyLAppModelType = {
    PyVarObject_HEAD_INIT(NULL, 0) "live2d.LAppModel", /* tp_name */
    sizeof(PyLAppModelObject),                         /* tp_basicsize */
    0,                                                 /* tp_itemsize */
    (destructor)PyLAppModel_dealloc,                   /* tp_dealloc */
    0,                                                 /* tp_print */
    0,                                                 /* tp_getattr */
    0,                                                 /* tp_setattr */
    0,                                                 /* tp_reserved */
    0,                                                 /* tp_repr */
    0,                                                 /* tp_as_number */
    0,                                                 /* tp_as_sequence */
    0,                                                 /* tp_as_mapping */
    0,                                                 /* tp_hash  */
    0,                                                 /* tp_call */
    0,                                                 /* tp_str */
    0,                                                 /* tp_getattro */
    0,                                                 /* tp_setattro */
    0,                                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                                /* tp_flags */
    "LAppModel objects",                               /* tp_doc */
    0,                                                 /* tp_traverse */
    0,                                                 /* tp_clear */
    0,                                                 /* tp_richcompare */
    0,                                                 /* tp_weaklistoffset */
    0,                                                 /* tp_iter */
    0,                                                 /* tp_iternext */
    PyLAppModel_methods,                               /* tp_methods */
    0,                                                 /* tp_members */
    0,                                                 /* tp_getset */
    0,                                                 /* tp_base */
    0,                                                 /* tp_dict */
    0,                                                 /* tp_descr_get */
    0,                                                 /* tp_descr_set */
    0,                                                 /* tp_dictoffset */
    (initproc)PyLAppModel_init,                        /* tp_init */
    0,                                                 /* tp_alloc */
    PyType_GenericNew,                                 /* tp_new */
};

static PyObject *live2d_init()
{
    _cubismOption.LogFunction = LAppPal::PrintLn;
    _cubismOption.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;

    Csm::CubismFramework::StartUp(&_cubismAllocator, &_cubismOption);
    Csm::CubismFramework::Initialize();
    Py_RETURN_NONE;
}

static PyObject *live2d_dispose()
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    for (auto &pair : g_model)
    {
        delete pair.second;
        Info("[G] release: LAppModel(at=%p)", pair.second);
    }

    g_model.clear();

    Csm::CubismFramework::Dispose();
    PyGILState_Release(gstate);
    Py_RETURN_NONE;
}

static PyObject *live2d_glew_init()
{
    if (glewInit() != GLEW_OK)
    {
        Info("Can't initilize glew.");
    }
    Py_RETURN_NONE;
}

static PyObject *live2d_set_gl_properties()
{
    // テクスチャサンプリング設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // 透過設定
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Py_RETURN_NONE;
}

static PyObject *live2d_clear_buffer(PyObject *self, PyObject *args)
{
    // 默认为黑色
    float r = 0.0, g = 0.0, b = 0.0, a = 0.0;

    // 解析传入的参数，允许指定颜色
    if (!PyArg_ParseTuple(args, "|ffff", &r, &g, &b, &a))
    {
        return NULL;
    }

    // 设置清屏颜色
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0);

    Py_RETURN_NONE;
}

extern bool live2dLogEnable;

static PyObject *live2d_set_log_enable(PyObject *self, PyObject *args)
{
    bool enable;
    if (!PyArg_ParseTuple(args, "p", &enable))
    {
        PyErr_SetString(PyExc_TypeError, "invalid param");
        return NULL;
    }

    live2dLogEnable = enable;

    Py_RETURN_NONE;
}

static PyObject *live2d_log_enable(PyObject *self, PyObject *args)
{

    if (live2dLogEnable)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// 定义live2d模块的方法
static PyMethodDef live2d_methods[] = {
    {"init", (PyCFunction)live2d_init, METH_VARARGS, ""},
    {"dispose", (PyCFunction)live2d_dispose, METH_VARARGS, ""},
    {"glewInit", (PyCFunction)live2d_glew_init, METH_VARARGS, ""},
    {"setGLProperties", (PyCFunction)live2d_set_gl_properties, METH_VARARGS, ""},
    {"clearBuffer", (PyCFunction)live2d_clear_buffer, METH_VARARGS, ""},
    {"setLogEnable", (PyCFunction)live2d_set_log_enable, METH_VARARGS, ""},
    {"logEnable", (PyCFunction)live2d_log_enable, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL} // 哨兵
};

// 定义live2d模块
static struct PyModuleDef liv2d_module = {
    PyModuleDef_HEAD_INIT,
    "live2d",
    "Module that creates live2d objects",
    -1,
    live2d_methods};

// 模块初始化函数的实现
PyMODINIT_FUNC PyInit_live2d(void)
{
    PyObject *m;
    if (PyType_Ready(&PyLAppModelType) < 0)
        return NULL;

    m = PyModule_Create(&liv2d_module);
    if (m == NULL)
        return NULL;

    Py_INCREF(&PyLAppModelType);
    if (PyModule_AddObject(m, "LAppModel", (PyObject *)&PyLAppModelType) < 0)
    {
        Py_DECREF(&PyLAppModelType);
        Py_DECREF(m);
        return NULL;
    }

    module_live2d_v3_params = PyImport_ImportModule("live2d.v3.params");
    if (module_live2d_v3_params == NULL)
    {
        PyErr_Print();
        return NULL;
    }

    class_parameter = PyObject_GetAttrString(module_live2d_v3_params, "Parameter");
    if (class_parameter == NULL)
    {
        Py_DECREF(module_live2d_v3_params);
        PyErr_Print();
        return NULL;
    }

#ifdef WIN32
    // 强制utf-8
    SetConsoleOutputCP(65001);
#endif

    return m;
}
