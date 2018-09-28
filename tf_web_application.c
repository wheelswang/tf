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
#include "ext/standard/php_filestat.h"
#include "tf.h"
#include "tf_application.h"
#include "tf_web_application.h"
#include "tf_router.h"
#include "tf_loader.h"
#include "tf_config.h"
#include "tf_request.h"
#include "tf_controller.h"
#include "tf_common.h"

zend_class_entry *tf_web_application_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_web_application_construct_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, config_file)
    ZEND_ARG_INFO(0, config_section)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_web_application_set_auto_display_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, auto_display)
ZEND_END_ARG_INFO()

extern zend_class_entry *tf_application_ce;

void tf_web_application_set_module(zval *web_application, zval *module TSRMLS_DC) {
    zend_update_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULE), module TSRMLS_CC);
    zval *application_root = tf_application_get_root_path(web_application TSRMLS_CC);
    char *module_path;
    if (Z_TYPE_P(module) != IS_NULL) {
        spprintf(&module_path, 0, "%s/application/web/%s", Z_STRVAL_P(application_root), Z_STRVAL_P(module));
    } else {
        spprintf(&module_path, 0, "%s/application/web", Z_STRVAL_P(application_root));
    }

    zval *module_path_pzval;
    MAKE_STD_ZVAL(module_path_pzval);
    ZVAL_STRING(module_path_pzval, module_path, 0);
    zend_update_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULE_PATH), module_path_pzval TSRMLS_CC);
    zval_ptr_dtor(&module_path_pzval);
}

zval * tf_web_application_get_auto_display(zval *web_application TSRMLS_DC) {
    return zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_AUTO_DISPLAY), 1 TSRMLS_CC);
}

zval * tf_web_application_get_modules(zval *web_application TSRMLS_DC) {
    return zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULES), 1 TSRMLS_CC);
}

zval * tf_web_application_get_default_module(zval *web_application TSRMLS_DC) {
    return zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_DEFAULT_MODULE), 1 TSRMLS_CC);
}

zval * tf_web_application_get_module_path(zval *web_application TSRMLS_DC) {
    return zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULE_PATH), 1 TSRMLS_CC);
}

void * tf_web_application_run_error_controller(zval *web_application, int error_type, char *error_msg, char *error_file, int error_lineno TSRMLS_DC) {
    zval *controller_name = zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_ERROR_CONTROLLER), 1 TSRMLS_CC);
    zval *module = zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULE), 1 TSRMLS_CC);
    zend_class_entry *controller_ce = tf_loader_load_controller_class(tf_application_get_loader(web_application TSRMLS_CC), controller_name, module TSRMLS_CC);
    if (!controller_ce) {
        php_printf("%s", error_msg);
        return;
    }

    zval *controller, *method, *error_type_pzval, *error_msg_pzval, *error_file_pzval, *error_lineno_pzval, *ret;
    MAKE_STD_ZVAL(controller);
    object_init_ex(controller, controller_ce);
    zval *request = zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_REQUEST), 1 TSRMLS_CC);
    zval *router = zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_ROUTER), 1 TSRMLS_CC);
    zval *config = zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_CONFIG), 1 TSRMLS_CC);
    zval *view_ext = tf_config_get(config, "view.ext" TSRMLS_CC);
    if (view_ext) {
        convert_to_string(view_ext);
    }
    tf_controller_constructor(controller, request, router, view_ext TSRMLS_CC);

    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "index", 1);
    MAKE_STD_ZVAL(error_type_pzval);
    ZVAL_LONG(error_type_pzval, error_type);
    MAKE_STD_ZVAL(error_msg_pzval);
    ZVAL_STRING(error_msg_pzval, error_msg, 1);
    MAKE_STD_ZVAL(error_file_pzval);
    ZVAL_STRING(error_file_pzval, error_file ? error_file : "-", 1);
    MAKE_STD_ZVAL(error_lineno_pzval);
    ZVAL_LONG(error_lineno_pzval, error_lineno);
    MAKE_STD_ZVAL(ret);
    zval *params[4] = {error_type_pzval, error_msg_pzval, error_file_pzval, error_lineno_pzval};
    call_user_function(&controller_ce->function_table, &controller, method, ret, 4, params TSRMLS_CC);
    zval_ptr_dtor(&method);
    zval_ptr_dtor(&error_type_pzval);
    zval_ptr_dtor(&error_msg_pzval);
    zval_ptr_dtor(&error_file_pzval);
    zval_ptr_dtor(&error_lineno_pzval);
    zval_ptr_dtor(&ret);

    zval *auto_display = zend_read_property(tf_web_application_ce, web_application, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_AUTO_DISPLAY), 1 TSRMLS_CC);
    if (Z_BVAL_P(auto_display) && !tf_request_is_ajax(TSRMLS_DC)) {
        tf_controller_display(controller, NULL, NULL TSRMLS_CC);
    }

    zval_ptr_dtor(&controller);
}

