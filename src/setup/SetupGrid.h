/*
 *  SetupGrid.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/14/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SETUPGRID_
#define _SETUPGRID_

#include "SetupConstants.h"

#include "StructureGrid.h"
#include "MaterialType.h"
#include "RunlineType.h"

#include "geometry.h"

#include "tinyxml.h"

#include "Pointer.h"

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <set>


// Predeclarations here make life easier for the compiler and help with
// circular inclusion problems.

class SetupGrid;
typedef Pointer<SetupGrid> SetupGridPtr;
class StructureGrid;
typedef Pointer<StructureGrid> StructureGridPtr;
class SetupOutput;
typedef Pointer<SetupOutput> SetupOutputPtr;
class SetupInput;
typedef Pointer<SetupInput> SetupInputPtr;
class SetupSource;
typedef Pointer<SetupSource> SetupSourcePtr;
class SetupLink;
typedef Pointer<SetupLink> SetupLinkPtr;
class SetupTFSFSource;
typedef Pointer<SetupTFSFSource> SetupTFSFSourcePtr;
class SetupTFSFBufferSet;
typedef Pointer<SetupTFSFBufferSet> SetupTFSFBufferSetPtr;
class SetupMaterialModel;
typedef Pointer<SetupMaterialModel> SetupMaterialPtr;

class SetupGrid
{
public:
    SetupGrid(const TiXmlElement* gridElem,
    std::string & inName, int nnx, int nny, int nnz, Rect3i inActiveRegion,
    Rect3i inRegionOfInterest, bool doSaveBoundaries = 0);
	
	SetupGrid(const SetupGrid & parentGrid,
		std::string inName,
		const Rect3i & copyFromRect,
		const Rect3i & copyToRect,
		const Rect3i & boundingRectAndActiveRegion,
		const Rect3i & regionOfInterest,
		SetupTFSFSourcePtr auxSource);
    
    ~SetupGrid();
	
	void
	addSource(SetupSourcePtr src);
	
	void
	addOutput(SetupOutputPtr output);
    
	void
	addAFPSource(SetupTFSFSourcePtr src);
	
	void
	makeLinkBuffers(const Map<std::string, SetupGridPtr> & grids);
	
	void
	makeAFPBuffers();
	
	// just a call-through to StructureGrid.  sigh.
	vmlib::SMat<3,bool> getSymmetries(const Rect3i & boundingRect) const;
	
	Vector3b getPeriodicDimensions(const Rect3i & boundingRect) const;
	Vector3b getSingularDimensions() const;
	
	void assertFullGrid() const;
	
    void
    insertTFSF();
	
    const std::string &
    getName() const;
    
    int
    get_nnx() const { return m_nnx; }
    
    int
    get_nny() const { return m_nny; }
    
    int
    get_nnz() const { return m_nnz; }
    
    int
    get_nx() const { return m_nx; }
    
    int
    get_ny() const { return m_ny; }
    
    int
    get_nz() const { return m_nz; }
    
    Rect3i
    getBounds() const;
    
    Rect3d
    getYeeBounds() const;
    
    Rect3i
    getActiveRegion() const;
    
    Rect3i
    getRegionOfInterest() const;
    
	const std::vector<SetupTFSFBufferSetPtr> &
	getBuffers() const;
	
    const std::vector<SetupSourcePtr> &
    getSources() const;
    
    const Map<std::string, SetupMaterialPtr> &
    getMaterials() const;
    
	const std::vector<SetupTFSFSourcePtr> &
	getTFSFSources() const;
    
    const std::vector<SetupOutputPtr> &
    getOutputs() const;
    
    const std::vector<SetupInputPtr> &
    getInputs() const;
    
    Pointer< std::vector<RunlineType> >
    makeMaterialRunlines() const;
    
	void
	addLink(SetupLinkPtr link);
	
	void
	writeAFPRequests(float dx, float dy, float dz, float dt, int numT) const;
	
    void
    saveOutputCrossSections() const;
	
	void
	saveMaterialBoundariesBeta() const;
    
    void
	print(std::ostream & str) const;
	
private:
	void
	readSetupMaterials(const TiXmlElement* gridElem);
	
	void
	readSetupSources(const TiXmlElement* gridElem);
	
	void
	convertLegacySource(Map<std::string, std::string> & attribs,
		Map<std::string, std::string> & params);
	
	void
	readSetupLinks(const TiXmlElement* gridElem);
	
	void
	readSetupOutputs(const TiXmlElement* gridElem);
	
	void
	readSetupInputs(const TiXmlElement* gridElem);
	
	void
	readSetupTFSFSources(const TiXmlElement* gridElem);
    
    void
    readSetupAssembly(const TiXmlElement* gridElem);
    
    void
    assembleFromBlock(const TiXmlElement* elem);
    
    void
    assembleFromAscii(const TiXmlElement* elem);
    
    void
    assembleFromKeyImage(const TiXmlElement* elem);
    
    void
    assembleFromHeightMap(const TiXmlElement* elem);
    
	void
	assembleFromEllipsoid(const TiXmlElement* elem);
	
    void
    insertPML();
    
    bool
    validateTFSFRegions(std::string & outErrorMessage, int differenceOrder)
        const;
    
    void
    makeRunlines(std::vector<RunlineType> & outRunlines, Rect3i clipRect,
		int field_ii, int field_jj, int field_kk) const;
    
    typedef std::pair<Vector3i, Vector3i> OrientedPoint3i;
    void
    getOrientedOuterBoundary( std::set<OrientedPoint3i> & outSet,
        const Rect3i & inRegion, const Rect3i & clipRect ) const;
	
	void
	consolidateRects(const std::vector<OrientedRect3i> & smallRects,
		std::list<OrientedRect3i> & bigRects, int normalIndex) const;
    
    std::string mName;
    
    Rect3i mActiveRegion;
    Rect3i m_roi;
    int m_nnx;
    int m_nny;
    int m_nnz;
    int m_nx;
    int m_ny;
    int m_nz;
	
	StructureGridPtr mStructureGrid;
    std::vector<SetupOutputPtr> mOutputs;
    std::vector<SetupInputPtr> mInputs;
    std::vector<SetupSourcePtr> mSources;
    std::vector<SetupLinkPtr> mLinks;
	std::vector<SetupTFSFSourcePtr> mTFSFSources;
	std::vector<SetupTFSFSourcePtr> mAFPSources;
	std::vector<SetupTFSFBufferSetPtr> mBuffers;
    Map<std::string, SetupMaterialPtr> mMaterials; // by name
    
};


Map<std::string, std::string> getAttributes(const TiXmlElement* elem);

#endif
