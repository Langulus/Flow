cmake_minimum_required(VERSION 3.24)
project(LangulusFlow
    VERSION         1.0.0
    DESCRIPTION     "Langulus flow library, and code parser"
    HOMEPAGE_URL    https://langulus.com
)

# Check if this project is built as standalone, or a part of something else 
if(PROJECT_IS_TOP_LEVEL OR NOT LANGULUS)
    include(LangulusUtilities.cmake)

    # Add Langulus::Core/Logger/RTTI/SIMD/Fractalloc/Anyness libraries      
	fetch_langulus_module(Core    GIT_TAG 39ae88b)
	fetch_langulus_module(Logger  GIT_TAG 3314fbe)
	fetch_langulus_module(RTTI    GIT_TAG 3e05451)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG 4985d65)
    endif()
	fetch_langulus_module(SIMD    GIT_TAG 57e150d)
	fetch_langulus_module(Anyness GIT_TAG 0e3eb3d)
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

target_include_directories(LangulusFlow
    PUBLIC      include
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