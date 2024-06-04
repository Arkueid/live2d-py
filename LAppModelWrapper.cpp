#include <Python.h>
#include "LAppModel.hpp"
#include <CubismFramework.hpp>
#include <LAppPal.hpp>
#include <LAppAllocator.hpp>
#include <Log.hpp>

static LAppAllocator _cubismAllocator;
static Csm::CubismFramework::Option _cubismOption;

struct PyLAppModelObject
{
    PyObject_HEAD LAppModel *model;
};

static void PyLAppModel_dealloc(PyLAppModelObject *self)
{
    delete self->model;
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int PyLAppModel_init(PyLAppModelObject *self)
{
    self->model = new LAppModel();
    return 0;
}

// 包装LAppModel类的函数
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

// Update
static PyObject *PyLAppModel_update(PyLAppModelObject *self, PyObject *args)
{
    LAppPal::UpdateTime();

    int winWidth, winHeight;

    if (!PyArg_ParseTuple(args, "ii", &winWidth, &winHeight))
    {
        return NULL;
    }

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

// 包装模块方法的方法列表
static PyMethodDef PyLAppModel_methods[] = {
    {"LoadAssets", (PyCFunction)PyLAppModel_load_assets, METH_VARARGS, "Load model assets."},
    {"Update", (PyCFunction)PyLAppModel_update, METH_VARARGS, "update model buffer."},
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

// 定义模块的方法
static PyMethodDef module_methods[] = {
    {"InitializeCubism", (PyCFunction)live2d_initialize_cubism, METH_VARARGS, "initialize sdk"},
    {"ReleaseCubism", (PyCFunction)live2d_release_cubism, METH_VARARGS, "release sdk"},
    {"InitializeGlew", (PyCFunction)live2d_initialzie_glew, METH_VARARGS, "release sdk"},
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
