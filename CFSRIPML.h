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

/*
class CFSRIPMLFactory
{
public:
    static Pointer<PML> newPML(Paint* parentPaint,
        std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
};
*/

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
        float dt);
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
    
    Vector3f mDxyz;
    float mDt;

};

// Completes the implementation of the PML interface.
// This is the dude that gets fed to the template hopper.
template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
class CFSRIPML : public CFSRIPMLBase
{
public:
    CFSRIPML() { assert(!"This may compile but shouldn't be called."); }
    CFSRIPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
    
    // Now for the inline functions and all that.
    struct LocalDataEx { 
        float* Phi_ij, *Phi_ik;         // e.g. Phi_xy, Phi_xz
        float c_JijH, c_JikH;           // constants
        float c_Phi_ijH, c_Phi_ikH;     // also constant, hoorah!
        float c_Phi_ijJ, c_Phi_ikJ;     // and still constant!
    };
    struct LocalDataEy {
        float *Phi_ij, *Phi_ik;         // e.g. Phi_xy, Phi_xz
        float c_JijH, *c_JikH;
        float c_Phi_ijH, *c_Phi_ikH;
        float c_Phi_ijJ, *c_Phi_ikJ;
    };
    struct LocalDataEz {
        float* Phi_ij, *Phi_ik;         // e.g. Phi_xy, Phi_xz
        float *c_JijH, c_JikH;
        float *c_Phi_ijH, c_Phi_ikH;
        float *c_Phi_ijJ, c_Phi_ikJ;
    };
    struct LocalDataHx {
        float* Psi_ij, *Psi_ik;         // e.g. Phi_xy, Phi_xz
        float c_MijE, c_MikE;           // constants
        float c_Psi_ijE, c_Psi_ikE;     // also constant, hoorah!
        float c_Psi_ijM, c_Psi_ikM;     // and still constant!
    }; 
    struct LocalDataHy {
        float* Psi_ij, *Psi_ik;         // e.g. Phi_xy, Phi_xz
        float c_MijE, *c_MikE;
        float c_Psi_ijE, *c_Psi_ikE;     // also constant, hoorah!
        float c_Psi_ijM, *c_Psi_ikM;     // and still constant!
    }; 
    struct LocalDataHz {
        float* Psi_ij, *Psi_ik;         // e.g. Phi_xy, Phi_xz
        float *c_MijE, c_MikE;           // constants
        float *c_Psi_ijE, c_Psi_ikE;     // also constant, hoorah!
        float *c_Psi_ijM, c_Psi_ikM;     // and still constant!
    };   
    
    void initLocalEx(LocalDataEx & data);
    void initLocalEy(LocalDataEy & data);
    void initLocalEz(LocalDataEz & data);
    void onStartRunlineEx(LocalDataEx & data, const SimpleAuxPMLRunline & rl);
    void onStartRunlineEy(LocalDataEy & data, const SimpleAuxPMLRunline & rl);
    void onStartRunlineEz(LocalDataEz & data, const SimpleAuxPMLRunline & rl);
    void beforeUpdateEx(LocalDataEx & data, float Ei, float dHj, float dHk);
    void beforeUpdateEy(LocalDataEy & data, float Ei, float dHj, float dHk);
    void beforeUpdateEz(LocalDataEz & data, float Ei, float dHj, float dHk); 
    float updateJx(LocalDataEx & data, float Ei, float dHj, float dHk);
    float updateJy(LocalDataEy & data, float Ei, float dHj, float dHk);
    float updateJz(LocalDataEz & data, float Ei, float dHj, float dHk);
    void afterUpdateEx(LocalDataEx & data, float Ei, float dHj, float dHk);
    void afterUpdateEy(LocalDataEy & data, float Ei, float dHj, float dHk);
    void afterUpdateEz(LocalDataEz & data, float Ei, float dHj, float dHk);
    
