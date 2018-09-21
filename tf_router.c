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
#include "ext/standard/php_string.h"
#include "ext/pcre/php_pcre.h"
#include "tf_router.h"
#include "tf.h"
#include "tf_common.h"
#include "tf_web_application.h"

zend_class_entry *tf_router_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_router_construct_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, url)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_router_add_rule_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, search)
    ZEND_ARG_INFO(0, replace)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_router_add_rules_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, rules)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_router_get_param_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

zval * tf_router_parse_url(zval *router TSRMLS_DC) {
    zval *url = zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_URL), 1 TSRMLS_CC);
    Z_ADDREF_P(url);

    zval *rules = zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_RULES), 1 TSRMLS_CC);
    do {
        if (Z_TYPE_P(rules) != IS_ARRAY) {
            break;
        }

        zval **replacement, matches, *subpats;
        pcre_cache_entry *pce_regexp;
        char *str_key;
        int str_key_len, result_len, replace_count = 0;
        ulong index_key;
        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(rules));
             zend_hash_get_current_data(Z_ARRVAL_P(rules), (void **)&replacement) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(rules))) {
            zend_hash_get_current_key_ex(Z_ARRVAL_P(rules), &str_key, &str_key_len, &index_key, 0, NULL);
            char *replaced_str = php_pcre_replace(str_key, str_key_len - 1, Z_STRVAL_P(url), Z_STRLEN_P(url), *replacement, 0, &result_len, -1, &replace_count TSRMLS_CC);
            if (!replace_count) {
                efree(replaced_str);
                continue;
            }

            zval_ptr_dtor(&url);
            MAKE_STD_ZVAL(url);
            ZVAL_STRINGL(url, replaced_str, result_len, 0);
        }
    } while (0);

    zval *delim, *items, *params, *modules = NULL;
    char *param_key = NULL;
    MAKE_STD_ZVAL(delim);
    ZVAL_STRING(delim, "/", 1);
    MAKE_STD_ZVAL(items);
    array_init(items);
    MAKE_STD_ZVAL(params);
    array_init(params);
    php_explode(delim, url, items, LONG_MAX);
    int step = 1;
    zval *application = tf_get_application(TSRMLS_CC);
    if (Z_TYPE_P(application) != IS_NULL) {
        modules = tf_web_application_get_modules(application TSRMLS_CC);
        if (modules && Z_TYPE_P(modules) != IS_NULL) {
            step = 0;
        }
    }

    zval **tmp;
    zend_hash_move_forward(Z_ARRVAL_P(items));
    zend_hash_move_forward(Z_ARRVAL_P(items));
    zend_hash_move_forward(Z_ARRVAL_P(items));
    for (; zend_hash_get_current_data(Z_ARRVAL_P(items), (void **)&tmp) == SUCCESS;
         zend_hash_move_forward(Z_ARRVAL_P(items))) {
        char *item_str = php_trim(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp), NULL, 0, NULL, 3 TSRMLS_CC);
        if (!strlen(item_str)) {
            efree(item_str);
            continue;
        }

        if (step == 0) {
            zval **module;
            zend_bool find_module = 0;
            for(zend_hash_internal_pointer_reset(Z_ARRVAL_P(modules));
                zend_hash_get_current_data(Z_ARRVAL_P(modules), (void **)&module) == SUCCESS;
                zend_hash_move_forward(Z_ARRVAL_P(modules))) {
                if (strcmp(Z_STRVAL_PP(module), item_str) == 0) {
                    zval *module_zval;
                    MAKE_STD_ZVAL(module_zval);
                    ZVAL_STRING(module_zval, item_str, 0);
                    zend_update_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_MODULE), module_zval TSRMLS_CC);
                    zval_ptr_dtor(&module_zval);
                    find_module = 1;
                    break;
                }
            }
            if (!find_module) {
                zval *controller;
                MAKE_STD_ZVAL(controller);
                ZVAL_STRING(controller, item_str, 0);
                zend_update_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_CONTROLLER), controller TSRMLS_CC);
                zval_ptr_dtor(&controller);
                step++;
            }
        } else if (step == 1) {
            zval *controller;
            MAKE_STD_ZVAL(controller);
            ZVAL_STRING(controller, item_str, 0);
            zend_update_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_CONTROLLER), controller TSRMLS_CC);
            zval_ptr_dtor(&controller);
        } else if (step == 2) {
            zval *action;
            MAKE_STD_ZVAL(action);
            ZVAL_STRING(action, item_str, 0);
            zend_update_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_ACTION), action TSRMLS_CC);
            zval_ptr_dtor(&action);
        } else {
            if (param_key) {
                add_assoc_string(params, param_key, item_str, 0);
                efree(param_key);
                param_key = NULL;
            } else {
                param_key = item_str;
            }
        }

        step++;
    }

    zend_update_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARAMS), params TSRMLS_CC);

    zval_ptr_dtor(&params);
    zval_ptr_dtor(&delim);
    zval_ptr_dtor(&items);
    if (param_key) {
        efree(param_key);
    }
    zval_ptr_dtor(&url);

    zval *parsed;
    MAKE_STD_ZVAL(parsed);
    ZVAL_BOOL(parsed, 1);
    zend_update_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARSED), parsed TSRMLS_CC);
    zval_ptr_dtor(&parsed);
}

