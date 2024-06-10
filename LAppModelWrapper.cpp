#include <LAppModel.hpp>
#include <CubismFramework.hpp>
#include <LAppPal.hpp>
#include <LAppAllocator.hpp>
#include <Log.hpp>
#include <unordered_map>
#include <mutex>
#include <MatrixManager.hpp>
#include <Default.hpp>

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

static LAppAllocator _cubismAllocator;
static Csm::CubismFramework::Option _cubismOption;

struct PyLAppModelObject
{
    PyObject_HEAD LAppModel *model;
    MatrixManager matrixManager;
    time_t key;
};

static std::mutex mutex_g_model;
static std::unordered_map<size_t, LAppModel *> g_model;

// LAppModel()
static int PyLAppModel_init(PyLAppModelObject *self, PyObject *args, PyObject *kwds)
{

    self->model = new LAppModel();

    mutex_g_model.lock();
    self->key = (size_t)self->model;
    g_model[self->key] = self->model;
    mutex_g_model.unlock();

    self->matrixManager.Initialize();

    return 0;
}

static void PyLAppModel_dealloc(PyLAppModelObject *self)
{

    mutex_g_model.lock();
    if (g_model.find(self->key) != g_model.end())
    {
        g_model.erase(self->key);
        delete self->model;
    }
    mutex_g_model.unlock();

    Info("deallocate: PyLAppModelObject(at=%ld)", self);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

// LAppModel->LoadAssets
static PyObject *PyLAppModel_LoadAssets(PyLAppModelObject *self, PyObject *args)
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
static PyObject *PyLAppModel_Update(PyLAppModelObject *self, PyObject *args)
{
    LAppPal::UpdateTime();

    int winWidth, winHeight;

    if (!PyArg_ParseTuple(args, "ii", &winWidth, &winHeight))
    {
        return NULL;
    }

    self->model->Update();
    self->model->Draw(self->matrixManager.GetProjection(self->model, winWidth, winHeight));
    Py_RETURN_NONE;
}

typedef Live2D::Cubism::Framework::ACubismMotion::FinishedMotionCallback FinishedMotionCallback;
typedef Live2D::Cubism::Framework::ACubismMotion ACubismMotion;

// 全局回调函数，任何时刻只可能有一个 motion 在播放
// 而优先权高的 motion 会顶替当前 motion，当前 motion 作废
static PyObject *g_start_callback;
static PyObject *g_finish_callback;

static void default_start_call_back(const char *group, int no)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    PyObject *result = PyObject_CallFunction(g_start_callback, "si", group, no);
    if (result != NULL)
        Py_DECREF(result);

    Py_DECREF(g_start_callback);
    g_start_callback = nullptr;
    PyGILState_Release(gstate);
};

static void default_finish_call_back(ACubismMotion *self)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    PyObject *result = PyObject_CallFunction(g_finish_callback, NULL);

    if (result != NULL)
        Py_DECREF(result);

    Py_DECREF(g_finish_callback);
    g_finish_callback = nullptr;
    PyGILState_Release(gstate);
};

