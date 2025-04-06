#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <CubismFramework.hpp>
#include <LAppAllocator.hpp>
#include <LAppPal.hpp>
#include <LAppModel.hpp>
#include <Windows.h>

#include <fine-grained/Model.hpp>

Model *model;

int main()
{
    SetConsoleOutputCP(65001);

    glfwInit();
    GLFWwindow *window = glfwCreateWindow(640, 480, "GLFW test", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    LAppAllocator allocator;
    Csm::CubismFramework::Option option;

    option.LogFunction = LAppPal::PrintLn;
    option.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;

    Csm::CubismFramework::StartUp(&allocator, &option);
    Csm::CubismFramework::Initialize();

    model = new Model();
    model->LoadModelJson("../../Resources/v3/小九/小九皮套（紫）/小九.model3.json");
    model->Resize(640, 480);

    model->LoadExtraMotion("extra", 0, "../../Resources/v3/public_motions/drag_down.motion3.json");

    glfwMakeContextCurrent(window);
    gladLoadGL();

    model->CreateRenderer();

    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double x, double y)
                             { model->Drag(float(x), float(y)); });

    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
                       {
                            // d, e, r, f
                            if (key == GLFW_KEY_D && action == GLFW_PRESS)
                            {
                                model->SetDefaultExpression("expression6");
                            }
                            else if (key == GLFW_KEY_E && action == GLFW_PRESS)
                            {
                                model->SetRandomExpression();
                            }
                            else if (key == GLFW_KEY_R && action == GLFW_PRESS)
                            {
                                model->ResetExpression();
                            }
                            else if (key == GLFW_KEY_F && action == GLFW_PRESS)
                            {
                                model->SetFadeOutExpression("expression5", 5000);
                            }

                        });

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        double ct = glfwGetTime();
        float deltaSeconds = float(ct - lastTime);
        lastTime = ct;

        // ---update---
        if (model->IsMotionFinished())
        {
            model->StartRandomMotion(nullptr, 3, nullptr, [](ACubismMotion *motion){
                std::cout << "[INFO]  start motion: [" << motion->group << "_" << motion->no << "]" << std::endl;
            });
        }

        model->LoadParameters();
        bool motionUpdated = model->UpdateMotion(deltaSeconds);
        model->SaveParameters();
        model->UpdateDrag(deltaSeconds);
        model->UpdateBreath(deltaSeconds);
        if (motionUpdated)
        {
            model->UpdateBlink(deltaSeconds);
        }
        model->UpdateExpression(deltaSeconds);
        model->UpdatePhysics(deltaSeconds);
        model->UpdatePose(deltaSeconds);
        // ---update---

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