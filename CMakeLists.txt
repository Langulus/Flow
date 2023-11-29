cmake_minimum_required(VERSION 3.24)
project(LangulusFlow
    VERSION         1.0.0
    DESCRIPTION     "Langulus flow library, and code parser"
    HOMEPAGE_URL    https://langulus.com
)

# Check if this project is built as standalone, or a part of something else 
if(PROJECT_IS_TOP_LEVEL OR NOT LANGULUS)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

    include(LangulusUtilities.cmake)

    # Add Langulus::Core/Logger/RTTI/SIMD/Fractalloc/Anyness libraries      
	fetch_langulus_module(Core    GIT_TAG c11832957380709e316ffaf737d47c4578ed8caf)
	fetch_langulus_module(Logger  GIT_TAG e137aac46f3d682a06e8d0b1a160748fcbca21a4)
	fetch_langulus_module(RTTI    GIT_TAG 74d1a5fd0f5f6c23fd23c7921e1ba1c5df8c94c3)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG 9c46cbf6e10585b98f25b3d97ed35c1be7b14e45)
    endif()
	fetch_langulus_module(SIMD    GIT_TAG f70c0a00839c23024acc045d0ffb5eda1e3671a0)
	fetch_langulus_module(Anyness GIT_TAG 603ec6c815c539e8e2ed0c57716573e6d6f9e722)
endif()

# Build and install Flow library                                            
add_library(LangulusFlow ${LANGULUS_LIBRARY_TYPE}
    $<TARGET_OBJECTS:LangulusLogger>
    $<TARGET_OBJECTS:LangulusRTTI>
    $<$<BOOL:${LANGULUS_FEATURE_MANAGED_MEMORY}>:$<TARGET_OBJECTS:LangulusFractalloc>>
    $<TARGET_OBJECTS:LangulusAnyness>
    source/Code.cpp
    source/Construct.cpp
    source/Executor.cpp
    source/Temporal.cpp
    source/Verb.cpp
)

target_include_directories(LangulusFlow PUBLIC include
    $<TARGET_PROPERTY:LangulusLogger,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:LangulusRTTI,INTERFACE_INCLUDE_DIRECTORIES>
    $<$<BOOL:${LANGULUS_FEATURE_MANAGED_MEMORY}>:$<TARGET_PROPERTY:LangulusFractalloc,INTERFACE_INCLUDE_DIRECTORIES>>
    $<TARGET_PROPERTY:LangulusAnyness,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(LangulusFlow
    PUBLIC      LangulusCore
                fmt
)

target_compile_definitions(LangulusFlow
    PRIVATE     LANGULUS_EXPORT_ALL
)

if(LANGULUS_TESTING)
    enable_testing()
	add_subdirectory(test)
endif()