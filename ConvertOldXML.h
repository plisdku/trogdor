/*
 *  ConvertOldXML.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/12/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _CONVERTOLDXML_
#define _CONVERTOLDXML_

#include "tinyxml.h"
#include "Pointer.h"
#include "Exception.h"

class ConvertOldXML
{
public:
    static Pointer<TiXmlDocument> convert4to5(const Pointer<TiXmlDocument> &
        doc) throw(Exception);
private:
    ConvertOldXML();
    
    static TiXmlElement* simulation(const TiXmlElement* old);
    static TiXmlElement* grid(const TiXmlElement* old);
    static TiXmlElement* material(const TiXmlElement* old);
    static TiXmlElement* block(const TiXmlElement* old);
    static TiXmlElement* ellipsoid(const TiXmlElement* old);
    static TiXmlElement* keyImage(const TiXmlElement* old);
    static TiXmlElement* heightMap(const TiXmlElement* old);
    static TiXmlElement* assembly(const TiXmlElement* old);
    static TiXmlElement* output(const TiXmlElement* old);
    static TiXmlElement* source(const TiXmlElement* old);
    static TiXmlElement* tfsfSource(const TiXmlElement* old);
    static TiXmlElement* link(const TiXmlElement* old);
};




#endif
