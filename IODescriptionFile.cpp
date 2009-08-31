/*
 *  IODescriptionFile.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/27/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "IODescriptionFile.h"
#include "VoxelizedPartition.h"
#include "YeeUtilities.h"
#include "Version.h"
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cstdio>

using namespace std;
using namespace boost::posix_time;
using namespace YeeUtilities;


void IODescriptionFile::
write(std::string fileName, OutputDescPtr description,
    const VoxelizedPartition & vp,
    const vector<Region> & outputRegions,
    const vector<Duration> & outputDurations)
{
    ofstream file(fileName.c_str());
    writeOutputHeader(file, vp, fileName);
    
    file << "datafile " << description->file() << "\n";
    //file << "materialfile " << description->file() << ".mat\n";
    
    int nn;
    if (!description->isInterpolated())
    {
        // E fields
        for (nn = 0; nn < 3; nn++)
        if (description->whichE()[nn])
        {
            file << "field e" << char('x' + nn) << " "
                << eFieldPosition(nn) << " 0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (description->whichH()[nn])
        {
            file << "field h" << char('x' + nn) << " "
                << hFieldPosition(nn) << " 0.5\n";
        }
    }
    else
    {
        // E fields
        for (nn = 0; nn < 3; nn++)
        if (description->whichE()[nn])
        {
            file << "field e" << char('x' + nn) << " "
                << description->interpolationPoint() << " 0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (description->whichH()[nn])
        {
            file << "field h" << char('x' + nn) << " "
                << description->interpolationPoint() << " 0.5 \n";
        }
        
        // J fields
        for (nn = 0; nn < 3; nn++)
        if (description->whichJ()[nn])
        {
            file << "field j" << char('x' + nn) << " "
                << eFieldPosition(nn) << " 0.0 \n";
        }
        
        // K fields
        for (nn = 0; nn < 3; nn++)
        if (description->whichK()[nn])
        {
            file << "field k" << char('x' + nn) << " "
                << hFieldPosition(nn) << " 0.5\n";
        }
    }
    
    if (norm2(description->whichE()) > 0 || norm2(description->whichH()) > 0)
    {
        assert(norm2(description->whichJ()) == 0);
        assert(norm2(description->whichK()) == 0);
        file << "unitVector0 " << Vector3i(1,0,0) << "\n"
            << "unitVector1 " << Vector3i(0,1,0) << "\n"
            << "unitVector2 " << Vector3i(0,0,1) << "\n";
    }
    else if (norm2(description->whichJ())>0 || norm2(description->whichK())>0)
    {
        Vector3i unitVector0 = cardinal(2*vp.lattice().runlineDirection()+1);
        file << "unitVector0 " << unitVector0 << "\n"
            << "unitVector1 " << cyclicPermute(unitVector0, 1) << "\n"
            << "unitVector2 " << cyclicPermute(unitVector0, 2) << "\n";
    }
    
    writeOutputRegions(file, vp, outputRegions);
    writeOutputDurations(file, vp, outputDurations);
    
    file.close();
}

void IODescriptionFile::
write(std::string fileName, CurrentSourceDescPtr description,
    const VoxelizedPartition & vp,
    const vector<vector<Region> > & regionsJ,
    const vector<vector<Region> > & regionsK,
    const vector<Duration> & outputDurations)
{
    ofstream file(fileName.c_str());
    file.close();
}

void IODescriptionFile::
write(std::string fileName, HuygensSurfaceDescPtr description,
    const VoxelizedPartition & vp)
{
    ofstream file(fileName.c_str());
    file.close();
}

/*
IODescriptionFile::
IODescriptionFile(const VoxelizedPartition & vp) :
    mRunlineDirection(vp.lattice().runlineDirection()),
    mOriginYee(vp.originYee()),
    m_dxyz(vp.gridDescription()->dxyz()),
    m_dt(vp.gridDescription()->dt())
{
}
*/

