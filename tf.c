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
#include "tf.h"
#include "tf_error.h"
#include "tf_application.h"
#include "tf_session.h"
#include "tf_logger.h"

#define PHP_TF_VERSION "1.0"

ZEND_DECLARE_MODULE_GLOBALS(tf);

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("tf.environ", "product", PHP_INI_SYSTEM, OnUpdateString, environ, zend_tf_globals, tf_globals)
PHP_INI_END();

ZEND_BEGIN_ARG_INFO_EX(tf_log_common_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, msg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_setError_arginfo, 0, 0, 3)
    ZEND_ARG_INFO(0, error_code)
    ZEND_ARG_INFO(0, error_msg)
    ZEND_ARG_INFO(0, error_detail)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_setErrorMsg_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, error_msg)
    ZEND_ARG_INFO(0, error_detail)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_getRedis_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()


zend_class_entry *tf_ce;
extern zend_class_entry *tf_error_ce;

inline zval * tf_set_application(zval *application TSRMLS_DC) {
    zend_update_static_property(tf_ce, ZEND_STRL(TF_PROPERTY_NAME_APPLICATION), application TSRMLS_CC);
}

inline zval * tf_get_application(TSRMLS_DC) {
    return zend_read_static_property(tf_ce, ZEND_STRL(TF_PROPERTY_NAME_APPLICATION), 1 TSRMLS_CC);
}

inline zval * tf_get_error(TSRMLS_DC) {
    return zend_read_static_property(tf_ce, ZEND_STRL(TF_PROPERTY_NAME_ERROR), 1 TSRMLS_CC);
}

void tf_set_error(int error_code, char *error_msg, int error_msg_len, char *error_detail, int error_detail_len TSRMLS_DC) {
    zval *error_code_pzval, *error_msg_pzval = NULL, *error_detail_pzval = NULL;
    MAKE_STD_ZVAL(error_code_pzval);
    ZVAL_LONG(error_code_pzval, error_code);
    if (error_msg) {
        MAKE_STD_ZVAL(error_msg_pzval);
        ZVAL_STRINGL(error_msg_pzval, error_msg, error_msg_len, 1);
    }
    if (error_detail) {
        MAKE_STD_ZVAL(error_detail_pzval);
        ZVAL_STRINGL(error_detail_pzval, error_detail, error_detail_len, 1);
    }
    tf_error_set_error(tf_get_error(TSRMLS_CC), error_code_pzval, error_msg_pzval, error_detail_pzval TSRMLS_CC);
    zval_ptr_dtor(&error_code_pzval);
    if (error_msg_pzval) {
        zval_ptr_dtor(&error_msg_pzval);
    }
    if (error_detail_pzval) {
        zval_ptr_dtor(&error_detail_pzval);
    }
}

inline void tf_set_error_msg(char *error_msg, int error_msg_len, char *error_detail, int error_detail_len TSRMLS_DC) {
    tf_set_error(-1, error_msg, error_msg_len, error_detail, error_detail_len TSRMLS_CC);
}

inline void tf_set_error_simple_msg(char *error_msg, int error_msg_len TSRMLS_DC) {
    tf_set_error(-1, error_msg, error_msg_len, NULL, 0 TSRMLS_CC);
}

inline void tf_clear_error(TSRMLS_DC) {
    tf_error_clear(tf_get_error(TSRMLS_CC) TSRMLS_CC);
}

PHP_METHOD(tf, getEnv) {
    char *environ = TF_G(environ);
    RETURN_STRING(environ, 1);
}

PHP_METHOD(tf, getApp) {
    zval *application = tf_get_application(TSRMLS_CC);
    RETURN_ZVAL(application, 1, 0);
}

PHP_METHOD(tf, getConfig) {
    zval *application = tf_get_application(TSRMLS_CC);
    if (Z_TYPE_P(application) == IS_NULL) {
        RETURN_FALSE;
    }

    zval *config = tf_application_get_config(application TSRMLS_CC);
    RETURN_ZVAL(config, 1, 0);
}

PHP_METHOD(tf, getErrorCode) {
    zval *error = tf_get_error(TSRMLS_CC);
    zval *error_code = tf_error_get_error_code(error TSRMLS_CC);
    RETURN_ZVAL(error_code, 1, 0);
}

PHP_METHOD(tf, getErrorMsg) {
    zval *error = tf_get_error(TSRMLS_CC);
    zval *error_msg = tf_error_get_error_msg(error TSRMLS_CC);
    RETURN_ZVAL(error_msg, 1, 0);
}

PHP_METHOD(tf, getErrorDetail) {
    zval *error = tf_get_error(TSRMLS_CC);
    zval *error_detail = tf_error_get_error_detail(error TSRMLS_CC);
    RETURN_ZVAL(error_detail, 1, 0);
}

PHP_METHOD(tf, setError) {
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
    zval *error = tf_get_error(TSRMLS_CC);
    tf_error_set_error(error, error_code, error_msg, error_detail TSRMLS_CC);
}

PHP_METHOD(tf, setErrorMsg) {
    zval *error_msg, *error_detail = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &error_msg, &error_detail) != SUCCESS) {
        return;
    }

    convert_to_string(error_msg);
    if (error_detail) {
        convert_to_string(error_detail);
    }

    zval *error = tf_get_error(TSRMLS_CC);
    tf_error_set_error_msg(error, error_msg, error_detail TSRMLS_CC);
}

PHP_METHOD(tf, getDB) {
    zval *application = tf_get_application(TSRMLS_CC);
    if (Z_TYPE_P(application) == IS_NULL) {
        RETURN_FALSE;
    }

    zval *db = tf_application_get_db(application TSRMLS_CC);

    RETURN_ZVAL(db, 1, 0);
}

