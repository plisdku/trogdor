/*
 *  MaterialModel.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/20/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "MaterialModel.h"


#include <cassert>


using namespace std;

MaterialModel::
MaterialModel() :
	mRunlinesEx(),
	mRunlinesEy(),
	mRunlinesEz(),
	mRunlinesHx(),
	mRunlinesHy(),
	mRunlinesHz(),
	mParams(),
    mNumCellsEx(0),
    mNumCellsEy(0),
    mNumCellsEz(0),
    mNumCellsHx(0),
    mNumCellsHy(0),
    mNumCellsHz(0)
{
}

MaterialModel::
MaterialModel(const Map<std::string, std::string> & inParams) :
	mRunlinesEx(),
	mRunlinesEy(),
	mRunlinesEz(),
	mRunlinesHx(),
	mRunlinesHy(),
	mRunlinesHz(),
    mParams(inParams),
    mNumCellsEx(0),
    mNumCellsEy(0),
    mNumCellsEz(0),
    mNumCellsHx(0),
    mNumCellsHy(0),
    mNumCellsHz(0)
{
}


MaterialModel::
~MaterialModel()
{
    LOGF << "destructor." << endl;
}


void MaterialModel::
addRunline(Runline rl, int ii0, int jj0, int kk0)
{
    onAddRunline(rl, ii0, jj0, kk0);
    
    if (ii0 % 2 == 0)
    {
        if (jj0 % 2 == 0)
        {
            if (kk0 % 2 == 0)                       // 0 0 0
                assert(!"No 0 0 0 component!");
            else                                    // 0 0 1
            {
                mRunlinesEz.push_back(rl);
                mNumCellsEz += rl.getLength();
            }
        }
        else
        {
            if (kk0 % 2 == 0)                       // 0 1 0
            {
                mRunlinesEy.push_back(rl);
                mNumCellsEy += rl.getLength();
            }
            else                                    // 0 1 1
            {
                mRunlinesHx.push_back(rl);
                mNumCellsHx += rl.getLength();
            }
        }
    }
    else
    {
        if (jj0 % 2 == 0)
        {
            if (kk0 % 2 == 0)                       // 1 0 0
            {
                mRunlinesEx.push_back(rl);
                mNumCellsEx += rl.getLength();
            }
            else                                    // 1 0 1
            {
                mRunlinesHy.push_back(rl);
                mNumCellsHy += rl.getLength();
            }
        }
        else
        {
            if (kk0 % 2 == 0)                       // 1 1 0
            {
                mRunlinesHz.push_back(rl);
                mNumCellsHz += rl.getLength();
            }
            else                                    // 1 1 1
                assert(!"No 1 1 1 component!");
        }
    }
}

int MaterialModel::
numCellsEx() const
{
    return mNumCellsEx;
}

int MaterialModel::
numCellsEy() const
{
    return mNumCellsEy;
}

int MaterialModel::
numCellsEz() const
{
    return mNumCellsEz;
}

int MaterialModel::
numCellsHx() const
{
    return mNumCellsHx;
}

int MaterialModel::
numCellsHy() const
{
    return mNumCellsHy;
}

int MaterialModel::
numCellsHz() const
{
    return mNumCellsHz;
}

int MaterialModel::
numCells() const
{
    return mNumCellsEx + mNumCellsEy + mNumCellsEz + mNumCellsHx + mNumCellsHy
        + mNumCellsHz;
}

int MaterialModel::
numRunlines() const
{
	return mRunlinesEx.size() + mRunlinesEy.size() + mRunlinesEz.size() +
		mRunlinesHx.size() + mRunlinesHy.size() + mRunlinesHz.size();
}

string MaterialModel::
getMaterialName() const
{
    return string("undefined");
}

void MaterialModel::
allocate(float dx, float dy, float dz, float dt)
{
}

MaterialModel* MaterialModel::
createPML(Vector3i direction, Rect3i activeRect, Rect3i regionOfInterest,
          float dx, float dy, float dz, float dt) const
{
    return NULL;
}

void MaterialModel::
onAddRunline(Runline & rl, int ii, int jj, int kk)
{
}


void MaterialModel::
calcEx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	LOG << "Doing nothing.\n";
	/*
	int len = mRunlinesEx.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//mRunlinesEx[ii].assertZero();
		stepEx(mRunlinesEx[ii], dxinv, dyinv, dzinv, dt);
		//mRunlinesEx[ii].assertZero();
	}
	*/
}

void MaterialModel::
calcEy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	LOG << "Doing nothing.\n";
	/*
	int len = mRunlinesEy.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//mRunlinesEy[ii].assertZero();
		stepEy(mRunlinesEy[ii], dxinv, dyinv, dzinv, dt);
		//mRunlinesEy[ii].assertZero();
	}
	*/
}

void MaterialModel::
calcEz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	LOG << "Doing nothing.\n";
	/*
	int len = mRunlinesEz.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//mRunlinesEz[ii].assertZero();
		stepEz(mRunlinesEz[ii], dxinv, dyinv, dzinv, dt);
		//mRunlinesEz[ii].assertZero();
	}
	*/
}

void MaterialModel::
calcHx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	LOG << "Doing nothing.\n";
	/*
	int len = mRunlinesHx.size();
	for (int ii = start; ii < len; ii += stride)
		stepHx(mRunlinesHx[ii], dxinv, dyinv, dzinv, dt);
	*/
}

void MaterialModel::
calcHy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	LOG << "Doing nothing.\n";
	/*
	int len = mRunlinesHy.size();
	for (int ii = start; ii < len; ii += stride)
		stepHy(mRunlinesHy[ii], dxinv, dyinv, dzinv, dt);
	*/
}

void MaterialModel::
calcHz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	LOG << "Doing nothing.\n";
	/*
	int len = mRunlinesHz.size();
	for (int ii = start; ii < len; ii += stride)
		stepHz(mRunlinesHz[ii], dxinv, dyinv, dzinv, dt);
	*/
}

/*
void MaterialModel::
stepEx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
}

void MaterialModel::
stepEy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
}

void MaterialModel::
stepEz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
}

void MaterialModel::
stepHx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
}

void MaterialModel::
stepHy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
}

void MaterialModel::
stepHz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
}
*/


