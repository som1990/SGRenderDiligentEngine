
cmake_minimum_required(VERSION 3.15)
project(OmegaSGDiligent)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(NOT SGRENDER_DIR)
    set(SGRENDER_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "Location of SGRenderDiligentEngine")
elseif(NOT IS_ABSOLUTE "${SGRENDER_DIR}")
    get_filename_component(SGRENDER_DIR "${SGRENDER_DIR}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

set(CMAKE_INSTALL_PREFIX "./install")

# Adding folder Structure

add_subdirectory(DiligentEngine)
add_subdirectory(Prototypes)




