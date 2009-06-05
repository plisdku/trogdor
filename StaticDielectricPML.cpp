/*
 *  StaticDielectricPML.hh
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/4/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifdef _STATICDIELECTRICPML_

#include "StaticDielectricPML.h"
#include "CalculationPartition.h"
#include "Paint.h"
#include <sstream>

using namespace std;

StaticDielectricPMLDelegate::
StaticDielectricPMLDelegate() :
    SimpleBulkPMLMaterialDelegate()
{
    mPMLDir = mStartPaint->getPMLDirections();
    mPMLHalfCells = vp.getPMLHalfCells(mPMLDir);
}

void StaticDielectricPMLDelegate::
setNumCells(int octant, int number)
{
	const int STRIDE = 1;
	int eIndex = octantENumber(octant);
    int hIndex = octantHNumber(octant);
    
    LOG << "Setting number of cells.\n";
    
    ostringstream bufName;
    if (eIndex != -1)
    {
        bufName.clear();
        bufName << mDesc->getName() << " AccumEj " << eIndex;
        mCurrents[eIndex] = MemoryBufferPtr(new MemoryBuffer(
            bufName.str(), number, STRIDE));
    }
    else if (hIndex != -1)
    {
    }
}

MaterialPtr StaticDielectricPMLDelegate::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    MaterialPtr m;
    
    // This disgusting dispatch procedure is necessary because the attenuation
    // directions are handled via templates, which are static creatures...
    if (mPMLDir[0] != 0)
    {
        if (mPMLDir[1] && mPMLDir[2])
        {
            m = MaterialPtr(new StaticDielectricPML<1,1,1>(*this,
                *mStartPaint->getBulkMaterial(), mPMLHalfCells, mPMLDir,
                cp.getDxyz(), cp.getDt() ));
        }
        else if (mPMLDir[1])
        {
            m = MaterialPtr(new StaticDielectricPML<1,1,0>(*this,
                *mStartPaint->getBulkMaterial(), mPMLHalfCells, mPMLDir,
                cp.getDxyz(), cp.getDt() ));
        }
        else if (mPMLDir[2])
        {
            m = MaterialPtr(new StaticDielectricPML<1,0,1>(*this,
                *mStartPaint->getBulkMaterial(), mPMLHalfCells, mPMLDir,
                cp.getDxyz(), cp.getDt() ));
        }
        else
        {
            m = MaterialPtr(new StaticDielectricPML<1,0,0>(*this,
                *mStartPaint->getBulkMaterial(), mPMLHalfCells, mPMLDir,
                cp.getDxyz(), cp.getDt() ));
        }
    }
    else if (mPMLDir[1] != 0)
    {
        if (mPMLDir[2])
        {
            m = MaterialPtr(new StaticDielectricPML<0,1,1>(*this,
                *mStartPaint->getBulkMaterial(), mPMLHalfCells, mPMLDir,
                cp.getDxyz(), cp.getDt() ));
        }
        else
        {
            m = MaterialPtr(new StaticDielectricPML<0,1,0>(*this,
                *mStartPaint->getBulkMaterial(), mPMLHalfCells, mPMLDir,
                cp.getDxyz(), cp.getDt() ));
        }
    }
    else if (mPMLDir[2] != 0)
    {
        m = MaterialPtr(new StaticDielectricPML<0,0,1>(*this,
            *mStartPaint->getBulkMaterial(), mPMLHalfCells, mPMLDir,
            cp.getDxyz(), cp.getDt() ));
    }
    else
        assert(!"What, a directionless PML???");
    
    return m;
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
StaticDielectricPML::
StaticDielectricPML(const StaticDielectricPMLDelegate & deleg,
    const MaterialDescription & descrip, Rect3i pmlHalfCells, Vector3i pmlDir,
    Vector3f dxyz, float dt) :
    Material(),
    mPMLHalfCells(pmlHalfCells),
    mPMLDir(pmlDir),
    mDxyz(dxyz),
    mDt(dt),
    m_epsr(1.0),
    m_mur(1.0)
{
    if (descrip.getParams().count("epsr"))
        istringstream(descrip.getParams()["epsr"]) >> m_epsr;
    if (descrip.getParams().count("mur"))
        istringstream(descrip.getParams()["mur"]) >> m_mur;
    
    // Make the runlines
    for (int field = 0; field < 6; field++)
    {
        const std::vector<SBPMRunlinePtr> & setupRunlines =
            deleg.getRunlines(field);
        
        mRunlines[field].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
            mRunlines[field][nn] = SimplePMLRunline(*setupRunlines[nn]);
    }
    
    // For simplicity all field octants use the same size of PML array.
    for (int attenDir = 0; attenDir < 3; attenDir++)
    if (mPMLDir[attenDir] != 0)
    {
        int depthYee = mPMLHalfCells.p2[attenDir]/2
            - mPMLHalfCells.p1[attenDir]/2 + 1;
        
        mC_JjH[attenDir].resize(depthYee);
        mC_Jjk[attenDir].resize(depthYee);
        mC_PhijH[attenDir].resize(depthYee);
        mC_PhikH[attenDir].resize(depthYee);
        mC_PhijJ[attenDir].resize(depthYee);
        mC_PhikJ[attenDir].resize(depthYee);
        mC_MjE[attenDir].resize(depthYee);
        mC_MkE[attenDir].resize(depthYee);
        mC_PsijE[attenDir].resize(depthYee);
        mC_PsikE[attenDir].resize(depthYee);
        mC_PsijM[attenDir].resize(depthYee);
        mC_PsikM[attenDir].resize(depthYee);
    }
    
    /*
    // Allocate the auxiliary thingies
    int attenDir;
    for (attenDir = 0; attenDir < 3; attenDir++)
    if (mPMLDir[attenDir] != 0)
    {
        int depthYee;
        int jDir = (attenDir+1)%3;
        int kDir = (jDir+1)%3;
        
        for (int nField = 1; nField <= 2; nField++)
        {
            int jkDir = (attenDir+nField)%3;
            
            // E field things!
            Rect3i ePMLYee = rectHalfToYee(mPMLHalfCells,
                eOctantNumber(jkDir));
            depthYee = ePMLYee.size(attenDir)+1;
            
            mC_J
                        
            // H field things!
            Rect3i hPMLYee = rectHalfToYee(mPMLHalfCells,
                hOctantNumber(jkDir));
            depthYee = hPMLYee.size(attenDir)+1;
        }
    }
    */
    
    //LOG << "Created all runlines.\n";
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticDielectricPML::
calcEPhase(int direction)
{
    //LOG << "Calculating E.\n";
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticDielectricPML::
calcHPhase(int direction)
{
    //LOG << "Calculating H.\n";
}


#endif