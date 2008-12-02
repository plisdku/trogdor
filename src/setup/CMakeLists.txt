add_library (setup
GridWalker.cpp
IndexStencil.cpp
MaterialType.cpp
RunlineType.cpp
SetupGrid.cpp
SetupInput.cpp
SetupLink.cpp
SetupMaterialModel.cpp
SetupOutput.cpp
SetupSource.cpp
SetupTFSFSource.cpp
SetupTFSFBufferSet.cpp
StructureGrid.cpp
ValidateSetupAttributes.cpp
)

include_directories(
${TROGDOR_SOURCE_DIR}/src/utility
${TROGDOR_SOURCE_DIR}/src/tinyxml
${TROGDOR_SOURCE_DIR}/src
)
