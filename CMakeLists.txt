cmake_minimum_required(VERSION 3.22)
include(cmake/FetchLangulusModule.cmake)
project(Langulus.Flow)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure the features you require, all these are disabled by default
option(LANGULUS_ENABLE_SAFE_MODE 
    "Overrides additional error checking and sanity checks, \
    incurs a serious runtime overhead")

option(LANGULUS_ENABLE_PARANOIA 
    "Make sure that any unused memory is nullified, \
    incurs a serious runtime overhead")

option(LANGULUS_ENABLE_DEBUGGING 
    "Explicitly define debug mode, in the case that default flags are \
    not detected by some odd reason")

set(LANGULUS_FEATURE_MANAGED_MEMORY ON CACHE BOOL "Managed memory feature")
set(LANGULUS_FEATURE_MANAGED_REFLECTION ON CACHE BOOL "Managed reflection feature")

add_compile_definitions(LANGULUS_ENABLE_FEATURE_MANAGED_MEMORY)
add_compile_definitions(LANGULUS_ENABLE_FEATURE_MANAGED_REFLECTION)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message(STATUS "Debug mode, all assertions are enabled")
	add_compile_definitions(LANGULUS_ENABLE_ASSERTION_LEVEL=1000)
    add_compile_definitions(LANGULUS_ENABLE_FEATURE_MEMORY_STATISTICS)
    add_compile_definitions(LANGULUS_ENABLE_DEBUGGING)
endif()

if (CMAKE_CXX_FLAGS)
    STRING(FIND ${CMAKE_CXX_FLAGS} "/Zc:trigraphs" TRIGRAPHS_ENABLED)
    if (${TRIGRAPHS_ENABLED} GREATER -1)
        message(FATAL_ERROR "Langulus can't be built with trigraphs enabled")
    endif()

    STRING(FIND ${CMAKE_CXX_FLAGS} "-trigraphs" TRIGRAPHS_ENABLED)
    if (${TRIGRAPHS_ENABLED} GREATER -1)
        message(FATAL_ERROR "Langulus can't be built with trigraphs enabled")
    endif()
endif()

# Check if this project is built as standalone, or a part of something else
if (PROJECT_IS_TOP_LEVEL)
	fetch_langulus_module(Core)
	fetch_langulus_module(Logger)
	fetch_langulus_module(RTTI)
	fetch_langulus_module(Anyness)

    add_compile_definitions(LANGULUS_ENABLE_TESTING)
    enable_testing()
	add_subdirectory(test)
endif()

# Build and install Flow library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source/verbs SOURCE_FILES)
add_library(Langulus.Flow ${SOURCE_FILES})
target_include_directories(Langulus.Flow PUBLIC include)
target_link_libraries(Langulus.Flow PUBLIC Langulus.RTTI Langulus.Anyness)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(Langulus.Flow PUBLIC -Wno-trigraphs)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(Langulus.Flow PUBLIC -Wno-trigraphs)
elseif (MSVC)
    target_compile_options(Langulus.Flow PRIVATE /utf-8)
endif()
