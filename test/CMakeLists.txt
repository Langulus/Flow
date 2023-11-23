file(GLOB_RECURSE
	LANGULUS_FLOW_TEST_SOURCES 
	LIST_DIRECTORIES FALSE CONFIGURE_DEPENDS
	*.cpp *.hpp
)

add_executable(LangulusFlowTest ${LANGULUS_FLOW_TEST_SOURCES})

target_link_libraries(LangulusFlowTest
	PRIVATE		LangulusFlow
				Catch2
)

add_test(
	NAME		LangulusFlowTest
	COMMAND		LangulusFlowTest
)