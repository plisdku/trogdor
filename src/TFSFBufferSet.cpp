/*
 *  TFSFBufferSet.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/20/08.
 *  Copyright 2008 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "TFSFBufferSet.h"

#include "SetupTFSFBufferSet.h"
#include "SetupGrid.h"
#include "Fields.h"

#include "Log.h"

#include <iomanip>

using namespace std;

TFSFBufferSet::Buffer::Buffer() :
	destRectYee(),
	srcRectYee(),
	sumField(0L),
	addendField(0L),
	type(kNoType),
	data(),
	filebuffer(),
	mainFields(0L),
	auxFields(0L),
	nx(0), ny(0), nz(0),
	nx_sum(0), ny_sum(0), nz_sum(0),
	nx_add(0), ny_add(0), nz_add(0)
{
}

TFSFBufferSet::
TFSFBufferSet(const SetupTFSFBufferSetPtr setupBufferSet, FieldsPtr mainFields,
	FieldsPtr auxFields) :
	mEx(6),
	mEy(6),
	mEz(6),
	mHx(6),
	mHy(6),
	mHz(6),
	mType(kLinkType),
	mFile()
{
   	LOGF << "Parameterized constructor (from grid).\n";
	//mainFields->sanityCheck();
	//LOG << "Description of setup buffer set:\n";
	//cout << setup->getDescription();
	
	// Set up a little lookup table in half-cell coordinates; you can access
	// the buffers by their offset in the Yee cell as though it were a 2x2x2
	// array.
	mBuffers[0] = 0;
	mBuffers[1] = &mEx;
	mBuffers[2] = &mEy;
	mBuffers[3] = &mHz;
	mBuffers[4] = &mEz;
	mBuffers[5] = &mHy;
	mBuffers[6] = &mHx;
	mBuffers[7] = 0;
	
	// destRectYee      Setup…Set::getYeeBufferRect(bufferNumber, field_pt)
	// srcRectYee       Setup…Set::getYeeAuxBufferRect(...)
	// sumField         Fields::getFieldByOffset(ii,jj,kk)
	// addendField		Fields::getFieldByOffset(ii,jj,kk)
	// type				Setup…Set::getBufferType(bufferNumber, field_pt)
	// data				allocate to size of destRectYee
	// nx, ny, nz		set to size of destRectYee
	
	for (int nn = 0; nn < 6; nn++)
	if (!setupBufferSet->omits(nn))
	{
		constructBuffer(mEx[nn], nn, setupBufferSet, mainFields, auxFields,
			Vector3i(1,0,0));
		constructBuffer(mEy[nn], nn, setupBufferSet, mainFields, auxFields,
			Vector3i(0,1,0));
		constructBuffer(mEz[nn], nn, setupBufferSet, mainFields, auxFields,
			Vector3i(0,0,1));
		constructBuffer(mHx[nn], nn, setupBufferSet, mainFields, auxFields,
			Vector3i(0,1,1));
		constructBuffer(mHy[nn], nn, setupBufferSet, mainFields, auxFields,
			Vector3i(1,0,1));
		constructBuffer(mHz[nn], nn, setupBufferSet, mainFields, auxFields,
			Vector3i(1,1,0));
	}
	
	//LOG << "Printing:\n";
	//print(cout);
}

TFSFBufferSet::
TFSFBufferSet(const SetupTFSFBufferSetPtr setupBufferSet, FieldsPtr mainFields,
	const string & fileName):
	mEx(6),
	mEy(6),
	mEz(6),
	mHx(6),
	mHy(6),
	mHz(6),
	mType(kFileType)
{
	LOGF << "Parameterized constructor (from file).\n";
	//mainFields->sanityCheck();
	//LOG << "Description of setup buffer set:\n";
	//cout << setup->getDescription();
	
	// Set up a little lookup table in half-cell coordinates; you can access
	// the buffers by their offset in the Yee cell as though it were a 2x2x2
	// array.
	mBuffers[0] = 0;
	mBuffers[1] = &mEx;
	mBuffers[2] = &mEy;
	mBuffers[3] = &mHz;
	mBuffers[4] = &mEz;
	mBuffers[5] = &mHy;
	mBuffers[6] = &mHx;
	mBuffers[7] = 0;
	
	//LOG << "Here.\n";
	// destRectYee      Setup…Set::getYeeBufferRect(bufferNumber, field_pt)
	// srcRectYee       Setup…Set::getYeeAuxBufferRect(...)
	// sumField         Fields::getFieldByOffset(ii,jj,kk)
	// addendField		Fields::getFieldByOffset(ii,jj,kk)
	// type				Setup…Set::getBufferType(bufferNumber, field_pt)
	// data				allocate to size of destRectYee
	// nx, ny, nz		set to size of destRectYee
	
	for (int nn = 0; nn < 6; nn++)
	if (!setupBufferSet->omits(nn))
	{
		//LOG << "Here, " << nn << "\n";
		constructBuffer(mEx[nn], nn, setupBufferSet, mainFields,
			Vector3i(1,0,0));
		//LOG << "Here, " << nn << "\n";
		constructBuffer(mEy[nn], nn, setupBufferSet, mainFields,
			Vector3i(0,1,0));
		//LOG << "Here, " << nn << "\n";
		constructBuffer(mEz[nn], nn, setupBufferSet, mainFields,
			Vector3i(0,0,1));
		//LOG << "Here, " << nn << "\n";
		constructBuffer(mHx[nn], nn, setupBufferSet, mainFields,
			Vector3i(0,1,1));
		//LOG << "Here, " << nn << "\n";
		constructBuffer(mHy[nn], nn, setupBufferSet, mainFields,
			Vector3i(1,0,1));
		//LOG << "Here, " << nn << "\n";
		constructBuffer(mHz[nn], nn, setupBufferSet, mainFields,
			Vector3i(1,1,0));
		//LOG << "Here, " << nn << "\n";
	}
	
	//LOG << "Here.\n";
	mFile.open(fileName.c_str());
	if (!mFile.good())
	{
		cerr << "Could not open source file " << fileName << ".\n";
		cerr << "Please generate fields data and re-run.\n";
		exit(1);
	}
	
	//LOG << "Printing:\n";
	//print(cout);
}

TFSFBufferSet::~TFSFBufferSet()
{
}

void TFSFBufferSet::
constructBuffer(Buffer & buffer, int bufferNumber, 
	const SetupTFSFBufferSetPtr setupBufferSet, FieldsPtr mainFields,
	FieldsPtr auxFields, const Vector3i & field_pt)
{
	buffer.destRectYee = setupBufferSet->getYeeBufferRect(bufferNumber,
		field_pt);
	buffer.srcRectYee = setupBufferSet->getYeeAuxBufferRect(bufferNumber,
		field_pt);
	buffer.sumField = mainFields->getFieldByOffset(field_pt);
	assert(buffer.sumField != 0L);
	buffer.addendField = auxFields->getFieldByOffset(field_pt);
	buffer.type = setupBufferSet->getBufferType(bufferNumber, field_pt);
	
	if (buffer.destRectYee.p1 == Vector3i(0,0,0) )
		int barbar = 0;
	
	int buffer_nx = buffer.destRectYee.p2[0] - buffer.destRectYee.p1[0] + 1;
	int buffer_ny = buffer.destRectYee.p2[1] - buffer.destRectYee.p1[1] + 1;
	int buffer_nz = buffer.destRectYee.p2[2] - buffer.destRectYee.p1[2] + 1;
	
	buffer.data.resize(buffer_nx*buffer_ny*buffer_nz);
	buffer.nx = buffer_nx;
	buffer.ny = buffer_ny;
	buffer.nz = buffer_nz;
	
	buffer.nx_sum = mainFields->get_nx();
	buffer.ny_sum = mainFields->get_ny();
	buffer.nz_sum = mainFields->get_nz();
	
	buffer.nx_add = auxFields->get_nx();
	buffer.ny_add = auxFields->get_ny();
	buffer.nz_add = auxFields->get_nz();
	
	buffer.mainFields = mainFields;
	buffer.auxFields = auxFields;
}

void TFSFBufferSet::
constructBuffer(Buffer & buffer, int bufferNumber, 
	const SetupTFSFBufferSetPtr setupBufferSet, FieldsPtr mainFields,
	const Vector3i & field_pt)
{
	buffer.destRectYee = setupBufferSet->getYeeBufferRect(bufferNumber,
		field_pt);
	buffer.srcRectYee = Rect3i(-1,-1,-1,-1,-1,-1);
	//buffer.srcRectYee = setupBufferSet->getYeeAuxBufferRect(bufferNumber,
	//	field_pt);
	buffer.sumField = mainFields->getFieldByOffset(field_pt);
	assert(buffer.sumField != 0L);
	buffer.addendField = 0L;
	//buffer.addendField = auxFields->getFieldByOffset(field_pt);
	buffer.type = setupBufferSet->getBufferType(bufferNumber, field_pt);
	
	int buffer_nx = buffer.destRectYee.p2[0] - buffer.destRectYee.p1[0] + 1;
	int buffer_ny = buffer.destRectYee.p2[1] - buffer.destRectYee.p1[1] + 1;
	int buffer_nz = buffer.destRectYee.p2[2] - buffer.destRectYee.p1[2] + 1;
	
	buffer.data.resize(buffer_nx*buffer_ny*buffer_nz);
	buffer.nx = buffer_nx;
	buffer.ny = buffer_ny;
	buffer.nz = buffer_nz;
	
	buffer.nx_sum = mainFields->get_nx();
	buffer.ny_sum = mainFields->get_ny();
	buffer.nz_sum = mainFields->get_nz();
	
	// I think that for AFP the buffer is the same size as the TF rect...
	// if it could be smaller by symmetry, I'd be using an auxilary grid.
	buffer.nx_add = buffer_nx;
	buffer.ny_add = buffer_ny;
	buffer.nz_add = buffer_nz;
	
	buffer.filebuffer.resize(buffer.nx_add*buffer.ny_add*buffer.nz_add);
}



float* TFSFBufferSet::
getBufferPointer(int bufferNumber, Vector3i fields_pt)
{
	fields_pt[0] %= 2;
	fields_pt[1] %= 2;
	fields_pt[2] %= 2;
	
	int fieldNum = fields_pt[0] + 2*fields_pt[1] + 4*fields_pt[2];
	assert(fieldNum > 0);
	assert(fieldNum < 7);
	
	vector<Buffer> & thisBuffer = *(mBuffers[fieldNum]);
	
	float* ptr = &(thisBuffer[bufferNumber].data[0]);
	assert(ptr != 0L);
	
	return ptr;
}


int TFSFBufferSet::
getNumBufferCells(int bufferNumber, Vector3i fields_pt) const
{
	fields_pt[0] %= 2;
	fields_pt[1] %= 2;
	fields_pt[2] %= 2;
	
	int fieldNum = fields_pt[0] + 2*fields_pt[1] + 4*fields_pt[2];
	assert(fieldNum > 0);
	assert(fieldNum < 7);
	
	vector<Buffer> & thisBuffer = *(mBuffers[fieldNum]);
	
	int nCells = thisBuffer[bufferNumber].nx * thisBuffer[bufferNumber].ny
		* thisBuffer[bufferNumber].nz;
	
	return nCells;
}

void TFSFBufferSet::
updateE()
{
	int ii;
	
	if (mType == kLinkType)
	{
		for (ii = 0; ii < 6; ii++)
			mEx[ii].updateWithAux();
		for (ii = 0; ii < 6; ii++)
			mEy[ii].updateWithAux();
		for (ii = 0; ii < 6; ii++)
			mEz[ii].updateWithAux();
	}
	else if (mType == kFileType)
	{
		for (ii = 0; ii < 6; ii++)
			mEx[ii].updateWithStream(mFile);
		for (ii = 0; ii < 6; ii++)
			mEy[ii].updateWithStream(mFile);
		for (ii = 0; ii < 6; ii++)
			mEz[ii].updateWithStream(mFile);
	}
	else
	{
		LOG << "Trouble.\n";
		assert(0);
	}
}

void TFSFBufferSet::
updateH()
{
	int ii;
	
	if (mType == kLinkType)
	{
		for (ii = 0; ii < 6; ii++)
			mHx[ii].updateWithAux();
		for (ii = 0; ii < 6; ii++)
			mHy[ii].updateWithAux();
		for (ii = 0; ii < 6; ii++)
			mHz[ii].updateWithAux();
	}
	else if (mType == kFileType)
	{
		for (ii = 0; ii < 6; ii++)
			mHx[ii].updateWithStream(mFile);
		for (ii = 0; ii < 6; ii++)
			mHy[ii].updateWithStream(mFile);
		for (ii = 0; ii < 6; ii++)
			mHz[ii].updateWithStream(mFile);
	}
	else
	{
		LOG << "Prollem.\n";
		assert(0);
	}
}
	
void TFSFBufferSet::
print(ostream & str) const
{
	/*
	str << "Buffer array sizes:\n";
	str << " Ex " << mEx.size() << "\n";
	str << " Ey " << mEy.size() << "\n";
	str << " Ez " << mEz.size() << "\n";
	str << " Hx " << mHx.size() << "\n";
	str << " Hy " << mHy.size() << "\n";
	str << " Hz " << mHz.size() << "\n";
	*/
	int ii;
	str << "BUFFERS:\n";
	for (ii = 0; ii < 6; ii++)
	{
		str << " Number " << ii << "\n";
		str << "Ex: \n";
		mEx[ii].print(str);
		str << "Ey: \n";
		mEy[ii].print(str);
		str << "Ez: \n";
		mEz[ii].print(str);
		str << "Hx: \n";
		mHx[ii].print(str);
		str << "Hy: \n";
		mHy[ii].print(str);
		str << "Hz: \n";
		mHz[ii].print(str);
	}
	
}	
	

