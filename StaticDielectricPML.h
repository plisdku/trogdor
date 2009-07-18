/*
 *  StaticDielectricPML.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/4/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */


#ifndef _STATICDIELECTRICPML_
#define _STATICDIELECTRICPML_

#include "SimulationDescription.h"
#include "MaterialBoss.h"
#include "StandardPML.h"
/*
class SetupStaticDielectricPML : public SimpleBulkPMLSetupMaterial
{
public:
    SetupStaticDielectricPML(
        const Map<Vector3i, Map<std::string, std::string> > & pmlParams);
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
        
    virtual void setNumCellsE(int fieldDir, int numCells);
    virtual void setNumCellsH(int fieldDir, int numCells);
    virtual void setPMLHalfCells(int pmlDir, Rect3i halfCellsOnSide,
        const GridDescription & gridDesc);
    
    const SetupStandardPML & getPML() const { return mPML; }
private:
    SetupStandardPML mPML;
};

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
class StaticDielectricPML : public Material
{
public:
    StaticDielectricPML(const SetupStaticDielectricPML & deleg,
        Vector3f dxyz, float dt);
    
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
    
    virtual void allocateAuxBuffers();
private:
    void calcEx();
    void calcEy();
    void calcEz();
    void calcHx();
    void calcHy();
    void calcHz();
    
    std::vector<SimpleAuxPMLRunline> mRunlinesE[3];
    std::vector<SimpleAuxPMLRunline> mRunlinesH[3];
    
    AbstractPML* mPML;
    //StandardPML mPML<X_ATTEN, Y_ATTEN, Z_ATTEN>;
    
    Vector3f mDxyz;
    float mDt;
    float m_epsr;
    float m_mur;
};
*/

#include "StaticDielectricPML.hpp"

#endif