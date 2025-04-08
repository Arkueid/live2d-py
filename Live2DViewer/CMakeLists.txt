set(CMAKE_PREFIX_PATH "D:/Qt/6.8.2/msvc2022_64/")
set(CMAKE_AUTOMOC ON)   # 自动处理 Q_OBJECT 宏的元对象编译（moc）
set(CMAKE_AUTOUIC ON)   # 自动编译 UI 文件（.ui → .h）
set(CMAKE_AUTORCC ON)   # 自动编译资源文件（.qrc → .cpp）

find_package(Qt6 COMPONENTS Core Widgets Gui OpenGLWidgets LinguistTools REQUIRED)

add_executable(Live2DViewer
    Main.cpp

    scene/Live2DScene.hpp
    scene/Live2DScene.cpp

    MainWindow.ui
    MainWindow.hpp
    MainWindow.cpp

    Live2DView.ui
    Live2DView.hpp
    Live2DView.cpp

    logo.rc
)

target_compile_options(Live2DViewer PRIVATE "/Zc:__cplusplus" "/permissive-" "/utf-8")

target_compile_definitions(Live2DViewer
    PRIVATE CSM_TARGET_WIN_GL UNICODE _UNICODE
)

set_target_properties(Live2DViewer PROPERTIES
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
)

target_include_directories(Live2DViewer
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QTINC}
    ${PROJECT_ROOT}/Framework/src
    ${PROJECT_ROOT}/Core/include
    ${PROJECT_ROOT}/Glad/include
    ${PROJECT_ROOT}/Main/src
)

target_link_libraries(Live2DViewer
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::OpenGLWidgets 
    Main
)

set_target_properties(Live2DViewer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_ROOT}/x64/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_ROOT}/x64/Release
)

file(COPY moeroid.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(MARKER $<IF:$<CONFIG:DEBUG>,${PROJECT_ROOT}/x64/Debug/.deployed,${PROJECT_ROOT}/x64/Release/.deployed>)

# 获取 windeployqt6 工具的路径
get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt6 HINTS "${_qt_bin_dir}")

add_custom_command(
    TARGET Live2DViewer
    POST_BUILD
    COMMAND
        if not exist ${MARKER} (${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:Live2DViewer> && echo done. > ${MARKER}) else (echo deployed)
    COMMAND
    ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/moeroid.ico $<TARGET_FILE_DIR:Live2DViewer>/moeroid.ico
)

set(ts_files
    Translations/moe_zh_CN.ts
    Translations/moe_en.ts
    Translations/moe_ja_JP.ts
)

qt6_add_translations(${PROJECT_NAME} TS_FILES ${ts_files})
