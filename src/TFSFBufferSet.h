/*
 *  TFSFBufferSet.h
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

#ifndef _TFSFBUFFERSET_H_
#define _TFSFBUFFERSET_H_

#include "geometry.h"
#include "SetupConstants.h"

#include "Pointer.h"

#include <iostream>
#include <istream>
#include <vector>

class SetupTFSFBufferSet;
typedef Pointer<SetupTFSFBufferSet> SetupTFSFBufferSetPtr;

class Fields;
typedef Pointer<Fields> FieldsPtr;

class TFSFBufferSet
{
private:
	struct Buffer
	{
		Buffer();
		void print(std::ostream & str) const;
		void updateWithAux();
		void updateWithStream(std::istream & str);
		Rect3i destRectYee;
		Rect3i srcRectYee;  // may be unused
		float* sumField;
		float* addendField;  // may be unused
		TFSFType type;
		std::vector<float> data;
		std::vector<float> filebuffer;
		
		FieldsPtr mainFields;
		FieldsPtr auxFields;
		
		// Size of the buffer itself
		int nx;
		int ny;
		int nz;
		
		// Size of the sum field
		int nx_sum;
		int ny_sum;
		int nz_sum;
		
		// Size of the addend field (may be unused)
		int nx_add;
		int ny_add;
		int nz_add;
	};
	
public:
	//TFSFBufferSet();
	TFSFBufferSet(const SetupTFSFBufferSetPtr setup, FieldsPtr mainFields,
		FieldsPtr auxFields);
	TFSFBufferSet(const SetupTFSFBufferSetPtr setup, FieldsPtr mainFields,
		const std::string & fileName);
	
	~TFSFBufferSet();
	
	float* getBufferPointer(int bufferNumber, Vector3i fields_pt);
	int getNumBufferCells(int bufferNumber, Vector3i fields_pt) const;
	
	void updateE();
	void updateH();
	
	void print(std::ostream & str) const;
	
private:
	void constructBuffer(Buffer & buffer, int bufferNumber,
		const SetupTFSFBufferSetPtr setup,
		FieldsPtr mainFields, FieldsPtr auxFields, const Vector3i & field_pt);
	
	void constructBuffer(Buffer & buffer, int bufferNumber,
		const SetupTFSFBufferSetPtr setup,
		FieldsPtr mainFields, const Vector3i & field_pt);
	
	void printBuffer(Buffer & buffer, std::ostream & str) const;
	
	std::vector<Buffer> mEx;
	std::vector<Buffer> mEy;
	std::vector<Buffer> mEz;
	
	std::vector<Buffer> mHx;
	std::vector<Buffer> mHy;
	std::vector<Buffer> mHz;
	
	std::vector<Buffer>* mBuffers[8];
	
	TFSFBufferType mType;
	
	// File stream is only used for kFileType buffers.
	std::ifstream mFile;
	
};
typedef Pointer<TFSFBufferSet> TFSFBufferSetPtr;

#endif
