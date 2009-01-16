/*
 *  MaterialModel.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/20/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _MATERIALMODEL_
#define _MATERIALMODEL_

#include "Pointer.h"
#include "Runline.h"

//  C++ headers
#include <vector>
#include <string>

/**
 *  Material model implementation
 *
 *  Abstract base class for all FDTD material implementations, including
 *  PML and total field/scattered field (TF/SF) variants.  An unmodified
 *  Material object will act as a perfect electric and magnetic conductor
 *  (a perfect reflector).
 *
 *  A new material model can be implemented by simply overloading
 *  stepEx() through step Hz().  Further possible functionality includes
 *  providing matched PML and TF/SF-corrected "clones."
 */
class MaterialModel
{
public:
  /**
   *  Constructor
   *
   *  Subclasses can use the constructor as the first chance to
   *  allocate memory, although the allocate() method will be
   *  called as well after all the runlines have been added.
   */
  MaterialModel();
  
  /**
   *  Parameterized constructor
   */
  MaterialModel(const Map<std::string, std::string> & inParams);
  
  /** Destructor */
  virtual ~MaterialModel();
  
   
  /**
   *  Store a new runline
   *
   *  The runline will be added to the appropriate field component's
   *  runline array based on the starting coordinate (ii0, jj0, kk0).
   *  Even and odd ii, jj, and kk determine which part of the Yee cell
   *  this runline covers, hence which field component.
   *
   *  Unnecessary to overload.
   *
   *  @param rl     runline to add
   *  @param ii0      x-coordinate of first cell in the runline
   *  @param jj0      y-coordinate of first cell in the runline
   *  @param kk0      z-coordinate of first cell in the runline
   */
  void addRunline(Runline rl, int ii0, int jj0, int kk0);
  
  /**
   *  Unnecessary to overload.
   *
   *  @return       number of cells in Ex runlines
   */
  int numCellsEx() const;
  
  /**
   *  Unnecessary to overload.
   *
   *  @return       number of cells in Ey runlines
   */
  int numCellsEy() const;
  
  /**
   *  Unnecessary to overload.
   *
   *  @return       number of cells in Ez runlines
   */
  int numCellsEz() const;
  
  /**
   *  Unnecessary to overload.
   *
   *  @return       number of cells in Hx runlines
   */
  int numCellsHx() const;
  
  /**
   *  Unnecessary to overload.
   *
   *  @return       number of cells in Hy runlines
   */
  int numCellsHy() const;
  
  /**
   *  Unnecessary to overload.
   *
   *  @return       number of cells in Hz runlines
   */
  int numCellsHz() const;
  
  /**
   *  Unnecessary to overload.
   *
   *  @return       total number of cells in all runlines
   */
  int numCells() const;
  
  /**
   *  Allocate extra memory
   *
   *  Before the simulation runs but after all the runlines are stored,
   *  the Material gets a second chance to allocate memory such as old
   *  field value storage and auxiliary variables.  Any memory allocated
   *  here must be released in the destructor.
   *
   *  @param dx       grid spatial step
   *  @param dt       grid temporal step
   */
  virtual void allocate(float dx, float dy, float dz, float dt);
  
  /**
   *  Descriptor string
   *
   *  Get a full-name description of this material model, e.g.
   *  "static dielectric."
   *
   *  @return       description string
   */
  virtual std::string getMaterialName() const;
  
  /**
   *  Get new matched PML object
   *
   *  It is the Material's responsibility to create a new PML object
   *  on demand, to ensure impedance matching at the edge of the space.
   *  Each Material is thus a factory object for its own PML.
   *
   *  @param direction  direction of increasing impedance, unit components
   *  @param activeRect the region of the grid which is updated
   *  @param regionOfInterest the non-PML innards of the grid
   *  @return       PML object made with new (default: NULL)
   */
  virtual MaterialModel* createPML(Vector3i direction, Rect3i activeRect,
    Rect3i regionOfInterest, float dx, float dy, float dz, float dt) const;

  /**
   *  Callback for adding runlines
   *
   *  A Material gets a chance to modify itself or set rl.aux when a
   *  runline is added, as part of the call to addRunline. Default does
   *  nothing.
   *
   *  @param rl     new runline
   *  @param ii     x-coordinate of first cell covered by runline
   *  @param jj     y-coordinate of first cell covered by runline
   *  @param kk     z-coordinate of first cell covered by runline
   */
  virtual void onAddRunline(Runline & rl, int ii, int jj, int kk);
  
  /**
   *  Call stepEx() for each Ex runline.  Probably unnecessary to overload.
   */
  virtual void calcEx(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1);
  
  /**
   *  Call stepEy() for each Ey runline.  Probably unnecessary to overload.
   */
  virtual void calcEy(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1);
  
  /**
   *  Call stepEz() for each Ez runline.  Probably unnecessary to overload.
   */
  virtual void calcEz(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1);

  /**
   *  Call stepHx() for each Hx runline.  Probably unnecessary to overload.
   */
  virtual void calcHx(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1);
  
  /**
   *  Call stepHy() for each Hy runline.  Probably unnecessary to overload.
   */
  virtual void calcHy(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1);
  
  /**
   *  Call stepHz() for each Hz runline.  Probably unnecessary to overload.
   */
  virtual void calcHz(float dxinv, float dyinv, float dzinv, float dt,
	int start = 0, int stride = 1);

  int numRunlines() const;
	/*
protected:
  std::vector<Runline>& getRunlinesEx() { return mRunlinesEx; }
  std::vector<Runline>& getRunlinesEy() { return mRunlinesEy; }
  std::vector<Runline>& getRunlinesEz() { return mRunlinesEz; }
  std::vector<Runline>& getRunlinesHx() { return mRunlinesHx; }
  std::vector<Runline>& getRunlinesHy() { return mRunlinesHy; }
  std::vector<Runline>& getRunlinesHz() { return mRunlinesHz; }
  */
public:
  
	/*
	virtual void stepE(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt);
	
	virtual void stepH(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
		float dt);
	
  virtual void stepEx(const Runline & rl, float dxinv, float dyinv, float dzinv,
    float dt);
  
  virtual void stepEy(const Runline & rl, float dxinv, float dyinv, float dzinv,
    float dt);
  
  virtual void stepEz(const Runline & rl, float dxinv, float dyinv, float dzinv,
    float dt);
  
  virtual void stepHx(const Runline & rl, float dxinv, float dyinv, float dzinv,
    float dt);
  
  virtual void stepHy(const Runline & rl, float dxinv, float dyinv, float dzinv,
    float dt);
  
  virtual void stepHz(const Runline & rl, float dxinv, float dyinv, float dzinv,
    float dt);
	*/
  
  std::vector<Runline> mRunlinesEx;
  std::vector<Runline> mRunlinesEy;
  std::vector<Runline> mRunlinesEz;
  std::vector<Runline> mRunlinesHx;
  std::vector<Runline> mRunlinesHy;
  std::vector<Runline> mRunlinesHz;
  
  Map<std::string, std::string> mParams;

private:
  int mNumCellsEx;
  int mNumCellsEy;
  int mNumCellsEz;
  int mNumCellsHx;
  int mNumCellsHy;
  int mNumCellsHz;
  
};

typedef Pointer<MaterialModel> MaterialPtr;

#endif
