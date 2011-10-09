/*
 * PHP Comport handling class PECL module
 * V1.0
 * (c)2011, Atanas Markov/http://awsumlabs.com
 * License: free to use, just include original copyright information
 */
#include "stdafx.h"

#include "php.h"
#include "ext/standard/info.h"
#include "comport.h"

/* declaration of functions to be exported */
PHP_MINIT_FUNCTION(comport);

PHP_MINIT_FUNCTION(comport) {
 comport_init_Comport(TSRMLS_C);
 return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(phpcomport)
{
	php_info_print_box_start(0);
	php_printf("<p>PHP COM port handling extenion</p>\n");
	php_printf("<p>Version 1.0.0 (2011-10-07)</p>\n");
	php_printf("<p>Author: <b>Atanas Markov, <a href='http://awsumlabs.com' target='_blank'>http://awsumlabs.com</a></b></p>\n");
	php_printf("<p>Usage:</p><p><i style='font-size: 0.9em'>$com= new Comport('COM1');<br/>$com->bauds=115200;<br/>$com->parity='E';<br/>$com->openPort();<br/>$com->writedata('Some data');<br/>$data= $com->readData(1);<br/>$com->closePort();</i></p>\n");
	php_info_print_box_end();
	/* add your stuff here */

}
/* }}} */

/* compiled function list so Zend knows what's in this module */
zend_function_entry comportModule_functions[] = {
    {NULL, NULL, NULL}
};

/* compiled module information */
zend_module_entry comportModule_module_entry = {
    STANDARD_MODULE_HEADER,
    "PHP COM port access class library",
    comportModule_functions,
    /*NULL*/
	PHP_MINIT(comport)
	, NULL, NULL, NULL, PHP_MINFO(phpcomport),
    "1.0", STANDARD_MODULE_PROPERTIES
};

/* implement standard "stub" routine to introduce ourselves to Zend */
ZEND_GET_MODULE(comportModule)
