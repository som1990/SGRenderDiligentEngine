cmake_minimum_required (VERSION 3.15)

project(Prototype01-GoochHighlighted CXX)

file(GLOB_RECURSE MODELS LIST_DIRECTORIES false ../Common/Assets/Models/*.*)

set(SHADERS
    assets/goochhighlighted.psh
    assets/goochhighlighted.vsh
    assets/structures.fxh
)

add_prototype_app(Prototype01-GoochHighlighted
    IDE_FOLDER
        Prototypes
    SOURCES
        src/GoochHighlighted.cpp
    INCLUDES
        src/GoochHighlighted.h
    SHADERS ${SHADERS}
    ASSETS ${MODELS}
)

target_link_libraries(Prototype01-GoochHighlighted 
PRIVATE
    Diligent-AssetLoader
)

foreach(FILE ${MODELS})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${FILE}")
    get_filename_component(REL_DIR "${REL_PATH}" DIRECTORY)
    set_source_files_properties(${FILE} PROPERTIES
        VS_DEPLOYMENT_LOCATION "${REL_DIR}"
        MACOSX_PACKAGE_LOCATION "Resources/${REL_DIR}"
    )
endforeach(FILE)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../Common/Assets PREFIX Assets FILES ${MODELS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/assets PREFIX Assets/Shaders FILES ${SHADERS})