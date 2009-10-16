/*
 *  CFSRIPML-inl.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/25/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

template <bool I_ATTEN, bool J_ATTEN, bool K_ATTEN>
CFSRIPML<I_ATTEN, J_ATTEN, K_ATTEN>::
CFSRIPML(Paint* parentPaint, std::vector<long> numCellsE,
        std::vector<long> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection ) :
    CFSRIPMLBase(parentPaint, numCellsE, numCellsH, pmlHalfCells,
        pmlParams, dxyz, dt, runlineDirection)
{
}


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

