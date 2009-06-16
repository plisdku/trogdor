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
	${TROGDOR_SOURCE_DIR}
	${TROGDOR_SOURCE_DIR}/utility
	${TROGDOR_SOURCE_DIR}/tinyxml
	${magIncDirs}
	/usr/local/include/
	/usr/local/include/boost-1_38
	/sw/include)

#include_directories( ${magIncDirs} )
#include_directories( ${magIncDirs} /sw/include )

link_directories(
${TROGDOR_BINARY_DIR}/utility
${TROGDOR_BINARY_DIR}/tinyxml
${magLibDirs}
/usr/local/lib
/sw/lib)

add_executable( trogdor
CalculationPartition.h
CalculationPartition.cpp
ConvertOldXML.h
ConvertOldXML.cpp
DrudeModel1.h
DrudeModel1.cpp
FDTDApplication.h
FDTDApplication.cpp
FormulaSource.h
FormulaSource.cpp
HuygensSurface.h
HuygensSurface.cpp
main.cpp
MaterialBoss.h
MaterialBoss.cpp
MemoryUtilities.h
MemoryUtilities.cpp
OutputBoss.h
OutputBoss.cpp
Paint.h
Paint.cpp
PartitionCellCount.h
PartitionCellCount.cpp
PerfectConductor.h
PerfectConductor.cpp
PhysicalConstants.h
PhysicalConstants.cpp
SetupConstants.h
SimpleEHOutput.h
SimpleEHOutput.cpp
SimulationDescription.h
SimulationDescription.cpp
SimulationDescriptionPredeclarations.h
SourceBoss.h
SourceBoss.cpp
StaticDielectric.h
StaticDielectric.cpp
StaticDielectricPML.h
StaticDielectricPML.hpp
StaticLossyDielectric.h
StaticLossyDielectric.cpp
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


target_link_libraries( trogdor
tinyxml
utility
boost_date_time-xgcc40-mt
boost_program_options-xgcc40-mt
boost_thread-xgcc40-mt
${magDepLibs}
)

add_subdirectory( utility )
add_subdirectory( tinyxml )

