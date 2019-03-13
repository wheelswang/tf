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
#include "tf_redis.h"
#include "tf_common.h"

zend_class_entry *tf_redis_ce;
zend_class_entry *redis_ce;
zend_class_entry *exception_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_redis___construct_arginfo, 0, 0, 5)
    ZEND_ARG_INFO(0, server)
    ZEND_ARG_INFO(0, password)
    ZEND_ARG_INFO(0, index)
    ZEND_ARG_INFO(0, serialize)
    ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_redis___call_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, method)
    ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

zval * tf_redis_constructor(zval *redis, zval *server, zval *password, zval *index, zval *serialize, zval *prefix TSRMLS_DC) {
    if (!redis) {
        MAKE_STD_ZVAL(redis);
        object_init_ex(redis, tf_redis_ce);
    }

    zend_update_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_SERVER), server TSRMLS_CC);
    if (password) {
        zend_update_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_PASSWORD), password TSRMLS_CC);
    }
    if (index) {
        zend_update_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_INDEX), index TSRMLS_CC);
    }
    if (serialize) {
        zend_update_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_SERIALIZE), serialize TSRMLS_CC);
    }
    if (prefix) {
        zend_update_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_PREFIX), prefix TSRMLS_CC);
    }

    return redis;
}

zend_bool tf_redis_connect(zval *redis, zend_bool is_reconnect TSRMLS_DC) {
    zval *client = zend_read_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_CLIENT), 1 TSRMLS_CC);
    if (!is_reconnect && Z_TYPE_P(client) != IS_NULL) {
        return TRUE;
    }

    MAKE_STD_ZVAL(client);
    object_init_ex(client, redis_ce);
    zend_update_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_CLIENT), client TSRMLS_CC);
    zval_ptr_dtor(&client);

    zval *server = zend_read_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_SERVER), 1 TSRMLS_CC);
    zval *host, *port, *method, *ret;
    MAKE_STD_ZVAL(host);
    MAKE_STD_ZVAL(port);
    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "connect", 1);
    MAKE_STD_ZVAL(ret);
    char *p;
    if (!(p = strstr(Z_STRVAL_P(server), ":"))) {
        ZVAL_STRINGL(host, Z_STRVAL_P(server), Z_STRLEN_P(server), 1);
        ZVAL_LONG(port, 6379);
    } else {
        ZVAL_STRINGL(host, Z_STRVAL_P(server), p - Z_STRVAL_P(server), 1);
        ZVAL_STRINGL(port, p + 1, Z_STRVAL_P(server) + Z_STRLEN_P(server) - p - 1, 1);
    }
    zval *params[2] = { host, port };

    call_user_function(&redis_ce->function_table, &client, method, ret, 2, params TSRMLS_CC);

    zval_ptr_dtor(&host);
    zval_ptr_dtor(&port);
    zval_ptr_dtor(&method);

    if (!Z_BVAL_P(ret)) {
        zval_ptr_dtor(&ret);
        tf_set_error_simple_msg(ZEND_STRL("redis connect error") TSRMLS_CC);
        if (EG(exception)) {
            zend_clear_exception(TSRMLS_CC);
        }

        return FALSE;
    }
    zval_ptr_dtor(&ret);

    zval *password = zend_read_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_PASSWORD), 1 TSRMLS_CC);
    if (Z_TYPE_P(password) != IS_NULL) {
        MAKE_STD_ZVAL(method);
        ZVAL_STRING(method, "auth", 1);
        MAKE_STD_ZVAL(ret);
        params[0] = password;

        call_user_function(&redis_ce->function_table, &client, method, ret, 1, params TSRMLS_CC);

        zval_ptr_dtor(&method);

        if (!Z_BVAL_P(ret)) {
            zval_ptr_dtor(&ret);
            tf_set_error_simple_msg(ZEND_STRL("redis password error") TSRMLS_CC);
            zend_update_property_null(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_CLIENT) TSRMLS_CC);
            if (EG(exception)) {
                zend_clear_exception(TSRMLS_CC);
            }

            return FALSE;
        }
        zval_ptr_dtor(&ret);
    }

    zval *index = zend_read_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_INDEX), 1 TSRMLS_CC);
    if (Z_LVAL_P(index) != 0) {
        MAKE_STD_ZVAL(method);
        ZVAL_STRING(method, "select", 1);
        MAKE_STD_ZVAL(ret);
        
        params[0] = index;

        call_user_function(&redis_ce->function_table, &client, method, ret, 1, params TSRMLS_CC);

        zval_ptr_dtor(&method);

        if (!Z_BVAL_P(ret)) {
            zval_ptr_dtor(&ret);
            tf_set_error_simple_msg(ZEND_STRL("redis select db error") TSRMLS_CC);
            zend_update_property_null(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_CLIENT) TSRMLS_CC);
            if (EG(exception)) {
                zend_clear_exception(TSRMLS_CC);
            }

            return FALSE;
        }
        zval_ptr_dtor(&ret);
    }

    zval *serialize = zend_read_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_SERIALIZE), 1 TSRMLS_CC);
    if (Z_BVAL_P(serialize)) {
        MAKE_STD_ZVAL(method);
        ZVAL_STRING(method, "setOption", 1);
        MAKE_STD_ZVAL(ret);

        zval **opt_serializer, **serializer_php;
        zend_hash_find(&redis_ce->constants_table, ZEND_STRS("OPT_SERIALIZER"), (void **)&opt_serializer);
        zend_hash_find(&redis_ce->constants_table, ZEND_STRS("SERIALIZER_PHP"), (void **)&serializer_php);
        params[0] = *opt_serializer;
        params[1] = *serializer_php;

        call_user_function(&redis_ce->function_table, &client, method, ret, 2, params TSRMLS_CC);

        zval_ptr_dtor(&method);

        if (!Z_BVAL_P(ret)) {
            zval_ptr_dtor(&ret);
            tf_set_error_simple_msg(ZEND_STRL("redis setOption serialize error") TSRMLS_CC);
            zend_update_property_null(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_CLIENT) TSRMLS_CC);
            if (EG(exception)) {
                zend_clear_exception(TSRMLS_CC);
            }

            return FALSE;
        }
        zval_ptr_dtor(&ret);
    }

    zval *prefix = zend_read_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_PREFIX), 1 TSRMLS_CC);
    if (Z_TYPE_P(prefix) != IS_NULL) {
        MAKE_STD_ZVAL(method);
        ZVAL_STRING(method, "setOption", 1);
        MAKE_STD_ZVAL(ret);

        zval **opt_prefix;
        zend_hash_find(&redis_ce->constants_table, ZEND_STRS("OPT_PREFIX"), (void **)&opt_prefix);
        params[0] = *opt_prefix;
        params[1] = prefix;

        call_user_function(&redis_ce->function_table, &client, method, ret, 2, params TSRMLS_CC);

        zval_ptr_dtor(&method);

        if (!Z_BVAL_P(ret)) {
            zval_ptr_dtor(&ret);
            tf_set_error_simple_msg(ZEND_STRL("redis setOption prefix error") TSRMLS_CC);
            zend_update_property_null(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_CLIENT) TSRMLS_CC);
            if (EG(exception)) {
                zend_clear_exception(TSRMLS_CC);
            }

            return FALSE;
        }
        zval_ptr_dtor(&ret);
    }

    return TRUE;
}

