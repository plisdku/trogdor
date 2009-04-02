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


include_directories( ${TROGDOR_SOURCE_DIR}/src/setup
	${TROGDOR_SOURCE_DIR}/src/utility
	${TROGDOR_SOURCE_DIR}/src/tinyxml
	${TROGDOR_SOURCE_DIR}/src
	${TROGDOR_SOURCE_DIR}/src/trogdorsetup
	${magIncDirs}
	/sw/include)

#include_directories( ${magIncDirs} )
#include_directories( ${magIncDirs} /sw/include )

link_directories( ${TROGDOR_BINARY_DIR}/setup
${TROGDOR_BINARY_DIR}/utility
${TROGDOR_BINARY_DIR}/tinyxml
${TROGDOR_BINARY_DIR}/trogdorsetup
${magLibDirs}
/sw/lib)

add_executable( trogdor 
ColocatedOutput.cpp
DrudeMetalModel.cpp
FDTDApplication.cpp
Fields.cpp
FileSource.cpp
FormulaSource.cpp
Input.cpp
InputFactory.cpp
MaterialFactory.cpp
MaterialModel.cpp
MaterialModelNew.cpp
MemoryUtilities.cpp
OneFieldInput.cpp
OneFieldOutput.cpp
Output.cpp
OutputFactory.cpp
PECModel.cpp
PhysicalConstants.cpp
Runline.cpp
SimulationGrid.cpp
Source.cpp
SourceFactory.cpp
StaticDielectricModel.cpp
StaticLossyDielectricModel.cpp
TFSFBufferSet.cpp
ThreeFieldInput.cpp
ThreeFieldOutput.cpp
main.cpp
VoxelizedGrid.cpp
YeeUtilities.cpp
)


target_link_libraries( trogdor
setup
tinyxml
utility
trogdorsetup
boost_program_options
boost_thread
${magDepLibs}
)

add_subdirectory( utility )
add_subdirectory( tinyxml )
add_subdirectory( setup )
add_subdirectory( trogdorsetup )

