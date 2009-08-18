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
#include "UpdateEquation.h"
#include "ObjFile.h"

#include <sstream>
#include <vector>
#include <list>

#include <Magick++.h>
using namespace Magick;
using namespace std;
using namespace YeeUtilities;


static void
consolidateRects(const vector<OrientedRect3i> & smallRects,
	list<OrientedRect3i> & bigRects, int normalIndex);

void StructuralReports::
saveOutputCrossSections(const GridDescription & grid,
    const VoxelizedPartition & vp)
{
    const vector<OutputDescPtr> & outs = grid.outputs();
    
    // For each output, select all regions which are 2D.  We'll output cross-
    // sections for these.
    for (int nn = 0; nn < outs.size(); nn++)
    for (int mm = 0; mm < outs[nn]->regions().size(); mm++)
    if (outs[nn]->regions()[mm].yeeCells().numNonSingularDims() == 2)
    {
        const Region & region(outs[nn]->regions()[mm]);
        Rect3i r(region.yeeCells());
        
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
        const VoxelGrid & vox(vp.voxelGrid());
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
        fileName << outs[nn]->file() << "_region_" << mm << ".bmp";
        image.write(fileName.str().c_str());
    }
}

void StructuralReports::
saveMaterialBoundariesBeta(const GridDescription & grid,
    const VoxelizedPartition & vp)
{
	ObjFile objFile;
	int ii,jj,kk;
    
	ostringstream foutname;
	foutname << grid.name() << "_faces.obj";
	ofstream fout(foutname.str().c_str());
	
    const Map<Paint*, RunlineEncoderPtr> materials(vp.setupMaterials());
    map<Paint*, RunlineEncoderPtr>::const_iterator itr;
    
    const VoxelGrid & vg(vp.voxelGrid());
	//const StructureGrid& grid = *mStructureGrid;
	
	// We'll write the materials one at a time.
	for (itr = materials.begin(); itr != materials.end(); itr++)
	{
		Paint* paint = (*itr).first->withoutModifications();
		RunlineEncoderPtr mat = (*itr).second;
        string name = paint->getFullName();
        Rect3i roi = grid.nonPMLHalfCells();
		Paint* lastMat;
        Paint* curMat;
		
		LOGF << "Considering material " << name << endl;
		
		//fout << name << endl;
		
		vector<OrientedRect3i> xFaces;
		vector<OrientedRect3i> yFaces;
		vector<OrientedRect3i> zFaces;
		
		// Do all the face determination stuff
		{
		// X-normal faces
		//	a.  Top layer
		ii = roi.p1[0];
		for (kk = roi.p1[2]; kk <= roi.p2[2]; kk++)
		for (jj = roi.p1[1]; jj <= roi.p2[1]; jj++)
		{
			curMat = vg(ii, jj, kk)->withoutModifications();
			if (curMat == paint)
			{
				xFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii,jj+1,kk+1),
					Vector3i(-1, 0, 0)) );
			}
		} 
		//	b.  Bottom layer
		ii = roi.p2[0];
		for (kk = roi.p1[2]; kk <= roi.p2[2]; kk++)
		for (jj = roi.p1[1]; jj <= roi.p2[1]; jj++)
		{
			curMat = vg(ii, jj, kk)->withoutModifications();
			if (curMat == paint)
			{
				xFaces.push_back( OrientedRect3i(
					Rect3i(ii+1,jj,kk,ii+1,jj+1,kk+1),
					Vector3i(1,0,0) ));
			}
		}
		//	c.  Middle (careful about X direction for-loop limits)
		for (ii = roi.p1[0]+1; ii <= roi.p2[0]; ii++)
		for (kk = roi.p1[2]; kk <= roi.p2[2]; kk++)
		for (jj = roi.p1[1]; jj <= roi.p2[1]; jj++)
		{
			lastMat = vg(ii-1, jj, kk)->withoutModifications();
			curMat = vg(ii, jj, kk)->withoutModifications();
			
			if (lastMat == paint && curMat != paint)
			{
				xFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii,jj+1,kk+1),
					Vector3i(1, 0, 0)) );
			}
			else if (lastMat != paint && curMat == paint)
			{
				xFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii,jj+1,kk+1),
					Vector3i(-1,0,0) ));
			}
		}
		
		// Y-normal faces
		//	a.  Top layer
		jj = roi.p1[1];
		for (ii = roi.p1[0]; ii <= roi.p2[0]; ii++)
		for (kk = roi.p1[2]; kk <= roi.p2[2]; kk++)
		{
			curMat = vg(ii, jj, kk)->withoutModifications();
			if (curMat == paint)
			{
				yFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj,kk+1),
					Vector3i(0, -1, 0) ));
			}
		} 
		//	b.  Bottom layer
		jj = roi.p2[1];
		for (ii = roi.p1[0]; ii <= roi.p2[0]; ii++)
		for (kk = roi.p1[2]; kk <= roi.p2[2]; kk++)
		{
			curMat = vg(ii, jj, kk)->withoutModifications();
			if (curMat == paint)
			{
				yFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj+1,kk,ii+1,jj+1,kk+1),
					Vector3i(0, 1, 0) ));
			}
		}
		//	c.  Middle (careful about X direction for-loop limits)
		for (jj = roi.p1[1]+1; jj <= roi.p2[1]; jj++)
		for (ii = roi.p1[0]; ii <= roi.p2[0]; ii++)
		for (kk = roi.p1[2]; kk <= roi.p2[2]; kk++)
		{
			lastMat = vg(ii, jj-1, kk)->withoutModifications();
			curMat = vg(ii, jj, kk)->withoutModifications();
			
			if (lastMat == paint && curMat != paint)
			{
				yFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj,kk+1),
					Vector3i(0, 1, 0) ));
			}
			else if (lastMat != paint && curMat == paint)
			{
				yFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj,kk+1),
					Vector3i(0, -1, 0) ));
			}
		}
		
		// Z-normal faces
		//	a.  Top layer
		kk = roi.p1[2];
		for (jj = roi.p1[1]; jj <= roi.p2[1]; jj++)
		for (ii = roi.p1[0]; ii <= roi.p2[0]; ii++)
		{
			curMat = vg(ii, jj, kk)->withoutModifications();
			if (curMat == paint)
			{
				zFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj+1,kk),
					Vector3i(0, 0, -1) ));
			}
		} 
		//	b.  Bottom layer
		kk = roi.p2[2];
		for (jj = roi.p1[1]; jj <= roi.p2[1]; jj++)
		for (ii = roi.p1[0]; ii <= roi.p2[0]; ii++)
		{
			curMat = vg(ii, jj, kk)->withoutModifications();
			if (curMat == paint)
			{
				zFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk+1,ii+1,jj+1,kk+1),
					Vector3i(0, 0, 1) ));
			}
		}
		//	c.  Middle (careful about X direction for-loop limits)
		for (kk = roi.p1[2]+1; kk <= roi.p2[2]; kk++)
		for (jj = roi.p1[1]; jj <= roi.p2[1]; jj++)
		for (ii = roi.p1[0]; ii <= roi.p2[0]; ii++)
		{
			lastMat = vg(ii, jj, kk-1)->withoutModifications();
			curMat = vg(ii, jj, kk)->withoutModifications();
			
			if (lastMat == paint && curMat != paint)
			{
				zFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj+1,kk),
					Vector3i(0, 0, 1) ));
			}
			else if (lastMat != paint && curMat == paint)
			{
				zFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj+1,kk),
					Vector3i(0, 0, -1) ));
			}
		}
		}
        
		// Now try to consolidate a little bit.		
		list<OrientedRect3i> xCompressed;
		list<OrientedRect3i> yCompressed;
		list<OrientedRect3i> zCompressed;
		
		consolidateRects(xFaces, xCompressed, 0);
		consolidateRects(yFaces, yCompressed, 1);
		consolidateRects(zFaces, zCompressed, 2);
		
		/*
		ostream_iterator<OrientedRect3i> fout_itr(fout, "\n");
		copy(xCompressed.begin(), xCompressed.end(), fout_itr);
		copy(yCompressed.begin(), yCompressed.end(), fout_itr);
		copy(zCompressed.begin(), zCompressed.end(), fout_itr);
		*/
        
		objFile.appendGroup(name, xCompressed);
		objFile.appendGroup(name, yCompressed);
		objFile.appendGroup(name, zCompressed);
	}
	
	objFile.write(fout, 0.5);  // The scale factor 0.5 undoes half-cells.
	fout.close();
}



