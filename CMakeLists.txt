set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_minimum_required(VERSION 3.16)
project(rs LANGUAGES C CXX)

# --- Default build type if none specified ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# --- Windows cross-compilation option ---
option(WINDOWS_CROSS_COMPILE "Cross-compile for Windows using MinGW" OFF)
set(SCIP_BINARY_PATH "~/scip" CACHE PATH "Path to SCIP binaries for Windows cross-compilation")

# --- 2. Shared source files ---
set(COMMON_SOURCES
        ../lib/roaring.c
        src/Utils.cpp
        src/rsParams.c
        src/MasterProblemBase.cpp
        src/HeuristicPricerBase.cpp
        src/binary/BooleanClause.hpp
        src/binary/MasterProblemBinary.cpp
        src/binary/ExactPricerBinary.cpp
        src/binary/HeuristicPricerBinary.cpp
        src/threshold/ExactPricerThreshold.cpp
        src/threshold/MasterProblemThreshold.cpp
        src/threshold/HeuristicPricerThreshold.cpp
        src/threshold/separation/UpperBoundOracle.cpp
        src/threshold/separation/UpperBoundOracle.hpp
        src/threshold/separation/LowerBoundOracle.cpp
        src/threshold/separation/MinConshdlr.cpp
        src/threshold/separation/MinConshdlr.hpp
        src/IterativePricer.cpp
        src/IterativePricer.hpp
        src/Generator.hpp
        src/Generator.hpp
)

#set_source_files_properties(../lib/roaring.c PROPERTIES C_STANDARD 11)

# --- 3. Executable ---
add_executable(rs src/main.cpp ${COMMON_SOURCES})

# --- 4. Include project-local libs ---
target_include_directories(rs PRIVATE "${CMAKE_SOURCE_DIR}/lib")

# --- 5. SCIP configuration ---
if(WINDOWS_CROSS_COMPILE)
    message(STATUS "Using SCIP binaries from: ${SCIP_BINARY_PATH}")
    target_include_directories(rs PRIVATE ${SCIP_BINARY_PATH}/include)
    target_link_directories(rs PRIVATE ${SCIP_BINARY_PATH}/lib)
    target_link_options(rs PRIVATE -Wl,--allow-multiple-definition)
else()
    find_package(scip REQUIRED)
endif()

# --- 6. Linking ---
target_link_libraries(rs PRIVATE scip)

# Linux-only libraries
if(NOT WINDOWS_CROSS_COMPILE AND NOT CMAKE_SYSTEM_NAME STREQUAL Windows)
    target_link_libraries(rs PRIVATE pthread m dl)
endif()

# --- 7. Optimization flags (config-dependent) ---
if(NOT MSVC AND NOT WINDOWS_CROSS_COMPILE)
    target_compile_options(rs PRIVATE
            -march=x86-64-v3
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:RelWithDebInfo>:-O2 -g>
            $<$<CONFIG:Debug>:-O0 -g>
    )
elseif(WINDOWS_CROSS_COMPILE)
    # MinGW on Linux targeting Windows
    target_compile_options(rs PRIVATE
            -march=x86-64-v3
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:RelWithDebInfo>:-O2 -g>
            $<$<CONFIG:Debug>:-O0 -g>
    )
else()
    # MSVC
    target_compile_options(rs PRIVATE
            /arch:AVX2
            /Zc:__cplusplus
            $<$<CONFIG:Release>:/O2>
            $<$<CONFIG:Debug>:/Od /Zi>
    )
endif()