void TFSFBufferSet::Buffer::
print(ostream & str) const
{
	if (type == kNoType)
	{
		str << "  Unused\n";
		return;
	}
	
	str << "  Dest rect " << destRectYee << "\n";
	str << "  Src rect " << srcRectYee << "\n";
	str << "  Sum field " << hex << sumField << dec << "\n";
	str << "  Addend field " << hex << addendField << dec << "\n";
	str << "  Type ";
	if (type == kTFType)
		str << " TF";
	else if (type == kSFType)
		str << " SF";
	else
		str << " none (ERROR!)";
	str << "\n";
	str << "  Data: size " << data.size() << " head " << hex << &(data[0])
		<< dec << "\n";
	str << "  Dimensions " << nx << " " << ny << " " << nz << "\n";
}

void TFSFBufferSet::Buffer::
updateWithAux()
{
	if (sumField == 0L)
		return;
	
	int ii,jj,kk;
	
	// Strides in the buffer, dest and addend (short names)
	int sx, sy, sz;
	int ssx, ssy, ssz;
	int sax, say, saz;
	
	sx = 1;
	sy = nx;
	sz = nx*ny;
	
	ssx = 1;
	ssy = nx_sum;
	ssz = nx_sum*ny_sum;
	
	sax = 1;
	say = nx_add;
	saz = nx_add*ny_add;
	
	if (srcRectYee.p1[0] == srcRectYee.p2[0])
		sax = 0;
	if (srcRectYee.p1[1] == srcRectYee.p2[1])
		say = 0;
	if (srcRectYee.p1[2] == srcRectYee.p2[2])
		saz = 0;
	
	// There are a pile of pointers involved.  Get this all straight.
	Vector3i & origin_dest = destRectYee.p1;
	Vector3i & origin_src = srcRectYee.p1;
	
	float* bufferPtr = &(data[0]);
	float* sumPtr = sumField + origin_dest[0]
		+ nx_sum*origin_dest[1]
		+ nx_sum*ny_sum*origin_dest[2];
	float* addPtr = addendField + origin_src[0]
		+ nx_add*origin_src[1]
		+ nx_add*ny_add*origin_src[2];
	
	float *bufferx, *buffery, *bufferz;
	float *sumx, *sumy, *sumz;
	float *addx, *addy, *addz;
	
	float sgn = (type == kTFType ? 1 : -1);
	
	// 1.  Copy current field data into the buffer.
		
	bufferz = bufferPtr;
	sumz = sumPtr;
	
	for (kk = 0; kk < nz; kk++)
	{
		buffery = bufferz;
		sumy = sumz;
		for (jj = 0; jj < ny; jj++)
		{
			bufferx = buffery;
			sumx = sumy;
			for (ii = 0; ii < nx; ii++)
			{
				//assert(sumx >= sumField &&
				//	sumx <= sumField + nx_sum*ny_sum*nz_sum);
				//assert(bufferx >= bufferPtr && bufferx <= bufferPtr+data.size());
				
				//assert(*bufferx == 0);
				//assert(*sumx == 0);
				
				//cout << "Buffer " << *bufferx << " becomes " << *sumx << "\n";
				
				/*
				if (ii + origin_dest[0] == 18)
				{
					cout << "jj " << origin_dest[1]+jj << " buffer " <<
						*bufferx << "\n";
				}
				*/
				//assert(!isnan(*bufferx));
				//assert(!isnan(*sumx));
				*bufferx = *sumx;
				//assert(!isnan(*bufferx));
				//assert(!isnan(*sumx));
				
				//*bufferx = 0; // DEBUG
				
				sumx += ssx;
				bufferx += sx;
			}
			sumy += ssy;
			buffery += sy;
		}
		sumz += ssz;
		bufferz += sz;
	}
	
	// 2.  Add or subtract addend data into the buffer
	
	
	//print(cout);
	
	bufferz = bufferPtr;
	addz = addPtr;
	
	for (kk = 0; kk < nz; kk++)
	{
		buffery = bufferz;
		addy = addz;
		for (jj = 0; jj < ny; jj++)
		{
			bufferx = buffery;
			addx = addy;
			for (ii = 0; ii < nx; ii++)
			{
				//assert(addx >= addendField &&
				//	addx <= addendField + nx_add*ny_add*nz_add);
				//assert(bufferx >= bufferPtr && bufferx <= bufferPtr+data.size());
				
				//assert(*bufferx == 0);
				//assert(*addx == 0);
				
				//cout << "Buffer " << *bufferx << " adds " << sgn << 
				//	" times " << *addx << "\n";
				/*
				if (ii + origin_dest[0] == 16)
				{
					cout << "jj " << origin_dest[1]+jj << " buffer "
						<< *bufferx << " adds " <<
						*addx*sgn << " to " << *bufferx + *addx*sgn << "\n";
				}
				*/
				//assert(!isnan(*bufferx));
				//assert(!isnan(*addx));
				*bufferx += *addx*sgn;
				//assert(!isnan(*bufferx));
				//assert(!isnan(*addx));
				
				//cout << " to " << *bufferx << "\n";
				
				addx += sax;
				bufferx += sx;
			}
			addy += say;
			buffery += sy;
		}
		addz += saz;
		bufferz += sz;
	}
}


