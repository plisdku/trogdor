/*
 *  Material.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/13/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _MATERIAL_
#define _MATERIAL_



class Material
{
public:
    Material();
    virtual ~Material() = 0;
    
    void writeJ(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    void writeP(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    
    void writeK(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    void writeM(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
};







#endif
