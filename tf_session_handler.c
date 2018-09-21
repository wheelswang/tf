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
#include "tf_common.h"
#include "tf_redis.h"
#include "tf_session_handler.h"

zend_class_entry *tf_session_handler_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_session_handler_open_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, save_path)
	ZEND_ARG_INFO(0, session_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_session_handler_read_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, session_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_session_handler_write_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, session_id)
	ZEND_ARG_INFO(0, session_data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_session_handler_destroy_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, session_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_session_handler_gc_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, maxlifetime)
ZEND_END_ARG_INFO()

zval * tf_session_handler_constructor(zval *redis TSRMLS_DC) {
	zval *handler;
	MAKE_STD_ZVAL(handler);
	object_init_ex(handler, tf_session_handler_ce);

	zend_update_property(tf_session_handler_ce, handler, ZEND_STRL(TF_SESSION_HANDLER_PROPERTY_NAME_REDIS), redis TSRMLS_CC);

	zval *method, *register_shutdown, *ret;
	MAKE_STD_ZVAL(method);
	ZVAL_STRING(method, "session_set_save_handler", 1);
	MAKE_STD_ZVAL(register_shutdown);
	ZVAL_TRUE(register_shutdown);
	MAKE_STD_ZVAL(ret);
	zval *params[2] = { handler,  register_shutdown };

	call_user_function(EG(function_table), NULL, method, ret, 2, params TSRMLS_CC);

	zval_ptr_dtor(&method);
	zval_ptr_dtor(&register_shutdown);
	zval_ptr_dtor(&ret);

	return handler;
}

PHP_METHOD(tf_session_handler, open) {
	RETURN_TRUE;
}

PHP_METHOD(tf_session_handler, read) {
	zval *session_id;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &session_id) != SUCCESS) {
		RETURN_FALSE;
	}

	zval *redis = zend_read_property(tf_session_handler_ce, getThis(), ZEND_STRL(TF_SESSION_HANDLER_PROPERTY_NAME_REDIS), 1 TSRMLS_CC);

	zval *method, *args;
	MAKE_STD_ZVAL(method);
	ZVAL_STRING(method, "get", 1);
	MAKE_STD_ZVAL(args);
	array_init(args);
	add_next_index_zval(args, session_id);

	zval *ret = tf_redis_exec(redis, method, args TSRMLS_CC);

	zval_ptr_dtor(&method);
	zval_ptr_dtor(&args);

	RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_session_handler, write) {
	zval *session_id, *session_data;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &session_id, &session_data) != SUCCESS) {
		RETURN_FALSE;
	}

	zval *redis = zend_read_property(tf_session_handler_ce, getThis(), ZEND_STRL(TF_SESSION_HANDLER_PROPERTY_NAME_REDIS), 1 TSRMLS_CC);

	long timeout = PS(gc_maxlifetime);

	zval *method, *args, *timeout_zval;
	MAKE_STD_ZVAL(method);
	ZVAL_STRING(method, "set", 1);
	MAKE_STD_ZVAL(args);
	array_init(args);
	MAKE_STD_ZVAL(timeout_zval);
	ZVAL_LONG(timeout_zval, timeout);
	add_next_index_zval(args, session_id);
	add_next_index_zval(args, session_data);
	add_next_index_zval(args, timeout_zval);

	zval *ret = tf_redis_exec(redis, method, args TSRMLS_CC);

	zval_ptr_dtor(&method);
	zval_ptr_dtor(&args);

	RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_session_handler, destroy) {
	zval *session_id;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &session_id) != SUCCESS) {
		RETURN_FALSE;
	}

	zval *redis = zend_read_property(tf_session_handler_ce, getThis(), ZEND_STRL(TF_SESSION_HANDLER_PROPERTY_NAME_REDIS), 1 TSRMLS_CC);

	zval *method, *args;
	MAKE_STD_ZVAL(method);
	ZVAL_STRING(method, "del", 1);
	MAKE_STD_ZVAL(args);
	array_init(args);
	add_next_index_zval(args, session_id);

	zval *ret = tf_redis_exec(redis, method, args TSRMLS_CC);

	zval_ptr_dtor(&method);
	zval_ptr_dtor(&args);

	RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_session_handler, close) {
	RETURN_TRUE;
}

PHP_METHOD(tf_session_handler, gc) {
	RETURN_TRUE;
}

zend_function_entry tf_session_handler_methods[] = {
	PHP_ME(tf_session_handler, open, tf_session_handler_open_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_session_handler, read, tf_session_handler_read_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_session_handler, write, tf_session_handler_write_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_session_handler, destroy, tf_session_handler_destroy_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_session_handler, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(tf_session_handler, gc, tf_session_handler_gc_arginfo, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

ZEND_MINIT_FUNCTION(tf_session_handler) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "TF\\SessionHandler", tf_session_handler_methods);
	tf_session_handler_ce = zend_register_internal_class(&ce TSRMLS_CC);

	zend_class_entry **session_interface_ce;
	if (zend_hash_find(CG(class_table), ZEND_STRS("sessionhandlerinterface"), (void **)&session_interface_ce)) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "fail to find SessionHandlerInterface class");
		return FAILURE;
	}
	zend_class_implements(tf_session_handler_ce TSRMLS_CC, 1, *session_interface_ce);

	zend_declare_property_null(tf_session_handler_ce, ZEND_STRL(TF_SESSION_HANDLER_PROPERTY_NAME_REDIS), ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}