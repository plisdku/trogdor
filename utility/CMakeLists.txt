add_library( utility
Exception.cpp
Exception.h
Log.cpp
Log.h
Map.h
ObjFile.cpp
ObjFile.h
Pointer.h
STLOutput.h
StreamFromString.h
StreamTee.cpp
StreamTee.h
TimeWrapper.cpp
TimeWrapper.h
VectorMatrix.h
VectorMatrix-inl.h
XMLExtras.h
geometry.h
geometry-inl.h
)

include_directories(
${TROGDOR_SOURCE_DIR}/src
)