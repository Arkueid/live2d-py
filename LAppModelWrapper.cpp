#include <Python.h>
#include "LAppModel.hpp"
#include <CubismFramework.hpp>
#include <LAppPal.hpp>
#include <LAppAllocator.hpp>
#include <Log.hpp>
#include <unordered_map>
#include <vector>
#include <mutex>

static LAppAllocator _cubismAllocator;
static Csm::CubismFramework::Option _cubismOption;

struct PyLAppModelObject
{
    PyObject_HEAD LAppModel *model;
    std::vector<PyLAppModelObject *>::iterator _vid;
};

static std::vector<PyLAppModelObject *> models;
static std::mutex mutex_models;

static void PyLAppModel_dealloc(PyLAppModelObject *self)
{
    mutex_models.lock();
    models.erase(self->_vid);
    mutex_models.unlock();

    delete self->model;
    self->model = nullptr;

    Py_TYPE(self)->tp_free((PyObject *)self);
}

// LAppModel()
static int PyLAppModel_init(PyLAppModelObject *self)
{
    self->model = new LAppModel();

    mutex_models.lock();
    models.push_back(self);
    self->_vid = models.end() - 1;
    mutex_models.unlock();

    return 0;
}

// LAppModel->LoadAssets
static PyObject *PyLAppModel_load_assets(PyLAppModelObject *self, PyObject *args)
{
    const char *dir;
    const char *fileName;
    if (!PyArg_ParseTuple(args, "ss", &dir, &fileName))
    {
        return NULL;
    }

    self->model->LoadAssets(dir, fileName);

    Py_RETURN_NONE;
}

// LAppModel->Update
static PyObject *PyLAppModel_update(PyLAppModelObject *self, PyObject *args)
{
    LAppPal::UpdateTime();

    int winWidth, winHeight;

    if (!PyArg_ParseTuple(args, "ii", &winWidth, &winHeight))
    {
        return NULL;
    }

    // 自适应窗口/画布大小
    Csm::CubismMatrix44 projection;
    // 念のため単位行列に初期化
    projection.LoadIdentity();

    if (self->model->GetModel()->GetCanvasWidth() > 1.0f && winWidth < winHeight)
    {
        // 横に長いモデルを縦長ウィンドウに表示する際モデルの横サイズでscaleを算出する
        self->model->GetModelMatrix()->SetWidth(2.0f);
        projection.Scale(1.0f, static_cast<float>(winWidth) / static_cast<float>(winHeight));
    }
    else
    {
        projection.Scale(static_cast<float>(winHeight) / static_cast<float>(winWidth), 1.0f);
    }

    self->model->Update();
    self->model->Draw(projection);
    Py_RETURN_NONE;
}

typedef Live2D::Cubism::Framework::ACubismMotion::FinishedMotionCallback FinishedMotionCallback;
typedef Live2D::Cubism::Framework::ACubismMotion ACubismMotion;

// 全局回调函数，任何时刻只可能有一个 motion 在播放
// 而优先权高的 motion 会顶替当前 motion，当前 motion 作废
static PyObject *g_py_callback;

static void default_call_back(ACubismMotion *self)
{
    // Call the Python callback function.
    PyGILState_STATE gstate = PyGILState_Ensure(); // Ensure GIL
    PyObject *result = PyObject_CallFunction(g_py_callback, NULL);

    Py_DECREF(result);          // Decrease reference count of the result
    PyGILState_Release(gstate); // Release GIL
    g_py_callback = nullptr;
};

static PyObject *PyLAppModel_StartMotion(PyLAppModelObject *self, PyObject *args)
{
    const char *group;
    int no, priority;
    PyObject *py_callback;
    FinishedMotionCallback callback;

    if (!(PyArg_ParseTuple(args, "sii|O", &group, &no, &priority, &py_callback)))
    {
        return NULL;
    }

    if (py_callback != nullptr)
    {
        if (!PyCallable_Check(py_callback))
        {
            PyErr_SetString(PyExc_TypeError, "Argument must be callable.");
            return NULL;
        }

        g_py_callback = py_callback;
        callback = default_call_back;
    }
    else
    {
        callback = nullptr;
    }

    Csm::CubismMotionQueueEntryHandle handle = self->model->StartMotion(group, no, priority, callback);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_StartRandomMotion(PyLAppModelObject *self, PyObject *args)
{
    const char *group;
    int priority;
    PyObject *py_callback;
    FinishedMotionCallback callback;

    if (!(PyArg_ParseTuple(args, "si|O", &group, &priority, &py_callback)))
    {
        return NULL;
    }

    if (py_callback != nullptr)
    {
        if (!PyCallable_Check(py_callback))
        {
            PyErr_SetString(PyExc_TypeError, "Argument must be callable.");
            return NULL;
        }
        g_py_callback = py_callback;
        callback = default_call_back;
    }
    else
    {
        callback = nullptr;
    }

    self->model->StartRandomMotion(group, priority, callback);

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

// 包装模块方法的方法列表
static PyMethodDef PyLAppModel_methods[] = {
    {"LoadAssets", (PyCFunction)PyLAppModel_load_assets, METH_VARARGS, "Load model assets."},
    {"Update", (PyCFunction)PyLAppModel_update, METH_VARARGS, "update model buffer."},
    {"StartMotion", (PyCFunction)PyLAppModel_StartMotion, METH_VARARGS, "start motion by its groupname and idx."},
    {"StartRandomMotion", (PyCFunction)PyLAppModel_StartRandomMotion, METH_VARARGS, "start random motion."},
    {"SetExpression", (PyCFunction)PyLAppModel_SetExpression, METH_VARARGS, "set expression by name."},
    {"SetRandomExpression", (PyCFunction)PyLAppModel_SetRandomExpression, METH_VARARGS, "set random expression."},
    {"HitTest", (PyCFunction)PyLAppModel_HitTest, METH_VARARGS, "Get the name of the area being hit."},
    {"HasMocConsistencyFromFile", (PyCFunction)PyLAppModel_HasMocConsistencyFromFile, METH_VARARGS, "start random motion."},
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

static PyObject *live2d_initialize_cubism()
{
    _cubismOption.LogFunction = LAppPal::PrintLn;
    _cubismOption.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;
    Csm::CubismFramework::StartUp(&_cubismAllocator, &_cubismOption);
    Csm::CubismFramework::Initialize();
    Py_RETURN_NONE;
}

static PyObject *live2d_release_cubism()
{
    mutex_models.lock();
    int last = models.size() - 1;
    for (; last >= 0; last--)
    {
        PyLAppModelObject *model = models.back();
        models.pop_back();
        Py_TYPE(model)->tp_free((PyObject *)model);
        model = nullptr;
    }
    mutex_models.unlock();
    Csm::CubismFramework::Dispose();

    Py_RETURN_NONE;
}

static PyObject *live2d_initialzie_glew()
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

static PyObject *live2d_clear_buffer()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0);

    Py_RETURN_NONE;
}

// 定义模块的方法
static PyMethodDef module_methods[] = {
    {"InitializeCubism", (PyCFunction)live2d_initialize_cubism, METH_VARARGS, "initialize sdk"},
    {"ReleaseCubism", (PyCFunction)live2d_release_cubism, METH_VARARGS, "release sdk"},
    {"InitializeGlew", (PyCFunction)live2d_initialzie_glew, METH_VARARGS, "release sdk"},
    {"SetGLProperties", (PyCFunction)live2d_set_gl_properties, METH_VARARGS, "configure gl"},
    {"ClearBuffer", (PyCFunction)live2d_clear_buffer, METH_VARARGS, "clear buffer"},
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

    return m;
}