    void initLocalHx(LocalDataHx & data);
    void initLocalHy(LocalDataHy & data);
    void initLocalHz(LocalDataHz & data);
    void onStartRunlineHx(LocalDataHx & data, const SimpleAuxPMLRunline & rl);
    void onStartRunlineHy(LocalDataHy & data, const SimpleAuxPMLRunline & rl);
    void onStartRunlineHz(LocalDataHz & data, const SimpleAuxPMLRunline & rl);
    void beforeUpdateHx(LocalDataHx & data, float Hi, float dEj, float dEk);
    void beforeUpdateHy(LocalDataHy & data, float Hi, float dEj, float dEk);
    void beforeUpdateHz(LocalDataHz & data, float Hi, float dEj, float dEk);
    float updateKx(LocalDataHx & data, float Hi, float dEj, float dEk);
    float updateKy(LocalDataHy & data, float Hi, float dEj, float dEk);
    float updateKz(LocalDataHz & data, float Hi, float dEj, float dEk);
    void afterUpdateHx(LocalDataHx & data, float Hi, float dEj, float dEk);
    void afterUpdateHy(LocalDataHy & data, float Hi, float dEj, float dEk);
    void afterUpdateHz(LocalDataHz & data, float Hi, float dEj, float dEk);
};

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
initLocalEx(LocalDataEx & data)
{
    //LOG << X_ATTEN << " " << Y_ATTEN << " " << Z_ATTEN << "\n";
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
initLocalEy(LocalDataEy & data)
{
    //LOG << X_ATTEN << " " << Y_ATTEN << " " << Z_ATTEN << "\n";
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
initLocalEz(LocalDataEz & data)
{
    //LOG << X_ATTEN << " " << Y_ATTEN << " " << Z_ATTEN << "\n";
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
onStartRunlineEx(LocalDataEx & data, const SimpleAuxPMLRunline & rl)
{
    const int DIRECTION = 0;
    if (Y_ATTEN)
    {
        data.Phi_ij = &(mAccumEj[DIRECTION][rl.auxIndex]);
        data.c_JijH = mC_JjH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Phi_ijH = mC_PhijH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Phi_ijJ = mC_PhijJ[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
    }
    
    if (Z_ATTEN)
    {
        data.Phi_ik = &mAccumEk[DIRECTION][rl.auxIndex];
        data.c_JikH = mC_JkH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Phi_ikH = mC_PhikH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Phi_ikJ = mC_PhikJ[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
onStartRunlineEy(LocalDataEy & data, const SimpleAuxPMLRunline & rl)
{
    const int DIRECTION = 1;
    if (Z_ATTEN)
    {
        data.Phi_ij = &mAccumEj[DIRECTION][rl.auxIndex];
        data.c_JijH = mC_JjH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Phi_ijH = mC_PhijH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Phi_ijJ = mC_PhijJ[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
    }
    
    if (X_ATTEN)
    {
        data.Phi_ik = &mAccumEk[DIRECTION][rl.auxIndex];
        data.c_JikH = &mC_JkH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Phi_ikH = &mC_PhikH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Phi_ikJ = &mC_PhikJ[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
onStartRunlineEz(LocalDataEz & data, const SimpleAuxPMLRunline & rl)
{
    const int DIRECTION = 2;
    if (X_ATTEN)
    {
        data.Phi_ij = &mAccumEj[DIRECTION][rl.auxIndex];
        data.c_JijH = &mC_JjH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Phi_ijH = &mC_PhijH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Phi_ijJ = &mC_PhijJ[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
    }
    
    if (Y_ATTEN)
    {
        data.Phi_ik = &mAccumEk[DIRECTION][rl.auxIndex];
        data.c_JikH = mC_JkH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Phi_ikH = mC_PhikH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Phi_ikJ = mC_PhikJ[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
beforeUpdateEx(LocalDataEx & data, float Ei, float dHj, float dHk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
beforeUpdateEy(LocalDataEy & data, float Ei, float dHj, float dHk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
beforeUpdateEz(LocalDataEz & data, float Ei, float dHj, float dHk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline float CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
updateJx(LocalDataEx & data, float Ei, float dHj, float dHk)
{
    float Jij, Jik;
    
    if (Y_ATTEN)
    {
        Jij = data.c_JijH*dHk + *data.Phi_ij;
        *data.Phi_ij += (data.c_Phi_ijH*dHk - data.c_Phi_ijJ*Jij);
        data.Phi_ij++;
    }
    
    if (Z_ATTEN)
    {
        Jik = data.c_JikH*dHj + *data.Phi_ik;
        *data.Phi_ik += (data.c_Phi_ikH*dHj - data.c_Phi_ikJ*Jik);
        data.Phi_ik++;
    }
    
    if (Y_ATTEN && Z_ATTEN)
        return -Jij + Jik;
    else if (Y_ATTEN)
        return -Jij;
    else if (Z_ATTEN)
        return Jik;
    
    return 0.0;
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline float CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
updateJy(LocalDataEy & data, float Ei, float dHj, float dHk)
{
    float Jij, Jik;
    
    if (Z_ATTEN)
    {
        Jij = data.c_JijH*dHk + *data.Phi_ij;
        *data.Phi_ij += (data.c_Phi_ijH*dHk - data.c_Phi_ijJ*Jij);
        data.Phi_ij++;
    }
    
    if (X_ATTEN)
    {
        Jik = *data.c_JikH*dHj + *data.Phi_ik;
        *data.Phi_ik += (*data.c_Phi_ikH*dHj - *data.c_Phi_ikJ*Jik);
        data.Phi_ik++;
        data.c_JikH++;
        data.c_Phi_ikH++;
        data.c_Phi_ikJ++;
    }
    
    if (X_ATTEN && Z_ATTEN)
        return -Jij + Jik;
    else if (X_ATTEN)
        return Jik;
    else if (Z_ATTEN)
        return -Jij;
    return 0.0;
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline float CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
updateJz(LocalDataEz & data, float Ei, float dHj, float dHk)
{
    // Be careful here.  We return -Jij, not Jij.
    float Jij, Jik;
    
    if (X_ATTEN)
    {
        Jij = *data.c_JijH*dHk + *data.Phi_ij;
        *data.Phi_ij += (*data.c_Phi_ijH*dHk - *data.c_Phi_ijJ*Jij);
        data.Phi_ij++;
        data.c_JijH++;
        data.c_Phi_ijH++;
        data.c_Phi_ijJ++;
    }
    
    if (Y_ATTEN)
    {
        Jik = data.c_JikH*dHj + *data.Phi_ik;
        *data.Phi_ik += (data.c_Phi_ikH*dHj - data.c_Phi_ikJ*Jik);
        data.Phi_ik++;
    }
    
    if (X_ATTEN && Y_ATTEN)
        return -Jij + Jik;
    else if (X_ATTEN)
        return -Jij;
    else if (Y_ATTEN)
        return Jik;
    return 0.0;
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
afterUpdateEx(LocalDataEx & data, float Ei, float dHj, float dHk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
afterUpdateEy(LocalDataEy & data, float Ei, float dHj, float dHk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
afterUpdateEz(LocalDataEz & data, float Ei, float dHj, float dHk)
{
}





template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
initLocalHx(LocalDataHx & data)
{
    //LOG << X_ATTEN << " " << Y_ATTEN << " " << Z_ATTEN << "\n";
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
initLocalHy(LocalDataHy & data)
{
    //LOG << X_ATTEN << " " << Y_ATTEN << " " << Z_ATTEN << "\n";
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
initLocalHz(LocalDataHz & data)
{
    //LOG << X_ATTEN << " " << Y_ATTEN << " " << Z_ATTEN << "\n";
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
onStartRunlineHx(LocalDataHx & data, const SimpleAuxPMLRunline & rl)
{
    const int DIRECTION = 0;
    if (Y_ATTEN)
    {
        data.Psi_ij = &mAccumHj[DIRECTION][rl.auxIndex];
        data.c_MijE = mC_MjE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Psi_ijE = mC_PsijE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Psi_ijM = mC_PsijM[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
    }
    
    if (Z_ATTEN)
    {
        data.Psi_ik = &mAccumHk[DIRECTION][rl.auxIndex];
        data.c_MikE = mC_MkE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Psi_ikE = mC_PsikE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Psi_ikM = mC_PsikM[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
onStartRunlineHy(LocalDataHy & data, const SimpleAuxPMLRunline & rl)
{
    const int DIRECTION = 1;
    if (Z_ATTEN)
    {
        data.Psi_ij = &mAccumHj[DIRECTION][rl.auxIndex];
        data.c_MijE = mC_MjE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Psi_ijE = mC_PsijE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Psi_ijM = mC_PsijM[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
    }
    
    if (X_ATTEN)
    {
        data.Psi_ik = &mAccumHk[DIRECTION][rl.auxIndex];
        data.c_MikE = &mC_MkE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Psi_ikE = &mC_PsikE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Psi_ikM = &mC_PsikM[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
onStartRunlineHz(LocalDataHz & data, const SimpleAuxPMLRunline & rl)
{
    const int DIRECTION = 2;
    if (X_ATTEN)
    {
        data.Psi_ij = &mAccumHj[DIRECTION][rl.auxIndex];
        data.c_MijE = &mC_MjE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Psi_ijE = &mC_PsijE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        data.c_Psi_ijM = &mC_PsijM[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
    }
    
    if (Y_ATTEN)
    {
        data.Psi_ik = &mAccumHk[DIRECTION][rl.auxIndex];
        data.c_MikE = mC_MkE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Psi_ikE = mC_PsikE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        data.c_Psi_ikM = mC_PsikM[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
beforeUpdateHx(LocalDataHx & data, float Hi, float dEj, float dEk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
beforeUpdateHy(LocalDataHy & data, float Hi, float dEj, float dEk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
beforeUpdateHz(LocalDataHz & data, float Hi, float dEj, float dEk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline float CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
updateKx(LocalDataHx & data, float Hi, float dEj, float dEk)
{
    float Mij, Mik;
    if (Y_ATTEN)
    {
        Mij = data.c_MijE*dEk + *data.Psi_ij;
        *data.Psi_ij += (data.c_Psi_ijE*dEk - data.c_Psi_ijM*Mij);
        data.Psi_ij++;
    }
    
    if (Z_ATTEN)
    {
        Mik = data.c_MikE*dEj + *data.Psi_ik;
        *data.Psi_ik += (data.c_Psi_ikE*dEj - data.c_Psi_ikM*Mik);
        data.Psi_ik++;
    }
    
    if (Y_ATTEN && Z_ATTEN)
        return Mij - Mik;
    else if (Y_ATTEN)
        return Mij;
    else if (Z_ATTEN)
        return -Mik;
    return 0.0;
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline float CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
updateKy(LocalDataHy & data, float Hi, float dEj, float dEk)
{
    float Mij, Mik;
    
    if (Z_ATTEN)
    {
        Mij = data.c_MijE*dEk + *data.Psi_ij;
        *data.Psi_ij += (data.c_Psi_ijE*dEk - data.c_Psi_ijM*Mij);
        data.Psi_ij++;
    }
    
    if (X_ATTEN)
    {
        Mik = *data.c_MikE*dEj + *data.Psi_ik;
        *data.Psi_ik += (*data.c_Psi_ikE*dEj - *data.c_Psi_ikM*Mik);
        data.c_MikE++;
        data.c_Psi_ikE++;
        data.c_Psi_ikM++;
        data.Psi_ik++;
    }
    
    if (Z_ATTEN && X_ATTEN)
        return Mij - Mik;
    else if (Z_ATTEN)
        return Mij;
    else if (X_ATTEN)
        return -Mik;
    return 0.0;
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline float CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
updateKz(LocalDataHz & data, float Hi, float dEj, float dEk)
{
    float Mij, Mik;
    
    if (X_ATTEN)
    {
        Mij = *data.c_MijE*dEk + *data.Psi_ij;
        *data.Psi_ij += (*data.c_Psi_ijE*dEk - *data.c_Psi_ijM*Mij);
        data.c_MijE++;
        data.c_Psi_ijE++;
        data.c_Psi_ijM++;
        data.Psi_ij++;
    }
    
    if (Y_ATTEN)
    {
        Mik = data.c_MikE*dEj + *data.Psi_ik;
        *data.Psi_ik += (data.c_Psi_ikE*dEj - data.c_Psi_ikM*Mik);
        data.Psi_ik++;
    }
    
    if (X_ATTEN && Y_ATTEN)
        return Mij - Mik;
    else if (X_ATTEN)
        return Mij;
    else if (Y_ATTEN)
        return -Mik;
    return 0.0;
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
afterUpdateHx(LocalDataHx & data, float Hi, float dEj, float dEk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
afterUpdateHy(LocalDataHy & data, float Hi, float dEj, float dEk)
{
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
inline void CFSRIPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
afterUpdateHz(LocalDataHz & data, float Hi, float dEj, float dEk)
{
}




#include "CFSRIPML.cpp"
#endif
