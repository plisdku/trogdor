project( TROGDOR )

EXECUTE_PROCESS(COMMAND Magick++-config --libs
	OUTPUT_VARIABLE mlibs)
EXECUTE_PROCESS(COMMAND Magick++-config --cppflags
	OUTPUT_VARIABLE mcppflags)
EXECUTE_PROCESS(COMMAND Magick++-config --ldflags
	OUTPUT_VARIABLE mldflags)

string(REGEX MATCHALL "[-]l[^ ;\n]+" rawLibs "${mlibs}")
string(REGEX REPLACE "[-]l([^ ;\n]+)" "\\1" magDepLibs "${rawLibs}")
message("ImageMagick libraries are ${magDepLibs}")

string(REGEX MATCHALL "[-]L[^ ;\n]+" rawLibDirs "${mldflags}")
string(REGEX REPLACE "[-]L([^ ;\n]+)" "\\1" magLibDirs "${rawLibDirs}")
message("ImageMagick lib directories are ${magLibDirs}")

string(REGEX MATCHALL "[-]I[^ ;\n]+" rawIncdirs "${mcppflags}")
string(REGEX REPLACE "[-]I([^ ;\n]+)" "\\1" magIncDirs "${rawIncdirs}")
message("ImageMagick inc directories are ${magIncDirs}")

include_directories(
	${TROGDOR_SOURCE_DIR}
	${TROGDOR_SOURCE_DIR}/utility
	${TROGDOR_SOURCE_DIR}/tinyxml
	${magIncDirs}
	/usr/local/include/
	/sw/include)

link_directories(
${TROGDOR_BINARY_DIR}/utility
${TROGDOR_BINARY_DIR}/tinyxml
${magLibDirs}
/usr/local/lib
/sw/lib)

source_group("Main Group" FILES main.cpp)

add_executable( trogdor
BufferedCurrent.h
BufferedCurrent.cpp
BufferedCurrent-inl.h
BulkSetupMaterials.h
BulkSetupMaterials.cpp
CalculationPartition.h
CalculationPartition.cpp
CFSRIPML.h
CFSRIPML.cpp
CFSRIPML-inl.h
ConvertOldXML.h
ConvertOldXML.cpp
CurrentPolarizationOutput.h
CurrentPolarizationOutput.cpp
CurrentSource.h
CurrentSource.cpp
DrudeModel1.h
DrudeModel1.cpp
FDTDApplication.h
FDTDApplication.cpp
FieldInput.h
FieldInput.cpp
HuygensSurface.h
HuygensSurface.cpp
HuygensLink.h
HuygensLink.cpp
InterleavedLattice.h
InterleavedLattice.cpp
main.cpp
Material.h
Material.cpp
MaterialFactory.h
MaterialFactory.cpp
MaterialRunlineEncoder.h
MaterialRunlineEncoder.cpp
MemoryUtilities.h
MemoryUtilities.cpp
NullCurrent.h
NullPML.h
Output.h
Output.cpp
Paint.h
Paint.cpp
PartitionCellCount.h
PartitionCellCount.cpp
PerfectConductor.h
PerfectConductor.cpp
Performance.h
Performance.cpp
PhysicalConstants.h
PhysicalConstants.cpp
Runline.h
Runline.cpp
RunlineEncoder.h
RunlineEncoder.cpp
SetupConstants.h
SetupMaterial.h
SetupMaterial.cpp
SimpleEHOutput.h
SimpleEHOutput.cpp
SimpleMaterialTemplates.h
SimpleMaterialTemplates.cpp
SimulationDescription.h
SimulationDescription.cpp
SimulationDescriptionPredeclarations.h
Source.h
Source.cpp
StaticDielectric.h
StaticDielectric.cpp
StaticLossyDielectric.h
StaticLossyDielectric.cpp
StructuralReports.h
StructuralReports.cpp
UpdateEquation.h
UpdateEquation.cpp
UpdateHarness.h
UpdateHarness.cpp
UpdateHarness-inl.h
Version.h
VoxelGrid.h
VoxelGrid.cpp
VoxelizedPartition.h
VoxelizedPartition.cpp
XMLParameterFile.h
XMLParameterFile.cpp
YeeUtilities.h
YeeUtilities.cpp
)

#set_target_properties(trogdor PROPERTIES COMPILE_FLAGS -Winline)

# MaterialFactory.cpp is the file that includes, ultimately, all the templated
# update equations.  For this reason I like to dump the optimized gimple and
# set some warnings pertaining to inlining and such, just to make sure that gcc
# is doing what I want it to do!
set_source_files_properties(MaterialFactory.cpp PROPERTIES COMPILE_FLAGS
    "-fdump-tree-optimized -Winline")

set(CMAKE_INCLUDE_PATH /sw/include)
set(CMAKE_LIBRARY_PATH /sw/lib)

find_package( Boost )
if (NOT Boost_FOUND)
	set(ENV{BOOST_ROOT} /sw)
endif(NOT Boost_FOUND)

find_package( Boost )
if (NOT Boost_FOUND)
	message("Can't find Boost yet at root $ENV{BOOST_ROOT}.")
endif(NOT Boost_FOUND)


#set(Boost_USE_STATIC_LIBS OFF)
#set(BOOST_ROOT /usr/local)
#set(BOOST_INCLUDE_DIR /usr/local/include/boost)
#set(Boost_USE_MULTITHREADED ON)
find_package( Boost COMPONENTS date_time program_options thread)

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	message("\nBoost includes are at ${Boost_INCLUDE_DIRS}")
	message("\nBoost libraries are ${Boost_LIBRARIES}")
	target_link_libraries(trogdor ${Boost_LIBRARIES})
else(Boost_FOUND)
	message("\nCan't find Boost!")
endif(Boost_FOUND)

target_link_libraries( trogdor
    tinyxml
    utility
    ${magDepLibs}
)

# this won't work without a recent boost version with the unit test framework

#if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    add_subdirectory( tests )
#    enable_testing() # only need to do this once, but
#    add_test(geom tests/testGeometry) # this once per test
#    add_test(lattice tests/testInterleavedLattice) # this once per test
#endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

add_subdirectory( utility )
add_subdirectory( tinyxml )



