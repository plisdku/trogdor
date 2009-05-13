project( TROGDOR )

INCLUDE(CPack)

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
	${TROGDOR_SOURCE_DIR}/utility
	${TROGDOR_SOURCE_DIR}/tinyxml
	${magIncDirs}
	/usr/local/include/
	/usr/local/include/boost-1_38
	/sw/include)

#include_directories( ${magIncDirs} )
#include_directories( ${magIncDirs} /sw/include )

link_directories( ${TROGDOR_BINARY_DIR}/setup
${TROGDOR_BINARY_DIR}/utility
${TROGDOR_BINARY_DIR}/tinyxml
${TROGDOR_BINARY_DIR}/trogdorsetup
${magLibDirs}
/usr/local/lib
/sw/lib)

add_executable( trogdor
CalculationPartition.cpp
ConvertOldXML.cpp
DrudeModel1.cpp
FDTDApplication.cpp
FormulaSource.cpp
MaterialBoss.cpp
MemoryUtilities.cpp
OutputBoss.cpp
Paint.cpp 
PartitionCellCount.cpp
PhysicalConstants.cpp
SimpleEHOutput.cpp
SimulationDescription.cpp
SourceBoss.cpp
StaticDielectric.cpp
StaticDielectricPML.cpp
main.cpp
VoxelizedPartition.cpp
VoxelGrid.cpp
XMLParameterFile.cpp
YeeUtilities.cpp
)


target_link_libraries( trogdor
tinyxml
utility
boost_program_options-xgcc40-mt
boost_thread-xgcc40-mt
${magDepLibs}
)

add_subdirectory( utility )
add_subdirectory( tinyxml )

