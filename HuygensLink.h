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


class HuygensLinkDelegate : public HuygensSurfaceDelegate
{
public:
    HuygensLinkDelegate();
    
    virtual HuygensSurfacePtr makeHuygensSurface() const;
private:
};


class HuygensLink : public HuygensSurface
{
public:
    HuygensLink();
    
    virtual void updateE();
    virtual void updateH();
};



#endif
