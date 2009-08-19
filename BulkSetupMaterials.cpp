/*
 *  SimpleSetupMaterial.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "BulkSetupMaterials.h"
#include "SimulationDescription.h"

#include "HuygensSurface.h"
#include "InterleavedLattice.h"
#include "VoxelizedPartition.h"
#include "VoxelGrid.h"
#include "PartitionCellCount.h"
#include "Paint.h"
#include "Log.h"
#include "YeeUtilities.h"

using namespace std;
using namespace YeeUtilities;

#pragma mark *** Simple Bulk Material ***

BulkSetupMaterial::
BulkSetupMaterial() :
	SetupMaterial(),
    mRunlineEncoder()
{
}

BulkSetupMaterial::
~BulkSetupMaterial()
{
}

void BulkSetupMaterial::
printRunlines(std::ostream & out) const
{
    int dir;
    unsigned int rr;
    for (dir = 0; dir < 3; dir++)
	{
		out << "E " << dir << "\n";
		for (rr = 0; rr < runlinesE(dir).size(); rr++)
		{
			out << rr << ": length " << runlinesE(dir)[rr]->length <<
				" aux " << runlinesE(dir)[rr]->auxIndex << "\n";
			out << "\t" << runlinesE(dir)[rr]->f_i << "\n";
			out << "\t" << runlinesE(dir)[rr]->f_j[0] << "\n";
			out << "\t" << runlinesE(dir)[rr]->f_j[1] << "\n";
			out << "\t" << runlinesE(dir)[rr]->f_k[0] << "\n";
			out << "\t" << runlinesE(dir)[rr]->f_k[1] << "\n";
		}
	}
    for (dir = 0; dir < 3; dir++)
	{
		out << "H " << dir << "\n";
		for (rr = 0; rr < runlinesH(dir).size(); rr++)
		{
			out << rr << ": length " << runlinesH(dir)[rr]->length <<
				" aux " << runlinesH(dir)[rr]->auxIndex << "\n";
			out << "\t" << runlinesH(dir)[rr]->f_i << "\n";
			out << "\t" << runlinesH(dir)[rr]->f_j[0] << "\n";
			out << "\t" << runlinesH(dir)[rr]->f_j[1] << "\n";
			out << "\t" << runlinesH(dir)[rr]->f_k[0] << "\n";
			out << "\t" << runlinesH(dir)[rr]->f_k[1] << "\n";
		}
	}
}

const vector<SBMRunlinePtr> & BulkSetupMaterial::
runlinesE(int dir) const
{
    return mRunlineEncoder.runlinesE(dir);
}

const vector<SBMRunlinePtr> & BulkSetupMaterial::
runlinesH(int dir) const
{
    return mRunlineEncoder.runlinesH(dir);
}

#pragma mark *** Simple Bulk PML Material ***

BulkPMLSetupMaterial::
BulkPMLSetupMaterial() :
	SetupMaterial(),
    mRunlineEncoder()
{
}

void BulkPMLSetupMaterial::
printRunlines(std::ostream & out) const
{
    int dir;
    unsigned int rr;
    for (dir = 0; dir < 3; dir++)
	{
		out << "E " << dir << "\n";
		for (rr = 0; rr < runlinesE(dir).size(); rr++)
		{
			out << rr << ": length " << runlinesE(dir)[rr]->length <<
				" aux " << runlinesE(dir)[rr]->auxIndex <<
				" pml depth";
			for (int nn = 0; nn < 3; nn++)
				out << " " << runlinesE(dir)[rr]->pmlDepthIndex[nn];
			//out << rr << ": length " << runlinesE(dir)[rr]->length <<
			//	" aux " << runlinesE(dir)[rr]->auxIndex << "\n";
			out << "\n\t" << runlinesE(dir)[rr]->f_i << "\n";
			out << "\t" << runlinesE(dir)[rr]->f_j[0] << "\n";
			out << "\t" << runlinesE(dir)[rr]->f_j[1] << "\n";
			out << "\t" << runlinesE(dir)[rr]->f_k[0] << "\n";
			out << "\t" << runlinesE(dir)[rr]->f_k[1] << "\n";
		}
	}
    for (dir = 0; dir < 3; dir++)
	{
		out << "H " << dir << "\n";
		for (rr = 0; rr < runlinesH(dir).size(); rr++)
		{
			out << rr << ": length " << runlinesH(dir)[rr]->length <<
				" aux " << runlinesH(dir)[rr]->auxIndex <<
				" pml depth";
			for (int nn = 0; nn < 3; nn++)
				out << " " << runlinesH(dir)[rr]->pmlDepthIndex[nn];
			out << "\n\t" << runlinesH(dir)[rr]->f_i << "\n";
			out << "\t" << runlinesH(dir)[rr]->f_j[0] << "\n";
			out << "\t" << runlinesH(dir)[rr]->f_j[1] << "\n";
			out << "\t" << runlinesH(dir)[rr]->f_k[0] << "\n";
			out << "\t" << runlinesH(dir)[rr]->f_k[1] << "\n";
		}
	}
}


const vector<SBPMRunlinePtr> & BulkPMLSetupMaterial::
runlinesE(int dir) const
{
    return mRunlineEncoder.runlinesE(dir);
}

const vector<SBPMRunlinePtr> & BulkPMLSetupMaterial::
runlinesH(int dir) const
{
    return mRunlineEncoder.runlinesH(dir);
}




















