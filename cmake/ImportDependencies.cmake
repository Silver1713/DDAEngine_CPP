include(FetchContent)
set(DEP_DIR "${CMAKE_BINARY_DIR}/lib/")

# Macro to import tinyobjloader
macro(import_tinyobjloader)
    if(NOT TARGET tinyobjloader) # Guard to prevent multiple inclusion
        FetchContent_Declare(
            tinyobjloader
            GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
            GIT_TAG release
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/tinyobjloader
        )
        FetchContent_MakeAvailable(tinyobjloader)

        # include_directories(${tinyobjloader_SOURCE_DIR})
    endif()
endmacro()

macro(import_lodepng)
    if(NOT TARGET lodepng)
        FetchContent_Declare(
            lodepng
            GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
            GIT_TAG master
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/lodepng
        )
        FetchContent_MakeAvailable(lodepng)
        include_directories(${lodepng_SOURCE_DIR})
    endif()
endmacro()

macro(import_glm)
    if(NOT TARGET glm)
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG master
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/glm
        )
        FetchContent_MakeAvailable(glm)
        include_directories(${glm_SOURCE_DIR})
    endif()
endmacro()

macro(import_glew)
    if(NOT TARGET glew_s)
        FetchContent_Declare(
            glew_s
            GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
            GIT_TAG master
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/glew
        )
        FetchContent_MakeAvailable(glew_s)
        include_directories(${glew_s_SOURCE_DIR}/include)
    endif()
endmacro()

macro(import_glfw)
    if(NOT TARGET glfw)
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG master
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/glfw
        )

        FetchContent_MakeAvailable(glfw)
        include_directories(${glfw_SOURCE_DIR}/include)
    endif()
endmacro()

# macro(import_fmod)
#     if(NOT TARGET fmod) 
#         FetchContent_Declare(
#             fmod
#             GIT_REPOSITORY https://github.com/rosshoyt/FMOD-Audio-Engine
#             GIT_TAG master
#             SOURCE_DIR ${DEP_DIR}/fmod
#         )
#         FetchContent_MakeAvailable(fmod)
#         include_directories(${fmod_SOURCE_DIR}/Include)
#     endif()
# endmacro()

macro(import_soil)
    if(NOT TARGET soil)
        FetchContent_Declare(
            soil
            GIT_REPOSITORY https://github.com/Silver1713/soil-cmake-gl4.5
            GIT_TAG master
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/soil
        )
        FetchContent_MakeAvailable(soil)
        include_directories(${soil_SOURCE_DIR}/inc/SOIL)
    endif()
endmacro()

macro(import_stb)
    if(NOT TARGET stb)
        FetchContent_Declare(
            stb
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_PROGRESS TRUE
            GIT_TAG master 
        )
        FetchContent_MakeAvailable(stb)
        add_library(stb INTERFACE)
        target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
        add_deps_all_targets(PUBLIC stb)
    endif()
endmacro()

macro(import_backward_stacktrace)
    if(NOT TARGET backward)
        FetchContent_Declare(
            backward
            GIT_REPOSITORY https://github.com/bombela/backward-cpp
            GIT_TAG master
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/backward
        )
        FetchContent_MakeAvailable(backward)
        include_directories(${backward_SOURCE_DIR})
    endif()

    if(TARGET backward)
        add_deps_all_targets(PUBLIC Backward::Backward)
    endif()
endmacro()

macro(import_json)
    if(NOT TARGET json)
        FetchContent_Declare(
            json
            GIT_REPOSITORY https://github.com/nlohmann/json
            GIT_TAG b36f4c477c40356a0ae1204b567cca3c2a57d201
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/json
        )
        FetchContent_MakeAvailable(json)
        include_directories(${json_SOURCE_DIR}/include)
    endif()
endmacro()

