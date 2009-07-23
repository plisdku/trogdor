# Test interleaved lattice
add_executable(testInterleavedLattice
    testInterleavedLattice.cpp
    ${TROGDOR_SOURCE_DIR}/InterleavedLattice.cpp
    ${TROGDOR_SOURCE_DIR}/MemoryUtilities.cpp
    ${TROGDOR_SOURCE_DIR}/YeeUtilities.cpp
)

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#find_package( Boost COMPONENTS unit_test_framework)

#if(Boost_FOUND)
#	include_directories(${Boost_INCLUDE_DIRS})
#	message("Boost testing includes are at ${Boost_INCLUDE_DIRS}")
#	message("Boost testing libraries are ${Boost_LIBRARIES}")
#endif()


target_link_libraries(testInterleavedLattice
    boost_unit_test_framework-xgcc40-mt
#    ${Boost_LIBRARIES}
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
#    ${Boost_LIBRARIES}
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
#	${Boost_INCLUDE_DIRS}
	/sw/include
)
	
link_directories(
    ${TROGDOR_BINARY_DIR}/utility
    ${TROGDOR_BINARY_DIR}/tinyxml
    ${magLibDirs}
    /usr/local/lib
    /sw/lib
)