static PyObject *PyLAppModel_StartMotion(PyLAppModelObject *self, PyObject *args, PyObject *kwargs)
{
    const char *group;
    int no, priority;
    PyObject *onStartHandler = nullptr;
    PyObject *onFinishHandler = nullptr;
    LAppModel::OnStartMotionHandler s_call = nullptr;
    FinishedMotionCallback f_call = nullptr;

    char *kwlist[] = {(char*)"group", (char*)"no", (char*)"priority", (char*)"onStartMotionHandler", (char*)"onFinishMotionHandler", NULL};
    if (!(PyArg_ParseTupleAndKeywords(args, kwargs, "sii|OO", kwlist, &group, &no, &priority, &onStartHandler, &onFinishHandler)))
    {
        return NULL;
    }

    if (onStartHandler != nullptr)
    {
        if (!PyCallable_Check(onStartHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 4 must be callable.");
            return NULL;
        }
        g_start_callback = onStartHandler;
        Py_INCREF(g_start_callback);
        s_call = default_start_call_back;
    }

    if (onFinishHandler != nullptr)
    {
        if (!PyCallable_Check(onFinishHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 5 must be callable.");
            return NULL;
        }
        g_finish_callback = onFinishHandler;
        Py_INCREF(g_finish_callback);
        f_call = default_finish_call_back;
    }

    Csm::CubismMotionQueueEntryHandle handle = self->model->StartMotion(group, no, priority, s_call, f_call);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_StartRandomMotion(PyLAppModelObject *self, PyObject *args, PyObject *kwargs)
{
    const char *group;
    int priority;

    PyObject *onStartHandler = nullptr;
    PyObject *onFinishHandler = nullptr;
    LAppModel::OnStartMotionHandler s_call = nullptr;
    FinishedMotionCallback f_call = nullptr;

    char *kwlist[] = {(char*)"group", (char*)"priority", (char*)"onStartMotionHandler", (char*)"onFinishMotionHandler", NULL};
    if (!(PyArg_ParseTupleAndKeywords(args, kwargs, "si|OO", kwlist, &group, &priority, &onStartHandler, &onFinishHandler)))
    {
        return NULL;
    }

    if (onStartHandler != nullptr)
    {
        if (!PyCallable_Check(onStartHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 3 must be callable.");
            return NULL;
        }
        g_start_callback = onStartHandler;
        Py_XINCREF(g_start_callback);
        s_call = default_start_call_back;
    }

    if (onFinishHandler != nullptr)
    {
        if (!PyCallable_Check(onFinishHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 4 must be callable.");
            return NULL;
        }
        g_finish_callback = onFinishHandler;
        Py_XINCREF(g_finish_callback);
        f_call = default_finish_call_back;
    }

    self->model->StartRandomMotion(group, priority, s_call, f_call);

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
    int mx, my;
    PyObject *onStartMotionHandler = nullptr;
    PyObject *onFinishMotionHandler = nullptr;

    PyObject *onStartHandler = nullptr;
    PyObject *onFinishHandler = nullptr;
    LAppModel::OnStartMotionHandler s_call = nullptr;
    FinishedMotionCallback f_call = nullptr;

    char *kwlist[] = {(char*)"mx", (char*)"my", (char*)"onStartMotionHandler", (char*)"onFinishMotionHandler", NULL};
    if (!(PyArg_ParseTupleAndKeywords(args, kwargs, "ii|OO", kwlist, &mx, &my, &onStartHandler, &onFinishHandler)))
    {
        return NULL;
    }

    if (onStartHandler != nullptr)
    {
        if (!PyCallable_Check(onStartHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 3 must be callable.");
            return NULL;
        }
        g_start_callback = onStartHandler;
        Py_XINCREF(g_start_callback);
        s_call = default_start_call_back;
    }

    if (onFinishHandler != nullptr)
    {
        if (!PyCallable_Check(onFinishHandler))
        {
            PyErr_SetString(PyExc_TypeError, "Argument 4 must be callable.");
            return NULL;
        }
        g_finish_callback = onFinishHandler;
        Py_XINCREF(g_finish_callback);
        f_call = default_finish_call_back;
    }

    float xf = (float)mx;
    float yf = (float)my;
    self->matrixManager.ScreenToScene(&xf, &yf);

    csmString hitArea = self->model->HitTest(xf, yf);
    if (strlen(hitArea.GetRawString()) != 0)
    {
        Info("hit area: [%s]", hitArea.GetRawString());
        if (strcmp(hitArea.GetRawString(), HIT_AREA_HEAD) == 0)
            self->model->SetRandomExpression();
        self->model->StartRandomMotion(hitArea.GetRawString(), MOTION_PRIORITY_FORCE, s_call, f_call);
    }

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_Drag(PyLAppModelObject *self, PyObject *args)
{
    int mx, my;
    if (!(PyArg_ParseTuple(args, "ii", &mx, &my)))
    {
        return NULL;
    }

    float xf = (float)mx;
    float yf = (float)my;
    self->matrixManager.ScreenToScene(&xf, &yf);

    self->model->SetDragging(xf, yf);

    Py_RETURN_NONE;
}

static PyObject *PyLAppModel_SetLipSyncN(PyLAppModelObject *self, PyObject *args)
{
    float n;

    if (PyArg_ParseTuple(args, "f", &n) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Missing param n (float)");
        return NULL;
    }

    self->model->SetLipSyncN(n);

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

// 包装模块方法的方法列表
static PyMethodDef PyLAppModel_methods[] = {
    {"LoadAssets", (PyCFunction)PyLAppModel_LoadAssets, METH_VARARGS, "Load model assets."},
    {"Resize", (PyCFunction)PyLAppModel_Resize, METH_VARARGS, "Update matrix."},
    {"Update", (PyCFunction)PyLAppModel_Update, METH_VARARGS, "Update model buffer."},
    {"StartMotion", (PyCFunction)PyLAppModel_StartMotion, METH_VARARGS | METH_KEYWORDS, "Start motion by its groupname and idx."},
    {"StartRandomMotion", (PyCFunction)PyLAppModel_StartRandomMotion, METH_VARARGS | METH_KEYWORDS, "Start random motion."},
    {"SetExpression", (PyCFunction)PyLAppModel_SetExpression, METH_VARARGS, "Set expression by name."},
    {"SetRandomExpression", (PyCFunction)PyLAppModel_SetRandomExpression, METH_VARARGS, "Set random expression."},
    {"HitTest", (PyCFunction)PyLAppModel_HitTest, METH_VARARGS, "Get the name of the area being hit."},
    {"HasMocConsistencyFromFile", (PyCFunction)PyLAppModel_HasMocConsistencyFromFile, METH_VARARGS, "Start random motion."},
    {"Touch", (PyCFunction)PyLAppModel_Touch, METH_VARARGS | METH_KEYWORDS, "Click at (x, y)."},
    {"Drag", (PyCFunction)PyLAppModel_Drag, METH_VARARGS, "Drag to (x, y)."},
    {"SetLipSyncN", (PyCFunction)PyLAppModel_SetLipSyncN, METH_VARARGS, "Set magnitude for lip sync."},
    {"IsMotionFinished", (PyCFunction)PyLAppModel_IsMotionFinished, METH_VARARGS, "Test if current motion is finished."},
    {"SetOffset", (PyCFunction)PyLAppModel_SetOffset, METH_VARARGS, "Set offset of the drawing center."},
    {"SetScale", (PyCFunction)PyLAppModel_SetScale, METH_VARARGS, "Set model scale."},
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
#ifdef LOG_MODE_RELEASE
    _cubismOption.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Off;
#else
    _cubismOption.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;
#endif
    Csm::CubismFramework::StartUp(&_cubismAllocator, &_cubismOption);
    Csm::CubismFramework::Initialize();
    Py_RETURN_NONE;
}

static PyObject *live2d_release_cubism()
{

    mutex_g_model.lock();

    for (auto &pair : g_model)
    {

        Info("release: LAppModel(at=%ld)", pair.second);

        delete pair.second;

        break;
    }

    g_model.clear();

    mutex_g_model.unlock();

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
