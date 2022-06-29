cmake_minimum_required(VERSION 3.22)
include(cmake/FetchLangulusModule.cmake)
project(Langulus_Flow)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(LANGULUS_FEATURE_MANAGED_MEMORY ON)
#set(LANGULUS_FEATURE_MANAGED_REFLECTION ON)
add_definitions(-DLANGULUS_ENABLE_MANAGED_MEMORY)
add_definitions(-DLANGULUS_ENABLE_MANAGED_REFLECTION)

# Check if this project is built as standalone, or a part of something else
if (PROJECT_IS_TOP_LEVEL)
	fetch_langulus_module(Core)
	fetch_langulus_module(Logger)
	fetch_langulus_module(RTTI)
	fetch_langulus_module(Anyness)

    enable_testing()
	add_subdirectory(test)
endif()

# Build and install Flow library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)
add_library(Langulus.Flow ${SOURCE_FILES})

target_include_directories(Langulus.Flow PUBLIC include)
target_link_libraries(Langulus.Flow PUBLIC Langulus.Anyness)

if (MSVC)
    target_compile_options(Langulus.Flow PRIVATE /utf-8)
endif()