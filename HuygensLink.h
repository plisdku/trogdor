/*
 *  HuygensLink.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _HUYGENSLINK_
#define _HUYGENSLINK_

#include "HuygensSurface.h"
#include "geometry.h"
#include <vector>

class HuygensLink : public HuygensUpdate
{
public:
    HuygensLink(const HuygensSurface & hs);
    
    virtual void updateE(HuygensSurface & hs);
    virtual void updateH(HuygensSurface & hs);
};

#endif
