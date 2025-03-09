
add_subdirectory(Main)

set_property(TARGET Main PROPERTY CXX_STANDARD 17)
set_property(TARGET Main PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(Main PUBLIC src)

find_package(OpenGL REQUIRED)

if(APPLE)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  
  set(CMAKE_OSX_ARCHITECTURES "arm64")
  
  find_package(OpenGL REQUIRED)
  find_library(COCOA_LIBRARY Cocoa REQUIRED)
  find_library(IOKIT_LIBRARY IOKit REQUIRED)
  find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
endif()

target_link_libraries(Main
  Framework
  ${OPENGL_LIBRARIES}
)

if(APPLE)
  target_link_libraries(Main
    ${COCOA_LIBRARY}
    ${IOKIT_LIBRARY} 
    ${COREVIDEO_LIBRARY}
  )
endif()