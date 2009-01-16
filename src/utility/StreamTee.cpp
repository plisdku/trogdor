/*
 *  StreamTee.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 12/5/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "StreamTee.h"

StreamTee::StreamTee() :
	m_numStreams(0),
	m_str1(0L),
	m_str2(0L)
{
}

StreamTee::StreamTee(std::ostream & str) :
	m_numStreams(1),
	m_str1(&str),
	m_str2(&str)
{
}

StreamTee::StreamTee(std::ostream & str1, std::ostream & str2) :
	m_numStreams(2),
	m_str1(&str1),
	m_str2(&str2)
{
}


void
StreamTee::setStream(std::ostream & str)
{
	m_str1 = &str;
	m_numStreams = 1;
}

void
StreamTee::setStreams(std::ostream & str1, std::ostream & str2)
{
	m_str1 = &str1;
	m_str2 = &str2;
	m_numStreams = 2;
}

StreamTee&
StreamTee::operator<<(std::ostream& (*x)(std::ostream&))
{
	if (m_numStreams > 0)
		*m_str1 << x;
	if (m_numStreams > 1)
		*m_str2 << x;
	
	return *this;
}