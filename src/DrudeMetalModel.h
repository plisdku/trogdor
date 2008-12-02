/*
 *  DrudeMetalModel.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/21/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _DRUDEMETALMODEL_
#define _DRUDEMETALMODEL_


#include "MaterialModel.h"
#include "Runline.h"
#include "geometry.h"

#include <string>
#include "Map.h"

class DrudeMetalModel: public MaterialModel
{
public:
    DrudeMetalModel(const Map<std::string, std::string> & inParams);
    virtual ~DrudeMetalModel();
    
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
    
	void stepE_orig(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	/*
	void stepE_orig_unrolled(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	void stepE_orig_unrolled_array(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	
	template<int PREFETCH>
	void stepE_orig_prefetch(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	template<int PREFETCH>
	void stepE_orig_unrolled_prefetch(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
		
	
	void stepE_split(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	void stepE_split_unrolled(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	template<int PREFETCH>
	void stepE_split_prefetch(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	template<int PREFETCH>
	void stepE_split_prefetchE(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	template<int PREFETCH>
	void stepE_split_unrolled_prefetch(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	*/
		
	void stepH(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt, int dir);
	
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
	
    float m_eps_inf;
    float m_mu;
    float m_omega_p;
    float m_tau_c;
    
    float m_ch1;
    float m_cj1;
    float m_cj2;
    float m_ce1;
    float m_ce2;
    
	std::vector<float> mJ[3];
	/*
	std::vector<float> mJx;
	std::vector<float> mJy;
	std::vector<float> mJz;
	*/
};





#endif
