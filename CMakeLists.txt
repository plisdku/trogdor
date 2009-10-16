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
	${TROGDOR_SOURCE_DIR}/materials
	${TROGDOR_SOURCE_DIR}/updateModules
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
BufferedFieldInput.cpp
BufferedFieldInput.h
BulkSetupMaterials.cpp
BulkSetupMaterials.h
CalculationPartition.cpp
CalculationPartition.h
ConvertOldXML.cpp
ConvertOldXML.h
CurrentPolarizationOutput.cpp
CurrentPolarizationOutput.h
CurrentSource.cpp
CurrentSource.h
FDTDApplication.cpp
FDTDApplication.h
HuygensLink.cpp
HuygensLink.h
HuygensSurface.cpp
HuygensSurface.h
InterleavedLattice.cpp
InterleavedLattice.h
IODescriptionFile.h
IODescriptionFile.cpp
main.cpp
MaterialFactory.cpp
MaterialFactory.h
MaterialRunlineEncoder.cpp
MaterialRunlineEncoder.h

materials/DrudeModel1.cpp
materials/DrudeModel1.h
materials/PerfectConductor.cpp
materials/PerfectConductor.h
materials/StaticDielectric.cpp
materials/StaticDielectric.h
materials/StaticLossyDielectric.cpp
materials/StaticLossyDielectric.h

MemoryUtilities.cpp
MemoryUtilities.h
ModularUpdateEquation-inl.h
ModularUpdateEquation.h
Output.cpp
Output.h
Paint.cpp
Paint.h
PartitionCellCount.cpp
PartitionCellCount.h
Performance.cpp
Performance.h
PhysicalConstants.cpp
PhysicalConstants.h
Runline.cpp
Runline.h
RunlineEncoder.cpp
RunlineEncoder.h
SetupConstants.h
SetupUpdateEquation.h
SimpleEHOutput.cpp
SimpleEHOutput.h
SimulationDescription.cpp
SimulationDescription.h
SimulationDescriptionPredeclarations.h
Source.cpp
Source.h
StreamedFieldInput.cpp
StreamedFieldInput.h
StructuralReports.cpp
StructuralReports.h
UpdateEquation.cpp
UpdateEquation.h

updateModules/BufferedCurrent-inl.h
updateModules/BufferedCurrent.cpp
updateModules/BufferedCurrent.h
updateModules/CFSRIPML-inl.h
updateModules/CFSRIPML.cpp
updateModules/CFSRIPML.h
updateModules/Material.cpp
updateModules/Material.h
updateModules/NullCurrent.h
updateModules/NullPML.h
updateModules/SetupModularUpdateEquation.h
updateModules/SetupModularUpdateEquation-inl.h

Version.h
VoxelGrid.cpp
VoxelGrid.h
VoxelizedPartition.cpp
VoxelizedPartition.h
XMLParameterFile.cpp
XMLParameterFile.h
YeeUtilities.cpp
YeeUtilities.h
)


#set_target_properties(trogdor PROPERTIES COMPILE_FLAGS -Wshorten-64-to-32)

# MaterialFactory.cpp is the file that includes, ultimately, all the templated
# update equations.  For this reason I like to dump the optimized gimple and
# set some warnings pertaining to inlining and such, just to make sure that gcc
# is doing what I want it to do!
set_source_files_properties(MaterialFactory.cpp PROPERTIES COMPILE_FLAGS
    "-fdump-tree-optimized -Winline -fdump-tree-vect")

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



