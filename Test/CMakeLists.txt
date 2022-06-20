project(TestFlow)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)

add_executable(TestFlow ${SOURCE_FILES})

target_link_libraries(TestFlow PRIVATE Flow Catch2)

install(TARGETS TestFlow DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})