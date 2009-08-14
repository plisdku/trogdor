/*
 *  UpdateEquation.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "UpdateEquation.h"

UpdateEquation::
UpdateEquation()
{
}

UpdateEquation::
~UpdateEquation()
{
}

void UpdateEquation::
writeJ(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

void UpdateEquation::
writeP(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

void UpdateEquation::
writeK(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

void UpdateEquation::
writeM(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

void UpdateEquation::
allocateAuxBuffers()
{
}

SetupUpdateEquation::
SetupUpdateEquation()
{
}

SetupUpdateEquation::
~SetupUpdateEquation()
{
}
