cmake_minimum_required(VERSION 3.15)
project(DDAEngine)

# Use modern C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(ImportDependencies)





# Define library dependencies (preserved)
set(ALL_LIBS)
importDependencies()

# === Collect source and header files ===
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "${CMAKE_SOURCE_DIR}/includes/*.h" "${CMAKE_SOURCE_DIR}/includes/*.hpp")

# === Filter out main.cpp from library build ===
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")

# === Unity plugin DLL (no main, clean C API) ===
add_library(DDAEngine SHARED
    ${SOURCE_FILES}
    ${HEADER_FILES}

)

target_include_directories(DDAEngine
    PRIVATE "${CMAKE_SOURCE_DIR}/src"
    PRIVATE "${CMAKE_SOURCE_DIR}/includes"
)

# Define export macro for Windows DLL
target_compile_definitions(DDAEngine PRIVATE DDA_ENGINE_EXPORTS)



# === Native test app with main() ===
add_executable(DDAEngineTest
    ${SOURCE_FILES}
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
)

target_include_directories(DDAEngineTest
    PRIVATE "${CMAKE_SOURCE_DIR}/src"
    PRIVATE "${CMAKE_SOURCE_DIR}/includes"
)

# Define export macro for test executable too since it compiles the sources directly
target_compile_definitions(DDAEngineTest PRIVATE DDA_ENGINE_EXPORTS)

# Link ALL_LIBS if needed later (won’t fail if not linked)
target_link_libraries(DDAEngine
    ${ALL_LIBS}
)

target_link_libraries(DDAEngineTest
    ${ALL_LIBS}
)


