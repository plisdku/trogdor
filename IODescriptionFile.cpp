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
write(std::string fileName, HuygensSurfaceDescPtr huygensSurface,
    const VoxelizedPartition & vp, Vector3i huygensSurfaceSymmetries)
{
    cerr << "Warning: NOT writing request file for HuygensSurface.\n";
    ofstream file(fileName.c_str());
    
    file << "clear afp;\n";
    
    file << "afp.dxdydzdt = [" << vp.simulationDescription()->dxyz()[0] << " "
        << vp.simulationDescription()->dxyz()[1] << " "
        << vp.simulationDescription()->dxyz()[2]
        << "];\n";
    
    int numT = huygensSurface->duration().last() -
        huygensSurface->duration().first() + 1;
    file << "afp.numT = [" << numT << "];\n";
    
    file << "afp.halfCells = " << huygensSurface->halfCells() << ";\n";
    file << "afp.tfRect = afp.halfCells;\n";
    file << "afp.inputFile = '" << huygensSurface->file() << "';\n";
    
    // These lines could stand to feature better names.
    // huygensSurfaceSymmetries stores the directions in which the structure
    //  in the main grid is symmetrical around the TFSF boundary.
    // huygensSurface->symmetries() returns the directions along which the
    //  source waveform is symmetrical.
    LOG << "Grid symmetries " << huygensSurfaceSymmetries << endl;
    LOG << "Source symmetries " << huygensSurface->symmetries() << endl;
    
    
    
    /*
    // Now write the materials and their parameters.
    int ii, jj, kk;
    int numMaterials = 0;
    Rect3i sampleRect = ss->tiFRect();
    const vmlib::SMat<3,bool> & gridSymmetries =
        ss->cachedGridSymmetries();
    Vector3b sourceSymmetries = ss->symmetries();
    Vector3b periodicDimensions = periodicDimensions(sampleRect);
    Vector3b combinedSymmetries(0,0,0);
    
    LOGF << "Grid symmetries are " << gridSymmetries << endl;
    LOGF << "Source symmetries are " << sourceSymmetries << endl;
    LOGF << "Periodic dimensions are " << periodicDimensions << endl;
    
    for (int nSym = 0; nSym < 3; nSym++)
    {
        // There are two conditions under which a dimension can be dropped.
        //	 1.  If the sourceRect has full X symmetry (which means that
        //		gridSymmetries(nSym,:) == 1) then the X direction collapses.
        //	 2.  If the sourceRect has X symmetry on some faces, but
        //      the remaining faces don't need TFSF correction, then that
        //		is good enough.  That condition is usually the same as
        //		having a periodic dimension.  (Warning: this is a kludge.)
        
        Vector3b symmetryOrPeriodicity(
            periodicDimensions[0] || gridSymmetries(nSym,0),
            periodicDimensions[1] || gridSymmetries(nSym,1),
            periodicDimensions[2] || gridSymmetries(nSym,2) );
        
        if (minval(symmetryOrPeriodicity) == 1) // if vector == (1 1 1)
            combinedSymmetries[nSym] = 1;
    }
    
    LOGF << "Combined symmetries are " << combinedSymmetries << endl;
    
    for (int xyz = 0; xyz < 3; xyz++)
    if (combinedSymmetries[nn])
        sampleRect.p2[nn] = sampleRect.p1[nn];
    
    // Determine all the materials in this chunk of grid.
    Map<int, int> tagToParent;
    Map<int, int> parentToIndex;
    vector<long> materials;
    for (kk = sampleRect.p1[2]; kk <= sampleRect.p2[2]; kk++)
    for (jj = sampleRect.p1[1]; jj <= sampleRect.p2[1]; jj++)
    for (ii = sampleRect.p1[0]; ii <= sampleRect.p2[0]; ii++)
    {
        Paint* mat = mVoxels(ii,jj,kk);
        int parentTag;
        //cout << mat << "\n";
        if (tagToParent.count(mat) == 0)
        {
            const MaterialType & matType =
                mStructureGrid->materialType(mat);
            if (matType.isTFSF())
                parentTag = mStructureGrid->materialIndex(
                    matType.name());
            else
                parentTag = mat;
            
            tagToParent[mat] = parentTag;
        }
        
        parentTag = tagToParent[mat];
        if (parentToIndex.count(parentTag) == 0)
        {
            parentToIndex[parentTag] = numMaterials;
            materials.push_back(parentTag);
            numMaterials++;
        }
    }
    // Write the material descriptions
    for (int mm = 0; mm < numMaterials; mm++)
    {
        const MaterialType & matType = mStructureGrid->materialType(
            materials[mm]);
        const SetupUpdateEquationPtr setupMat = mMaterials[matType.name()];
        file << "afp.materials{" << mm+1 << "}.class = '" << 
            setupMat->getClass() << "';\n";
        file << "afp.materials{" << mm+1 << "}.name = '" <<
            setupMat->name() << "';\n";
        
        const Map<string, string> & params = setupMat->parameters();
        Map<string, string>::const_iterator itr;
        for (itr = params.begin(); itr != params.end(); itr++)
        {
            file << "afp.materials{" << mm+1 << "}." << itr->first
                << " = " << itr->second << ";\n";
        }
    }
    // Write the grid
    file << "afp.sampleBounds = " << sampleRect << ";\n";
    file << "afp.grid = [";
    for (kk = sampleRect.p1[2]; kk <= sampleRect.p2[2]; kk++)
    {
        file << "...\n";
        for (jj = sampleRect.p1[1]; jj <= sampleRect.p2[1]; jj++)
        {
            file << "\t";
            for (ii = sampleRect.p1[0]; ii <= sampleRect.p2[0]; ii++)
            {
                Paint* mat = mVoxels(ii,jj,kk);
                //int matInd = parentToIndex[tagToParent[mat]];
                //file << matInd+1 << " ";
            }
            file << "...\n";
        }
    }
    file << "];\n";
    
    // Write the buffer points required, first in half cells, then in meters
    SetupTFSFBufferSet temporaryBuffer(*ss, mActiveRegion);
    
    // Make a list of parities of Ex, Ey, Ez, Hx, Hy and Hz
    vector<Vector3i> fieldParities;
    fieldParities.push_back(Vector3i(1,0,0));
    fieldParities.push_back(Vector3i(0,1,0));
    fieldParities.push_back(Vector3i(0,0,1));
    fieldParities.push_back(Vector3i(0,1,1));
    fieldParities.push_back(Vector3i(1,0,1));
    fieldParities.push_back(Vector3i(1,1,0));
    vector<string> fieldNames;
    fieldNames.push_back("Ex");
    fieldNames.push_back("Ey");
    fieldNames.push_back("Ez");
    fieldNames.push_back("Hx");
    fieldNames.push_back("Hy");
    fieldNames.push_back("Hz");
    
    int ff, gg;
    for (ff = 0; ff < 6; ff++)
    {
        file << "afp.yee"+fieldNames[ff] << " = [ ...\n";
        
        for (gg = 0; gg < 6; gg++)  // iterate over sides (-x, +x, etc)
        if (!temporaryBuffer.omits(gg))
        {
            Rect3i yeeBounds = temporaryBuffer.yeeBufferRect(gg,
                fieldParities[ff]);
            for (kk = yeeBounds.p1[2]; kk <= yeeBounds.p2[2]; kk++)
            for (jj = yeeBounds.p1[1]; jj <= yeeBounds.p2[1]; jj++)
            for (ii = yeeBounds.p1[0]; ii <= yeeBounds.p2[0]; ii++)
                file << "\t[" << ii << ", " << jj << ", " << kk << "]; ...\n";
        }
        file << "];\n";
    }
    
    for (ff = 0; ff < 6; ff++)
    {
        file << "afp.pos"+fieldNames[ff] << " = [ ...\n";
        
        for (gg = 0; gg < 6; gg++)  // iterate over sides (-x, +x, etc)
        if (!temporaryBuffer.omits(gg))
        {
            Rect3i yeeBounds = temporaryBuffer.yeeBufferRect(gg,
                fieldParities[ff]);
            for (kk = yeeBounds.p1[2]; kk <= yeeBounds.p2[2]; kk++)
            for (jj = yeeBounds.p1[1]; jj <= yeeBounds.p2[1]; jj++)
            for (ii = yeeBounds.p1[0]; ii <= yeeBounds.p2[0]; ii++)
            {
                file << "\t[" << dx*(ii+0.5*fieldParities[ff][0]) << ", "
                    << dy*(jj+0.5*fieldParities[ff][1]) << ", "
                    << dz*(kk+0.5*fieldParities[ff][2]) << "]; ...\n";
            }
        }
        file << "];\n";
    }
    file << "% end auto-generated file \n";
    */
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