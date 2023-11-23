cmake_minimum_required(VERSION 3.24)
project(LangulusFlow
    VERSION         1.0.0
    DESCRIPTION     "Langulus flow library, and code parser"
    HOMEPAGE_URL    https://langulus.com
)

# Check if this project is built as standalone, or a part of something else 
if(PROJECT_IS_TOP_LEVEL OR NOT LANGULUS)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/lib)

    include(LangulusUtilities.cmake)

    # Add Langulus::Core/Logger/RTTI/SIMD/Fractalloc/Anyness libraries      
	fetch_langulus_module(Core    GIT_TAG 0097bd180a5c625f88db0db0514bbf93aff53c7a)
	fetch_langulus_module(Logger  GIT_TAG b63513abb9fb3014107335949e17e41e6e8a334b)
	fetch_langulus_module(RTTI    GIT_TAG f7c610255b819b99a6adcdefa970387490093c25)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG 17a8741cfa252b30e20257bb9d953d89278c4039)
    endif()
	fetch_langulus_module(SIMD    GIT_TAG 3cfb3bd47a8e0d610e0d473d74fbbde14dcea298)
	fetch_langulus_module(Anyness GIT_TAG b0ba5561250825aaeb3a660d6e246c04166349b6)
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