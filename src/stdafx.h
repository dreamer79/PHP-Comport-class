/*
 * PHP Comport handling class PECL module
 * V1.0
 * (c)2011, Atanas Markov/http://awsumlabs.com
 * License: free to use, just include original copyright information
 */
#pragma once

#ifndef STDAFX_H_INC

#define STDAFX_H_INC
#define ZEND_DEBUG 0

/*
windows specific rules
*/
#ifdef WIN32

#define ZTS 1
//thread safe version of php. in using nts, comment previous line...

#define _HAS_ITERATOR_DEBUGGING 0
#define PHP_WIN32
#define ZEND_WIN32

/* PHP Zend Extension headers */
#include "zend_config.w32.h"

#include "targetver.h"

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN             
// Windows Header Files:
#include <windows.h>

#endif

#include "php.h"
#endif