PHP_METHOD(tf, getRedis) {
    zval *application = tf_get_application(TSRMLS_CC);
    if (Z_TYPE_P(application) == IS_NULL) {
        RETURN_FALSE;
    }

    char *name = NULL;
    int name_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &name, &name_len) != SUCCESS) {
        RETURN_FALSE;
    }

    zval *redis;
    if (name == NULL) {
        redis = tf_application_get_redis(application, ZEND_STRL("redis") TSRMLS_CC);
    } else {
        redis = tf_application_get_redis(application, name, name_len TSRMLS_CC);
    }

    RETURN_ZVAL(redis, 1, 0);
}

PHP_METHOD(tf, getSession) {
    zval *application = tf_get_application(TSRMLS_CC);
    if (Z_TYPE_P(application) == IS_NULL) {
        RETURN_FALSE;
    }

    zval *session = tf_application_get_session(application TSRMLS_CC);

    RETURN_ZVAL(session, 1, 0);
}

PHP_METHOD(tf, logInfo) {
    char *msg;
    int msg_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &msg, &msg_len) != SUCCESS) {
        return;
    }

    zval *application = tf_get_application(TSRMLS_CC);
    if (Z_TYPE_P(application) == IS_NULL) {
        return;
    }
    zval *logger = tf_application_get_logger(application TSRMLS_CC);

    tf_logger_write(logger, msg, msg_len, TF_LOGGER_TYPE_INFO TSRMLS_CC);
}

PHP_METHOD(tf, logWarn) {
    char *msg;
    int msg_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &msg, &msg_len) != SUCCESS) {
        return;
    }

    zval *application = tf_get_application(TSRMLS_CC);
    if (Z_TYPE_P(application) == IS_NULL) {
        return;
    }
    zval *logger = tf_application_get_logger(application TSRMLS_CC);

    tf_logger_write(logger, msg, msg_len, TF_LOGGER_TYPE_WARN TSRMLS_CC);
}

PHP_METHOD(tf, logError) {
    char *msg;
    int msg_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &msg, &msg_len) != SUCCESS) {
        return;
    }

    zval *application = tf_get_application(TSRMLS_CC);
    if (Z_TYPE_P(application) == IS_NULL) {
        return;
    }
    zval *logger = tf_application_get_logger(application TSRMLS_CC);

    tf_logger_write(logger, msg, msg_len, TF_LOGGER_TYPE_ERROR TSRMLS_CC);
}

zend_function_entry tf_methods[] = {
    PHP_ME(tf, getEnv, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, getApp, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, getConfig, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, getErrorCode, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, getErrorMsg, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, getErrorDetail, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, setError, tf_setError_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, setErrorMsg, tf_setErrorMsg_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, getDB, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, getRedis, tf_getRedis_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, getSession, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, logInfo, tf_log_common_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, logWarn, tf_log_common_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(tf, logError, tf_log_common_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};

PHP_GINIT_FUNCTION(tf) {
    tf_globals->configs_cache = NULL;
    tf_globals->config_data_tmp = NULL;
    tf_globals->config_section = NULL;
}

PHP_MINIT_FUNCTION(tf) {
    REGISTER_INI_ENTRIES();

    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF", tf_methods);
    tf_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
    zend_declare_property_null(tf_ce, ZEND_STRL(TF_PROPERTY_NAME_APPLICATION), ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);
    zend_declare_property_null(tf_ce, ZEND_STRL(TF_PROPERTY_NAME_ERROR), ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);

    if (ZEND_MODULE_STARTUP_N(tf_application)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_console_application)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_web_application)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_request)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_router)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_config)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_loader)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_controller)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_view)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_model)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_error)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_db)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_db_model)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_redis)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_session_handler)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_session)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }
    if (ZEND_MODULE_STARTUP_N(tf_logger)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {
        return FAILURE;
    }

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(tf) {
    if (TF_G(configs_cache)) {
        zend_hash_destroy(TF_G(configs_cache));
        pefree(TF_G(configs_cache), 1);
        TF_G(configs_cache) = NULL;
    }

    return SUCCESS;
}

PHP_RINIT_FUNCTION(tf) {
    zval *error;
    MAKE_STD_ZVAL(error);
    object_init_ex(error, tf_error_ce);
    zend_update_static_property(tf_ce, ZEND_STRL(TF_PROPERTY_NAME_ERROR), error TSRMLS_CC);
    zval_ptr_dtor(&error);

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(tf) {
    if (TF_G(config_data_tmp)) {
        zval_ptr_dtor(&TF_G(config_data_tmp));
        TF_G(config_data_tmp) = NULL;
    }

    if (TF_G(config_section)) {
        efree(TF_G(config_section));
        TF_G(config_section) = NULL;
    }

    return SUCCESS;
}

zend_module_dep tf_deps[] = {
    ZEND_MOD_REQUIRED("spl")
    ZEND_MOD_REQUIRED("session")
    ZEND_MOD_REQUIRED("pcre")
    ZEND_MOD_REQUIRED("json")
    ZEND_MOD_REQUIRED("pdo_mysql")
    ZEND_MOD_REQUIRED("redis")
    ZEND_MOD_END
};

zend_function_entry tf_functions[] = {
    {NULL, NULL, NULL}
};

zend_module_entry tf_module_entry = {
    STANDARD_MODULE_HEADER_EX,
    NULL,
    tf_deps,
    "tf",
    tf_functions,
    PHP_MINIT(tf),
    PHP_MSHUTDOWN(tf),
    PHP_RINIT(tf),
    PHP_RSHUTDOWN(tf),
    NULL, //PHP_MINFO(tf),
    PHP_TF_VERSION,
    PHP_MODULE_GLOBALS(tf),
    NULL,
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_TF
ZEND_GET_MODULE(tf)
#endif

