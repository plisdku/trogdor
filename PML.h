/*
 *  PML.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _PML_
#define _PML_

#include <vector>
#include "SimpleSetupMaterial.h"
#include <iostream>

using namespace std;

// Concept for PML factory (does NOT need to be inherited, it's a concept):
/*
class PMLFactory
{
public:
    static Pointer<PML> newPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
};
*/

struct PMLRunline
{
    PMLRunline() {}
    PMLRunline(const SBPMRunline & setupRunline);
    
    float* fi;
    float* gj[2];
    float* gk[2];
    unsigned long length;
    unsigned long auxIndex;
    unsigned long pmlIndex[3];
};


class PML
{
public:
    virtual ~PML();
    virtual void calcEx() = 0;
    virtual void calcEy() = 0;
    virtual void calcEz() = 0;
    virtual void calcHx() = 0;
    virtual void calcHy() = 0;
    virtual void calcHz() = 0;
    virtual void allocateAuxBuffers() = 0;
    
    virtual void setRunlinesE(int direction,
        const std::vector<SBPMRunlinePtr> & rls);
    virtual void setRunlinesH(int direction,
        const std::vector<SBPMRunlinePtr> & rls);
protected:
    std::vector<PMLRunline> mRunlinesE[3];
    std::vector<PMLRunline> mRunlinesH[3];
};

std::ostream & operator<<(std::ostream & str, const PMLRunline & rl);

#endif

