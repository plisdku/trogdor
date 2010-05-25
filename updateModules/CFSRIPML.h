/*
 *  CFSRIPML.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _CFSRIPML_
#define _CFSRIPML_

#include "Pointer.h"

#include <vector>
#include "geometry.h"
#include "Map.h"
#include <string>
#include "MemoryUtilities.h"
#include "Runline.h"

class Paint;

// This class handles everything that's in common between the various templated
// PML update classes; the templates just add direction-specific update
// equations.
class CFSRIPMLBase
{
public:
    CFSRIPMLBase() { assert(!"This may compile but shouldn't be called."); }
    CFSRIPMLBase(Paint* parentPaint, std::vector<long> numCellsE,
        std::vector<long> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );
    virtual ~CFSRIPMLBase() {}
    
    std::string modelName() const;
    
    virtual void allocateAuxBuffers();
    
    Vector3i pmlDirection() const { return mPMLDirection; }
    
protected:
    // three setup functions
    void setNumCellsE(int fieldDir, int numCells, Paint* parentPaint);
    void setNumCellsH(int fieldDir, int numCells, Paint* parentPaint);
    void setPMLHalfCells(int pmlDir, Rect3i halfCellsOnSide,
        Paint* parentPaint);
    
    // Formerly setup things:
    std::vector<float> mSigmaE[3][3];
    std::vector<float> mSigmaH[3][3];
    std::vector<float> mAlphaE[3][3];
    std::vector<float> mAlphaH[3][3];
    std::vector<float> mKappaE[3][3];
    std::vector<float> mKappaH[3][3];
    
    std::vector<Rect3i> mPMLHalfCells;
    Map<Vector3i, Map<std::string, std::string> > mPMLParams;
    
    // These vectors are the actual location of the allocated fields and
    // update constants.
    std::vector<float> mAccumEj[3], mAccumEk[3],
        mAccumHj[3], mAccumHk[3];
    std::vector<float> mC_JjH[3], mC_JkH[3],
        mC_PhijH[3], mC_PhikH[3],
        mC_PhijJ[3], mC_PhikJ[3];
    std::vector<float> mC_MjE[3], mC_MkE[3],
        mC_PsijE[3], mC_PsikE[3],
        mC_PsijM[3], mC_PsikM[3];
    
    MemoryBufferPtr mBufAccumEj[3], mBufAccumEk[3],
        mBufAccumHj[3], mBufAccumHk[3];
    Vector3i mPMLDirection;
    
    Vector3f mDxyz;
    float mDt;
    
    int mRunlineDirection;
};

// Completes the implementation of the PML interface.
// This is the dude that gets fed to the template hopper.
template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
class CFSRIPML : public CFSRIPMLBase
{
public:
    CFSRIPML() { assert(!"This may compile but shouldn't be called."); }
    CFSRIPML(Paint* parentPaint, std::vector<long> numCellsE,
        std::vector<long> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );
    
    // This will be specialized below.  The DOESNOTHING parameter is here for
    // a REALLY DUMB C++ REASON.  If it's not there, this won't compile.  I am
    // consequently mad at the C++ language.
    template<int MEMORYDIRECTION, int DOESNOTHING=0>
    struct LocalDataE {};
    template<int MEMORYDIRECTION, int DOESNOTHING=0>
    struct LocalDataH {};
    
    void onStartRunlineE(LocalDataE<0> & data, const SimpleAuxPMLRunline & rl,
        int dir0, int dir1, int dir2);
    void onStartRunlineE(LocalDataE<1> & data, const SimpleAuxPMLRunline & rl,
        int dir0, int dir1, int dir2);
    void onStartRunlineE(LocalDataE<2> & data, const SimpleAuxPMLRunline & rl,
        int dir0, int dir1, int dir2);
        
    float updateJ(LocalDataE<0> & data, float Ei, float dHj, float dHk);
    float updateJ(LocalDataE<1> & data, float Ei, float dHj, float dHk);
    float updateJ(LocalDataE<2> & data, float Ei, float dHj, float dHk);
    
    
    void onStartRunlineH(LocalDataH<0> & data, const SimpleAuxPMLRunline & rl,
        int dir0, int dir1, int dir2);
    void onStartRunlineH(LocalDataH<1> & data, const SimpleAuxPMLRunline & rl,
        int dir0, int dir1, int dir2);
    void onStartRunlineH(LocalDataH<2> & data, const SimpleAuxPMLRunline & rl,
        int dir0, int dir1, int dir2);
        
    float updateK(LocalDataH<0> & data, float Hi, float dEj, float dEk);
    float updateK(LocalDataH<1> & data, float Hi, float dEj, float dEk);
    float updateK(LocalDataH<2> & data, float Hi, float dEj, float dEk);
};

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
template<int DOESNOTHING>
struct CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::LocalDataE<0, DOESNOTHING>
{
    float* Phi_ij, *Phi_ik;         // e.g. Phi_xy, Phi_xz
    float c_JijH, c_JikH;
    float c_Phi_ijH, c_Phi_ikH;
    float c_Phi_ijJ, c_Phi_ikJ;
};

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
template<int DOESNOTHING>
struct CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::LocalDataE<1, DOESNOTHING>
{
    float *Phi_ij, *Phi_ik;         // e.g. Phi_xy, Phi_xz
    float c_JijH, *c_JikH;
    float c_Phi_ijH, *c_Phi_ikH;
    float c_Phi_ijJ, *c_Phi_ikJ;
};

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
template<int DOESNOTHING>
struct CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::LocalDataE<2, DOESNOTHING>
{
    float* Phi_ij, *Phi_ik;         // e.g. Phi_xy, Phi_xz
    float *c_JijH, c_JikH;
    float *c_Phi_ijH, c_Phi_ikH;
    float *c_Phi_ijJ, c_Phi_ikJ;
};


template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
template<int DOESNOTHING>
struct CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::LocalDataH<0, DOESNOTHING>
{
    float* Psi_ij, *Psi_ik;         // e.g. Phi_xy, Phi_xz
    float c_MijE, c_MikE;           // constants
    float c_Psi_ijE, c_Psi_ikE;     // also constant, hoorah!
    float c_Psi_ijM, c_Psi_ikM;     // and still constant!
};

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
template<int DOESNOTHING>
struct CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::LocalDataH<1, DOESNOTHING>
{
    float* Psi_ij, *Psi_ik;         // e.g. Phi_xy, Phi_xz
    float c_MijE, *c_MikE;
    float c_Psi_ijE, *c_Psi_ikE;     // also constant, hoorah!
    float c_Psi_ijM, *c_Psi_ikM;     // and still constant!
};

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
template<int DOESNOTHING>
struct CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::LocalDataH<2, DOESNOTHING>
{
    float* Psi_ij, *Psi_ik;         // e.g. Phi_xy, Phi_xz
    float *c_MijE, c_MikE;           // constants
    float *c_Psi_ijE, c_Psi_ikE;     // also constant, hoorah!
    float *c_Psi_ijM, c_Psi_ikM;     // and still constant!
};



#include "CFSRIPML-inl.h"

#endif
