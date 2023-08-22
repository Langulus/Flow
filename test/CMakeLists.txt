add_executable(LangulusFlowTest
	Main.cpp
	TestFactory.cpp
	TestFlow.cpp
	TestResolvable.cpp
	TestSerialization.cpp
	TestTemporal.cpp
	TestVerbs.cpp
)

target_link_libraries(LangulusFlowTest
	PRIVATE		LangulusFlow
				Catch2
)

add_test(
	NAME		LangulusFlowTest
	COMMAND		LangulusFlowTest
)

langulus_copy_dlls(LangulusFlowTest ON LangulusFlowTest)
