# Test interleaved lattice
add_executable(testInterleavedLattice
    testInterleavedLattice.cpp
    ${TROGDOR_SOURCE_DIR}/InterleavedLattice.cpp
    ${TROGDOR_SOURCE_DIR}/MemoryUtilities.cpp
    ${TROGDOR_SOURCE_DIR}/YeeUtilities.cpp
)
target_link_libraries(testInterleavedLattice
    boost_unit_test_framework-xgcc40-mt
    utility
)
# gets "LOCATION" of "tester" and stores it in "testStuffLocation"
#get_target_property( testStuffLocation testInterleavedLattice LOCATION )

#add_custom_command(TARGET testInterleavedLattice
#    POST_BUILD
#    COMMAND ${testStuffLocation}
#)


# Test geometry
add_executable(testGeometry
    testGeometry.cpp
)
target_link_libraries(testGeometry
    boost_unit_test_framework-xgcc40-mt
)
#add_custom_command(TARGET testGeometry
#    POST_BUILD
#    COMMAND ${testStuffLocation}
#)

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


