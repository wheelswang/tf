/*
  +----------------------------------------------------------------------+
  | TF                                                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | wheelswang@gmail.com so we can mail you a copy immediately.          |
  +----------------------------------------------------------------------+
  | Author: wheelswang  <wheelswang@gmail.com>                           |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "tf_console_application.h"

ZEND_BEGIN_ARG_INFO_EX(tf_console_application___construct_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, config_file)
    ZEND_ARG_INFO(0, config_section)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_console_application__run_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, method)
    ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

zend_class_entry *tf_console_application_ce;
extern zend_class_entry *tf_application_ce;

PHP_METHOD(tf_console_application, __construct) {
    char *config_file, *config_section = NULL;
    int config_file_len, config_section_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &config_file, &config_file_len, &config_section, &config_section_len) == FAILURE) {
        return;
    }
    
    tf_application_constructor(getThis(), config_file, config_file_len, config_section, config_section_len TSRMLS_CC);
}

PHP_METHOD(tf_console_application, run) {
    zend_fcall_info fci;
    zend_fcall_info_cache fci_cache;
    zval *retval_ptr;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f*", &fci, &fci_cache, &fci.params, &fci.param_count) == FAILURE) {
        return;
    }

    fci.retval_ptr_ptr = &retval_ptr;

    if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
        COPY_PZVAL_TO_ZVAL(*return_value, *fci.retval_ptr_ptr);
    }

    if (fci.params) {
        efree(fci.params);
    }
}

zend_function_entry tf_console_application_methods[] = {
    PHP_ME(tf_console_application, __construct, tf_console_application___construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_console_application, run, tf_console_application__run_arginfo, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};

ZEND_MINIT_FUNCTION(tf_console_application) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\ConsoleApplication", tf_console_application_methods);
    tf_console_application_ce = zend_register_internal_class_ex(&ce, tf_application_ce, NULL TSRMLS_CC);

    return SUCCESS;
}
