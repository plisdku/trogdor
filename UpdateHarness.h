/*
 *  UpdateHarness.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/13/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _UPDATEHARNESS_
#define _UPDATEHARNESS_



// TEMPLATE REQUIREMENTS:
//  Material must have constructor with appropriate arguments (TBD?)
//  RunlineClass needs to have constructor for given runline type
template<class RunlineClass>
class UpdateHarnessBase : public UpdateEquation
{
public:
    UpdateHarnessBase();
    //  Because templated virtual functions are not allowed in C++, I have to
    // overload the setRunlines functions manually.  What a pain!
    
    virtual void setRunlinesE(int direction,
        const std::vector<SBMRunlinePtr> & rls);
    virtual void setRunlinesE(int direction,
        const std::vector<SBPMRunlinePtr> & rls);
    
    virtual void setRunlinesH(int direction,
        const std::vector<SBMRunlinePtr> & rls);
    virtual void setRunlinesH(int direction,
        const std::vector<SBPMRunlinePtr> & rls);
    
    std::vector<RunlineClass> & getRunlinesE(int direction)
        { return mRunlinesE[direction]; }
    std::vector<RunlineClass> & getRunlinesH(int direction)
        { return mRunlinesH[direction]; }
    
    virtual long getNumRunlinesE() const;
    virtual long getNumRunlinesH() const;
    virtual long getNumHalfCellsE() const;
    virtual long getNumHalfCellsH() const;
protected:
    std::vector<RunlineClass> mRunlinesE[3];
    std::vector<RunlineClass> mRunlinesH[3];
};


// this may be templatized by runline type as well.
// TEMPLATE REQUIREMENTS:
//  NonPMLMaterial must inherit or look like SimpleMaterial
template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
class UpdateHarness : public UpdateHarnessBase<RunlineT>
{
public:
    UpdateHarness(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, Vector3f dxyz, float dt,
        int runlineDirection );
    
    UpdateHarness(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );
    
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
    
    virtual void writeJ(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeP(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeK(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeM(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    
    virtual void allocateAuxBuffers();
    
    virtual std::string getModelName() const
        { return mMaterial.getModelName(); }
    
    
private:
    template<int DIRECTION_PML>
    void calcE(int fieldDirection);
    
    template<int MEMORY_DIRECTION>
    void calcH(int fieldDirection);
    
    Vector3f mDxyz;
    Vector3f mDxyz_inverse;
    float mDt;
    
    int mRunlineDirection;
    
    MaterialT mMaterial;
    PMLT mPML;
    CurrentT mCurrent;
};

#include "UpdateHarness-inl.h"



#endif
