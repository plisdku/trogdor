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
    const vector<vector<Rect3i> > & rectsJ,
    const vector<vector<Rect3i> > & rectsK)
{
    ofstream file(fileName.c_str());
    
    file << "clear A;\n";
    file << "A.trogdorMajorVersion = '" << TROGDOR_MAJOR_VERSION << "';\n";
    file << "A.trogdorBuildDate = '" << __DATE__ << "';\n";
    file << "A.dxdydzdt = ["
        << vp.gridDescription()->dxyz()[0] << " "
        << vp.gridDescription()->dxyz()[1] << " "
        << vp.gridDescription()->dxyz()[2] << " "
        << vp.gridDescription()->dt()
        << "];\n";
    file << "A.date = '"
        << to_iso_extended_string(second_clock::local_time()) << "';\n";
    file << "A.runlineDirection = " << vp.lattice().runlineDirection()
        << ";\n";
    if (description->hasMask())
        file << "A.maskFile = '" << description->spaceFile() << "';\n";
    if (description->isSpaceVarying())
        file << "A.dataFile = '" << description->spaceTimeFile() << "';\n";
    else
        file << "A.dataFile = '" << description->timeFile() << "';\n";
    
    // Tell it:
    //  needed Jx
    //  needed Jy
    //  needed Jz
    //  needed Kx
    //  needed Ky
    //  needed Kz
    
    // Then tell it the timesteps.
    
    const int d0 = vp.lattice().runlineDirection();
    const int d1 = (d0+1)%3;
    const int d2 = (d0+2)%3;
    Rect3i r;
    Vector3i x;
    Vector3f dxyz = vp.gridDescription()->dxyz();
    
    // Print yee indices of J
    for (int direction = 0; direction < 3; direction++)
    if (rectsJ.at(direction).size() != 0)
    {
        file << "A.yeeJ" << char('x'+direction) << " = [...\n";
        for (int nn = 0; nn < rectsJ[direction].size(); nn++)
        {
            r = rectsJ[direction][nn] - vp.gridDescription()->originYee();
            //LOG << "Requesting " << r << endl;
            for (x[d2] = r.p1[d2]; x[d2] <= r.p2[d2]; x[d2] ++)
            for (x[d1] = r.p1[d1]; x[d1] <= r.p2[d1]; x[d1] ++)
            for (x[d0] = r.p1[d0]; x[d0] <= r.p2[d0]; x[d0] ++)
                file << "\t[" << x[0] << ", " << x[1] << ", " << x[2]
                    << "]; ...\n";
        }
        file << "];\n";
    }
    
    // Print yee indices of K
    for (int direction = 0; direction < 3; direction++)
    if (rectsK.at(direction).size() != 0)
    {
        file << "A.yeeK" << char('x'+direction) << " = [...\n";
        for (int nn = 0; nn < rectsK[direction].size(); nn++)
        {
            r = rectsK[direction][nn] - vp.gridDescription()->originYee();
            for (x[d2] = r.p1[d2]; x[d2] <= r.p2[d2]; x[d2] ++)
            for (x[d1] = r.p1[d1]; x[d1] <= r.p2[d1]; x[d1] ++)
            for (x[d0] = r.p1[d0]; x[d0] <= r.p2[d0]; x[d0] ++)
                file << "\t[" << x[0] << ", " << x[1] << ", " << x[2]
                    << "]; ...\n";
        }
        file << "];\n";
    }
    
    // Print positions of J
    for (int direction = 0; direction < 3; direction++)
    if (rectsJ.at(direction).size() != 0)
    {
        file << "A.posJ" << char('x'+direction) << " = [...\n";
        for (int nn = 0; nn < rectsJ[direction].size(); nn++)
        {
            r = rectsJ[direction][nn] - vp.gridDescription()->originYee();
            for (x[d2] = r.p1[d2]; x[d2] <= r.p2[d2]; x[d2] ++)
            for (x[d1] = r.p1[d1]; x[d1] <= r.p2[d1]; x[d1] ++)
            for (x[d0] = r.p1[d0]; x[d0] <= r.p2[d0]; x[d0] ++)
            {
                Vector3f p = Vector3f(x) + eFieldPosition(direction);
                file << "\t[" << p[0]*dxyz[0] << ", " << p[1]*dxyz[1] << ", "
                    << p[2]*dxyz[2] << "]; ...\n";
            }
        }
        file << "];\n";
    }
    
    // Print positions of K
    for (int direction = 0; direction < 3; direction++)
    if (rectsK.at(direction).size() != 0)
    {
        file << "A.posK" << char('x'+direction) << " = [...\n";
        for (int nn = 0; nn < rectsK[direction].size(); nn++)
        {
            r = rectsK[direction][nn] - vp.gridDescription()->originYee();
            for (x[d2] = r.p1[d2]; x[d2] <= r.p2[d2]; x[d2] ++)
            for (x[d1] = r.p1[d1]; x[d1] <= r.p2[d1]; x[d1] ++)
            for (x[d0] = r.p1[d0]; x[d0] <= r.p2[d0]; x[d0] ++)
            {
                Vector3f p = Vector3f(x) + hFieldPosition(direction);
                file << "\t[" << p[0]*dxyz[0] << ", " << p[1]*dxyz[1] << ", "
                    << p[2]*dxyz[2] << "]; ...\n";
            }
        }
        file << "];\n";
    }
    
    // Print the durations
    file << "A.timesteps = [...\n";
    
    for (int nn = 0; nn < description->durations().size(); nn++)
    {
        int first = description->durations()[nn].first();
        int last = description->durations()[nn].last();
        
        if (last >= vp.simulationDescription()->numTimesteps())
            last = vp.simulationDescription()->numTimesteps()-1;
        
        file << first << ":" << last << ", ...\n";
    }
    file << "];\n";
    
    file << "% end auto-generated file\n";
    
    file.close();
}