PHP_METHOD(tf_web_application, __construct) {
    char *config_file, *config_section = NULL;
    int config_file_len, config_section_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &config_file, &config_file_len, &config_section, &config_section_len) == FAILURE) {
        return;
    }
    
    tf_application_constructor(getThis(), config_file, config_file_len, config_section, config_section_len TSRMLS_CC);

    zval *config = zend_read_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_CONFIG), 1 TSRMLS_CC);
    zval *modules = tf_config_get(config, "module.availables" TSRMLS_CC);
    if (modules) {
        zval *modules_pzval, *delim;
        MAKE_STD_ZVAL(modules_pzval);
        array_init(modules_pzval);
        MAKE_STD_ZVAL(delim);
        ZVAL_STRING(delim, ",", 1);
        php_explode(delim, modules, modules_pzval, LONG_MAX);
        zend_update_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULES), modules_pzval TSRMLS_CC);
        zval_ptr_dtor(&modules_pzval);
        zval_ptr_dtor(&delim);

        zval *default_module = tf_config_get(config, "module.default" TSRMLS_CC);
        if (default_module) {
            zend_update_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_DEFAULT_MODULE), default_module TSRMLS_CC);
        }
    }

    zval *request = tf_request_constructor(NULL TSRMLS_CC);
    zend_update_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_REQUEST), request TSRMLS_CC);
    zval_ptr_dtor(&request);

    zval *url = tf_request_get_url(request TSRMLS_CC);
    zval *router = tf_router_constructor(NULL, Z_STRVAL_P(url) TSRMLS_CC);
    zend_update_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_ROUTER), router TSRMLS_CC);
    zval_ptr_dtor(&router);
    zval *rules = tf_config_get(config, "router.rules");
    if (rules && Z_TYPE_P(rules) == IS_ARRAY) {
        zval **rule;
        zval **search, **replace;
        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(rules));
             zend_hash_get_current_data(Z_ARRVAL_P(rules), (void **)&rule) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(rules))) {
            if (Z_TYPE_PP(rule) != IS_ARRAY) {
                continue;
            }
            if (zend_hash_find(Z_ARRVAL_PP(rule), ZEND_STRS("search"), (void **)&search) != SUCCESS){
                continue;
            }
            if (zend_hash_find(Z_ARRVAL_PP(rule), ZEND_STRS("replace"), (void **)&replace) != SUCCESS){
                continue;
            }
            if (Z_TYPE_PP(search) != IS_STRING || Z_TYPE_PP(replace) != IS_STRING) {
                continue;
            }

            tf_router_add_rule(router, *search, *replace TSRMLS_CC);
        }
    }

    zval *module = tf_router_get_module(router);
    tf_web_application_set_module(getThis(), module TSRMLS_CC);

    zval *function, *ret;
    MAKE_STD_ZVAL(function);
    ZVAL_STRING(function, "register_shutdown_function", 1);
    zval *callback, *callback_name;
    MAKE_STD_ZVAL(callback);
    array_init(callback);
    MAKE_STD_ZVAL(callback_name);
    ZVAL_STRING(callback_name, "shutdownCallback", 1);
    add_next_index_zval(callback, getThis());
    Z_ADDREF_P(getThis());
    add_next_index_zval(callback, callback_name);
    zval *params[1] = {callback};
    MAKE_STD_ZVAL(ret);
    call_user_function(EG(function_table), NULL, function, ret, 1, params TSRMLS_CC);
    zval_ptr_dtor(&function);
    zval_ptr_dtor(&ret);
    zval_ptr_dtor(&callback);
}

