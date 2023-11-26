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
	fetch_langulus_module(Core    GIT_TAG 11f1ae8b23562ea759f0a0388d505b9a8ce39253)
	fetch_langulus_module(Logger  GIT_TAG 21c138042da9fbe03abbdc303934183d08e03655)
	fetch_langulus_module(RTTI    GIT_TAG 2b2cde5fb80a5fbb32b29019972e59ce04dde6ba)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG f0670504fd5ec06ce82810e44320819d2e39189c)
    endif()
	fetch_langulus_module(SIMD    GIT_TAG 1391ffacc287399921f63351349c4a36890d11b5)
	fetch_langulus_module(Anyness GIT_TAG 0902af4b6b6625ac8c0202934cc3b6d4b26d1caf)
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