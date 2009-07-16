/*
 *  StandardPML.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/15/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "StandardPML.h"
#include "calc.hh"
#include "Log.h"

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

#pragma mark *** SetupStandardPML ***


void SetupStandardPML::
setNumCellsE(int fieldDir, int numCells, Paint* parentPaint)
{
    const int STRIDE = 1;
    int jDir = (fieldDir+1)%3;
    int kDir = (jDir+1)%3;
    char fieldChar = '0' + fieldDir;
    Vector3i pmlDir = parentPaint->getPMLDirections();
    string prefix = parentPaint->getBulkMaterial()->getName() + " accum E";
    
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

void SetupStandardPML::
setNumCellsH(int fieldDir, int numCells, Paint* parentPaint)
{
    const int STRIDE = 1;
    int jDir = (fieldDir+1)%3;
    int kDir = (jDir+1)%3;
    char fieldChar = '0' + fieldDir;
    Vector3i pmlDir = parentPaint->getPMLDirections();
    string prefix = parentPaint->getBulkMaterial()->getName() + " accum H";
    
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

void SetupStandardPML::
setPMLHalfCells(int faceNum, Rect3i halfCellsOnSide,
    const GridDescription & gridDesc, Paint* parentPaint)
{
    Vector3i pmlDir = parentPaint->getPMLDirections();
    
    if (dot(pmlDir, cardinal(faceNum)) < 0) // check which side it is
        return;
    
    Rect3i pmlYee;
    int pmlDepthYee;
    int nYee, nHalf, nHalf0;
    int fieldDir;
    float depthHalf, depthFrac;
    float pmlDepthHalf = halfCellsOnSide.size(faceNum/2)+1;
    string alphaStr, kappaStr, sigmaStr;
    
    const int ONE = 1; // if I set it to 0, the PML layer includes depth==0.
    
    //LOG << "------------ " << parentPaint->getPMLDirections() << "\n";
    
    calc_defs::Calculator<float> calculator;
    calculator.set("eps0", Constants::eps0);
    calculator.set("mu0", Constants::mu0);
    calculator.set("L", pmlDepthHalf*gridDesc.getDxyz()[faceNum/2]);
    calculator.set("dx", gridDesc.getDxyz()[faceNum/2]);
    
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
        
        for (nYee = 0, nHalf=nHalf0; nYee < pmlDepthYee; nYee++, nHalf+=2)
        {
            if (faceNum%2 == 0) // going left/down etc. (negative direction)
                depthHalf = float(halfCellsOnSide.p2[faceNum/2]-nHalf+ONE);
            else
                depthHalf = float(nHalf-halfCellsOnSide.p1[faceNum/2]+ONE);
            depthFrac = depthHalf / pmlDepthHalf;
            calculator.set("d", depthFrac);
            /*
            LOG << "nYee " << nYee << " nHalf " << nHalf << " within "
                << halfCellsOnSide << " depth " << depthFrac << "\n";
            */
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
            
            /*
            mSigmaE[fieldDir][faceNum/2][nYee] = depthFrac;
            mKappaE[fieldDir][faceNum/2][nYee] = 1.0;
            mAlphaE[fieldDir][faceNum/2][nYee] = 0.0;
            */
        }
        /*
        LOG << "E: field " << fieldDir << " faceNum " << faceNum << " num "
            << pmlDepthYee << "\n";
        LOGMORE << "Sigma " << sigmaStr << "\n" <<
            mSigmaE[fieldDir][faceNum/2] << endl;
        LOGMORE << "Alpha " << alphaStr << "\n" <<
            mAlphaE[fieldDir][faceNum/2] << endl;
        LOGMORE << "Kappa " << kappaStr << "\n" <<
            mKappaE[fieldDir][faceNum/2] << endl;
        */
        
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
            /*
            LOG << "nYee " << nYee << " nHalf " << nHalf << " within "
                << halfCellsOnSide << "\n";
            */
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
            
            /*
            mSigmaH[fieldDir][faceNum/2][nYee] = depthFrac;
            mKappaH[fieldDir][faceNum/2][nYee] = 1.0;
            mAlphaH[fieldDir][faceNum/2][nYee] = 0.0;
            */
        }
        /*
        LOG << "H: field " << fieldDir << " faceNum " << faceNum << " num "
            << pmlDepthYee << "\n";
        LOGMORE << "Sigma " << sigmaStr << "\n" <<
            mSigmaH[fieldDir][faceNum/2] << endl;
        LOGMORE << "Alpha " << alphaStr << "\n" <<
            mAlphaH[fieldDir][faceNum/2] << endl;
        LOGMORE << "Kappa " << kappaStr << "\n" <<
            mKappaH[fieldDir][faceNum/2] << endl;
        */
    }
    
    if (ONE != 1)
        LOG << "Warning: ONE is not 1.\n";
}



#pragma mark *** StandardPML ***