void IODescriptionFile::
write(std::string fileName, const HuygensSurface & huygensSurface,
    const VoxelizedPartition & vp, Vector3i materialSymmetries)
{
    ofstream file(fileName.c_str());
    
    file << "% Auto-generated TFSF source request file.\n";
    file << "% Please provide fields in this order:\n";
    file << "% Hx Hy Hz Ex Ey Ez\n";
    file << "clear afp;\n";
    
    file << "afp.dxyz = [" << vp.simulationDescription()->dxyz()[0] << " "
        << vp.simulationDescription()->dxyz()[1] << " "
        << vp.simulationDescription()->dxyz()[2]
        << "];\n";
    file << "afp.dt = " << vp.simulationDescription()->dt() << ";\n";
    
    int numT = huygensSurface.description()->duration().last() -
        huygensSurface.description()->duration().first() + 1;
    file << "afp.numT = [" << numT << "];\n";
    
    file << "afp.halfCells = [";
    for (int mm = 0; mm < 3; mm++)
        file << huygensSurface.halfCells().p1[mm] << " ";
    for (int mm = 0; mm < 3; mm++)
        file << huygensSurface.halfCells().p2[mm] << " ";
    file << "];\n";
    file << "afp.tfRect = afp.halfCells;\n";
    file << "afp.inputFile = '" << huygensSurface.description()->file()
        << "';\n";
    
    // These lines could stand to feature better names.
    // huygensSurfaceSymmetries stores the directions in which the structure
    //  in the main grid is symmetrical around the TFSF boundary.
    // huygensSurface->symmetries() returns the directions along which the
    //  source waveform is symmetrical.
    //LOG << "Grid symmetries " << materialSymmetries << endl;
//    LOG << "Source symmetries " << huygensSurface.description()->symmetries()
//        << endl;
    
    /*
        There are only two jobs left, and they're simple to state:
        
        1.  Provide the material in each half-cell of the grid.  If the grid
            is symmetrical in some direction, the samples may be collapsed.
        2.  Provide Yee cell coordinates and locations in meters of all E and H
            fields, in the right order.  This will mean every single boundary
            half cell.
        
        I'll do these in order.
    */
    
    // Choose the rectangle of materials to print.
    // At most, the entire TF region will be presented.  However, if the grid
    // is symmetrical in some direction, then one edge will be enough.  (Pick
    // an edge that is not omitted.)
    
    Rect3i sampleHalfCells = huygensSurface.halfCells();
    for (int direction = 0; direction < 3; direction++)
    if (materialSymmetries[direction])
    {
        Vector3i face0 = cardinal(2*direction);
        Vector3i face1 = -face0;
        
        // Try to save materials on a non-omitted side of the TF box.  If
        // this is not possible, print a warning and pick arbitrarily.
        if (huygensSurface.hasBuffer(2*direction+1) &&
            !huygensSurface.hasBuffer(2*direction))
        {
            sampleHalfCells.p1[direction] = sampleHalfCells.p2[direction];
        }
        else if (huygensSurface.hasBuffer(2*direction) &&
            !huygensSurface.hasBuffer(2*direction+1))
        {
            sampleHalfCells.p2[direction] = sampleHalfCells.p1[direction];
        }
        else if (huygensSurface.hasBuffer(2*direction))
            sampleHalfCells.p2[direction] = sampleHalfCells.p1[direction];
        else
        {
            cerr << "Warning: it's not obvious which sides of the TFSF region"
                " need to have materials saved to file.  The choice here is"
                " arbitrary." << endl;
            sampleHalfCells.p2[direction] = sampleHalfCells.p1[direction];
        }
    }
    
    file << "afp.sampleHalfCells = [";
    for (int mm = 0; mm < 3; mm++)
        file << sampleHalfCells.p1[mm] << " ";
    for (int mm = 0; mm < 3; mm++)
        file << sampleHalfCells.p2[mm] << " ";
    file << "];\n";
    //LOG << "Sample half cells: " << sampleHalfCells << endl;
    
    // ----------------
    // COPIED at first from StructuralReports.  This is so silly!
    
    // Following VoxelGrid::operator<<, we'll establish a mapping from Paint*
    // to unsigned ints.
    Map<Paint*, unsigned int> code;
    vector<string> fullNames;
    fullNames.push_back("Empty");
    unsigned int curInt = 1;
    Map<Paint,PaintPtr>::const_iterator itr;
    for (itr = Paint::palette().begin(); itr != Paint::palette().end(); itr++)
    {
        code[itr->second] = curInt;
        fullNames.push_back(itr->first.fullName());
        file << "afp.materials{" << curInt << "} = '" << itr->first.fullName()
            << "';\n";
        curInt++;
    }
    code[0L] = 0;
    
    //string binaryName = fileName + "_binary";
    //ofstream binaryFile(binaryName.c_str(), ofstream::binary);
    file << "afp.grid = [...\n";
    Vector3i x;
    for (x[2] = sampleHalfCells.p1[2]; x[2] <= sampleHalfCells.p2[2]; x[2]++)
    for (x[1] = sampleHalfCells.p1[1]; x[1] <= sampleHalfCells.p2[1]; x[1]++)
    for (x[0] = sampleHalfCells.p1[0]; x[0] <= sampleHalfCells.p2[0]; x[0]++)
    {
        unsigned int codeVal = code[vp.voxels()(x)];
        file << "\t" << codeVal << " ...\n";
        //binaryFile.write((char*)(&codeVal),
        //    (std::streamsize)(sizeof(codeVal)));
    }
    file << "];\n";
    file << "afp.grid = reshape(afp.grid, afp.sampleHalfCells(4:6) - "
        "afp.sampleHalfCells(1:3) + 1);\n";
    
    // END COPIED STUFF
    // ----------------
    
    // Write the field positions.  The order will be
    //  Ex Ey Ez Hx Hy Hz
    
    // Make a list of offsets of Ex, Ey, Ez, Hx, Hy and Hz
    vector<Vector3i> fieldOffsets;
    fieldOffsets.push_back(eFieldOffset(0));
    fieldOffsets.push_back(eFieldOffset(1));
    fieldOffsets.push_back(eFieldOffset(2));
    fieldOffsets.push_back(hFieldOffset(0));
    fieldOffsets.push_back(hFieldOffset(1));
    fieldOffsets.push_back(hFieldOffset(2));
    vector<string> fieldNames;
    fieldNames.push_back("Ex");
    fieldNames.push_back("Ey");
    fieldNames.push_back("Ez");
    fieldNames.push_back("Hx");
    fieldNames.push_back("Hy");
    fieldNames.push_back("Hz");
    
    for (int field = 0; field < 6; field++)
    {
        file << "afp.yee" + fieldNames[field] << " = [ ...\n";
        
        for (int faceNum = 0; faceNum < 6; faceNum++)
        if (huygensSurface.hasBuffer(faceNum))
        {
            Rect3i yee;
            yee = halfToYee(huygensSurface.buffer(faceNum)->destHalfCells(),
                octant(fieldOffsets[field]));
            for (int kk = yee.p1[2]; kk <= yee.p2[2]; kk++)
            for (int jj = yee.p1[1]; jj <= yee.p2[1]; jj++)
            for (int ii = yee.p1[0]; ii <= yee.p2[0]; ii++)
                file << "\t[" << ii << ", " << jj << " , " << kk << "];...\n";
        }
        file << "];\n";
    }
    
    Vector3f dxyz = vp.simulationDescription()->dxyz();
    for (int field = 0; field < 6; field++)
    {
        file << "afp.pos" + fieldNames[field] << " = [ ...\n";
        
        for (int faceNum = 0; faceNum < 6; faceNum++)
        if (huygensSurface.hasBuffer(faceNum))
        {
            Rect3i yee;
            yee = halfToYee(huygensSurface.buffer(faceNum)->destHalfCells(),
                octant(fieldOffsets[field]));
            Vector3f theOffset(0.5*fieldOffsets[field][0],
                0.5*fieldOffsets[field][1],
                0.5*fieldOffsets[field][2]);
            for (int kk = yee.p1[2]; kk <= yee.p2[2]; kk++)
            for (int jj = yee.p1[1]; jj <= yee.p2[1]; jj++)
            for (int ii = yee.p1[0]; ii <= yee.p2[0]; ii++)
            {
                file << "\t["
                    << dxyz[0]*(theOffset[0]+ii) << " , "
                    << dxyz[1]*(theOffset[1]+jj) << " , "
                    << dxyz[2]*(theOffset[2]+kk) << "]; ...\n";
            }
        }
        file << "];\n";
    }
    file << "% End auto-generated file.\n";
    
    file.close();
}

void IODescriptionFile::
write(string fileName, const GridReportDescPtr description,
    const VoxelizedPartition & vp, vector<Rect3i> halfCells,
    vector<string> materialNames)
{
    ofstream file(fileName.c_str());
    writeOutputHeader(file, vp, fileName);
    
    file << "\n";
    
    for (int nn = 0; nn < materialNames.size(); nn++)
        file << "material " << nn << " " << materialNames[nn] << "\n";
    
    file << "\n";
    
    for (int nn = 0; nn < halfCells.size(); nn++)
        file << "halfCells " << halfCells[nn] << "\n";
    
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

