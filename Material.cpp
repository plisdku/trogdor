/*
 *  Material.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "Material.h"

Material::
Material()
{
}

Material::
~Material()
{
}

void Material::
writeJ(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

void Material::
writeK(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

void Material::
allocateAuxBuffers()
{
}

SetupMaterial::
SetupMaterial()
{
}

SetupMaterial::
~SetupMaterial()
{
}
