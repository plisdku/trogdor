/*
 *  PECModel.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/10/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "PECModel.h"


using namespace std;

PECModel::
PECModel() :
	MaterialModel()
{
}

string PECModel::
getMaterialName() const
{
    return string("PEC");
}

MaterialModel* PECModel::
createPML(Vector3i direction, Rect3i activeRect, Rect3i regionOfInterest,
    float dx, float dy, float dz, float dt) const
{
    return new PECModel();
}

/*
void PECModel::
calcEx(float dx, float dy, float dz, float dt)
{
}

void PECModel::
calcEy(float dx, float dy, float dz, float dt)
{
}

void PECModel::
calcEz(float dx, float dy, float dz, float dt)
{
}

void PECModel::
calcHx(float dx, float dy, float dz, float dt)
{
}

void PECModel::
calcHy(float dx, float dy, float dz, float dt)
{
}

void PECModel::
calcHz(float dx, float dy, float dz, float dt)
{
}
*/

