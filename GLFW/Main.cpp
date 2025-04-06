#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <CubismFramework.hpp>
#include <LAppAllocator.hpp>
#include <LAppPal.hpp>
#include <LAppModel.hpp>
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(65001);

    glfwInit();
    GLFWwindow* window = glfwCreateWindow(640, 480, "GLFW test", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();

    LAppAllocator allocator;
    Csm::CubismFramework::Option option;

    option.LogFunction = LAppPal::PrintLn;
    option.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;

    Csm::CubismFramework::StartUp(&allocator, &option);
    Csm::CubismFramework::Initialize();

    LAppModel* model = new LAppModel();
    model->LoadModelJson("../../Resources/v3/小九/小九皮套（紫）/小九.model3.json");
    model->Resize(640, 480);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        model->Update();
        model->Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Csm::CubismFramework::Dispose();
    delete model;

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}