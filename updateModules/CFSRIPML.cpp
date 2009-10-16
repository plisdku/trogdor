/*
 *  CFSRIPML.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "CFSRIPML.h"

#include "calc.hh"
#include "Log.h"
#include "SimulationDescription.h"
#include "Paint.h"
#include "PhysicalConstants.h"
#include "YeeUtilities.h"
#include "Map.h"

using namespace std;
using namespace YeeUtilities;

static vector<float>
calcC_JH(const vector<float> & kappa,
    const vector<float> & sigma, const vector<float> & alpha, float dt)
{
    assert(kappa.size() == sigma.size() && sigma.size() == alpha.size());
    vector<float> c(sigma.size());
    for (unsigned int nn = 0; nn < sigma.size(); nn++)
    {
        c[nn] = ( Constants::eps0 + 0.5*alpha[nn]*dt ) /
            ( kappa[nn]*Constants::eps0 +
            0.5*dt*(kappa[nn]*alpha[nn]+sigma[nn]) )
            - 1.0;
    }
    return c;
}

static vector<float>
calcC_PhiH(const vector<float> & kappa,
    const vector<float> & sigma, const vector<float> & alpha, float dt)
{
    assert(kappa.size() == sigma.size() && sigma.size() == alpha.size());
    vector<float> c(sigma.size());
    for (unsigned int nn = 0; nn < sigma.size(); nn++)
    {
        c[nn] = dt*( (1.0-kappa[nn])*alpha[nn] - sigma[nn] ) /
            (kappa[nn]*Constants::eps0 +
            0.5*dt*(kappa[nn]*alpha[nn]+sigma[nn]));
    }
    return c;
}

static vector<float>
calcC_PhiJ(const vector<float> & kappa,
    const vector<float> & sigma, const vector<float> & alpha, float dt)
{
    assert(kappa.size() == sigma.size() && sigma.size() == alpha.size());
    vector<float> c(sigma.size());
    for (unsigned int nn = 0; nn < sigma.size(); nn++)
    {
        c[nn] = dt*( kappa[nn]*alpha[nn] + sigma[nn] ) /
            ( kappa[nn]*Constants::eps0 +
                0.5*dt*(kappa[nn]*alpha[nn]+sigma[nn]) );
    }
    return c;
}

#pragma mark *** CFSRIPMLBase ***

CFSRIPMLBase::
CFSRIPMLBase(Paint* parentPaint, std::vector<long> numCellsE,
        std::vector<long> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection ) :
    mPMLParams(pmlParams),
    mPMLDirection(parentPaint->pmlDirections()),
    mDxyz(dxyz),
    mDt(dt),
    mRunlineDirection(runlineDirection)
{
    assert(runlineDirection >= 0 && runlineDirection < 3);
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        setNumCellsE(xyz, numCellsE[xyz], parentPaint);
        setNumCellsH(xyz, numCellsH[xyz], parentPaint);
    }
    for (int xyz = 0; xyz < 6; xyz++)
    {
        setPMLHalfCells(xyz, pmlHalfCells[xyz], parentPaint);
    }
    
//    LOG << "------- " << mPMLDirection << "\n";
        
    // Allocate and calculate update constants.
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        int jDir = (fieldDir+1)%3;
        int kDir = (fieldDir+2)%3;
        
        if (mPMLDirection[jDir] != 0)
        {
            mC_JjH[fieldDir] = calcC_JH(mKappaE[fieldDir][jDir],
                mSigmaE[fieldDir][jDir], mAlphaE[fieldDir][jDir],
                dt);
            mC_PhijH[fieldDir] = calcC_PhiH(mKappaE[fieldDir][jDir],
                mSigmaE[fieldDir][jDir], mAlphaE[fieldDir][jDir],
                dt);
            mC_PhijJ[fieldDir] = calcC_PhiJ(mKappaE[fieldDir][jDir],
                mSigmaE[fieldDir][jDir], mAlphaE[fieldDir][jDir],
                dt);
            
            // the magnetic coefficients are of the same form as the electric
            // ones and can be handled with the same functions 
            mC_MjE[fieldDir] = calcC_JH(mKappaH[fieldDir][jDir],
                mSigmaH[fieldDir][jDir], mAlphaH[fieldDir][jDir],
                dt);
            mC_PsijE[fieldDir] = calcC_PhiH(mKappaH[fieldDir][jDir],
                mSigmaH[fieldDir][jDir], mAlphaH[fieldDir][jDir],
                dt);
            mC_PsijM[fieldDir] = calcC_PhiJ(mKappaH[fieldDir][jDir],
                mSigmaH[fieldDir][jDir], mAlphaH[fieldDir][jDir],
                dt);
            
//            LOG << mC_JjH[fieldDir] << "\n";
//            LOG << mC_PhijH[fieldDir] << "\n";
//            LOG << mC_PhijJ[fieldDir] << "\n";
//            LOG << mC_MjE[fieldDir] << "\n";
//            LOG << mC_PsijE[fieldDir] << "\n";
//            LOG << mC_PsijM[fieldDir] << "\n";
            
        }
        
        if (mPMLDirection[kDir] != 0)
        {
            mC_JkH[fieldDir] = calcC_JH(mKappaE[fieldDir][kDir],
                mSigmaE[fieldDir][kDir], mAlphaE[fieldDir][kDir],
                dt);
            mC_PhikH[fieldDir] = calcC_PhiH(mKappaE[fieldDir][kDir],
                mSigmaE[fieldDir][kDir], mAlphaE[fieldDir][kDir],
                dt);
            mC_PhikJ[fieldDir] = calcC_PhiJ(mKappaE[fieldDir][kDir],
                mSigmaE[fieldDir][kDir], mAlphaE[fieldDir][kDir],
                dt);
            
            // the magnetic coefficients are of the same form as the electric
            // ones and can be handled with the same functions 
            mC_MkE[fieldDir] = calcC_JH(mKappaH[fieldDir][kDir],
                mSigmaH[fieldDir][kDir], mAlphaH[fieldDir][kDir],
                dt);
            mC_PsikE[fieldDir] = calcC_PhiH(mKappaH[fieldDir][kDir],
                mSigmaH[fieldDir][kDir], mAlphaH[fieldDir][kDir],
                dt);
            mC_PsikM[fieldDir] = calcC_PhiJ(mKappaH[fieldDir][kDir],
                mSigmaH[fieldDir][kDir], mAlphaH[fieldDir][kDir],
                dt);
            
            
//            LOG << mC_JkH[fieldDir] << "\n";
//            LOG << mC_PhikH[fieldDir] << "\n";
//            LOG << mC_PhikJ[fieldDir] << "\n";
//            LOG << mC_MkE[fieldDir] << "\n";
//            LOG << mC_PsikE[fieldDir] << "\n";
//            LOG << mC_PsikM[fieldDir] << "\n";
            
        }
    }
}

void CFSRIPMLBase::
setNumCellsE(int fieldDir, int numCells, Paint* parentPaint)
{
    const int STRIDE = 1;
    int jDir = (fieldDir+1)%3;
    int kDir = (jDir+1)%3;
    char fieldChar = '0' + fieldDir;
    Vector3i pmlDir = parentPaint->pmlDirections();
    string prefix = parentPaint->bulkMaterial()->name() + " accum E";
    
    if (pmlDir[jDir])
    {
        mBufAccumEj[fieldDir] = MemoryBufferPtr(
            new MemoryBuffer( prefix + 'j' + fieldChar, numCells, STRIDE ) );
    }
    if (pmlDir[kDir])
    {
        mBufAccumEk[fieldDir] = MemoryBufferPtr(
            new MemoryBuffer( prefix + 'k' + fieldChar, numCells, STRIDE ) );
    }
}

void CFSRIPMLBase::
setNumCellsH(int fieldDir, int numCells, Paint* parentPaint)
{
    const int STRIDE = 1;
    int jDir = (fieldDir+1)%3;
    int kDir = (jDir+1)%3;
    char fieldChar = '0' + fieldDir;
    Vector3i pmlDir = parentPaint->pmlDirections();
    string prefix = parentPaint->bulkMaterial()->name() + " accum H";
    
    if (pmlDir[jDir])
    {
        mBufAccumHj[fieldDir] = MemoryBufferPtr(
            new MemoryBuffer( prefix + 'j' + fieldChar, numCells, STRIDE ) );
    }
    if (pmlDir[kDir])
    {
        mBufAccumHk[fieldDir] = MemoryBufferPtr(
            new MemoryBuffer( prefix + 'k' + fieldChar, numCells, STRIDE ) );
    }
}

// This entire function is unaffected by the memory allocation direction.
void CFSRIPMLBase::
setPMLHalfCells(int faceNum, Rect3i halfCellsOnSide,
    Paint* parentPaint)
{
    Vector3i pmlDir = parentPaint->pmlDirections();
    
    if (dot(pmlDir, cardinal(faceNum)) <= 0) // check which side it is
        return;
    
    Rect3i pmlYee;
    int pmlDepthYee;
    int nYee, nHalf, nHalf0;
    int fieldDir;
    float depthHalf, depthFrac;
    float pmlDepthHalf = halfCellsOnSide.size(faceNum/2)+1;
    string alphaStr, kappaStr, sigmaStr;
    
    const int ONE = 1; // if I set it to 0, the PML layer includes depth==0.
    
    //LOG << "------------ " << parentPaint->pmlDirections() << "\n";
    
    calc_defs::Calculator<float> calculator;
    calculator.set("eps0", Constants::eps0);
    calculator.set("mu0", Constants::mu0);
    calculator.set("L", pmlDepthHalf*mDxyz[faceNum/2]);
    calculator.set("dx", mDxyz[faceNum/2]);
    
    for (fieldDir = 0; fieldDir < 3; fieldDir++)
    if (fieldDir != faceNum/2)
    {
        // E field auxiliary constants
        pmlYee = halfToYee(halfCellsOnSide, octantE(fieldDir));
        pmlDepthYee = pmlYee.size(faceNum/2)+1;
        mSigmaE[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mAlphaE[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mKappaE[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        
        // the first half cell of the current octant.  we jump through hoops
        // to make sure that it has the right half cell offset.
        nHalf0 = yeeToHalf(pmlYee, octantE(fieldDir)).p1[faceNum/2];
        
        alphaStr = mPMLParams[cardinal(faceNum)]["alpha"];
        kappaStr = mPMLParams[cardinal(faceNum)]["kappa"];
        sigmaStr = mPMLParams[cardinal(faceNum)]["sigma"];
        /*
        LOG << "Alpha: " << alphaStr << "\n"
            << "Kappa: " << kappaStr << "\n"
            << "Sigma: " << sigmaStr << "\n";
        */
        for (nYee = 0, nHalf=nHalf0; nYee < pmlDepthYee; nYee++, nHalf+=2)
        {
            if (faceNum%2 == 0) // going left/down etc. (negative direction)
                depthHalf = float(halfCellsOnSide.p2[faceNum/2]-nHalf+ONE);
            else
                depthHalf = float(nHalf-halfCellsOnSide.p1[faceNum/2]+ONE);
            depthFrac = depthHalf / pmlDepthHalf;
            calculator.set("d", depthFrac);
            
//            LOG << "nYee " << nYee << " nHalf " << nHalf << " within "
//                << halfCellsOnSide << " depth " << depthFrac << "\n";
            
            bool parseError = calculator.parse(sigmaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mSigmaE[fieldDir][faceNum/2][nYee] = calculator.get_value();
            parseError = calculator.parse(kappaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mKappaE[fieldDir][faceNum/2][nYee] = calculator.get_value();
            parseError = calculator.parse(alphaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mAlphaE[fieldDir][faceNum/2][nYee] = calculator.get_value();
        }
        
//        LOG << "E: field " << fieldDir << " faceNum " << faceNum << " num "
//            << pmlDepthYee << "\n";
//        LOGMORE << "Sigma " << sigmaStr << "\n" <<
//            mSigmaE[fieldDir][faceNum/2] << endl;
//        LOGMORE << "Alpha " << alphaStr << "\n" <<
//            mAlphaE[fieldDir][faceNum/2] << endl;
//        LOGMORE << "Kappa " << kappaStr << "\n" <<
//            mKappaE[fieldDir][faceNum/2] << endl;

        // H field auxiliary constants
        pmlYee = halfToYee(halfCellsOnSide, octantH(fieldDir));
        pmlDepthYee = pmlYee.size(faceNum/2)+1;
        mSigmaH[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mAlphaH[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mKappaH[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        
        nHalf0 = yeeToHalf(pmlYee, octantH(fieldDir)).p1[faceNum/2];
        
        for (nYee = 0, nHalf=nHalf0; nYee < pmlDepthYee; nYee++, nHalf+=2)
        {
            if (faceNum%2 == 0) // going left/down etc. (negative direction)
                depthHalf = float(halfCellsOnSide.p2[faceNum/2]-nHalf+ONE);
            else
                depthHalf = float(nHalf-halfCellsOnSide.p1[faceNum/2]+ONE);
            depthFrac = depthHalf / pmlDepthHalf;
            calculator.set("d", depthFrac);
            
//            LOG << "nYee " << nYee << " nHalf " << nHalf << " within "
//                << halfCellsOnSide << " depth " << depthFrac << "\n";
            
            bool parseError = calculator.parse(sigmaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mSigmaH[fieldDir][faceNum/2][nYee] = calculator.get_value();
            parseError = calculator.parse(kappaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mKappaH[fieldDir][faceNum/2][nYee] = calculator.get_value();
            parseError = calculator.parse(alphaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mAlphaH[fieldDir][faceNum/2][nYee] = calculator.get_value();
        }
//        LOG << "H: field " << fieldDir << " faceNum " << faceNum << " num "
//            << pmlDepthYee << "\n";
//        LOGMORE << "Sigma " << sigmaStr << "\n" <<
//            mSigmaH[fieldDir][faceNum/2] << endl;
//        LOGMORE << "Alpha " << alphaStr << "\n" <<
//            mAlphaH[fieldDir][faceNum/2] << endl;
//        LOGMORE << "Kappa " << kappaStr << "\n" <<
//            mKappaH[fieldDir][faceNum/2] << endl;
    }
    
    if (ONE != 1)
        LOG << "Warning: ONE is not 1.\n";
}


void CFSRIPMLBase::
allocateAuxBuffers()
{
    Vector3i rotatedPMLDir = cyclicPermute(mPMLDirection,
        (3-mRunlineDirection)%3);
    // Allocate auxiliary variables
    MemoryBufferPtr p;
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        int jDir = (fieldDir+1)%3;
        int kDir = (fieldDir+2)%3;
        
        if (mPMLDirection[jDir] != 0)
        {
            p = mBufAccumEj[fieldDir];
            mAccumEj[fieldDir].resize(p->length());
            p->setHeadPointer(&(mAccumEj[fieldDir][0]));
            
            p = mBufAccumHj[fieldDir];
            mAccumHj[fieldDir].resize(p->length());
            p->setHeadPointer(&(mAccumHj[fieldDir][0]));
        }
        
        if (mPMLDirection[kDir] != 0)
        {
            p = mBufAccumEk[fieldDir];
            mAccumEk[fieldDir].resize(p->length());
            p->setHeadPointer(&(mAccumEk[fieldDir][0]));
            
            p = mBufAccumHk[fieldDir];
            mAccumHk[fieldDir].resize(p->length());
            p->setHeadPointer(&(mAccumHk[fieldDir][0]));
        }
    }
}
