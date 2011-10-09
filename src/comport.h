/*
 * PHP Comport handling class PECL module
 * V1.0
 * (c)2011, Atanas Markov/http://awsumlabs.com
 * License: free to use, just include original copyright information
 */
#ifndef Comport_H
#define Comport_H

#ifdef WIN32
#include <winbase.h>
#include <tchar.h>
#include <stdio.h>
#endif

/*
 misc constants
 */

#ifdef WIN32
#define DEFAULT_COM_PORT "COM1"
#else
#define DEFAULT_COM_PORT "/dev/ttyS0"
#endif
/*
PHP function definitions
*/
void comport_init_Comport(TSRMLS_D);

//class comport
PHP_METHOD(Comport, __construct);
PHP_METHOD(Comport, createComport);
PHP_METHOD(Comport, openPort);
PHP_METHOD(Comport, closePort);
PHP_METHOD(Comport, writeData);
PHP_METHOD(Comport, readData);

/*
misc data structures
*/
typedef struct comport_secrets {
	// required
	zend_object std;

#ifdef WIN32
              //windows definitions
	HANDLE hCom;
	DCB dcb;
#else 
                //linux data
        int hCom;
#endif
} comport_secrets;

/*
helper methods
*/
zend_object_value create_comport_secrets(zend_class_entry *class_type TSRMLS_DC);
void free_comport_secrets(void *object TSRMLS_DC);

#endif