cmake_minimum_required(VERSION 3.21)

# execute_process(COMMAND vcpkg x-update-baseline)

include(cmake/warnings.cmake)
include(cmake/static_analysis.cmake)

project(gvox-cli VERSION 0.1.0)
add_executable(${PROJECT_NAME} "src/main.cpp")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
set_project_warnings(${PROJECT_NAME})

find_package(gvox CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    gvox::gvox
)
target_compile_definitions(${PROJECT_NAME} PRIVATE GVOX_CLI_VERSION_STRING="${PROJECT_VERSION}")

include(cmake/packaging.cmake)