PHP_METHOD(tf_web_application, run) {
    zval *module = zend_read_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULE), 1 TSRMLS_CC);
    // load module init
    char *module_init_file;
    zval *root_path = zend_read_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_ROOT_PATH), 1 TSRMLS_CC);
    if (Z_TYPE_P(module) != IS_NULL) {
        spprintf(&module_init_file, 0, "%s/application/web/%s/init.php", Z_STRVAL_P(root_path), Z_STRVAL_P(module));
    } else {
        spprintf(&module_init_file, 0, "%s/application/web/init.php", Z_STRVAL_P(root_path));
    }
    zval *init_file_exists;
    MAKE_STD_ZVAL(init_file_exists);
    php_stat(module_init_file, strlen(module_init_file), FS_EXISTS, init_file_exists TSRMLS_CC);
    if (Z_BVAL_P(init_file_exists)) {
        tf_loader_load_file(module_init_file TSRMLS_CC);
    }
    zval_ptr_dtor(&init_file_exists);
    efree(module_init_file);

    zval *router = zend_read_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_ROUTER), 1 TSRMLS_CC);
    zval *controller_name = tf_router_get_controller(router TSRMLS_CC);
    zend_class_entry *controller_ce = tf_loader_load_controller_class(tf_application_get_loader(getThis() TSRMLS_CC), controller_name, module TSRMLS_CC);
    if (!controller_ce) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "cannot loade controller: %s", Z_STRVAL_P(controller_name));
    }
    zval *config = zend_read_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_APPLICATION_PROPERTY_NAME_CONFIG), 1 TSRMLS_CC);
    zval *request = zend_read_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_REQUEST), 1 TSRMLS_CC);
    zval *view_ext = tf_config_get(config, "view.ext" TSRMLS_CC);
    if (view_ext) {
        convert_to_string(view_ext);
    }
    zval *controller;
    MAKE_STD_ZVAL(controller);
    object_init_ex(controller, controller_ce);
    tf_controller_constructor(controller, request, router, view_ext TSRMLS_CC);
    zval *action = tf_router_get_action(router TSRMLS_CC);
    tf_controller_run_action(controller, action TSRMLS_CC);
    zval_ptr_dtor(&controller);
}

PHP_METHOD(tf_web_application, setAutoDisplay) {
    zend_bool *auto_display;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &auto_display) == FAILURE) {
        return;
    }

    zend_update_property_bool(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_AUTO_DISPLAY), (long)auto_display TSRMLS_CC);
}

PHP_METHOD(tf_web_application, getRouter) {
    zval *router = zend_read_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_ROUTER), 1 TSRMLS_CC);
    RETURN_ZVAL(router, 1, 0);
}

PHP_METHOD(tf_web_application, getRequest) {
    zval *request = zend_read_property(tf_web_application_ce, getThis(), ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_REQUEST), 1 TSRMLS_CC);
    RETURN_ZVAL(request, 1, 0);
}

PHP_METHOD(tf_web_application, shutdownCallback) {
    if (!PG(last_error_message)) {
        return;
    }

    if (PG(last_error_type) != E_ERROR && PG(last_error_type) != E_USER_ERROR) {
        return;
    }

    tf_web_application_run_error_controller(getThis(), PG(last_error_type), PG(last_error_message), PG(last_error_file), PG(last_error_lineno) TSRMLS_CC);
}

zend_function_entry tf_web_application_methods[] = {
    PHP_ME(tf_web_application, __construct, tf_web_application_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_web_application, run, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_web_application, setAutoDisplay, tf_web_application_set_auto_display_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_web_application, getRouter, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_web_application, getRequest, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_web_application, shutdownCallback, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_web_application) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\WebApplication", tf_web_application_methods);

    tf_web_application_ce = zend_register_internal_class_ex(&ce, tf_application_ce, NULL TSRMLS_CC);
    zend_declare_property_bool(tf_web_application_ce, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_AUTO_DISPLAY), 1, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_web_application_ce, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULES), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_web_application_ce, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULE), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_web_application_ce, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_MODULE_PATH), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_web_application_ce, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_DEFAULT_MODULE), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_web_application_ce, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_REQUEST), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_web_application_ce, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_ROUTER), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_string(tf_web_application_ce, ZEND_STRL(TF_WEB_APPLICATION_PROPERTY_NAME_ERROR_CONTROLLER), "ErrorController", ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}
