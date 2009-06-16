/*
 *  HuygensLink.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "HuygensLink.h"
#include "Log.h"

using namespace std;

HuygensSurfacePtr HuygensLinkDelegate::
makeHuygensSurface() const
{
    return HuygensSurfacePtr(new HuygensLink);
}


HuygensLink::
HuygensLink()
{
    LOG << "Constructor.\n";
}


void HuygensLink::
updateE()
{
    LOG << "Update E.\n";
}

void HuygensLink::
updateH()
{
    LOG << "Update H.\n";
}