zval * tf_router_constructor(zval *router, char *url TSRMLS_DC) {
    if (!router) {
        MAKE_STD_ZVAL(router);
        object_init_ex(router, tf_router_ce);
    }

    zval *new_url;
    MAKE_STD_ZVAL(new_url);
    char *ptr;
    if ((ptr = strchr(url, '?'))) {
        ZVAL_STRINGL(new_url, url, ptr - url, 1);
    } else {
        ZVAL_STRING(new_url, url, 1);
    }
    zend_update_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_URL), new_url TSRMLS_CC);
    zval_ptr_dtor(&new_url);

    return router;
}

void tf_router_add_rule(zval *router, zval *search, zval *replace TSRMLS_DC) {
    zval *current_rules = zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_RULES), 1 TSRMLS_CC);
    if (Z_TYPE_P(current_rules) == IS_NULL) {
        MAKE_STD_ZVAL(current_rules);
        array_init(current_rules);
        zend_update_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_RULES), current_rules TSRMLS_CC);
        zval_ptr_dtor(&current_rules);
    }

    add_assoc_zval_ex(current_rules, Z_STRVAL_P(search), Z_STRLEN_P(search), replace);
    Z_ADDREF_P(replace);

    zend_update_property_bool(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARSED), 0 TSRMLS_CC);
}

zval * tf_router_get_module(zval *router TSRMLS_DC) {
    zval *parsed = zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARSED), 1 TSRMLS_CC);
    if (!Z_BVAL_P(parsed)) {
        tf_router_parse_url(router TSRMLS_CC);
    }

    zval *module = zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_MODULE), 1 TSRMLS_CC);
    zval *application = tf_get_application(TSRMLS_CC);
    if (Z_TYPE_P(module) == IS_NULL && Z_TYPE_P(application) != IS_NULL) {
        zval *default_module = tf_web_application_get_default_module(application TSRMLS_CC);
        if (Z_TYPE_P(default_module) != IS_NULL) {
            module = default_module;
        }
    }
    
    return module;
}

zval * tf_router_get_controller(zval *router TSRMLS_DC) {
    zval *parsed = zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARSED), 1 TSRMLS_CC);
    if (!Z_BVAL_P(parsed)) {
        tf_router_parse_url(router TSRMLS_CC);
    }

    return zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_CONTROLLER), 1 TSRMLS_CC);
}

zval * tf_router_get_action(zval *router TSRMLS_DC) {
    zval *parsed = zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARSED), 1 TSRMLS_CC);
    if (!Z_BVAL_P(parsed)) {
        tf_router_parse_url(router TSRMLS_CC);
    }

    return zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_ACTION), 1 TSRMLS_CC);
}

zval * tf_router_get_param(zval *router, char *name, uint name_len TSRMLS_DC) {
    zval *parsed = zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARSED), 1 TSRMLS_CC);
    if (!Z_BVAL_P(parsed)) {
        tf_router_parse_url(router TSRMLS_CC);
    }

    zval *params = zend_read_property(tf_router_ce, router, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARAMS), 1 TSRMLS_CC);
    if (Z_TYPE_P(params) == IS_NULL) {
        return NULL;
    }

    zval **ppzval;
    if (zend_symtable_find(Z_ARRVAL_P(params), name, name_len + 1, (void **)&ppzval) == FAILURE) {
        return NULL;
    }

    return *ppzval;
}

PHP_METHOD(tf_router, __construct) {
    char *url;
    int url_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &url, &url_len) == FAILURE) {
        return;
    }

    tf_router_constructor(getThis(), url);
}

