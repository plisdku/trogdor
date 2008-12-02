/*
 *  StaticDielectricModel.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/10/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _STATICDIELECTRICMODEL_
#define _STATICDIELECTRICMODEL_


//  Project headers
#include "MaterialModel.h"
#include "Runline.h"
#include "geometry.h"

//  C++ headers
#include <string>
#include "Map.h"

class StaticDielectricModel: public MaterialModel
{
public:
    StaticDielectricModel(const Map<std::string, std::string> & inParams);
    
    
    virtual void onAddRunline(Runline & rl, int ii, int jj, int kk);
	
    virtual void allocate(float dx, float dy, float dz, float dt);
    virtual std::string getMaterialName() const;
    
    virtual MaterialModel* createPML(Vector3i direction, Rect3i activeRect,
        Rect3i regionOfInterest, float dx, float dy, float dz, float dt) const;
	
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
    
	void stepE(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt);
	void stepH(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt);
	
	/*
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
    */
	
    float m_epsr;
    float m_mur;
    
};

#endif