zval * tf_redis_exec(zval *redis, zval *method, zval *args TSRMLS_DC) {
    if (!tf_redis_connect(redis, 0 TSRMLS_CC)) {
        zval *ret;
        MAKE_STD_ZVAL(ret);
        ZVAL_FALSE(ret);
        return ret;
    }

    zval *client = zend_read_property(tf_redis_ce, redis, ZEND_STRL(TF_REDIS_PROPERTY_NAME_CLIENT), 1 TSRMLS_CC);

    int i = 0;
    for (; i < 2; i++) {
        zval *ret, **params = NULL;
        MAKE_STD_ZVAL(ret);
        int args_num = zend_hash_num_elements(Z_ARRVAL_P(args));
        if (args_num) {
            params = (zval **)emalloc(sizeof(zval **) * args_num);
        }

        int j = 0;
        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(args));
             zend_hash_has_more_elements(Z_ARRVAL_P(args)) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(args))) {
            zval **ppzval;
            zend_hash_get_current_data(Z_ARRVAL_P(args), (void **)&ppzval);
            *(params + j) = *ppzval;
            j++;
        }

        if (call_user_function(&tf_redis_ce->function_table, &client, method, ret, args_num, params TSRMLS_CC) != SUCCESS) {
            if (params) {
                efree(params);
            }
            zval_ptr_dtor(&ret);
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "called undefined method on redis");
        }

        if (params) {
            efree(params);
        }

        if (EG(exception)) {
            ZVAL_FALSE(ret);
            if (i == 0) {
                zend_clear_exception(TSRMLS_CC);
                if (!tf_redis_connect(redis, 1 TSRMLS_CC)) {
                    return ret;
                }
                
                zval_ptr_dtor(&ret);
                continue;
            }

            zval *error_msg = zend_read_property(exception_ce, EG(exception), ZEND_STRL("message"), 1 TSRMLS_CC);
            tf_set_error_simple_msg(Z_STRVAL_P(error_msg), Z_STRLEN_P(error_msg) TSRMLS_CC);
            zend_clear_exception(TSRMLS_CC);

            return ret;
        }

        return ret;
    }
}