PHP_METHOD(tf_router, getModule) {
    zval *module = tf_router_get_module(getThis() TSRMLS_CC);
    RETVAL_ZVAL(module, 1, 0);
}

PHP_METHOD(tf_router, getController) {
    zval *controller = tf_router_get_controller(getThis() TSRMLS_CC);
    RETVAL_ZVAL(controller, 1, 0);
}

PHP_METHOD(tf_router, getAction) {
    zval *action = tf_router_get_action(getThis() TSRMLS_CC);
    RETVAL_ZVAL(action, 1, 0);
}

/**
 * pattern => replacement
 * {"/room/(\d+)" => "/room/index/id/$1"} e.g
 * {"http://domain/room/(\d+)" => "http://domain/pc/room/index/id/$1"} e.g
 * {"http://(.*?)/room/(\d+)" => "http://($1)/pc/room/index/id/$2"} e.g
 */

PHP_METHOD(tf_router, addRule) {
    zval *search, *replace;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &search , &replace) == FAILURE) {
        RETURN_FALSE;
    }

    if (Z_TYPE_P(search) != IS_STRING || Z_TYPE_P(replace) != IS_STRING) {
        RETURN_FALSE;
    }

    tf_router_add_rule(getThis(), search, replace TSRMLS_CC);

    RETURN_TRUE;
}

PHP_METHOD(tf_router, addRules) {
    zval *rules = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &rules) == FAILURE) {
        RETURN_FALSE;
    }

    zval *current_rules = zend_read_property(tf_router_ce, getThis(), ZEND_STRL(TF_ROUTER_PROPERTY_NAME_RULES), 1 TSRMLS_CC);
    if (Z_TYPE_P(current_rules) == IS_NULL) {
        MAKE_STD_ZVAL(current_rules);
        array_init(current_rules);
        zend_update_property(tf_router_ce, getThis(), ZEND_STRL(TF_ROUTER_PROPERTY_NAME_RULES), current_rules TSRMLS_CC);
        zval_ptr_dtor(&current_rules);
    }

    zval **ppzval;
    for(; zend_hash_get_current_data(Z_ARRVAL_P(rules), (void **)&ppzval) == SUCCESS;
            zend_hash_move_forward(Z_ARRVAL_P(rules))) {
        if (Z_TYPE_PP(ppzval) != IS_STRING) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "rule replacement must be string");
            continue;
        }

        char *str_key = NULL;
        int str_key_len;
        ulong num_key;
        if (zend_hash_get_current_key_ex(Z_ARRVAL_P(rules), &str_key, &str_key_len, &num_key, 0, NULL) != HASH_KEY_IS_STRING) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "rule must be string");
            continue;
        }

        add_assoc_zval_ex(current_rules, str_key, str_key_len, *ppzval);
        Z_ADDREF_PP(ppzval);
    }

    zend_update_property_bool(tf_router_ce, getThis(), ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARSED), 0 TSRMLS_CC);

    RETURN_TRUE;
}

PHP_METHOD(tf_router, getRules) {
    zval *rules = zend_read_property(tf_router_ce, getThis(), ZEND_STRL(TF_ROUTER_PROPERTY_NAME_RULES), 1 TSRMLS_CC);
    RETVAL_ZVAL(rules, 1, 0);
}

PHP_METHOD(tf_router, getParams) {
    zval *params = zend_read_property(tf_router_ce, getThis(), ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARAMS), 1 TSRMLS_CC);
    RETVAL_ZVAL(params, 1, 0);
}

PHP_METHOD(tf_router, getParam) {
    char *name;
    int name_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
        return;
    }

    zval *value = tf_router_get_param(getThis(), name, name_len);
    if (value) {
        RETVAL_ZVAL(value, 1, 0);
    }
}

zend_function_entry tf_router_methods[] = {
    PHP_ME(tf_router, __construct, tf_router_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_router, getModule, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_router, getController, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_router, getAction, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_router, addRule, tf_router_add_rule_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_router, addRules, tf_router_add_rules_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_router, getRules, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_router, getParams, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_router, getParam, tf_router_get_param_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_router) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Router", tf_router_methods);

    tf_router_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

    zend_declare_property_null(tf_router_ce, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_URL), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_router_ce, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_MODULE), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_string(tf_router_ce, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_CONTROLLER), "index", ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_string(tf_router_ce, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_ACTION), "index", ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_router_ce, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_RULES), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_router_ce, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARAMS), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_bool(tf_router_ce, ZEND_STRL(TF_ROUTER_PROPERTY_NAME_PARSED), 0, ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}