void IODescriptionFile::
writeOutputHeader(ostream & file,
    const VoxelizedPartition & vp,
    string fileName)
{
    ptime now(second_clock::local_time());
    // Header: all IO description files will have this.
    file << "trogdor5data\n";
    file << "trogdorMajorVersion " << TROGDOR_MAJOR_VERSION << "\n";
    file << "trogdorSVNVersion NOTUSED\n";
    file << "trogdorBuildDate " << __DATE__ << "\n";
    file << "date " << to_iso_extended_string(now) << "\n";
    file << "dxyz " << vp.gridDescription()->dxyz() << "\n";
    file << "dt " << vp.gridDescription()->dt() << "\n";
    file << "runlineDirection " << vp.lattice().runlineDirection() << "\n";
    file << "specfile " << fileName << "\n";
}


void IODescriptionFile::
writeOutputRegions(ostream & file, 
    const VoxelizedPartition & vp,
    const vector<Region> & regions)
{
    for (int nn = 0; nn < regions.size(); nn++)
    {
        file << "region "
            << regions[nn].yeeCells() - vp.gridDescription()->originYee()
            << " stride "
            << regions[nn].stride()
            << "\n";
    }
}

void IODescriptionFile::
writeOutputDurations(ostream & file,
    const VoxelizedPartition & vp,
    const vector<Duration> & durations)
{
    for (int nn = 0; nn < durations.size(); nn++)
    {
        file << "duration from "
            << durations[nn].first() << " to "
            << durations[nn].last() << " period "
            << durations[nn].period() << "\n";
    }
}

/*
void IODescriptionFile::
read(string fileName)
{
    ifstream file;
    file.open(fileName.c_str());
    if (!file.good())
        throw(Exception("Cannot open file."));
    
    mDurations.clear();
    mRegions.clear();
    mFieldNames.clear();
    
    // What we read:
    // -- runline direction
    // -- origin
    // -- dxyz, dt
    // -- regions
    // -- durations
    // -- fields (e, h, j, k)
    // -- polarization
    // -- mask file name
    // The applicable fields are Exyz and also just E, for instance, since
    // there can be a polarization vector.
    
    string nowhere;
    string s;
    int n;
    while (getline(file, s))
    {
        istringstream str(s);
        string name;
        str >> name;
        
        if (name == "dxyz")
        {
            n = sscanf(s.c_str(),
                "dxyz [%f %f %f]", &m_dxyz[0], &m_dxyz[1], &m_dxyz[2]);
            assert(n == 3);
        }
        else if (name == "dt")
        {
            str >> m_dt;
        }
        else if (name == "field")
        {
            string fieldName;
            str >> fieldName;
            mFieldNames.push_back(fieldName);
        }
        else if (name == "unitVector0")
        {
            Vector3i vec;
            n = sscanf(s.c_str(),
                "unitVector0 [%d %d %d]", &vec[0], &vec[1], &vec[2]);
            assert(n == 3);
        }
        else if (name == "unitVector1")
        {
            Vector3i vec;
            n = sscanf(s.c_str(),
                "unitVector1 [%d %d %d]", &vec[0], &vec[1], &vec[2]);
            assert(n == 3);
        }
        else if (name == "unitVector2")
        {
            Vector3i vec;
            n = sscanf(s.c_str(),
                "unitVector2 [%d %d %d]", &vec[0], &vec[1], &vec[2]);
            assert(n == 3);
        }
        else if (name == "region")
        {
            Rect3i rect;
            Vector3i stride;
            
            n = sscanf(s.c_str(),
                "region [[%d %d %d] [%d %d %d]] stride [%d %d %d]",
                &rect.p1[0], &rect.p1[1], &rect.p1[2],
                &rect.p2[0], &rect.p2[1], &rect.p2[2],
                &stride[0], &stride[1], &stride[2]);
            assert(n == 9);
            mRegions.push_back(Region(rect, stride));
        }
        else if (name == "duration")
        {
            int first, last, period;
            n = sscanf(s.c_str(),
                "duration from %d to %d period %d",
                &first, &last, &period);
            assert(n == 3);
            mDurations.push_back(Duration(first, last, period)); 
        }
        else
        {
            // do nothing, we don't care...
        }
    }
    
    LOG << "Now I need to do something with this information.\n";
    file.close();
}
*/