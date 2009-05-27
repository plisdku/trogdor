/*
 *  STLOutput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _STLOUTPUT_
#define _STLOUTPUT_

#include <iostream>
#include <vector>
#include <set>
#include <map>

template<typename T>
std::ostream & operator<<(std::ostream & str, const std::vector<T> & aaa)
{
    unsigned int nn;
    for (nn = 0; nn < aaa.size(); nn++)
        str << nn << ": " << aaa[nn] << "\n";
    return str;
}

template<class T, class S>
std::ostream & operator<<(std::ostream & str, const std::map<T,S> & aaa)
{
    typename std::map<T,S>::const_iterator itr;
    for (itr = aaa.begin(); itr != aaa.end(); itr++)
        str << itr->first << ": " << itr->second << "\n";
    return str;
}

template<typename T>
std::ostream & operator<<(std::ostream & str, const std::set<T> & aaa)
{
    typename std::set<T>::const_iterator itr;
    for (itr = aaa.begin(); itr != aaa.end(); itr++)
        str << *itr << "\n";
    return str;
}



#endif