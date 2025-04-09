#include <QApplication>

#include "MainWindow.hpp"

#include <LAppAllocator.hpp>
#include <LAppPal.hpp>
#include <CubismFramework.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char *argv[])
{

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    LAppAllocator allocator;
    Csm::CubismFramework::Option option;

    option.LogFunction = LAppPal::PrintLn;
    option.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;

    Csm::CubismFramework::StartUp(&allocator, &option);
    Csm::CubismFramework::Initialize();

    QApplication app(argc, argv);

    QObject::connect(&app, &QApplication::aboutToQuit, []()
                     { Csm::CubismFramework::Dispose(); });

    MainWindow w;
    w.show();
    return app.exec();
}
