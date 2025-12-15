cmake_minimum_required(VERSION 4.2.0)
project(canvas 
    VERSION 0.1.0
    DESCRIPTION "Canvas-0.1.0"
    LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME}
    src/Main.cpp
    src/App.cpp
    src/Screen.cpp
    src/Shape.cpp
    src/Slider.cpp
    src/Line.cpp
    src/Shader.cpp
    
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
include_directories(${GLFW_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE include)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    OpenGL::GL 
    glfw 
    glad::glad
    glm::glm
)