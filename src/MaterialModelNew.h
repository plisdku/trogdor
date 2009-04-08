/*
 *  MaterialModelNew.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/2/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MATERIALMODELNEW_
#define _MATERIALMODELNEW_

#include "MemoryUtilities.h"
#include "Pointer.h"
#include <vector>

class SetupRunlineBase
{
public:
private:
	BufferPointer fi[2];
	BufferPointer fj[2];
	BufferPointer fk[2];
	BufferPointer fHere;
};

class RunlineBase
{
};

class RunlineEncoderBase
{
public:
	virtual void startRunline(int position, int whateverElse);
	virtual void endRunline();
	virtual bool canContinueRunline(int pos, int grid, int spaceVar, int etc);
private:
};

class MaterialModel
{
public:
	virtual RunlineEncoderBase& getEncoder() const;
	virtual RunlineBase* newRunline() const;
	virtual SetupRunlineBase* newSetupRunline() const;
	
	virtual void setupEncoder();
	virtual void clearSetup();
	virtual void makeRealRunlines();
private:
};


#endif
