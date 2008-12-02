/*
*  Version.h
*  TROGDOR
*
*  Created by Paul Hansen on 12/05/07.
*
*/

#ifndef _VERSION_H_
#define _VERSION_H_

const int TROGDOR_MAJOR_VERSION = 4;
const int TROGDOR_MINOR_VERSION = 5;
const int TROGDOR_PATCH_VERSION = 3;

#define TROGDOR_VERSION_TEXT "4.5.3"

#ifdef __MINGW32__
#define TROGDOR_OS "MinGW 32"
#else
#define TROGDOR_OS "LINUX/UNIX"
#endif

#endif


