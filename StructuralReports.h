/*
 *  StructuralReports.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/26/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _STRUCTURALREPORTS_
#define _STRUCTURALREPORTS_


class GridDescription;
class VoxelizedPartition;


class StructuralReports
{
public:
    static void saveOutputCrossSections(const GridDescription & grid,
        const VoxelizedPartition & vp);
    
    static void saveMaterialBoundariesBeta(const GridDescription & grid,
        const VoxelizedPartition & vp);

private:
};






#endif
