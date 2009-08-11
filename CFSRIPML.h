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

// This class handles everything that's in common between the various templated
// PML update classes; the templates just add direction-specific update
// equations.
class CFSRIPMLBase
{
public:
    CFSRIPMLBase() { assert(!"This may compile but shouldn't be called."); }
    CFSRIPMLBase(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );
    virtual ~CFSRIPMLBase() {}
    
    std::string getModelName() const;
    
    virtual void allocateAuxBuffers();
        
    Vector3i getPMLDirection() const { return mPMLDirection; }
    
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
    int mRunlineDirection;
    
    Vector3f mDxyz;
    float mDt;

};

// Completes the implementation of the PML interface.
// This is the dude that gets fed to the template hopper.
template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
class CFSRIPML : public CFSRIPMLBase
{
public:
    CFSRIPML() { assert(!"This may compile but shouldn't be called."); }
    CFSRIPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
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

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
void CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
onStartRunlineE(LocalDataE<0> & data,
    const SimpleAuxPMLRunline & rl, int dir0, int dir1, int dir2)
{
    /*
    Vector3i pmlPML = cyclicPermute(mPMLDirection, (3-mRunlineDirection)%3);
    LOG << "PML dir " << mPMLDirection << " feels like " << pmlPML << "\n";
    LOG << "Field " << dir0 << " feels like 0.\n";
    LOG << "So, set the accum variables right: sizes are\n"
        "\t" << mAccumEj[dir0].size() << ", " << mAccumEk[dir0].size()
        << "\n";
    */
    if (J_ATTEN)
    {
        data.Phi_ij = &(mAccumEj[dir0][rl.auxIndex]);
        data.c_JijH = mC_JjH[dir0][rl.pmlIndex[dir1]];
        data.c_Phi_ijH = mC_PhijH[dir0][rl.pmlIndex[dir1]];
        data.c_Phi_ijJ = mC_PhijJ[dir0][rl.pmlIndex[dir1]];
    }
    
    if (K_ATTEN)
    {
        data.Phi_ik = &mAccumEk[dir0][rl.auxIndex];
        data.c_JikH = mC_JkH[dir0][rl.pmlIndex[dir2]];
        data.c_Phi_ikH = mC_PhikH[dir0][rl.pmlIndex[dir2]];
        data.c_Phi_ikJ = mC_PhikJ[dir0][rl.pmlIndex[dir2]];
    }
}

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
void CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
onStartRunlineE(LocalDataE<1> & data,
    const SimpleAuxPMLRunline & rl, int dir0, int dir1, int dir2)
{
    if (K_ATTEN)
    {
        data.Phi_ij = &mAccumEj[dir0][rl.auxIndex];
        data.c_JijH = mC_JjH[dir0][rl.pmlIndex[dir1]];
        data.c_Phi_ijH = mC_PhijH[dir0][rl.pmlIndex[dir1]];
        data.c_Phi_ijJ = mC_PhijJ[dir0][rl.pmlIndex[dir1]];
    }
    
    if (I_ATTEN)
    {
        data.Phi_ik = &mAccumEk[dir0][rl.auxIndex];
        data.c_JikH = &mC_JkH[dir0][rl.pmlIndex[dir2]];
        data.c_Phi_ikH = &mC_PhikH[dir0][rl.pmlIndex[dir2]];
        data.c_Phi_ikJ = &mC_PhikJ[dir0][rl.pmlIndex[dir2]];
    }
}

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
void CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
onStartRunlineE(LocalDataE<2> & data,
    const SimpleAuxPMLRunline & rl, int dir0, int dir1, int dir2)
{
    if (I_ATTEN)
    {
        data.Phi_ij = &mAccumEj[dir0][rl.auxIndex];
        data.c_JijH = &mC_JjH[dir0][rl.pmlIndex[dir1]];
        data.c_Phi_ijH = &mC_PhijH[dir0][rl.pmlIndex[dir1]];
        data.c_Phi_ijJ = &mC_PhijJ[dir0][rl.pmlIndex[dir1]];
    }
    
    if (J_ATTEN)
    {
        data.Phi_ik = &mAccumEk[dir0][rl.auxIndex];
        data.c_JikH = mC_JkH[dir0][rl.pmlIndex[dir2]];
        data.c_Phi_ikH = mC_PhikH[dir0][rl.pmlIndex[dir2]];
        data.c_Phi_ikJ = mC_PhikJ[dir0][rl.pmlIndex[dir2]];
    }
}


template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
inline float CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
updateJ(LocalDataE<0> & data, float Ei, float dHj, float dHk)
{
    float Jij, Jik;
    
    if (J_ATTEN)
    {
        Jij = data.c_JijH*dHk + *data.Phi_ij;
        *data.Phi_ij += (data.c_Phi_ijH*dHk - data.c_Phi_ijJ*Jij);
        data.Phi_ij++;
    }
    
    if (K_ATTEN)
    {
        Jik = data.c_JikH*dHj + *data.Phi_ik;
        *data.Phi_ik += (data.c_Phi_ikH*dHj - data.c_Phi_ikJ*Jik);
        data.Phi_ik++;
    }
    
    if (J_ATTEN && K_ATTEN)
        return -Jij + Jik;
    else if (J_ATTEN)
        return -Jij;
    else if (K_ATTEN)
        return Jik;
    
    return 0.0;
}

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
inline float CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
updateJ(LocalDataE<1> & data, float Ei, float dHj, float dHk)
{
    float Jij, Jik;
    
    if (K_ATTEN)
    {
        Jij = data.c_JijH*dHk + *data.Phi_ij;
        *data.Phi_ij += (data.c_Phi_ijH*dHk - data.c_Phi_ijJ*Jij);
        data.Phi_ij++;
    }
    
    if (I_ATTEN)
    {
        Jik = *data.c_JikH*dHj + *data.Phi_ik;
        *data.Phi_ik += (*data.c_Phi_ikH*dHj - *data.c_Phi_ikJ*Jik);
        data.Phi_ik++;
        data.c_JikH++;
        data.c_Phi_ikH++;
        data.c_Phi_ikJ++;
    }
    
    if (I_ATTEN && K_ATTEN)
        return -Jij + Jik;
    else if (I_ATTEN)
        return Jik;
    else if (K_ATTEN)
        return -Jij;
    return 0.0;
}

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
inline float CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
updateJ(LocalDataE<2> & data, float Ei, float dHj, float dHk)
{
    // Be careful here.  We return -Jij, not Jij.
    float Jij, Jik;
    
    if (I_ATTEN)
    {
        Jij = *data.c_JijH*dHk + *data.Phi_ij;
        *data.Phi_ij += (*data.c_Phi_ijH*dHk - *data.c_Phi_ijJ*Jij);
        data.Phi_ij++;
        data.c_JijH++;
        data.c_Phi_ijH++;
        data.c_Phi_ijJ++;
    }
    
    if (J_ATTEN)
    {
        Jik = data.c_JikH*dHj + *data.Phi_ik;
        *data.Phi_ik += (data.c_Phi_ikH*dHj - data.c_Phi_ikJ*Jik);
        data.Phi_ik++;
    }
    
    if (I_ATTEN && J_ATTEN)
        return -Jij + Jik;
    else if (I_ATTEN)
        return -Jij;
    else if (J_ATTEN)
        return Jik;
    return 0.0;
}





template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
void CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
onStartRunlineH(LocalDataH<0> & data,
    const SimpleAuxPMLRunline & rl, int dir0, int dir1, int dir2)
{
    if (J_ATTEN)
    {
        data.Psi_ij = &mAccumHj[dir0][rl.auxIndex];
        data.c_MijE = mC_MjE[dir0][rl.pmlIndex[dir1]];
        data.c_Psi_ijE = mC_PsijE[dir0][rl.pmlIndex[dir1]];
        data.c_Psi_ijM = mC_PsijM[dir0][rl.pmlIndex[dir1]];
    }
    
    if (K_ATTEN)
    {
        data.Psi_ik = &mAccumHk[dir0][rl.auxIndex];
        data.c_MikE = mC_MkE[dir0][rl.pmlIndex[dir2]];
        data.c_Psi_ikE = mC_PsikE[dir0][rl.pmlIndex[dir2]];
        data.c_Psi_ikM = mC_PsikM[dir0][rl.pmlIndex[dir2]];
    }
}

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
void CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
onStartRunlineH(LocalDataH<1> & data,
    const SimpleAuxPMLRunline & rl, int dir0, int dir1, int dir2)
{
    if (K_ATTEN)
    {
        data.Psi_ij = &mAccumHj[dir0][rl.auxIndex];
        data.c_MijE = mC_MjE[dir0][rl.pmlIndex[dir1]];
        data.c_Psi_ijE = mC_PsijE[dir0][rl.pmlIndex[dir1]];
        data.c_Psi_ijM = mC_PsijM[dir0][rl.pmlIndex[dir1]];
    }
    
    if (I_ATTEN)
    {
        data.Psi_ik = &mAccumHk[dir0][rl.auxIndex];
        data.c_MikE = &mC_MkE[dir0][rl.pmlIndex[dir2]];
        data.c_Psi_ikE = &mC_PsikE[dir0][rl.pmlIndex[dir2]];
        data.c_Psi_ikM = &mC_PsikM[dir0][rl.pmlIndex[dir2]];
    }
}

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
void CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
onStartRunlineH(LocalDataH<2> & data,
    const SimpleAuxPMLRunline & rl, int dir0, int dir1, int dir2)
{
    if (I_ATTEN)
    {
        data.Psi_ij = &mAccumHj[dir0][rl.auxIndex];
        data.c_MijE = &mC_MjE[dir0][rl.pmlIndex[dir1]];
        data.c_Psi_ijE = &mC_PsijE[dir0][rl.pmlIndex[dir1]];
        data.c_Psi_ijM = &mC_PsijM[dir0][rl.pmlIndex[dir1]];
    }
    
    if (J_ATTEN)
    {
        data.Psi_ik = &mAccumHk[dir0][rl.auxIndex];
        data.c_MikE = mC_MkE[dir0][rl.pmlIndex[dir2]];
        data.c_Psi_ikE = mC_PsikE[dir0][rl.pmlIndex[dir2]];
        data.c_Psi_ikM = mC_PsikM[dir0][rl.pmlIndex[dir2]];
    }
}

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
inline float CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
updateK(LocalDataH<0> & data, float Hi, float dEj, float dEk)
{
    float Mij, Mik;
    if (J_ATTEN)
    {
        Mij = data.c_MijE*dEk + *data.Psi_ij;
        *data.Psi_ij += (data.c_Psi_ijE*dEk - data.c_Psi_ijM*Mij);
        data.Psi_ij++;
    }
    
    if (K_ATTEN)
    {
        Mik = data.c_MikE*dEj + *data.Psi_ik;
        *data.Psi_ik += (data.c_Psi_ikE*dEj - data.c_Psi_ikM*Mik);
        data.Psi_ik++;
    }
    
    if (J_ATTEN && K_ATTEN)
        return Mij - Mik;
    else if (J_ATTEN)
        return Mij;
    else if (K_ATTEN)
        return -Mik;
    return 0.0;
}

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
inline float CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
updateK(LocalDataH<1> & data, float Hi, float dEj, float dEk)
{
    float Mij, Mik;
    
    if (K_ATTEN)
    {
        Mij = data.c_MijE*dEk + *data.Psi_ij;
        *data.Psi_ij += (data.c_Psi_ijE*dEk - data.c_Psi_ijM*Mij);
        data.Psi_ij++;
    }
    
    if (I_ATTEN)
    {
        Mik = *data.c_MikE*dEj + *data.Psi_ik;
        *data.Psi_ik += (*data.c_Psi_ikE*dEj - *data.c_Psi_ikM*Mik);
        data.c_MikE++;
        data.c_Psi_ikE++;
        data.c_Psi_ikM++;
        data.Psi_ik++;
    }
    
    if (K_ATTEN && I_ATTEN)
        return Mij - Mik;
    else if (K_ATTEN)
        return Mij;
    else if (I_ATTEN)
        return -Mik;
    return 0.0;
}

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
inline float CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
updateK(LocalDataH<2> & data, float Hi, float dEj, float dEk)
{
    float Mij, Mik;
    
    if (I_ATTEN)
    {
        Mij = *data.c_MijE*dEk + *data.Psi_ij;
        *data.Psi_ij += (*data.c_Psi_ijE*dEk - *data.c_Psi_ijM*Mij);
        data.c_MijE++;
        data.c_Psi_ijE++;
        data.c_Psi_ijM++;
        data.Psi_ij++;
    }
    
    if (J_ATTEN)
    {
        Mik = data.c_MikE*dEj + *data.Psi_ik;
        *data.Psi_ik += (data.c_Psi_ikE*dEj - data.c_Psi_ikM*Mik);
        data.Psi_ik++;
    }
    
    if (I_ATTEN && J_ATTEN)
        return Mij - Mik;
    else if (I_ATTEN)
        return Mij;
    else if (J_ATTEN)
        return -Mik;
    return 0.0;
}



#include "CFSRIPML.cpp"
#endif
