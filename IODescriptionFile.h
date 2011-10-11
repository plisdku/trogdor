/*
 *  IODescriptionFile.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/27/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _IODESCRIPTIONFILE_
#define _IODESCRIPTIONFILE_

#include "SimulationDescription.h"
#include "Exception.h"
#include "HuygensSurface.h"
#include <vector>
#include <string>
#include <iostream>

class VoxelizedPartition;

class IODescriptionFile
{
private:
    IODescriptionFile() {}
public:
    /**
     *  Write the spec file for an output (E/H or J/K/P/M).  The spec file will
     *  be a simple plain-text format that Matlab can parse pretty easily.
     */
    static void write(std::string fileName, OutputDescPtr description,
        const VoxelizedPartition & vp,
        const std::vector<Region> & outputRegions,
        const std::vector<Duration> & outputDurations);
    
    /**
     *  Write the data request file for a current source.  The data request file
     *  will be an m-file with a function that returns a useful data structure.
     */
    static void write(std::string funcName, CurrentSourceDescPtr description,
        const VoxelizedPartition & vp,
        const std::vector< std::vector<Rect3i> > & rectsJ,
        const std::vector< std::vector<Rect3i> > & rectsK);
    
    /**
     *  Write the data request for a custom TFSF source.  The data request file
     *  will be an m-file with a function that returns a useful data structure.
     */
    static void write(std::string fileName,
        const HuygensSurface & huygensSurface,
        const VoxelizedPartition & vp, Vector3i materialSymmetries);
    
    /**
     *  Write the spec file for a GridReport.
     */
    static void write(std::string fileName, GridReportDescPtr description,
        const VoxelizedPartition & vp, std::vector<Rect3i> halfCells,
        std::vector<std::string> materialNames);
    
private:
    static void writeOutputHeader(std::ostream & file,
        const VoxelizedPartition & vp,
        std::string fileName);
    static void writeOutputRegions(std::ostream & file,
        const VoxelizedPartition & vp,
        const std::vector<Region> & regions);
    static void writeOutputDurations(std::ostream & file,
        const VoxelizedPartition & vp,
        const std::vector<Duration> & durations);
};



#endif
