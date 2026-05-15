cmake_minimum_required(VERSION 3.15)

if(NOT DILIGENT_CORE_FOUND)
	message(FATAL_ERROR "DiligentCore module is not found. To build the samples and tutorials, please clone the main repository as described in the documentation (see https://github.com/DiligentGraphics/DiligentSamples/blob/master/README.md)")
endif()
if(NOT DILIGENT_TOOLS_FOUND)
	message(FATAL_ERROR "DiligentTools module is not found. Please add DiligentTools module to the project before SGPrototypes module.")
endif()
if(NOT DILIGENT_FX_FOUND)
	message(FATAL_ERROR "DiligentFX module is not found. Please add DiligentFX module to the project before SGPrototypes module.")
endif()
if(NOT DILIGENT_SAMPLES_FOUND)
    message(FATAL ERROR "DiligentSamples module is not found. Please add DiligentSamples module to the project before SGPrototypes module.")
endif()

set(SG_DILIGENT_PROTOTYPES_FOUND TRUE CACHE INTERNAL "SG Diligent Enine Prototypes Found")

if(PLATFORM_WIN32 OR PLATFORM_LINUX)
    option(INSTALL_PROTOTYPES "Enable Installations of SG Prototypes" ON)
else()
    set(INSTALL_PROTOTYPES OFF)
endif()

