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
	fetch_langulus_module(Core    GIT_TAG d22b2ae2bae11deec5ec20e6baed4839fbf95aaa)
	fetch_langulus_module(Logger  GIT_TAG 3314fbe3bf5611a5bd9ee649f2a6ba5c43f6b4c4)
	fetch_langulus_module(RTTI    GIT_TAG ca00976e185070ca55dada3d4cc950ab5abba042)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG 0126c1750d9101ab29227a735d2a7a06da672e02)
    endif()
	fetch_langulus_module(SIMD    GIT_TAG 4aadc7d243c10a09e8095bf502909931b6ec6122)
	fetch_langulus_module(Anyness GIT_TAG 4608cf580abcb8700ba910c3fb9a7563164c31c2)
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