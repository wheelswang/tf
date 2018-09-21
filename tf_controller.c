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
#include "SAPI.h"
#include "ext/json/php_json.h"
#include "tf.h"
#include "tf_controller.h"
#include "tf_web_application.h"
#include "tf_request.h"
#include "tf_router.h"
#include "tf_loader.h"
#include "tf_view.h"

zend_class_entry *tf_controller_ce;
extern zend_class_entry *tf_view_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_controller_assign_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_controller_render_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, tpl_name)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_controller_display_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, tpl_name)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_controller_ajaxError_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, error_msg)
	ZEND_ARG_INFO(0, error_code)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_controller_ajaxSuccess_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

void tf_controller_constructor(zval *controller, zval *request, zval *router, zval *view_ext TSRMLS_DC) {
	zend_update_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_REQUEST), request TSRMLS_CC);
	zend_update_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ROUTER), router TSRMLS_CC);
	if (view_ext) {
		zend_update_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW_EXT), view_ext TSRMLS_CC);
	}
}

zval * tf_controller_init_view(zval *controller TSRMLS_DC) {
	zval *view = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW), 1 TSRMLS_CC);
	if (Z_TYPE_P(view) == IS_NULL) {
		zval *router = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ROUTER), 1 TSRMLS_CC);
		zval *controller_name = tf_router_get_controller(router TSRMLS_CC);
		char *tpl_dir;
		zval *module_path = tf_web_application_get_module_path(tf_get_application(TSRMLS_CC) TSRMLS_CC);
		spprintf(&tpl_dir, 0, "%s/view/%s", Z_STRVAL_P(module_path), Z_STRVAL_P(controller_name));
		zval *view_ext = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW_EXT), 1 TSRMLS_CC);
		zval *tpl_dir_pzval;
		MAKE_STD_ZVAL(tpl_dir_pzval);
		ZVAL_STRING(tpl_dir_pzval, tpl_dir, 0);
		view = tf_view_constructor(NULL, controller, tpl_dir_pzval, view_ext TSRMLS_CC);
		zend_update_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW), view TSRMLS_CC);
		zval_ptr_dtor(&view);
	}

	return view;
} 

zval * tf_controller_get_param(zval *controller, char *name, uint name_len TSRMLS_DC) {
	zval *router = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ROUTER), 1 TSRMLS_CC);
	zval *pzval = tf_router_get_param(router, name, name_len TSRMLS_CC);
	if (pzval) {
		return pzval;
	}

	pzval = tf_request_query(TRACK_VARS_GET, name, name_len TSRMLS_CC);
	if (pzval) {
		return pzval;
	}

	pzval = tf_request_query(TRACK_VARS_POST, name, name_len TSRMLS_CC);
	if (pzval) {
		return pzval;
	}

	return tf_request_query(TRACK_VARS_FILES, name, name_len TSRMLS_CC);
}

zval * tf_controller_render(zval *controller, char *tpl_name, zval *params TSRMLS_DC) {
	zval *router = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ROUTER), 1 TSRMLS_CC);
	zval *action = tf_router_get_action(router TSRMLS_CC);
	zval *view = tf_controller_init_view(controller TSRMLS_CC);
	
	return tf_view_render(view, tpl_name ? tpl_name : Z_STRVAL_P(action), params TSRMLS_CC);
}

void tf_controller_display(zval *controller, char *tpl_name, zval *params TSRMLS_DC) {
	zval *router = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ROUTER), 1 TSRMLS_CC);
	zval *action = tf_router_get_action(router TSRMLS_CC);
	zval *view = tf_controller_init_view(controller TSRMLS_CC);
	
	tf_view_display(view, tpl_name ? tpl_name : Z_STRVAL_P(action), params TSRMLS_CC);
}