# adapted from add_sample_app
function(add_prototype_app APP_NAME)
    set(options)
    set(oneValueArgs IDE_FOLDER DXC_REQUIRED)
    set(multiValueArgs SOURCES INCLUDES SHADERS ASSETS)
    cmake_parse_arguments(PARSE_ARGV 1 arg "${options}" "${oneValueArgs}" "${multiValueArgs}")
    if (arg_DXC_REQUIRED)
        set(DXC_REQUIRED ${arg_DXC_REQUIRED})
    else()
        set(DXC_REQUIRED NO)
    endif()

    set_source_files_properties(${arg_SHADERS} PROPERTIES VS_TOOL_OVERRIDE "None")
    set(ALL_ASSETS ${arg_ASSETS} ${arg_SHADERS})
    add_target_platform_app(${APP_NAME} "${arg_SOURCES}" "${arg_INCLUDES}" "${ALL_ASSETS}")

    set_source_files_properties(${ALL_ASSETS} PROPERTIES
        VS_DEPLOYMENT_LOCATION "."
        MACOSX_PACKAGE_LOCATION "Resources"
    )

    if(PLATFORM_WIN32)
        set_target_properties(${APP_NAME} PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        )
        copy_required_dlls(${APP_NAME} DXC_REQUIRED ${DXC_REQUIRED})
        append_sample_base_win32_source(${APP_NAME})
    elseif(PLATFORM_UNIVERSAL_WINDOWS)
        append_sample_base_uwp_source(${APP_NAME})
        package_required_dlls(${APP_NAME} DXC_REQUIRED ${DXC_REQUIRED})
    endif()
    
    target_include_directories(${APP_NAME}
    PRIVATE
        src
    )

    target_link_libraries(${APP_NAME}
    PRIVATE
        # On Linux we must have Diligent-NativeAppBase go first, otherwise the linker 
        # will fail to resolve Diligent::CreateApplication() function.
        Diligent-NativeAppBase
        Diligent-BuildSettings
        Diligent-SampleBase
    )
    set_common_target_properties(${APP_NAME})

    if(MSVC)
        # Disable MSVC-specific warnings
        # - w4201: nonstandard extension used: nameless struct/union
        target_compile_options(${APP_NAME} PRIVATE /wd4201)
    endif()

    if(arg_IDE_FOLDER)
        set_target_properties(${APP_NAME} PROPERTIES
            FOLDER ${arg_IDE_FOLDER}
        )
    endif()

    source_group("src" FILES ${arg_SOURCES} ${arg_INCLUDES})
    source_group("assets" FILES ${ALL_ASSETS})

    target_sources(${APP_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/readme.md" PROPERTIES HEADER_FILE_ONLY TRUE
    )

    if(PLATFORM_WIN32 OR PLATFORM_LINUX)
        # Copy assets to target folder
        add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                "\"$<TARGET_FILE_DIR:${APP_NAME}>\"")
    endif()

    if(PLATFORM_MACOS AND VULKAN_LIB_PATH)
        # Configure rpath so that executables can find vulkan library
        set_target_properties(${APP_NAME} PROPERTIES
            BUILD_RPATH "${VULKAN_LIB_PATH}"
        )
    endif()

    if(PLATFORM_WEB)
        set(RESOURCE_PATH "${PROJECT_SOURCE_DIR}/assets/")
        target_link_options(${APP_NAME} PRIVATE "SHELL: -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB --preload-file ${RESOURCE_PATH}@")

        if (${APP_NAME} STREQUAL "GLTFViewer")
            set(EXCLUDE_DIRECTORIES
                ".git"
                "BarbieDodgePickup"
                "IridescentDishWithOlives"
            )

            foreach(EXCLUDE_DIR IN LISTS EXCLUDE_DIRECTORIES)
                target_link_options(${APP_NAME} PRIVATE "SHELL: --exclude-file=${PROJECT_SOURCE_DIR}/assets/models/${EXCLUDE_DIR}")
            endforeach()
        endif()
        append_sample_base_emscripten_source(${APP_NAME})
    endif()

    if(INSTALL_PROTOTYPES)
         # Install instructions
        file(RELATIVE_PATH TUTORIAL_REL_PATH "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
        install(TARGETS     ${APP_NAME}
                DESTINATION "${CMAKE_INSTALL_BINDIR}/${TUTORIAL_REL_PATH}/$<CONFIG>")

        if(PLATFORM_LINUX OR PLATFORM_WIN32)
            install(DIRECTORY   "${CMAKE_CURRENT_SOURCE_DIR}/assets/"
                    DESTINATION "${CMAKE_INSTALL_BINDIR}/${TUTORIAL_REL_PATH}/$<CONFIG>")
        endif()

        if(PLATFORM_WIN32)
            get_supported_backends(BACKEND_LIBRARIES)
            install(TARGETS  ${BACKEND_LIBRARIES}
                    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${TUTORIAL_REL_PATH}/$<CONFIG>"
                    LIBRARY DESTINATION "${CMAKE_INSTALL_BINDIR}/${TUTORIAL_REL_PATH}/$<CONFIG>"
                    ARCHIVE DESTINATION "${CMAKE_INSTALL_BINDIR}/${TUTORIAL_REL_PATH}/$<CONFIG>")

            # Dawn uses DXC, so we need to copy the DXC dlls even if DXC_REQUIRED is not set
            # to get consistent shader compilation results
            if(((DXC_REQUIRED AND D3D12_SUPPORTED) OR WEBGPU_SUPPORTED) AND DXC_COMPILER_PATH AND DXIL_SIGNER_PATH)
                install(FILES "${DXC_COMPILER_PATH}" "${DXIL_SIGNER_PATH}"
                        DESTINATION "${CMAKE_INSTALL_BINDIR}/${TUTORIAL_REL_PATH}/$<CONFIG>")
            endif()
            if(DXC_REQUIRED AND VULKAN_SUPPORTED AND DILIGENT_DXCOMPILER_FOR_SPIRV_PATH AND EXISTS "${DILIGENT_DXCOMPILER_FOR_SPIRV_PATH}")
                install(FILES "${DILIGENT_DXCOMPILER_FOR_SPIRV_PATH}"
                        DESTINATION "${CMAKE_INSTALL_BINDIR}/${TUTORIAL_REL_PATH}/$<CONFIG>"
                        RENAME spv_dxcompiler.dll)
            endif()
        endif()

        if(PLATFORM_LINUX)
            set_target_properties(${APP_NAME} PROPERTIES
                INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${DILIGENT_CORE_DIR}/${CMAKE_BUILD_TYPE}"
            )
        endif()
    endif()
endfunction()

add_subdirectory(Prototype01-GoochHighlighted)

add_format_validation_target(Prototypes "${CMAKE_CURRENT_SOURCE_DIR}" Prototypes)

