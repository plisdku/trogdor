add_executable(testInterleavedLattice
    testInterleavedLattice.cpp
    ${TROGDOR_SOURCE_DIR}/InterleavedLattice.cpp
)

include_directories(
	${TROGDOR_SOURCE_DIR}
	${TROGDOR_SOURCE_DIR}/utility
	${TROGDOR_SOURCE_DIR}/tinyxml
	${magIncDirs}
	/usr/local/include/
	/usr/local/include/boost-1_38
	/sw/include
)
	
link_directories(
    ${TROGDOR_BINARY_DIR}/utility
    ${TROGDOR_BINARY_DIR}/tinyxml
    ${magLibDirs}
    /usr/local/lib
    /sw/lib
)

target_link_libraries(testInterleavedLattice
    boost_unit_test_framework-xgcc40-mt
)

# gets "LOCATION" of "tester" and stores it in "myExecutableIsHere"
get_target_property( myExecutableIsHere testInterlavedLattice LOCATION )
message("The location is ${myExecutableIsHere}.")

# make a target called RUN_TESTS that is built (run) every time that tester
# changes... something like that.
# Run the tests with "make RUN_TESTS"
add_custom_target( RUN_TESTS
    ${myExecutableIsHere} --report_level=no
    DEPENDS testInterleavedLattice
)


