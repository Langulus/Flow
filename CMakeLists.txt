cmake_minimum_required(VERSION 3.20)
include(cmake/FetchLangulusModule.cmake)

project(Langulus_Flow)

# Check if this project is built as standalone, or a part of something else
if (PROJECT_IS_TOP_LEVEL)
    # We're building it as standalone
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(LANGULUS_FEATURE_MANAGED_MEMORY ON)

	fetch_langulus_module(Core)
	fetch_langulus_module(Logger)
	fetch_langulus_module(Anyness)

	target_link_libraries(Langulus.Logger PUBLIC Langulus.Core)
	target_link_libraries(Langulus.Anyness PUBLIC Langulus.Core)
    enable_testing()
	add_subdirectory(test)
endif()

# Build and install Logger library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)
add_library(Langulus.Flow ${SOURCE_FILES})

target_include_directories(Langulus.Flow
	PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Langulus.Flow
	PUBLIC Langulus.Anyness
)

if (MSVC)
    target_compile_options(Langulus.Flow PRIVATE /utf-8)
endif()