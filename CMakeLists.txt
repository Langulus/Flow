add_subdirectory(Test)

project(Flow)

add_definitions(-DPC_EXPORT_FLOW)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Block SOURCE_FILES)

add_library(Flow SHARED ${SOURCE_FILES})

target_include_directories(Flow
	PRIVATE		${CMAKE_CURRENT_SOURCE_DIR}
	INTERFACE	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Flow
	PUBLIC		PCMemory
)

install(TARGETS Flow DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})