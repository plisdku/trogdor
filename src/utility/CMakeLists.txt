add_library( utility 
Log.cpp
ObjFile.cpp
StreamTee.cpp
TimeWrapper.cpp
geometry.hh
calc.hh
)

include_directories(
${TROGDOR_SOURCE_DIR}/src
)