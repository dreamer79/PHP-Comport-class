/*
 * PHP Comport handling class PECL module
 * V1.0
 * (c)2011, Atanas Markov/http://awsumlabs.com
 * License: free to use, just include original copyright information
 */
/*
 This is the Comport php class definition. It handles both windows and linux compilations so in 
 methods there are plenty of ifdef statements.
 */
#include "stdafx.h"

/*
 Linux config
 */
#ifndef WIN32
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <time.h>
#endif

#include <php.h>
#include "phpcomport.h"
#include "comport.h"

zend_class_entry *comport_ce_Comport;

/*
 arginfo- describes function parameters so zend can handle them correctly
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_comport_writedata, 0, 0, 1)
ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_comport_readdata, 0, 0, 1)
ZEND_ARG_INFO(0, bytestoread)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_comport_construct, 0, 0, 1)
ZEND_ARG_INFO(0, com)
ZEND_END_ARG_INFO()

/*
methods for the Comport class
*/
static function_entry Comport_methods[] = {
    PHP_ME(Comport, __construct, arginfo_comport_construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(Comport, writeData, arginfo_comport_writedata, ZEND_ACC_PUBLIC)
    PHP_ME(Comport, readData, arginfo_comport_readdata, ZEND_ACC_PUBLIC)
    PHP_ME(Comport, openPort, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Comport, closePort, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Comport, createComport, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) {
        NULL, NULL, NULL
    }
};

void comport_init_Comport(TSRMLS_D) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Comport", Comport_methods);

    ce.create_object = create_comport_secrets;

    comport_ce_Comport = zend_register_internal_class(&ce TSRMLS_CC);
}

PHP_METHOD(Comport, createComport) {
    object_init_ex(return_value, comport_ce_Comport);
    CALL_METHOD(Comport, __construct, return_value, return_value);
}

PHP_METHOD(Comport, __construct) {
    char *com;
    int com_len;

    int i;
    i = ZEND_NUM_ARGS();

    if (i < 1 || zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s/", &com, &com_len) == FAILURE) {
        zend_update_property_stringl(comport_ce_Comport, getThis(), "com", strlen("com"), DEFAULT_COM_PORT, strlen(DEFAULT_COM_PORT) TSRMLS_CC);
    } else {
        zend_update_property_stringl(comport_ce_Comport, getThis(), "com", strlen("com"), com, com_len TSRMLS_CC);
    }

    zend_update_property_long(comport_ce_Comport, getThis(), "timeout", strlen("timeout"), 500 TSRMLS_CC);
    zend_update_property_long(comport_ce_Comport, getThis(), "bauds", strlen("bauds"), 9600 TSRMLS_CC);
    zend_update_property_bool(comport_ce_Comport, getThis(), "rts", strlen("rts"), 1 TSRMLS_CC);
    zend_update_property_bool(comport_ce_Comport, getThis(), "xon", strlen("xon"), 0 TSRMLS_CC);
    zend_update_property_bool(comport_ce_Comport, getThis(), "dtr", strlen("dtr"), 0 TSRMLS_CC);
    zend_update_property_stringl(comport_ce_Comport, getThis(), "parity", strlen("parity"), "N", 1 TSRMLS_CC);
    zend_update_property_long(comport_ce_Comport, getThis(), "data", strlen("data"), 8 TSRMLS_CC);
    zend_update_property_long(comport_ce_Comport, getThis(), "stop", strlen("stop"), 1 TSRMLS_CC);
}

zend_object_value create_comport_secrets(zend_class_entry *class_type TSRMLS_DC) {
    zend_object_value retval;
    comport_secrets *intern;
    zval *tmp;

    // allocate the struct we're going to use
    intern = (comport_secrets*) emalloc(sizeof (comport_secrets));
    memset(intern, 0, sizeof (comport_secrets));

    // create a table for class properties
    zend_object_std_init(&intern->std, class_type TSRMLS_CC);
    zend_hash_copy(intern->std.properties,
            &class_type->default_properties,
            (copy_ctor_func_t) zval_add_ref,
            (void *) &tmp,
            sizeof (zval *));

#ifdef WIN32
    intern->hCom = INVALID_HANDLE_VALUE;
#else
    intern->hCom = -1; //result of open with error...
#endif    
    // create a destructor for this struct
    retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t) zend_objects_destroy_object, free_comport_secrets, NULL TSRMLS_CC);
    retval.handlers = zend_get_std_object_handlers();

    return retval;
}

