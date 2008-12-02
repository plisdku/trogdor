/*
 *  StaticDielectricPML.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/16/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef _STATICLOSSYDIELECTRICPML_
#define _STATICLOSSYDIELECTRICPML_

//  Project headers
#include "MaterialModel.h"
#include "Runline.h"
#include "geometry.h"

//  C++ headers
#include <string>
#include "Map.h"

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
class StaticLossyDielectricPML: public MaterialModel
{
public:
    StaticLossyDielectricPML(const Map<std::string, std::string> & inParams,
        Vector3i inDirection, const Rect3i & inPMLExtent, float dx, float dy,
        float dz, float dt);
    
    virtual ~StaticLossyDielectricPML();
    
    virtual void onAddRunline(Runline & rl, int ii, int jj, int kk);
    
    virtual void allocate(float dx, float dy, float dz, float dt);
    virtual std::string getMaterialName() const;
	
	virtual void calcEx(float dxinv, float dyinv, float dzinv, float dt,
		int start = 0, int stride = 1);

	virtual void calcEy(float dxinv, float dyinv, float dzinv, float dt,
		int start = 0, int stride = 1);

	virtual void calcEz(float dxinv, float dyinv, float dzinv, float dt,
		int start = 0, int stride = 1);

	virtual void calcHx(float dxinv, float dyinv, float dzinv, float dt,
		int start = 0, int stride = 1);

	virtual void calcHy(float dxinv, float dyinv, float dzinv, float dt,
		int start = 0, int stride = 1);

	virtual void calcHz(float dxinv, float dyinv, float dzinv, float dt,
		int start = 0, int stride = 1);
        
protected:
    Vector3b mAttenuation;
	
	/*
	template<int DIR, bool ATTENX, bool ATTENY, bool ATTENZ>
	void stepE(const Runline & rl, float dxinv1, float dxinv2, float dxinv3,
		float dt);
		
	template<int DIR, bool ATTENX, bool ATTENY, bool ATTENZ>
	void stepH(const Runline & rl, float dxinv1, float dxinv2, float dxinv3,
		float dt);
	*/
	
	void stepEx(const Runline & rl, float dx, float dy, float dz,
        float dt);
    void stepEy(const Runline & rl, float dx, float dy, float dz,
        float dt);
    void stepEz(const Runline & rl, float dx, float dy, float dz,
        float dt);
    void stepHx(const Runline & rl, float dx, float dy, float dz,
        float dt);
    void stepHy(const Runline & rl, float dx, float dy, float dz,
        float dt);
    void stepHz(const Runline & rl, float dx, float dy, float dz,
        float dt);
	
	
    void setConductivity(int halfCells, float* & sigEx, float* & sigEy,
                         float* & sigEz, float* & sigHx, float* & sigHy,
                         float* & sigHz);
    
    Rect3i mExtents;
	
	// these are the auxiliary fields.
	std::vector<float> mAccumEj[3];
	std::vector<float> mAccumEk[3];
	std::vector<float> mAccumHj[3];
	std::vector<float> mAccumHk[3];
	
	// these are the PML constants.
	// unlike the auxiliary fields, these are indexed along either the X,
	// Y or Z direction, so their dimensions are the appropriate PML
	// dimensions.
	std::vector<float> mAEj[3];
	std::vector<float> mAEk[3];
	std::vector<float> mAHj[3];
	std::vector<float> mAHk[3];
	std::vector<float> mBEj[3];
	std::vector<float> mBEk[3];
	std::vector<float> mBHj[3];
	std::vector<float> mBHk[3];
	std::vector<float> mKEj[3];
	std::vector<float> mKEk[3];
	std::vector<float> mKHj[3];
	std::vector<float> mKHk[3];
    
    float m_epsr;
    float m_mur;
    float m_sigma;
    
};

#include "StaticLossyDielectricPML.hh"


#endif
