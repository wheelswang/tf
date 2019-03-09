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
#include "Zend/zend_interfaces.h"
#include "tf.h"
#include "tf_config.h"
#include "tf_application.h"
#include "tf_loader.h"
#include "tf_error.h"
#include "tf_db.h"
#include "tf_redis.h"
#include "tf_session.h"
#include "tf_session_handler.h"
#include "tf_logger.h"

zend_class_entry *tf_application_ce;
extern zend_class_entry *tf_error_ce;
extern zend_class_entry *tf_web_application_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_application_getSession_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, session_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_application_getRedis_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

zval * tf_application_constructor(zval *application, char *config_file, int config_file_len, char *config_section, int config_section_len TSRMLS_DC) {
    if (Z_TYPE_P(tf_get_application(TSRMLS_CC)) != IS_NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "application has already been initialized");
    }
    tf_set_application(application TSRMLS_CC);

    zval *config = tf_config_constructor(NULL, config_file, config_file_len, config_section, config_section_len TSRMLS_CC);
    zend_update_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_CONFIG), config TSRMLS_CC);
    zval_ptr_dtor(&config);

    zval *application_root = tf_config_get(config, ZEND_STRL("root") TSRMLS_CC);
    if (!application_root) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "root not defined");
    }
    convert_to_string(application_root);
    zend_update_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_ROOT_PATH), application_root TSRMLS_CC);
    
    zval *load_paths = tf_config_get(config, ZEND_STRL("loader.paths") TSRMLS_CC);
    zval *loader = tf_loader_constructor(NULL, application_root, load_paths TSRMLS_CC);
    zend_update_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_LOADER), loader TSRMLS_CC);
    zval_ptr_dtor(&loader);

    zval *use_session = tf_config_get(config, ZEND_STRL("session.use") TSRMLS_CC);
    if (use_session) {
        convert_to_boolean(use_session);
    }
    if (!use_session || Z_BVAL_P(use_session)) {
        zval *session_config = tf_config_get(config, ZEND_STRL("session"));
        if (session_config) {
            convert_to_array(session_config);
            zval *session = tf_session_constructor(NULL, session_config TSRMLS_CC);
               zend_update_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_SESSION), session TSRMLS_CC);
               zval_ptr_dtor(&session);
        }
    }
}

zval * tf_application_get_root_path(zval *application TSRMLS_DC) {
    return zend_read_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_ROOT_PATH), 1 TSRMLS_CC);
}

zval * tf_application_get_config(zval *application TSRMLS_DC) {
    return zend_read_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_CONFIG), 1 TSRMLS_CC);
}

zval * tf_application_get_db(zval *application TSRMLS_DC) {
    zval *db = zend_read_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_DB), 1 TSRMLS_CC);
    if (Z_TYPE_P(db) != IS_NULL) {
        return db;
    }

    zval *config = tf_application_get_config(application TSRMLS_CC);
    zval *server = tf_config_get(config, ZEND_STRL("db.server"));
    zval *user = tf_config_get(config, ZEND_STRL("db.user"));
    zval *password = tf_config_get(config, ZEND_STRL("db.password"));
    zval *dbname = tf_config_get(config, ZEND_STRL("db.dbname"));
    zval *charset = tf_config_get(config, ZEND_STRL("db.charset"));
    zval *persistent = tf_config_get(config, ZEND_STRL("db.persistent"));
    zval *slave_configs = tf_config_get(config, ZEND_STRL("db.slaves"));
    if (!server || !user || !password || !dbname) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "db config miss");
    }

    convert_to_string(server);
    convert_to_string(user);
    convert_to_string(password);
    convert_to_string(dbname);
    if (charset) {
        convert_to_string(charset);
    }
    if (persistent) {
        convert_to_boolean(persistent);
    }
    if (slave_configs) {
        convert_to_array(slave_configs);
    }

    db = tf_db_constructor(NULL, server, user, password, dbname, charset, persistent, slave_configs TSRMLS_CC);
    zend_update_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_DB), db TSRMLS_CC);
    zval_ptr_dtor(&db);

    return db;
}