macro(import_fmod)
    # import LOCAL version of fmod in FMOD/ folder
    set(FMOD_IMPORT_SUCCESS TRUE) # Set flag to test for import if successful
    set(FMOD_API_INSTALL_DIR "${CMAKE_SOURCE_DIR}/FMOD") # Location of FMOD Folder
    message(STATUS "${CMAKE_SOURCE_DIR}/FMOD") 
    set(FMOD_INCL_DIR "${FMOD_API_INSTALL_DIR}/api/core/inc") #FMOD API include directory
    set(FMOD_LIB_DIR "${FMOD_API_INSTALL_DIR}/api/core/lib/x64") #FMOD API library directory
    #set(FMOD_LIB_DIR_RELATIVE "../FMOD/api/core/lib/x64") 
    if(NOT EXISTS "${FMOD_API_INSTALL_DIR}") # Check if FMOD folder exists
        message(STATUS "FMOD API not found in FMOD/ folder")
        set(FMOD_IMPORT_SUCCESS FALSE) # SET flag to false if FMOD folder not found
    endif()

    if(NOT EXISTS "${FMOD_INCL_DIR}") # Check if FMOD API include directory exists
        message(STATUS "FMOD API include directory not found in FMOD/ folder but FMOD API found check FMOD/api/core/inc")
        set(FMOD_IMPORT_SUCCESS FALSE) # Set to false
    endif()

    IF(${FMOD_IMPORT_SUCCESS}) # IF Everything is found import FMOD API
    # Find the .lib file in the lib dir, this is for both debug and release
        find_library(FMOD_LIBRARY_DEBUG fmodL_vc PATHS ${FMOD_LIB_DIR}) # Find the .lib fiile in the lib dir
        find_library(FMOD_LIBRARY_RELEASE fmod_vc PATHS ${FMOD_LIB_DIR}) # Find the .lib file in the lib dir
    # IF no library is found in the lib dir, set flag to false
        if(NOT FMOD_LIBRARY_DEBUG OR NOT FMOD_LIBRARY_RELEASE)
            message(STATUS "FMOD API library not found in FMOD/ folder")
            set(FMOD_IMPORT_SUCCESS FALSE)
        endif()

        add_library(fmod SHARED IMPORTED) # Add FMOD as a shared library
        # Configure the properties of the FMOD library
        set_target_properties(fmod PROPERTIES

            # Debug configuration
            IMPORTED_LOCATION_DEBUG ${FMOD_LIB_DIR}/fmodL.dll
            IMPORTED_IMPLIB_DEBUG ${FMOD_LIBRARY_DEBUG}
        

            # Release configuration
            IMPORTED_LOCATION_RELEASE ${FMOD_LIB_DIR}/fmod.dll
            IMPORTED_IMPLIB_RELEASE ${FMOD_LIBRARY_RELEASE}

            # MinSizeRel configuration
            IMPORTED_LOCATION_MINSIZEREL ${FMOD_LIB_DIR}/fmod.dll
            IMPORTED_IMPLIB_MINSIZEREL ${FMOD_LIBRARY_RELEASE}

            # RelWithDebInfo configuration
            IMPORTED_LOCATION_RELWITHDEBINFO ${FMOD_LIB_DIR}/fmod.dll
            IMPORTED_IMPLIB_RELWITHDEBINFO ${FMOD_LIBRARY_RELEASE}

            #Installer configuration
            IMPORTED_LOCATION_INSTALLER ${FMOD_LIB_DIR}/fmod.dll
            IMPORTED_IMPLIB_INSTALLER ${FMOD_LIBRARY_RELEASE}

            INTERFACE_INCLUDE_DIRECTORIES ${FMOD_INCL_DIR}
        )

        # Add the library to the list of dependencies
        add_deps_all_targets(PUBLIC fmod)

    ELSE()
        message(STATUS "FMOD Import Failed.")
    ENDIF()
endmacro()

macro(import_freetype)

if (NOT TARGET freetype)
    FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype/
        GIT_TAG master
        GIT_PROGRESS TRUE
        SOURCE_DIR ${DEP_DIR}/freetype
    )
    FetchContent_MakeAvailable(freetype)
    include_directories(${freetype_SOURCE_DIR}/include)
    add_deps_all_targets(PUBLIC freetype)
endif()
endmacro()

macro (import_imgui)
    if (NOT TARGET imgui)
        FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui
            GIT_TAG docking
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/imgui
        )
        FetchContent_MakeAvailable(imgui)
        file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
        add_library(
            imgui
            ${IMGUI_SOURCES}
            ${imgui_SOURCE_DIR}/imgui_internal.h
        )
        add_library(
            imgui_backends
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )
        include_directories(${imgui_SOURCE_DIR})
        include_directories(${imgui_SOURCE_DIR}/backends)
        add_deps_all_targets(PUBLIC imgui imgui_backends)
    endif()
