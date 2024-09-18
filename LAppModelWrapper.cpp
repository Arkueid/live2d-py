#include <gl/glew.h>
#include "LAppModel.h"
#include "Log.hpp"
#include <unordered_map>
#include <mutex>
#include "Default.hpp"
#include <Live2DFramework.h>
#include "PlatformManager.h"
#include "MatrixManager.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <Live2DModelOpenGL.h>

#ifdef WIN32
#include <Windows.h>
#endif

struct PyLAppModelObject
{
    PyObject_HEAD LAppModel *model;
    MatrixManager matrixManager;
    size_t key;
};

// static std::mutex mutex_g_model;
static std::unordered_map<size_t, LAppModel *> g_model;

// LAppModel()
static int PyLAppModel_init(PyLAppModelObject *self, PyObject *args, PyObject *kwds)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    self->model = new LAppModel();

    // mutex_g_model.lock();
    self->key = (size_t)self->model;
    g_model[self->key] = self->model;
    // mutex_g_model.unlock();

    self->matrixManager.init();

    Info("[M] allocate: LAppModel(at=%p)", self->model);
    PyGILState_Release(gstate);
    return 0;
}

static void PyLAppModel_dealloc(PyLAppModelObject *self)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    // mutex_g_model.lock();
    if (g_model.find(self->key) != g_model.end())
    {
        g_model.erase(self->key);
        delete self->model;
        Info("[M] deallocate: LAppModel(at=%p)", self->model);
    }
    // mutex_g_model.unlock();
    PyGILState_Release(gstate);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

// LAppModel->LoadAssets
static PyObject *PyLAppModel_LoadModelJson(PyLAppModelObject *self, PyObject *args)
{
    const char *fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName))
    {
        return NULL;
    }

    self->model->load(fileName);

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

    self->matrixManager.setDeviceSize(ww, wh);
    self->model->resize(ww, wh);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetAutoEyeblinkEnable(PyLAppModelObject *self, PyObject *args)
{
    bool enable;
    if (!PyArg_ParseTuple(args, "p", &enable))
    {
        PyErr_SetString(PyExc_TypeError, "invalid param.");
        return NULL;
    }
    self->model->setEyeBlinkEnable(enable);
    Py_RETURN_NONE;
}

