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
#include "ext/session/php_session.h"
#include "tf_session.h"
#include "tf_session_handler.h"
#include "tf_common.h"
#include "tf_redis.h"
#include "tf.h"

zend_class_entry *tf_session_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_session___construct_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_session_get_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_session_set_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_session_delete_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_session_setSessionId_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, session_id)
ZEND_END_ARG_INFO()

zval * tf_session_constructor(zval *session, zval *config TSRMLS_DC) {
    if (!session) {
        MAKE_STD_ZVAL(session);
        object_init_ex(session, tf_session_ce);
    }

    zval **server;
    if (zend_hash_find(Z_ARRVAL_P(config), ZEND_STRS("server"), (void **)&server) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "session config miss server");
    }

    zval **password = NULL;
    zend_hash_find(Z_ARRVAL_P(config), ZEND_STRS("password"), (void **)&password);
    if (password) {
        convert_to_string(*password);
    }

    zval **index = NULL;
    zend_hash_find(Z_ARRVAL_P(config), ZEND_STRS("index"), (void **)&index);
    if (index) {
        convert_to_long(*index);
    }

    zval **serialize = NULL;
    zend_hash_find(Z_ARRVAL_P(config), ZEND_STRS("serialize"), (void **)&serialize);
    if (serialize) {
        convert_to_boolean(*serialize);
    }

    zval **prefix = NULL, *prefix_default = NULL;
    zend_hash_find(Z_ARRVAL_P(config), ZEND_STRS("prefix"), (void **)&prefix);
    if (prefix) {
        convert_to_string(*prefix);
    } else {
        MAKE_STD_ZVAL(prefix_default);
        ZVAL_STRING(prefix_default, "sess_", 1);        
    }

    zval *redis = tf_redis_constructor(NULL, *server, password ? *password : NULL, index ? *index : NULL, serialize ? *serialize : NULL, prefix ? *prefix : prefix_default TSRMLS_CC);
    zend_update_property(tf_session_ce, session, ZEND_STRL(TF_SESSION_PROPERTY_NAME_REDIS), redis TSRMLS_CC);
    zval_ptr_dtor(&redis);
    if (prefix_default) {
        zval_ptr_dtor(&prefix_default);
    }
    
    zval *handler = tf_session_handler_constructor(redis TSRMLS_CC);
    zend_update_property(tf_session_ce, session, ZEND_STRL(TF_SESSION_PROPERTY_NAME_HANDLER), handler TSRMLS_CC);
    zval_ptr_dtor(&handler);

    return session;
}

void tf_session_start(zval *session TSRMLS_DC) {
    zval *started = zend_read_property(tf_session_ce, session, ZEND_STRL(TF_SESSION_PROPERTY_NAME_STARTED), 1 TSRMLS_CC);
    if (Z_BVAL_P(started)) {
        return;
    }

    php_session_start(TSRMLS_CC);

    //default property reference to the declared zval not copy. so must assign a new zval
    zval *new_started;
    MAKE_STD_ZVAL(new_started);
    ZVAL_TRUE(new_started);
    zend_update_property(tf_session_ce, session, ZEND_STRL(TF_SESSION_PROPERTY_NAME_STARTED), new_started TSRMLS_CC);
    zval_ptr_dtor(&new_started);
}

/* Dispatched by RINIT and by php_session_destroy */
static inline void php_rinit_session_globals(TSRMLS_D) /* {{{ */
{
    PS(id) = NULL;
    PS(session_status) = php_session_none;
    PS(mod_data) = NULL;
    PS(mod_user_is_open) = 0;
    /* Do NOT init PS(mod_user_names) here! */
    PS(http_session_vars) = NULL;
}

/* Dispatched by RSHUTDOWN and by php_session_destroy */
static inline void php_rshutdown_session_globals(TSRMLS_D) /* {{{ */
{
    if (PS(http_session_vars)) {
        zval_ptr_dtor(&PS(http_session_vars));
        PS(http_session_vars) = NULL;
    }
    /* Do NOT destroy PS(mod_user_names) here! */
    if (PS(mod_data) || PS(mod_user_implemented)) {
        zend_try {
            PS(mod)->s_close(&PS(mod_data) TSRMLS_CC);
        } zend_end_try();
    }
    if (PS(id)) {
        efree(PS(id));
        PS(id) = NULL;
    }
}
/* }}} */

static int php_session_destroy(TSRMLS_D) /* {{{ */
{
    int retval = SUCCESS;

    if (PS(session_status) != php_session_active) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Trying to destroy uninitialized session");
        return FAILURE;
    }

    if (PS(id) && PS(mod)->s_destroy(&PS(mod_data), PS(id) TSRMLS_CC) == FAILURE) {
        retval = FAILURE;
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Session object destruction failed");
    }

    php_rshutdown_session_globals(TSRMLS_C);
    php_rinit_session_globals(TSRMLS_C);

    return retval;
}

PHP_METHOD(tf_session, __construct) {
    zval *config;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &config) != SUCCESS) {
        return;
    }

    tf_session_constructor(getThis(), config TSRMLS_CC);
}

PHP_METHOD(tf_session, start) {
    tf_session_start(getThis() TSRMLS_CC);
}

PHP_METHOD(tf_session, get) {
    char *name;
    int name_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) != SUCCESS) {
        RETURN_FALSE;
    }

    tf_session_start(getThis() TSRMLS_CC);

    zval **value;
    if (php_get_session_var(name, name_len, &value TSRMLS_CC) != SUCCESS) {
        RETURN_FALSE;
    }

    RETURN_ZVAL(*value, 1, 0);
}

PHP_METHOD(tf_session, set) {
    char *name;
    int name_len;
    zval *value;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &value) != SUCCESS) {
        return;
    }

    tf_session_start(getThis() TSRMLS_CC);

    php_set_session_var(name, name_len, value, NULL TSRMLS_CC);
}

PHP_METHOD(tf_session, delete) {
    char *name;
    int name_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) != SUCCESS) {
        return;
    }

    tf_session_start(getThis() TSRMLS_CC);

    PS_DEL_VARL(name, name_len);
}

PHP_METHOD(tf_session, getSessionId) {
    RETURN_STRING(PS(id), 1);
}

PHP_METHOD(tf_session, setSessionId) {
    char *session_id;
    int session_id_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &session_id, &session_id_len) != SUCCESS) {
        return;
    }

    if (PS(id)) {
        efree(PS(id));
    }
    PS(id) = estrndup(session_id, session_id_len);
}

PHP_METHOD(tf_session, destroy) {
    tf_session_start(getThis() TSRMLS_CC);
    php_session_destroy(TSRMLS_CC);
}

zend_function_entry tf_session_methods[] = {
    PHP_ME(tf_session, __construct, tf_session___construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_session, start, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_session, get, tf_session_get_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_session, set, tf_session_set_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_session, delete, tf_session_delete_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_session, getSessionId, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_session, setSessionId, tf_session_setSessionId_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_session, destroy, NULL, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};

ZEND_MINIT_FUNCTION(tf_session) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Session", tf_session_methods);
    tf_session_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(tf_session_ce, ZEND_STRL(TF_SESSION_PROPERTY_NAME_REDIS), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_session_ce, ZEND_STRL(TF_SESSION_PROPERTY_NAME_HANDLER), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_bool(tf_session_ce, ZEND_STRL(TF_SESSION_PROPERTY_NAME_STARTED), 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}