endmacro()

macro (import_imguizmo)
    if (NOT TARGET ImGuizmo)
        FetchContent_Declare(
            ImGuizmo
            GIT_REPOSITORY https://github.com/Silver1713/ImGuizmo-CMAKE.git
            GIT_TAG master
            GIT_PROGRESS TRUE
            SOURCE_DIR ${DEP_DIR}/ImGuizmo
        )
        FetchContent_MakeAvailable(ImGuizmo)
        include_directories(${ImGuizmo_SOURCE_DIR})
        add_deps_all_targets(PUBLIC ImGuizmo)
    endif()
endmacro()


macro(import_mono)
set(MONO_IMPORT_DIR "${CMAKE_SOURCE_DIR}/MONO")
set(MONO_IMPORT_INCLUDE_DIR "${MONO_IMPORT_DIR}/include/mono-2.0")
set(MONO_IMPORT_LIB_DIR "${MONO_IMPORT_DIR}/lib")
set(MONO_IMPORT_SUCCESS TRUE)
if(NOT EXISTS "${MONO_IMPORT_DIR}")
    message(STATUS "MONO not found in MONO/ folder")
    set(MONO_IMPORT_SUCCESS FALSE)
endif()
if (NOT EXISTS "${MONO_IMPORT_INCLUDE_DIR}")
    message(STATUS "MONO include directory not found in MONO/ folder")
    set(MONO_IMPORT_SUCCESS FALSE)
endif()
if (NOT EXISTS "${MONO_IMPORT_LIB_DIR}")
    message(STATUS "MONO lib directory not found in MONO/ folder")
    set(MONO_IMPORT_SUCCESS FALSE)
endif()

IF (${MONO_IMPORT_SUCCESS})
    message(STATUS "${MONO_IMPORT_LIB_DIR} HERE")
    find_library(MONO_LIBRARY mono-2.0-sgen PATHS ${MONO_IMPORT_LIB_DIR})
    find_library(MONO_POSIX_LIBRARY MonoPosixHelper PATHS ${MONO_IMPORT_LIB_DIR})

    # Collect necessary bin files
    set(MONO_DLL_PATH "${MONO_IMPORT_DIR}/bin/mono-2.0-sgen.dll")
    set(MONO_DLL_STATIC "${MONO_IMPORT_DIR}/bin/libmono-btls-shared")
    set(MONO_DLL_POSIX "${MONO_IMPORT_DIR}/bin/MonoPosixHelper.dll")

    if (NOT MONO_LIBRARY)
        message(STATUS "MONO library not found in MONO/ folder")
        set(MONO_IMPORT_SUCCESS FALSE)
    endif()
    
    set(CMAKE_CSharp_COMPILER "${MONO_IMPORT_DIR}/bin/csc")
    
    add_library(mono SHARED IMPORTED)
    message(STATUS "Mono: ${MONO_LIBRARY}")
    set_target_properties(mono PROPERTIES
        IMPORTED_IMPLIB_RELEASE "${MONO_LIBRARY}"
        IMPORTED_IMPLIB_DEBUG "${MONO_LIBRARY}"
        IMPORTED_IMPLIB_MINSIZEREL "${MONO_LIBRARY}"
        IMPORTED_IMPLIB_RELWITHDEBINFO "${MONO_LIBRARY}"
        IMPORTED_IMPLIB_INSTALLER "${MONO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES ${MONO_IMPORT_INCLUDE_DIR}
    )


    add_library(mono_interface INTERFACE)

    target_link_libraries(mono_interface 
    INTERFACE mono
    ${MONO_POSIX_LIBRARY}
    )
    add_deps_all_targets(PUBLIC mono_interface)
ELSE()
    message(STATUS "MONO Import Failed.")
ENDIF()



endmacro()
macro(add_deps_all_targets ...)
    list(APPEND ALL_LIBS ${ARGN})
endmacro()

# Macro to import all dependencies
macro(importDependencies)
    # import_tinyobjloader()
    #import_glew()
    #import_glfw()
    #import_glm()
    #import_lodepng()
    #import_soil()
    #import_stb()
    import_backward_stacktrace()
    #import_fmod()
    #import_freetype()
    import_json()
    #import_imgui()
     #import_imguizmo()
    #import_mono()
    message(STATUS "Importing dependencies...")
endmacro()