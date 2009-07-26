/*
 *  StructuralReports.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/26/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "StructuralReports.h"

#include "SimulationDescription.h"
#include "VoxelizedPartition.h"
#include "YeeUtilities.h"

#include <sstream>

#include <Magick++.h>
using namespace Magick;
using namespace std;
using namespace YeeUtilities;

void StructuralReports::
saveOutputCrossSections(const GridDescription & grid,
    const VoxelizedPartition & vp)
{
    const vector<OutputDescPtr> & outs = grid.getOutputs();
    
    // For each output, select all regions which are 2D.  We'll output cross-
    // sections for these.
    for (int nn = 0; nn < outs.size(); nn++)
    for (int mm = 0; mm < outs[nn]->getRegions().size(); mm++)
    if (outs[nn]->getRegions()[mm].getYeeCells().numNonSingularDims() == 2)
    {
        const Region & region(outs[nn]->getRegions()[mm]);
        Rect3i r(region.getYeeCells());
        
        //LOG << "printing.\n";
        int nCols, nRows;
        Vector3i row, col, origin, rowStart;
        
        // In order to draw the image in right-handed coordinates,
        // so the image as drawn looks like a cross-section through
        // xyz space, I set the "origin" to the top-left corner of
        // the image and proceed in scanline order.
        
        if (r.p1[0] == r.p2[0])
        {
            nCols = r.p2[1] - r.p1[1] + 1;
            nRows = r.p2[2] - r.p1[2] + 1;
            
            origin = Vector3i(r.p1[0], r.p1[1], r.p2[2]);
            row = Vector3i(0,1,0);
            col = Vector3i(0,0,-1);
        }
        else if (r.p1[1] == r.p2[1])
        {
            nCols = r.p2[2] - r.p1[2] + 1;
            nRows = r.p2[0] - r.p1[0] + 1;
            
            origin = Vector3i(r.p2[0], r.p1[1], r.p1[2]);
            row = Vector3i(0,0,1);
            col = Vector3i(-1,0,0);
        }
        else
        {
            nCols = r.p2[0] - r.p1[0] + 1;
            nRows = r.p2[1] - r.p1[1] + 1;
            
            origin = Vector3i(r.p1[0], r.p2[1], r.p1[2]);
            row = Vector3i(1,0,0);
            col = Vector3i(0,-1,0);
        }
        
        Image image(Geometry(nCols, nRows), ColorRGB(1.0, 1.0, 1.0));
        
        int theOctant = 0;
        // The commented block below was in Trogdor 4.  I am not sure how I want
        // to pick the octant, really.  I think dumping a cross-section from
        // the half-cell grid would be best, but then I have to know which
        // half cell plane to take the cross-section in.  Hmm.
        /*
        string field = output->getParameters()["field"]; // if there...
        if (field == "ex")
            octant = Vector3i(1,0,0);
        else if (field == "ey")
            octant = Vector3i(0,1,0);
        else if (field == "ez")
            octant = Vector3i(0,0,1);
        else if (field == "hx")
            octant = Vector3i(0,1,1);
        else if (field == "hy")
            octant = Vector3i(1,0,1);
        else if (field == "hz")
            octant = Vector3i(1,1,0);
        */
        
        // I am not writing edge pixels at the edge of the zone because
        // I am a first-rate juggler and acrobat extraordinnaire.
        // No, I mean I am not writing them because I don't want to have to
        // define an "edge" cell at the "edge" of the output region.
        Vector3i v1, v2, v3, v4; // this is cheezy.
        const VoxelGrid & vox(vp.getVoxelGrid());
        for (int rr = 1; rr < nRows-2; rr++)
        {
            //  rowStart and here are in half cells
            Vector3i rowStartYee = origin + rr*col;
            //rowStart = 2*origin + 2*rr*col + octant;
            for (int cc = 1; cc < nCols-2; cc++)
            {
                Paint* mat;
                Vector3i halfCell(yeeToHalf(rowStartYee+cc*row, theOctant));
                v1 = halfCell + 2*col;
                v2 = halfCell - 2*col;
                v3 = halfCell + 2*row;
                v4 = halfCell - 2*row;
                
                mat = vox(halfCell[0], halfCell[1], halfCell[2]);
                
                if (mat != vox(v1[0], v1[1], v1[2]) ||
                    mat != vox(v2[0], v2[1], v2[2]) ||
                    mat != vox(v3[0], v3[1], v3[2]) ||
                    mat != vox(v4[0], v4[1], v4[2]) )
                {
                    image.pixelColor(cc, rr, "Black");
                }
            }
        }
        ostringstream fileName; 
        fileName << outs[nn]->getFile() << "_region_" << mm << ".bmp";
        image.write(fileName.str().c_str());
    }
}

void StructuralReports::
saveMaterialBoundariesBeta(const GridDescription & grid,
    const VoxelizedPartition & vp)
{
}