// LAppModel->Update
static PyObject *PyLAppModel_Update(PyLAppModelObject *self, PyObject *args)
{
    self->model->update();
    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_Draw(PyLAppModelObject *self, PyObject *args)
{
    self->model->draw();
    Py_RETURN_NONE;
}

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
        Py_INCREF(onFinishHandler);
    }

    auto s_call = [=](const char *group, int no)
    {
        if (isStartNull)
            return;
        PyObject *result = PyObject_CallFunction(onStartHandler, "si", group, no);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onStartHandler);
    };

    auto f_call = [=]()
    {
        if (isFinishNull)
            return;
        PyObject *result = PyObject_CallFunction(onFinishHandler, NULL);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onFinishHandler);
    };

    self->model->startMotion(group, no, priority, s_call, f_call);

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
        Py_INCREF(onFinishHandler);
    }

    auto s_call = [=](const char *group, int no)
    {
        if (isStartNull)
            return;
        PyObject *result = PyObject_CallFunction(onStartHandler, "si", group, no);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onStartHandler);
    };

    auto f_call = [=]()
    {
        if (isFinishNull)
            return;
        PyObject *result = PyObject_CallFunction(onFinishHandler, NULL);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onFinishHandler);
    };

    self->model->startRandomMotion(group, priority, s_call, f_call);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetExpression(PyLAppModelObject *self, PyObject *args)
{
    const char *expressionID;

    if (!(PyArg_ParseTuple(args, "s", &expressionID)))
    {
        return NULL;
    }

    self->model->setExpression(expressionID);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetRandomExpression(PyLAppModelObject *self, PyObject *args)
{
    self->model->setRandomExpression();
    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_HitTest(PyLAppModelObject *self, PyObject *args)
{
    float x, y;
    if (!(PyArg_ParseTuple(args, "ff", &x, &y)))
    {
        return NULL;
    }

    std::string area = self->model->hitTest(x, y);

    return Py_BuildValue("s", area.c_str());
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
        Py_INCREF(onFinishHandler);
    }

    auto s_call = [=](const char *group, int no)
    {
        if (isStartNull)
            return;
        PyObject *result = PyObject_CallFunction(onStartHandler, "si", group, no);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onStartHandler);
    };

    auto f_call = [=]()
    {
        if (isFinishNull)
            return;
        PyObject *result = PyObject_CallFunction(onFinishHandler, NULL);
        if (result != NULL)
            Py_XDECREF(result);
        Py_XDECREF(onFinishHandler);
    };

    mx = self->matrixManager.transformDeviceToViewX(mx);
    my = self->matrixManager.transformDeviceToViewY(my);

    std::string hitArea = self->model->hitTest(mx, my);
    if (!hitArea.empty())
    {
        Info("hit area: [%s]", hitArea.c_str());
        if (strcmp(hitArea.c_str(), HIT_AREA_HEAD) == 0)
            self->model->setRandomExpression();
        self->model->startRandomMotion(hitArea.c_str(), MOTION_PRIORITY_FORCE, s_call, f_call);
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

    mx = self->matrixManager.transformDeviceToViewX(mx);
    my = self->matrixManager.transformDeviceToViewY(my);

    self->model->setDrag(mx, my);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_IsMotionFinished(PyLAppModelObject *self, PyObject *args)
{

    if (self->model->isMotionFinished())
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
        Py_RETURN_NONE;
    }

    self->model->setCenter(dx, dy);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetScale(PyLAppModelObject *self, PyObject *args)
{
    float scale;

    if (PyArg_ParseTuple(args, "f", &scale) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Missing param 'float scale'");
        Py_RETURN_NONE;
    }

    self->model->setScale(scale);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetParameterValue(PyLAppModelObject *self, PyObject *args)
{
    const char *paramId;
    float value, weight;

    if (PyArg_ParseTuple(args, "sff", &paramId, &value, &weight) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Invalid parmas");
        Py_RETURN_NONE;
    }

    self->model->setParameterValue(paramId, value, weight);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_AddParameterValue(PyLAppModelObject *self, PyObject *args)
{
    const char *paramId;
    float value;

    if (PyArg_ParseTuple(args, "sf", &paramId, &value) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Invalid params");
        Py_RETURN_NONE;
    }

    self->model->addParameterValue(paramId, value);

    Py_RETURN_NONE;
}

// 包装模块方法的方法列表
static PyMethodDef PyLAppModel_methods[] = {
    {"LoadModelJson", (PyCFunction)PyLAppModel_LoadModelJson, METH_VARARGS, ""},
    {"Resize", (PyCFunction)PyLAppModel_Resize, METH_VARARGS, ""},
    {"Update", (PyCFunction)PyLAppModel_Update, METH_VARARGS, ""},
    {"StartMotion", (PyCFunction)PyLAppModel_StartMotion, METH_VARARGS | METH_KEYWORDS, ""},
    {"StartRandomMotion", (PyCFunction)PyLAppModel_StartRandomMotion, METH_VARARGS | METH_KEYWORDS, ""},
    {"SetExpression", (PyCFunction)PyLAppModel_SetExpression, METH_VARARGS, ""},
    {"SetRandomExpression", (PyCFunction)PyLAppModel_SetRandomExpression, METH_VARARGS, ""},
    {"HitTest", (PyCFunction)PyLAppModel_HitTest, METH_VARARGS, ""},
    {"Touch", (PyCFunction)PyLAppModel_Touch, METH_VARARGS | METH_KEYWORDS, ""},
    {"Drag", (PyCFunction)PyLAppModel_Drag, METH_VARARGS, ""},
    {"IsMotionFinished", (PyCFunction)PyLAppModel_IsMotionFinished, METH_VARARGS, ""},
    {"SetOffset", (PyCFunction)PyLAppModel_SetOffset, METH_VARARGS, ""},
    {"SetScale", (PyCFunction)PyLAppModel_SetScale, METH_VARARGS, ""},
    {"SetAutoEyeBlinkEnable", (PyCFunction)PyLAppModel_SetAutoEyeblinkEnable, METH_VARARGS, ""},
    {"Draw", (PyCFunction)PyLAppModel_Draw, METH_VARARGS, ""},
    {"SetParameterValue", (PyCFunction)PyLAppModel_SetParameterValue, METH_VARARGS, ""},
    {"AddParameterValue", (PyCFunction)PyLAppModel_AddParameterValue, METH_VARARGS, ""},
    {NULL} // 方法列表结束的标志
};

// 定义Rectangle类的类型对象
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
    live2d::Live2D::init();

    Info("live2d version: %s", live2d::Live2D::getVersionStr());

    live2d::framework::Live2DFramework::setPlatformManager(new PlatformManager());
    Py_RETURN_NONE;
}

static PyObject *live2d_dispose()
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    // mutex_g_model.lock();

    for (auto &pair : g_model)
    {
        delete pair.second;
        Info("[G] release: LAppModel(at=%p)", pair.second);
    }

    g_model.clear();

    // mutex_g_model.unlock();
    live2d::Live2D::dispose();
    Info("[G] live2d::dispose success");
    PyGILState_Release(gstate);
    Py_RETURN_NONE;
}

static PyObject *live2d_clear_buffer()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0);

    Py_RETURN_NONE;
}

extern bool live2dLogEnable;

static PyObject *live2d_set_log_enbale(PyObject *self, PyObject *args)
{
    bool enable;
    if (!PyArg_ParseTuple(args, "p", &enable))
    {
        PyErr_SetString(PyExc_TypeError, "invalid param");
    }

    live2dLogEnable = enable;

    Py_RETURN_NONE;
}

static PyObject *live2d_log_enbale(PyObject *self, PyObject *args)
{
    if (live2dLogEnable)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// 定义模块的方法
static PyMethodDef module_methods[] = {
    {"init", (PyCFunction)live2d_init, METH_VARARGS, "initialize sdk"},
    {"dispose", (PyCFunction)live2d_dispose, METH_VARARGS, "release sdk"},
    {"clearBuffer", (PyCFunction)live2d_clear_buffer, METH_VARARGS, "clear buffer"},
    {"setLogEnable", (PyCFunction)live2d_set_log_enbale, METH_VARARGS, "set log mode"},
    {"logEnable", (PyCFunction)live2d_log_enbale, METH_VARARGS, "get log enable"},
    {NULL, NULL, 0, NULL} // 哨兵
};

// 定义模块
static struct PyModuleDef liv2d_module = {
    PyModuleDef_HEAD_INIT,
    "live2d",
    "Module that creates live2d objects",
    -1,
    module_methods};

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

#ifdef WIN32
    SetConsoleOutputCP(65001);
#endif
    return m;
}