PHP_METHOD(tf_redis, __construct) {
    zval *server, *password = NULL, *index = NULL, *serialize = NULL, *prefix = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|zzzz", &server, &password, &index, &serialize, &prefix) != SUCCESS) {
        RETURN_FALSE;
    }


    convert_to_string(server);
    if (password) {
        convert_to_string(password);
    }
    if (index) {
        convert_to_long(index);
    }
    if (serialize) {
        convert_to_boolean(serialize);
    }
    if (prefix) {
        convert_to_string(prefix);
    }
    tf_redis_constructor(getThis(), server, password, index, serialize, prefix TSRMLS_CC);
}

PHP_METHOD(tf_redis, __call) {
    zval *method, *args;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &method, &args) != SUCCESS) {
        RETURN_FALSE;
    }

    zval *ret = tf_redis_exec(getThis(), method, args TSRMLS_CC);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_redis, getClient) {
    tf_redis_connect(getThis(), 0 TSRMLS_CC);
    zval *client = zend_read_property(tf_redis_ce, getThis(), ZEND_STRL(TF_REDIS_PROPERTY_NAME_CLIENT), 1 TSRMLS_CC);
    if (client == NULL) {
        RETURN_NULL();
    }

    RETURN_ZVAL(client, 1, 0);
}

zend_function_entry tf_redis_methods[] = {
    PHP_ME(tf_redis, __construct, tf_redis___construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_redis, __call, tf_redis___call_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_redis, getClient, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_redis) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Redis", tf_redis_methods);
    tf_redis_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(tf_redis_ce, ZEND_STRL(TF_REDIS_PROPERTY_NAME_SERVER), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_redis_ce, ZEND_STRL(TF_REDIS_PROPERTY_NAME_PASSWORD), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_long(tf_redis_ce, ZEND_STRL(TF_REDIS_PROPERTY_NAME_INDEX), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_bool(tf_redis_ce, ZEND_STRL(TF_REDIS_PROPERTY_NAME_SERIALIZE), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_redis_ce, ZEND_STRL(TF_REDIS_PROPERTY_NAME_PREFIX), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_redis_ce, ZEND_STRL(TF_REDIS_PROPERTY_NAME_CLIENT), ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_class_entry **redis_tmp_ce;
    if (zend_hash_find(CG(class_table), ZEND_STRS("redis"), (void **)&redis_tmp_ce) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "fail to find redis class");
        return FAILURE;
    }
    redis_ce = *redis_tmp_ce;

    zend_class_entry **exception_tmp_ce;
    if (zend_hash_find(CG(class_table), ZEND_STRS("exception"), (void **)&exception_tmp_ce) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "fail to find exception class");
        return FAILURE;
    }
    exception_ce = *exception_tmp_ce;

    return SUCCESS;
}