zval * tf_application_get_redis(zval *application, char *name, int name_len TSRMLS_DC) {
    zval *redis_array = zend_read_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_REDIS_ARRAY), 1 TSRMLS_CC);
    if (Z_TYPE_P(redis_array) == IS_ARRAY) {
        zval **redis;
        if (zend_hash_find(Z_ARRVAL_P(redis_array), name, name_len + 1, (void **)&redis) == SUCCESS) {
            return *redis;
        }
    }

    if (Z_TYPE_P(redis_array) == IS_NULL) {
        zval *new_redis_array;
        MAKE_STD_ZVAL(new_redis_array);
        array_init(new_redis_array);
        redis_array = new_redis_array;
        zend_update_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_REDIS_ARRAY), redis_array TSRMLS_CC);
        zval_ptr_dtor(&redis_array);
    }

    zval *config = tf_application_get_config(application TSRMLS_CC);
    zval *redis_config = tf_config_get(config, name, name_len TSRMLS_CC);
    if (!redis_config || Z_TYPE_P(redis_config) != IS_ARRAY) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "redis config miss");
    }

    zval **server = NULL;
    if (zend_hash_find(Z_ARRVAL_P(redis_config), ZEND_STRS("server"), (void **)&server) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "redis config miss server");
    }


    zval **password = NULL;
    zend_hash_find(Z_ARRVAL_P(redis_config), ZEND_STRS("password"), (void **)&password);
    zval **index = NULL;
    zend_hash_find(Z_ARRVAL_P(redis_config), ZEND_STRS("index"), (void **)&index);
    zval **serialize = NULL;
    zend_hash_find(Z_ARRVAL_P(redis_config), ZEND_STRS("serialize"), (void **)&serialize);
    zval **prefix = NULL;
    zend_hash_find(Z_ARRVAL_P(redis_config), ZEND_STRS("prefix"), (void **)&prefix);

    convert_to_string(*server);
    if (password) {
        convert_to_string(*password);
    }
    if (index) {
        convert_to_long(*index);
    }
    if (serialize) {
        convert_to_boolean(*serialize);
    }
    if (prefix) {
        convert_to_string(*prefix);
    }

    zval *redis = tf_redis_constructor(NULL, *server, password ? *password : NULL, index ? *index : NULL, serialize ? *serialize : NULL, prefix ? *prefix : NULL TSRMLS_CC);
    add_assoc_zval(redis_array, name, redis);

    return redis;
}

zval * tf_application_get_session(zval *application TSRMLS_DC) {
    zval *session = zend_read_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_SESSION), 1 TSRMLS_CC);
    return session;
}

zval * tf_application_get_logger(zval *application TSRMLS_DC) {
    zval *logger = zend_read_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_LOGGER), 1 TSRMLS_CC);
    if (Z_TYPE_P(logger) != IS_NULL) {
        return logger;
    }

    zval *root_path = tf_application_get_root_path(application TSRMLS_CC);
    zval *config = tf_application_get_config(application TSRMLS_CC);
    zval *max_size = tf_config_get(config, ZEND_STRL("logMaxSize") TSRMLS_CC);
    if (max_size) {
        convert_to_long(max_size);
    }
    char *log_path;
    if (instanceof_function(Z_OBJCE_P(application), tf_web_application_ce TSRMLS_CC)) {
        spprintf(&log_path, 0, "%s/log/web", Z_STRVAL_P(root_path));
    } else {
        spprintf(&log_path, 0, "%s/log/console", Z_STRVAL_P(root_path));
    }

    logger = tf_logger_constructor(NULL, log_path, strlen(log_path), max_size ? Z_LVAL_P(max_size) : 0 TSRMLS_CC);
    zend_update_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_LOGGER), logger TSRMLS_CC);
    efree(log_path);
    zval_ptr_dtor(&logger);

    return logger;
}

inline zval * tf_application_get_loader(zval *application TSRMLS_DC) {
    return zend_read_property(tf_application_ce, application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_LOADER), 1 TSRMLS_CC);
}

PHP_METHOD(tf_application, getConfig) {
    zval *config = tf_application_get_config(getThis() TSRMLS_CC);
    RETURN_ZVAL(config, 1, 0);
}

PHP_METHOD(tf_application, getDB) {
    zval *db = tf_application_get_db(getThis() TSRMLS_CC);
    RETURN_ZVAL(db, 1, 0);
}

PHP_METHOD(tf_application, getRedis) {
    char *name = NULL;
    int name_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &name, &name_len) != SUCCESS) {
        RETURN_FALSE;
    }

    zval *redis;
    if (name == NULL) {
        redis = tf_application_get_redis(getThis(), ZEND_STRL("redis") TSRMLS_CC);
    } else {
        redis = tf_application_get_redis(getThis(), name, name_len TSRMLS_CC);
    }

    RETURN_ZVAL(redis, 1, 0);
}

PHP_METHOD(tf_application, getSession) {
    zval *session = zend_read_property(tf_application_ce, getThis(), ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_SESSION), 1 TSRMLS_CC);
    RETURN_ZVAL(session, 1, 0);
}

PHP_METHOD(tf_application, getLogger) {
    zval *logger = tf_application_get_logger(getThis() TSRMLS_CC);
    RETURN_ZVAL(logger, 1, 0);
}

PHP_METHOD(tf_application, getLoader) {
    zval *loader = tf_application_get_loader(getThis() TSRMLS_CC);
    RETURN_ZVAL(loader, 1, 0);
}

zend_function_entry tf_application_methods[] = {
    PHP_ME(tf_application, getConfig, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_application, getDB, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_application, getRedis, tf_application_getRedis_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_application, getSession, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_application, getLogger, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_application, getLoader, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_application) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Application", tf_application_methods);
    tf_application_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
    tf_application_ce->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;

    zend_declare_property_null(tf_application_ce, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_CONFIG), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(tf_application_ce, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_ROOT_PATH), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(tf_application_ce, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_DB), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(tf_application_ce, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_REDIS_ARRAY), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(tf_application_ce, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_SESSION), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(tf_application_ce, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_LOGGER), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(tf_application_ce, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_LOADER), ZEND_ACC_PROTECTED TSRMLS_CC);

    return SUCCESS;
}

