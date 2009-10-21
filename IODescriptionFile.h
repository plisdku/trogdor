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
    static void write(std::string fileName, CurrentSourceDescPtr description,
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
    
    /*
    IODescriptionFile(const VoxelizedPartition & vp);
    
    void setRegions(const std::vector<Region> & regions);
    void setDurations(const std::vector<Duration> & durations);
    
    const std::vector<Region> & regions() const { return mRegions; }
    const std::vector<Duration> & durations() const { return mDurations; }
    
    void write(std::string fileName, OutputDescPtr description) const;
    
    void write(std::string fileName, CurrentSourceDescPtr description) const;
    
    void read(std::string descriptionFileName);
    */
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
    /*
    int mRunlineDirection;
    Vector3i mOriginYee;
    Vector3f m_dxyz;
    float m_dt;
    
    OutputDescPtr mOutputDescription;
    CurrentSourceDescPtr mCurrentSourceDescription;
    SourceDescPtr mSourceDescription;
    
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
    
    std::vector<std::string> mFieldNames; // e.g. "ex" or "e" or "h" etc.
    */
};



#endif
