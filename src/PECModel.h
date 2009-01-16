/*
 *  PECModel.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/10/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _PECMODEL_
#define _PECMODEL_

#include "MaterialModel.h"

class PECModel : public MaterialModel
{
public:
    PECModel();
    
    virtual std::string getMaterialName() const;
    virtual MaterialModel* createPML(Vector3i direction, Rect3i activeRect,
        Rect3i regionOfInterest, float dx, float dy, float dz, float dt) const;
    
  virtual void calcEx(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1) {}
  
  virtual void calcEy(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1) {}
  
  virtual void calcEz(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1) {}

  virtual void calcHx(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1) {}
  
  virtual void calcHy(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1) {}
  
  virtual void calcHz(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1) {}
};

#endif