void TFSFBufferSet::Buffer::
updateWithStream(istream & str)
{
	if (sumField == 0L)
		return;
	
	int ii,jj,kk;
	
	// Strides in the buffer, dest and addend (short names)
	int sx, sy, sz;
	int ssx, ssy, ssz;
	int sax, say, saz;
	
	sx = 1;
	sy = nx;
	sz = nx*ny;
	
	ssx = 1;
	ssy = nx_sum;
	ssz = nx_sum*ny_sum;
	
	sax = 1;
	say = nx_add;
	saz = nx_add*ny_add;
	
	// There are a pile of pointers involved.  Get this all straight.
	Vector3i & origin_dest = destRectYee.p1;
	
	float* bufferPtr = &(data[0]);
	float* sumPtr = sumField + origin_dest[0]
		+ nx_sum*origin_dest[1]
		+ nx_sum*ny_sum*origin_dest[2];
	
	float* addPtr = &(filebuffer[0]);
	
	float *bufferx, *buffery, *bufferz;
	float *sumx, *sumy, *sumz;
	float *addx, *addy, *addz;
	
	float sgn = (type == kTFType ? 1 : -1);
	
	// 1.  Copy current field data into the buffer.
	
	bufferz = bufferPtr;
	sumz = sumPtr;
	
	for (kk = 0; kk < nz; kk++)
	{
		buffery = bufferz;
		sumy = sumz;
		for (jj = 0; jj < ny; jj++)
		{
			bufferx = buffery;
			sumx = sumy;
			for (ii = 0; ii < nx; ii++)
			{
				//assert(sumx >= sumField &&
				//	sumx <= sumField + nx_sum*ny_sum*nz_sum);
				//assert(bufferx >= bufferPtr && bufferx <= bufferPtr+data.size());
				
				//assert(*bufferx == 0);
				//assert(*sumx == 0);
				
				//cout << "Buffer " << *bufferx << " becomes " << *sumx << "\n";
				
				*bufferx = *sumx;
				
				sumx += ssx;
				bufferx += sx;
			}
			sumy += ssy;
			buffery += sy;
		}
		sumz += ssz;
		bufferz += sz;
	}
	
	// 2a.  Read addend data from the file.
	
	str.read((char*)addPtr, sizeof(float)*filebuffer.size());
	
	// 2b.  Add or subtract addend data into the buffer
	
	bufferz = bufferPtr;
	//addz = addPtr;
	
	for (kk = 0; kk < nz; kk++)
	{
		buffery = bufferz;
		//addy = addz;
		for (jj = 0; jj < ny; jj++)
		{
			bufferx = buffery;
			//addx = addy;
			for (ii = 0; ii < nx; ii++)
			{
				/*
				assert(addx >= addendField &&
					addx <= addendField + nx_add*ny_add*nz_add);
				assert(bufferx >= bufferPtr && bufferx <= bufferPtr+data.size());
				*/
				//assert(*bufferx == 0);
				//assert(*addx == 0);
				
				//cout << "Buffer " << *bufferx << " adds " << sgn << 
				//	" times " << *addx;
				
				//*bufferx += *addx*sgn;
				*bufferx += *addPtr*sgn;
				
				//cout << " to " << *bufferx << "\n";
				
				//addx += sax;
				addPtr++;
				bufferx += sx;
			}
			//addy += say;
			buffery += sy;
		}
		//addz += saz;
		bufferz += sz;
	}
}



















	
	