/*
 *  SetupLink.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/19/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _SETUPLINK_
#define _SETUPLINK_

#include "SetupConstants.h"
#include "Pointer.h"
#include "geometry.h"

#include <set>
#include <bitset>


class SetupLink
{
public:
    SetupLink();
    SetupLink(TFSFType inLinkType,
        const std::string & inSourceGridName,
        const Rect3i & inSourceRect, const Rect3i & inDestRect,
			std::bitset<6> inOmitSideFlags = std::bitset<6>() );    
    std::string getDescription() const;
    
    void omitSide(Vector3i inOmitSide);
    //bool omits(const Vector3i & inSide) const;
	bool omits(int sideNum) const { return mOmitSideFlags[sideNum]; }
	const std::bitset<6> getOmitSideFlags() const { return mOmitSideFlags; }
    
    TFSFType getLinkType() const;
    const Rect3i & getSourceRect() const;
    const Rect3i & getDestRect() const;
    //const std::set<Vector3i> & getOmittedSides() const;
    const std::string & getSourceGridName() const;
    
private:
    TFSFType mLinkType;
    std::string mSourceGridName;
    Rect3i mSourceRect;
    Rect3i mDestRect;
    //std::set<Vector3i> mOmittedSides;
	std::bitset<6> mOmitSideFlags;
    
    friend bool operator<(const SetupLink & lhs, const SetupLink & rhs);
};

typedef Pointer<SetupLink> SetupLinkPtr;

bool operator<(const SetupLink & lhs, const SetupLink & rhs);

#endif