void tf_controller_ajax_out(int error_code, char *error_msg, int error_msg_len, zval *data) {
	zval *arr;
	MAKE_STD_ZVAL(arr);
	array_init(arr);
	add_assoc_long(arr, "errCode", error_code);
	add_assoc_stringl(arr, "errMsg", error_msg, error_msg_len, 1);
	if (!data) {
		MAKE_STD_ZVAL(data);
		object_init(data);
		add_assoc_zval(arr, "data", data);
	} else {
		add_assoc_zval(arr, "data", data);
		Z_ADDREF_P(data);
	}

	smart_str buf = {0};
	php_json_encode(&buf, arr, PHP_JSON_UNESCAPED_UNICODE);
	smart_str_0(&buf);

	sapi_header_line ctr = {ZEND_STRL("Content-Type: application/json; charset=utf-8"), 200};
	sapi_header_op(SAPI_HEADER_REPLACE, &ctr TSRMLS_CC);
	php_printf(buf.c);

	efree(buf.c);
	zval_ptr_dtor(&arr);
}

PHP_METHOD(tf_controller, getRequest) {
	zval *request = zend_read_property(tf_controller_ce, getThis(), ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_REQUEST), 1 TSRMLS_CC);
	RETURN_ZVAL(request, 1, 0);
}

PHP_METHOD(tf_controller, getRouter) {
	zval *router = zend_read_property(tf_controller_ce, getThis(), ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ROUTER), 1 TSRMLS_CC);
	RETURN_ZVAL(router, 1, 0);
}

PHP_METHOD(tf_controller, assign) {
	zval *view = tf_controller_init_view(getThis() TSRMLS_CC);
	uint argc = ZEND_NUM_ARGS();
	if (argc == 1) {
		zval *arr;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &arr) == FAILURE) {
			return;
		}
		tf_view_assign_multi(view, arr TSRMLS_CC);
	} else if (argc == 2) {
		zval *value;
		char *name;
		uint name_len;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &value) == FAILURE) {
			return;
		}

		tf_view_assign_single(view, name, name_len, value TSRMLS_CC);
	} else {
		WRONG_PARAM_COUNT;
	}
}

PHP_METHOD(tf_controller, render) {
	char *tpl_name = NULL;
	int tpl_name_len;
	zval *params = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sa", &tpl_name, &tpl_name_len, &params) != SUCCESS) {
		return;
	}

	zval *ret = tf_controller_render(getThis(), tpl_name, params TSRMLS_CC);
	if (ret) {
		RETURN_ZVAL(ret, 0, 1);
	}
}

PHP_METHOD(tf_controller, display) {
	char *tpl_name = NULL;
	int tpl_name_len;
	zval *params = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sa", &tpl_name, &tpl_name_len, &params) != SUCCESS) {
		return;
	}

	tf_controller_display(getThis(), tpl_name, params TSRMLS_CC);
}

PHP_METHOD(tf_controller, ajaxError) {
	char *error_msg;
	int error_msg_len;
	long error_code = 100;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &error_msg, &error_msg_len, &error_code) != SUCCESS) {
		return;
	}

	tf_controller_ajax_out(error_code, error_msg, error_msg_len, NULL);

	// https://grokbase.com/t/php/php-internals/04727wymqn/zend-api-correct-way-to-terminate-entire-request-from-within-function
	zend_bailout();
}

PHP_METHOD(tf_controller, ajaxSuccess) {
	zval *data = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_DC, "|a", &data) != SUCCESS) {
		return;
	}

	tf_controller_ajax_out(0, "", 0, data);

	zend_bailout();
}

PHP_METHOD(tf_controller, error) {
	char *error;
	int error_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_DC, "|s", &error, &error_len) != SUCCESS) {
		return;
	}

	tf_web_application_run_error_controller(tf_get_application(TSRMLS_CC), E_USER_ERROR, error, NULL, 0 TSRMLS_CC);

	zend_bailout();
}

zend_function_entry tf_controller_methods[] = {
	PHP_ME(tf_controller, getRequest, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(tf_controller, getRouter, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(tf_controller, assign, tf_controller_assign_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_controller, render, tf_controller_render_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_controller, display, tf_controller_display_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_controller, ajaxError, tf_controller_ajaxError_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_controller, ajaxSuccess, tf_controller_ajaxSuccess_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_controller, error, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_controller) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Controller", tf_controller_methods);
    tf_controller_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
    
    zend_declare_property_null(tf_controller_ce, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_REQUEST), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_controller_ce, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_controller_ce, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ROUTER), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_string(tf_controller_ce, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW_EXT), "php", ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}
