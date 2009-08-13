/*
 *  Material.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/13/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "Material.h"


void Material::
writeJ(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

void Material::
writeP(int direction, std::ostream & binaryStream, long startingIndex,
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
writeM(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

