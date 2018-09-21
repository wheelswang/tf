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
#include "tf_error.h"

zend_class_entry *tf_error_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_error_setError_arginfo, 0, 0, 3)
    ZEND_ARG_INFO(0, error_code)
    ZEND_ARG_INFO(0, error_msg)
    ZEND_ARG_INFO(0, error_detail)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_error_setErrorMsg_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, error_msg)
    ZEND_ARG_INFO(0, error_detail)
ZEND_END_ARG_INFO()

void tf_error_set_error(zval *error, zval *error_code, zval *error_msg, zval *error_detail TSRMLS_DC) {
    zend_update_property(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_CODE), error_code TSRMLS_CC);
    
    if (error_msg) {
        zend_update_property(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_MSG), error_msg TSRMLS_CC);
    } else {
        zend_update_property_null(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_MSG) TSRMLS_CC);
    }

    if (error_detail) {
        zend_update_property(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_DETAIL), error_detail TSRMLS_CC);
    } else {
        zend_update_property_null(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_DETAIL) TSRMLS_CC);
    }
}

void tf_error_set_error_msg(zval *error, zval *error_msg, zval *error_detail TSRMLS_DC) {
    zval *error_code;
    MAKE_STD_ZVAL(error_code);
    ZVAL_LONG(error_code, -1);
    tf_error_set_error(error, error_code, error_msg, error_detail TSRMLS_CC);
    zval_ptr_dtor(&error_code);
}

inline zval * tf_error_get_error_code(zval *error TSRMLS_CC) {
    return zend_read_property(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_CODE), 1 TSRMLS_CC);
}

inline zval * tf_error_get_error_msg(zval *error TSRMLS_CC) {
    return zend_read_property(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_MSG), 1 TSRMLS_CC);
}

inline zval * tf_error_get_error_detail(zval *error TSRMLS_CC) {
    return zend_read_property(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_DETAIL), 1 TSRMLS_CC);
}

void tf_error_clear(zval *error TSRMLS_DC) {
    zend_update_property_long(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_CODE), 0 TSRMLS_CC);
    zend_update_property_null(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_MSG) TSRMLS_CC);
    zend_update_property_null(tf_error_ce, error, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_DETAIL) TSRMLS_CC);
}

PHP_METHOD(tf_error, setError) {
    zval *error_code, *error_msg = NULL, *error_detail = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|zz", &error_code, &error_msg, &error_detail) != SUCCESS) {
        return;
    }

    convert_to_long(error_code);
    if (error_msg) {
        convert_to_string(error_msg);
    }
    if (error_detail) {
        convert_to_string(error_detail);
    }
    tf_error_set_error(getThis(), error_code, error_msg, error_detail TSRMLS_CC);
}

PHP_METHOD(tf_error, setErrorMsg) {
    zval *error_msg, *error_detail = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &error_msg, &error_detail) != SUCCESS) {
        return;
    }

    convert_to_string(error_msg);
    if (error_detail) {
        convert_to_string(error_detail);
    }
    tf_error_set_error_msg(getThis(), error_msg, error_detail TSRMLS_CC);
}

PHP_METHOD(tf_error, getErrorCode) {
    zval *error_code = tf_error_get_error_code(getThis() TSRMLS_CC);
    RETURN_ZVAL(error_code, 1, 0);
}

PHP_METHOD(tf_error, getErrorMsg) {
    zval *error_msg = tf_error_get_error_msg(getThis() TSRMLS_CC);
    RETURN_ZVAL(error_msg, 1, 0);
}

PHP_METHOD(tf_error, getErrorDetail) {
    zval *error_detail = tf_error_get_error_detail(getThis() TSRMLS_CC);
    RETURN_ZVAL(error_detail, 1, 0);
}

zend_function_entry tf_error_methods[] = {
    PHP_ME(tf_error, setError, tf_error_setError_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_error, setErrorMsg, tf_error_setErrorMsg_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_error, getErrorCode, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_error, getErrorMsg, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_error, getErrorDetail, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_error) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Error", tf_error_methods);
    tf_error_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
    
    zend_declare_property_long(tf_error_ce, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_CODE), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_error_ce, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_MSG), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_error_ce, ZEND_STRL(TF_ERROR_PROPERTY_NAME_ERROR_DETAIL), ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}