StandardPML::
StandardPML(SetupStandardPML & setupPML, Paint* parentPaint) :
    mPMLDirection(parentPaint->getPMLDirections())
{
    // PML STUFF HERE
    
    // Grab memory buffers; we'll use these later to allocate the real fields.
    for (int nn = 0; nn < 3; nn++)
    {
        mBufAccumEj[nn] = setupPML.getBufAccumEj(nn);
        mBufAccumEk[nn] = setupPML.getBufAccumEk(nn);
        mBufAccumHj[nn] = setupPML.getBufAccumHj(nn);
        mBufAccumHk[nn] = setupPML.getBufAccumHk(nn);
    }
    
    //LOG << "------- " << pmlDir << "\n";
        
    // Allocate and calculate update constants.
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        int jDir = (fieldDir+1)%3;
        int kDir = (fieldDir+2)%3;
        
        if (mPMLDirection[jDir] != 0)
        {
            mC_JjH[fieldDir] = calcC_JH(setupPML.getKappaE(fieldDir,jDir),
                setupPML.getSigmaE(fieldDir,jDir), setupPML.getAlphaE(fieldDir,jDir),
                dt);
            mC_PhijH[fieldDir] = calcC_PhiH(setupPML.getKappaE(fieldDir,jDir),
                setupPML.getSigmaE(fieldDir,jDir), setupPML.getAlphaE(fieldDir,jDir),
                dt);
            mC_PhijJ[fieldDir] = calcC_PhiJ(setupPML.getKappaE(fieldDir,jDir),
                setupPML.getSigmaE(fieldDir,jDir), setupPML.getAlphaE(fieldDir,jDir),
                dt);
            
            // the magnetic coefficients are of the same form as the electric
            // ones and can be handled with the same functions 
            mC_MjE[fieldDir] = calcC_JH(setupPML.getKappaH(fieldDir,jDir),
                setupPML.getSigmaH(fieldDir,jDir), setupPML.getAlphaH(fieldDir,jDir),
                dt);
            mC_PsijE[fieldDir] = calcC_PhiH(setupPML.getKappaH(fieldDir,jDir),
                setupPML.getSigmaH(fieldDir,jDir), setupPML.getAlphaH(fieldDir,jDir),
                dt);
            mC_PsijM[fieldDir] = calcC_PhiJ(setupPML.getKappaH(fieldDir,jDir),
                setupPML.getSigmaH(fieldDir,jDir), setupPML.getAlphaH(fieldDir,jDir),
                dt);
            /*
            LOG << "SigmaE " << fieldDir << jDir << ":\n";
            LOGMORE << setupPML.getSigmaE(fieldDir,jDir) << "\n";
            LOG << "KappaE " << fieldDir << jDir << ":\n";
            LOGMORE << setupPML.getKappaE(fieldDir,jDir) << "\n";
            LOG << "AlphaE " << fieldDir << jDir << ":\n";
            LOGMORE << setupPML.getAlphaE(fieldDir,jDir) << "\n";
            
            
            LOG << "SigmaH " << fieldDir << jDir << ":\n";
            LOGMORE << setupPML.getSigmaH(fieldDir,jDir) << "\n";
            LOG << "KappaH " << fieldDir << jDir << ":\n";
            LOGMORE << setupPML.getKappaH(fieldDir,jDir) << "\n";
            LOG << "AlphaH " << fieldDir << jDir << ":\n";
            LOGMORE << setupPML.getAlphaH(fieldDir,jDir) << "\n";
            */
        }
        
        if (mPMLDirection[kDir] != 0)
        {
            mC_JkH[fieldDir] = calcC_JH(setupPML.getKappaE(fieldDir,kDir),
                setupPML.getSigmaE(fieldDir,kDir), setupPML.getAlphaE(fieldDir,kDir),
                dt);
            mC_PhikH[fieldDir] = calcC_PhiH(setupPML.getKappaE(fieldDir,kDir),
                setupPML.getSigmaE(fieldDir,kDir), setupPML.getAlphaE(fieldDir,kDir),
                dt);
            mC_PhikJ[fieldDir] = calcC_PhiJ(setupPML.getKappaE(fieldDir,kDir),
                setupPML.getSigmaE(fieldDir,kDir), setupPML.getAlphaE(fieldDir,kDir),
                dt);
            
            // the magnetic coefficients are of the same form as the electric
            // ones and can be handled with the same functions 
            mC_MkE[fieldDir] = calcC_JH(setupPML.getKappaH(fieldDir,kDir),
                setupPML.getSigmaH(fieldDir,kDir), setupPML.getAlphaH(fieldDir,kDir),
                dt);
            mC_PsikE[fieldDir] = calcC_PhiH(setupPML.getKappaH(fieldDir,kDir),
                setupPML.getSigmaH(fieldDir,kDir), setupPML.getAlphaH(fieldDir,kDir),
                dt);
            mC_PsikM[fieldDir] = calcC_PhiJ(setupPML.getKappaH(fieldDir,kDir),
                setupPML.getSigmaH(fieldDir,kDir), setupPML.getAlphaH(fieldDir,kDir),
                dt);
            /*
            LOG << "SigmaE " << fieldDir << kDir << ":\n";
            LOGMORE << setupPML.getSigmaE(fieldDir,kDir) << "\n";
            LOG << "KappaE " << fieldDir << kDir << ":\n";
            LOGMORE << setupPML.getKappaE(fieldDir,kDir) << "\n";
            LOG << "AlphaE " << fieldDir << kDir << ":\n";
            LOGMORE << setupPML.getAlphaE(fieldDir,kDir) << "\n";
            
            LOG << "SigmaH " << fieldDir << kDir << ":\n";
            LOGMORE << setupPML.getSigmaH(fieldDir,kDir) << "\n";
            LOG << "KappaH " << fieldDir << kDir << ":\n";
            LOGMORE << setupPML.getKappaH(fieldDir,kDir) << "\n";
            LOG << "AlphaH " << fieldDir << kDir << ":\n";
            LOGMORE << setupPML.getAlphaH(fieldDir,kDir) << "\n";
            */
        }
    }
}