static void
consolidateRects(const vector<OrientedRect3i> & smallRects,
	list<OrientedRect3i> & bigRects, int normalIndex)
{
	int nn;
	int ii = normalIndex;
	int jj = (ii+1)%3;
	int kk = (ii+2)%3;
	list<OrientedRect3i>::iterator itr, nextItr;
	
	if (smallRects.size() == 0)
		return;
	
	// First step: consolidate along the jj direction.  This is easy bkoz
	// small rects are not adjacent along j unless they actually at adjacent
	// indices.
	Rect3i curRect = smallRects[0].rect;
	Vector3i curNormal = smallRects[0].normal;
	for (nn = 1; nn < smallRects.size(); nn++)
	{
		const Rect3i & nextRect = smallRects[nn].rect;
		const Vector3i & nextNormal = smallRects[nn].normal;
		if (curRect.p1[ii] == nextRect.p1[ii] &&
			curRect.p2[jj] == nextRect.p1[jj] &&
			curRect.p1[kk] == nextRect.p1[kk] &&
			curRect.p2[kk] == nextRect.p2[kk] &&
			curNormal == nextNormal)
		{
			curRect.p2[jj] = nextRect.p2[jj];
		}
		else
		{
			bigRects.push_back(OrientedRect3i(curRect,curNormal));
			curRect = nextRect;
			curNormal = nextNormal;
		}
	}
	bigRects.push_back(OrientedRect3i(curRect, curNormal));
	
	// Second step: consolidate along the kk direction.  This is a little harder
	// bkoz there may be several non-abutting rects between mergeable rects.
	
	
	//LOG << "Now consolidate.\n";
	for (itr = bigRects.begin(); itr != bigRects.end(); itr++)
	{
		nextItr = itr;
		nextItr++;
		
		// While there IS another rect, while the rects are coplanar, and
		// while the rects might be adjacent in the k direction, continue.
		while (nextItr != bigRects.end() && 
			nextItr->rect.p1[ii] == itr->rect.p1[ii] &&
			nextItr->rect.p1[kk] <= itr->rect.p2[kk])
		{
			//LOG << *itr << " vs " << *nextItr << "\n";
			if (itr->rect.p2[kk] == nextItr->rect.p1[kk] &&
				itr->rect.p1[jj] == nextItr->rect.p1[jj] &&
				itr->rect.p2[jj] == nextItr->rect.p2[jj] &&
				itr->normal == nextItr->normal)
			{
				itr->rect.p2[kk] = nextItr->rect.p2[kk];  // the merge step
				bigRects.erase(nextItr++);  // this only works for list<>.
			}
			else
				nextItr++;
		}
	}
}