// this will be called when a Comport goes out of scope

void free_comport_secrets(void *object TSRMLS_DC) {
    comport_secrets *secrets = (comport_secrets*) object;
#ifdef WIN32
    if (secrets->hCom != INVALID_HANDLE_VALUE) {
        CloseHandle(secrets->hCom);
        secrets->hCom = INVALID_HANDLE_VALUE;
    }
#else
    if (secrets->hCom < 0) {
        close(secrets->hCom);
        secrets->hCom = -1;
    }
#endif
    efree(secrets);
}

PHP_METHOD(Comport, openPort) {
    comport_secrets *secrets;
    secrets = (comport_secrets*) zend_object_store_get_object(getThis() TSRMLS_CC);

#ifdef WIN32
    BOOL fSuccess;
#else
    int fSuccess;
    struct termios termctl;

    //setup 96008N1
    cfmakeraw(&termctl);
    cfsetospeed(&termctl, B9600);
    cfsetispeed(&termctl, B9600);
    termctl.c_cflag &= ~(CSTOPB);
    termctl.c_cflag &= ~(CSIZE);
    termctl.c_cflag |= CS8;
    termctl.c_cflag &= ~(PARENB);
    termctl.c_cflag &= ~(CLOCAL);
    termctl.c_cflag &= ~(HUPCL);
    termctl.c_cflag |= CREAD;
    termctl.c_cflag &= ~(CRTSCTS);
    termctl.c_iflag &= ~(IXON | IXOFF | IXANY);
    termctl.c_iflag |= IGNBRK;
#endif

    zval *comp;
    comp = zend_read_property(comport_ce_Comport, getThis(), "com", strlen("com"), 0 TSRMLS_CC);
    convert_to_string(comp);

    zval *zparity;
    zparity = zend_read_property(comport_ce_Comport, getThis(), "parity", strlen("parity"), 0 TSRMLS_CC);
    convert_to_string(zparity);
    char parity;
    parity = zparity->value.str.val[0];
#ifndef WIN32
    switch (parity) {
        case 'e':
        case 'E':
            termctl.c_cflag |= PARENB;
            termctl.c_cflag &= ~(PARODD);
            break;

        case 'o':
        case 'O':
            termctl.c_cflag |= PARENB | PARODD;
            break;

        default:
            termctl.c_cflag &= ~(PARENB);
    }
#endif    

    zval *zbauds;
    long bauds;
    zbauds = zend_read_property(comport_ce_Comport, getThis(), "bauds", strlen("bauds"), 0 TSRMLS_CC);
    convert_to_long(zbauds);
    bauds = zbauds->value.lval;
#ifndef WIN32
    speed_t sbauds;
    /*
    Set baud rate. Made to check if system supports a rate on compiling...
     */
    switch (bauds) {
#ifdef B0
        case 0:
            sbauds = B0;
            break;
#endif
#ifdef B50
        case 50:
            sbauds = B50;
            break;
#endif
#ifdef B75
        case 75:
            sbauds = B75;
            break;
#endif
#ifdef B110
        case 110:
            sbauds = B110;
            break;
#endif
#ifdef B134
        case 134:
            sbauds = B134;
            break;
#endif
#ifdef B150
        case 150:
            sbauds = B150;
            break;
#endif
#ifdef B200
        case 200:
            sbauds = B200;
            break;
#endif
#ifdef B300
        case 300:
            sbauds = B300;
            break;
#endif
#ifdef B600
        case 600:
            sbauds = B600;
            break;
#endif
#ifdef B1200
        case 1200:
            sbauds = B1200;
            break;
#endif
#ifdef B1800
        case 1800:
            sbauds = B1800;
            break;
#endif
#ifdef B2400
        case 2400:
            sbauds = B2400;
            break;
#endif
#ifdef B4800
        case 4800:
            sbauds = B4800;
            break;
#endif
#ifdef B9600
        case 9600:
            sbauds = B9600;
            break;
#endif
#ifdef B19200
        case 19200:
            sbauds = B19200;
            break;
#endif
#ifdef B38400
        case 38400:
            sbauds = B38400;
            break;
#endif
#ifdef B57600
        case 57600:
            sbauds = B57600;
            break;
#endif
#ifdef B115200
        case 115200:
            sbauds = B115200;
            break;
#endif
#ifdef B230400
        case 230400:
            sbauds = B230400;
            break;
#endif
#ifdef B460800
        case 460800:
            sbauds = B460800;
            break;
#endif
#ifdef B500000
        case 500000:
            sbauds = B500000;
            break;
#endif
#ifdef B576000
        case 576000:
            sbauds = B576000;
            break;
#endif
#ifdef B921600
        case 921600:
            sbauds = B921600;
            break;
#endif
#ifdef B1000000
        case 1000000:
            sbauds = B1000000;
            break;
#endif
#ifdef B1152000
        case 1152000:
            sbauds = B1152000;
            break;
#endif
#ifdef B1500000
        case 1500000:
            sbauds = B1500000;
            break;
#endif
#ifdef B2000000
        case 2000000:
            sbauds = B2000000;
            break;
#endif
#ifdef B2500000
        case 2500000:
            sbauds = B2500000;
            break;
#endif
#ifdef B3000000
        case 3000000:
            sbauds = B3000000;
            break;
#endif
#ifdef B3500000
        case 3500000:
            sbauds = B3500000;
            break;
#endif
#ifdef B4000000
        case 4000000:
            sbauds = B4000000;
            break;
#endif

        default:
            sbauds = B9600;
    }

    cfsetospeed(&termctl, sbauds);
    cfsetispeed(&termctl, sbauds);

#endif    

    zval *zdata;
    long data;
    zdata = zend_read_property(comport_ce_Comport, getThis(), "data", strlen("data"), 0 TSRMLS_CC);
    convert_to_long(zdata);
    data = zdata->value.lval;
#ifndef WIN32    
    switch (data) {
#ifdef CS5
        case 5:
            termctl.c_cflag &= ~(CSIZE);
            termctl.c_cflag |= CS5;
            break;
#endif        
#ifdef CS6
        case 6:
            termctl.c_cflag &= ~(CSIZE);
            termctl.c_cflag |= CS6;
            break;
#endif
#ifdef CS7
        case 7:
            termctl.c_cflag &= ~(CSIZE);
            termctl.c_cflag |= CS7;
            break;
#endif        
        default:
            termctl.c_cflag &= ~(CSIZE);
            termctl.c_cflag |= CS8;
    }
#endif    

    zval *zstop;
    long stop;
    zstop = zend_read_property(comport_ce_Comport, getThis(), "stop", strlen("stop"), 0 TSRMLS_CC);
    convert_to_long(zstop);
    stop = zstop->value.lval;
#ifndef WIN32
    switch (stop) {
        case 2:
            termctl.c_cflag |= CSTOPB;
            break;
        default:
            termctl.c_cflag &= ~(CSTOPB);
    }
#endif    

    zval *ztimeout;
    long timeout;
    ztimeout = zend_read_property(comport_ce_Comport, getThis(), "timeout", strlen("timeout"), 0 TSRMLS_CC);
    convert_to_long(ztimeout);
    timeout = ztimeout->value.lval;

    zval *zrts;
#ifdef WIN32
    char *rts;
#endif    
    zrts = zend_read_property(comport_ce_Comport, getThis(), "rts", strlen("rts"), 0 TSRMLS_CC);
    convert_to_long(zrts);
    if (zrts->value.lval) {
#ifdef WIN32        
        rts = "on";
#else
        termctl.c_cflag |= CRTSCTS;
#endif       
    } else {
#ifdef WIN32        
        rts = "off";
#else
        termctl.c_cflag &= ~CRTSCTS;
#endif        
    }

    zval *zxon;
#ifdef WIN32
    char *xon;
#endif    
    zxon = zend_read_property(comport_ce_Comport, getThis(), "xon", strlen("xon"), 0 TSRMLS_CC);
    convert_to_long(zxon);
    if (zxon->value.lval) {
#ifdef WIN32
        xon = "on";
#else
        termctl.c_iflag |= (IXON | IXOFF | IXANY);
        termctl.c_cc[VSTART] = 17;
        termctl.c_cc[VSTOP] = 19;
#endif        
    } else {
#ifdef WIN32        
        xon = "off";
#else
        termctl.c_iflag &= ~(IXON | IXOFF | IXANY);
#endif        
    }

    zval *zdtr;
#ifdef WIN32    
    char *dtr;
#endif    
    zdtr = zend_read_property(comport_ce_Comport, getThis(), "dtr", strlen("dtr"), 0 TSRMLS_CC);
    convert_to_long(zdtr);
    if (zdtr->value.lval) {
#ifdef WIN32
        dtr = "on";
#else
        termctl.c_cflag |= CLOCAL;
#endif        
    } else {
#ifdef WIN32       
        dtr = "off";
#else
        termctl.c_cflag &= ~CLOCAL;
#endif        
    }

    unsigned long result = 1;

#ifdef WIN32
    TCHAR *pcCommPort = TEXT(Z_STRVAL_P(comp)); // = A2T(Z_STRVAL_P(comp));

    if (secrets->hCom != INVALID_HANDLE_VALUE) {
        CloseHandle(secrets->hCom);
        secrets->hCom = INVALID_HANDLE_VALUE;
    }

    secrets->hCom = CreateFile(pcCommPort,
            GENERIC_READ | GENERIC_WRITE,
            0, //  must be opened with exclusive-access
            NULL, //  default security attributes
            OPEN_EXISTING, //  must use OPEN_EXISTING
            0, //  not overlapped I/O
            NULL); //  hTemplate must be NULL for comm devices

#else
    if (secrets->hCom >= 0) {
        close(secrets->hCom);
        secrets->hCom = -1;
    }

    secrets->hCom = open(Z_STRVAL_P(comp), O_RDWR | O_NOCTTY | O_NDELAY);
#endif

    //zval_ptr_dtor(&comp);

#ifdef WIN32
    if (secrets->hCom == INVALID_HANDLE_VALUE) {
#else
    if (secrets->hCom < 0) {
#endif        
        result = 0;
    }

    if (result != 0) {
#ifdef WIN32
        ZeroMemory(&secrets->dcb, sizeof (DCB));
        secrets->dcb.DCBlength = sizeof (DCB);

        fSuccess = GetCommState(secrets->hCom, &secrets->dcb);
        if (fSuccess) {
            char *tmpstr;
            tmpstr = (char *) emalloc(300);
            ZeroMemory(tmpstr, 200);
            sprintf(tmpstr, "baud=%d parity=%c data=%d stop=%d rts=%s xon=%s dtr=%s", bauds, parity, data, stop, rts, xon, dtr);
            BuildCommDCB(tmpstr, &secrets->dcb);
            efree(tmpstr);

            fSuccess = SetCommState(secrets->hCom, &secrets->dcb);

            if (fSuccess) {
                COMMTIMEOUTS ctout;
                ZeroMemory(&ctout, sizeof (COMMTIMEOUTS));
                ctout.ReadIntervalTimeout = timeout;
                SetCommTimeouts(secrets->hCom, &ctout);
            } else {
                result = 0;
            }
        } else {
#else
        fSuccess = tcsetattr(secrets->hCom, TCSANOW, &termctl);
        if (fSuccess < 0) {
#endif            
            result = 0;
        }
    }

    RETURN_LONG(result);
}

/*
 a function to solve a problem with parameters as seen in mysql php extension
 */
static void comport_writedata_internal(INTERNAL_FUNCTION_PARAMETERS) /* {{{ */ {
    zval *objectthis = getThis();
    comport_secrets *secrets;
    char *bufstr;
    int bufstr_len;
    zval *zparam;
    // 'z/' is for zval that is a always a copy of var
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z/", &zparam) == FAILURE) {
        RETURN_LONG(0);
    }

    secrets = (comport_secrets*) zend_object_store_get_object(objectthis TSRMLS_CC);

    convert_to_string(zparam);
    bufstr = zparam->value.str.val;
    bufstr_len = zparam->value.str.len;

#ifdef WIN32
    if (secrets->hCom == INVALID_HANDLE_VALUE) {
        zval_ptr_dtor(&zparam);
        RETURN_LONG(0);
    }
    BOOL fSuccess;
#else
    if (secrets->hCom < 0) {
        zval_ptr_dtor(&zparam);
        RETURN_LONG(0);
    }

   // RETURN_STRINGL(bufstr, bufstr_len, 1);
    //test for problem- string that comes as chr(xx).'xxxxxxxxxxxxxx' blocks php_method if used directly in it
    //strange behaviour as 'fff'.chr().'fff' or xxx().chr(xx).'xxxx' works

    int fSuccess;
#endif

#ifdef WIN32
    unsigned long test;

    fSuccess = WriteFile(secrets->hCom, bufstr, bufstr_len, &test, NULL);

    if (!fSuccess) {
#else
    fSuccess = write(secrets->hCom, bufstr, bufstr_len);
    if (fSuccess < 0) {
#endif
        zval_ptr_dtor(&zparam);
        //  Handle the error.
        RETURN_LONG(0);
    }

    zval_ptr_dtor(&zparam);
    RETURN_LONG(1);
}
/* }}} */

/* {{{ proto bool Comport.writeData(string data)    
        Writes data to port*/
PHP_METHOD(Comport, writeData) {
    comport_writedata_internal(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

PHP_METHOD(Comport, closePort) {

    comport_secrets *secrets;
    secrets = (comport_secrets*) zend_object_store_get_object(getThis() TSRMLS_CC);

#ifdef WIN32
    if (secrets->hCom != INVALID_HANDLE_VALUE) {
        CloseHandle(secrets->hCom);
        secrets->hCom = INVALID_HANDLE_VALUE;
    }
#else    
    if (secrets->hCom >= 0) {
        close(secrets->hCom);
        secrets->hCom = -1;
    }
#endif    
}

PHP_METHOD(Comport, readData) {
    char *bufstr;
    unsigned long bufstr_len;
#ifdef WIN32       
    BOOL fSuccess;
	unsigned long bytesread;
#else
    int fSuccess;
    long bytesread;
#endif
    int toread = 1;
    int i;

    i = ZEND_NUM_ARGS();

    if (i > 0) {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &toread) == FAILURE) {
            return;
        }
    }

    comport_secrets *secrets;
    secrets = (comport_secrets*) zend_object_store_get_object(getThis() TSRMLS_CC);

#ifdef WIN32    
    if (secrets->hCom == INVALID_HANDLE_VALUE) {
#else

    if (secrets->hCom < 0) {
#endif            
        RETURN_STRING("", 1);
    }

    bufstr = (char *) emalloc(toread + 1);
#ifdef WIN32
    ZeroMemory(bufstr, toread + 1);
#else
    memset(bufstr, 0, toread + 1);
#endif    

    bufstr_len = 0;
    bytesread = 0;

#ifdef WIN32
    while (bufstr_len < toread) {
        fSuccess = ReadFile(secrets->hCom, bufstr + bufstr_len, toread - bufstr_len, &bytesread, NULL);
        if (fSuccess) {
            bufstr_len += bytesread;
        } else {
            break;
        }
    }
#else

    zval *ztimeout;
    long timeout;
    ztimeout = zend_read_property(comport_ce_Comport, getThis(), "timeout", strlen("timeout"), 0 TSRMLS_CC);
    convert_to_long(ztimeout);
    timeout = ztimeout->value.lval;

    fd_set fds;
    struct timeval stimeout;

    while (bufstr_len < toread) {
        FD_ZERO(&fds);
        FD_SET(secrets->hCom, &fds);
        stimeout.tv_sec = 0;
        stimeout.tv_usec = timeout * 1000;

        fSuccess = select(sizeof (fds)*8, &fds, NULL, NULL, &stimeout);

        if (fSuccess >= 0) {
            bytesread = read(secrets->hCom, bufstr + bufstr_len, toread - bufstr_len);
            if (bytesread < 0) {
                break;
            } else {
                bufstr_len += bytesread;
            }
        } else {
            break;
        }
    }
#endif
    if (!bufstr_len) {
        efree(bufstr);
        RETURN_STRING("", 1);
    }

    RETURN_STRINGL(bufstr, bufstr_len, 1);
}
