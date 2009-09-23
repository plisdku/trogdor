/*
*  Version.h
*  TROGDOR
*
*  Created by Paul Hansen on 12/05/07.
*
*/

#ifndef _VERSION_H_
#define _VERSION_H_

const int TROGDOR_MAJOR_VERSION = 5;
const int TROGDOR_MINOR_VERSION = 0;
const int TROGDOR_PATCH_VERSION = 2;

#define TROGDOR_VERSION_TEXT "5.0.2"

#ifdef __MINGW32__
#define TROGDOR_OS "MinGW 32"
#else
#define TROGDOR_OS "LINUX/UNIX"
#endif

#endif


