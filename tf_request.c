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
#include "tf_request.h"
#include "tf_common.h"

zend_class_entry *tf_request_ce;

zval * tf_request_query(uint type, char *name, uint name_len TSRMLS_DC) {
	zval **carrier = NULL, **ret;

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
	zend_bool jit_initialization = (PG(auto_globals_jit) && !PG(register_globals) && !PG(register_long_arrays));
#else
	zend_bool jit_initialization = PG(auto_globals_jit);
#endif

	switch (type) {
		case TRACK_VARS_POST:
		case TRACK_VARS_GET:
		case TRACK_VARS_FILES:
		case TRACK_VARS_COOKIE:
			carrier = &PG(http_globals)[type];
			break;
		case TRACK_VARS_ENV:
			if (jit_initialization) {
				zend_is_auto_global(ZEND_STRL("_ENV") TSRMLS_CC);
			}
			carrier = &PG(http_globals)[type];
			break;
		case TRACK_VARS_SERVER:
			if (jit_initialization) {
				zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC);
			}
			carrier = &PG(http_globals)[type];
			break;
		case TRACK_VARS_REQUEST:
			if (jit_initialization) {
				zend_is_auto_global(ZEND_STRL("_REQUEST") TSRMLS_CC);
			}
			zend_hash_find(&EG(symbol_table), ZEND_STRS("_REQUEST"), (void **)&carrier);
			break;
		default:
			break;
	}

	if (!carrier || !(*carrier)) {
		return NULL;
	}

	if (!name_len) {
		return *carrier;
	}

	if (zend_hash_find(Z_ARRVAL_PP(carrier), name, name_len + 1, (void **)&ret) == FAILURE) {
		return NULL;
	}

	return *ret;
}

zval * tf_request_constructor(zval *request TSRMLS_DC) {
	if (!request) {
		MAKE_STD_ZVAL(request);
		object_init_ex(request, tf_request_ce);
	}

	return request;
}


zval * tf_request_get_schema(zval *request TSRMLS_DC) {
	zval *schema = zend_read_property(tf_request_ce, request, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_SCHEMA), 1 TSRMLS_CC);
	if (Z_TYPE_P(schema) != IS_NULL) {
		return schema;
	}

	MAKE_STD_ZVAL(schema);
	zval *https =  tf_request_query(TRACK_VARS_SERVER, ZEND_STRL("HTTPS") TSRMLS_CC);
	if (https && strcmp(Z_STRVAL_P(https), "on") == 0) {
		ZVAL_STRING(schema, "https", 1);
	} else {
		ZVAL_STRING(schema, "http", 1);
	}
	zend_update_property(tf_request_ce, request, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_SCHEMA), schema TSRMLS_CC);
	zval_ptr_dtor(&schema);

	return schema;
}

zval * tf_request_get_domain(zval *request TSRMLS_DC) {
	zval *domain = zend_read_property(tf_request_ce, request, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_DOMAIN), 1 TSRMLS_CC);
	if (Z_TYPE_P(domain) != IS_NULL) {
		return domain;
	}

	domain = tf_request_query(TRACK_VARS_SERVER, ZEND_STRL("SERVER_NAME") TSRMLS_CC);
	zend_update_property(tf_request_ce, request, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_DOMAIN), domain TSRMLS_CC);

	return domain;
}

zval * tf_request_get_uri(zval *request TSRMLS_DC) {
	zval *uri = zend_read_property(tf_request_ce, request, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_URI), 1 TSRMLS_CC);
	if (Z_TYPE_P(uri) != IS_NULL) {
		return uri;
	}

	uri = tf_request_query(TRACK_VARS_SERVER, ZEND_STRL("REQUEST_URI") TSRMLS_CC);
	zend_update_property(tf_request_ce, request, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_URI), uri TSRMLS_CC);

	return uri;
}


zval * tf_request_get_url(zval *request TSRMLS_DC) {
	zval *url = zend_read_property(tf_request_ce, request, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_URL), 1 TSRMLS_CC);
	if (Z_TYPE_P(url) != IS_NULL) {
		return url;
	}

	zval *schema = tf_request_get_schema(request TSRMLS_CC);
	zval *domain = tf_request_get_domain(request TSRMLS_CC);
	zval *uri = tf_request_get_uri(request TSRMLS_CC);
	MAKE_STD_ZVAL(url);
	char *url_str;
	spprintf(&url_str, 0, "%s://%s%s", Z_STRVAL_P(schema), Z_STRVAL_P(domain), Z_STRVAL_P(uri));
	ZVAL_STRING(url, url_str, 0);
	zend_update_property(tf_request_ce, request, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_URL), url TSRMLS_CC);
	zval_ptr_dtor(&url);

	return url;
}

zend_bool tf_request_is_ajax(TSRMLS_DC) {
	zval *pzval = tf_request_query(TRACK_VARS_SERVER, ZEND_STRL("HTTP_X_REQUESTED_WITH") TSRMLS_DC);
	if (!pzval) {
		return FALSE;
	}

	if (strcmp(Z_STRVAL_P(pzval), "XMLHttpRequest") != 0) {
		return FALSE;
	}

	return TRUE;
}

PHP_METHOD(tf_request, __construct) {
	tf_request_constructor(getThis());
}

PHP_METHOD(tf_request, getSchema) {
	zval *schema = zend_read_property(tf_request_ce, getThis(), ZEND_STRL(TF_REQUEST_PROPERTY_NAME_SCHEMA), 1 TSRMLS_CC);
	RETVAL_ZVAL(schema, 1, 0);
}

PHP_METHOD(tf_request, getDomain) {
	zval *domain = zend_read_property(tf_request_ce, getThis(), ZEND_STRL(TF_REQUEST_PROPERTY_NAME_DOMAIN), 1 TSRMLS_CC);
	RETVAL_ZVAL(domain, 1, 0);
}

PHP_METHOD(tf_request, getUri) {
	zval *uri = zend_read_property(tf_request_ce, getThis(), ZEND_STRL(TF_REQUEST_PROPERTY_NAME_URI), 1 TSRMLS_CC);
	RETVAL_ZVAL(uri, 1, 0);
}

PHP_METHOD(tf_request, getUrl) {
	zval *url = zend_read_property(tf_request_ce, getThis(), ZEND_STRL(TF_REQUEST_PROPERTY_NAME_URL), 1 TSRMLS_CC);
	RETVAL_ZVAL(url, 1, 0);
}

PHP_METHOD(tf_request, isAjax) {
	zend_bool ret = tf_request_is_ajax(TSRMLS_CC);
	RETURN_BOOL(ret);
}

zend_function_entry tf_request_methods[] = {
	PHP_ME(tf_request, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(tf_request, getSchema, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(tf_request, getDomain, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(tf_request, getUri, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(tf_request, getUrl, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(tf_request, isAjax, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_request) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "TF\\Request", tf_request_methods);

	tf_request_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	zend_declare_property_null(tf_request_ce, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_SCHEMA), ZEND_ACC_PRIVATE);
	zend_declare_property_null(tf_request_ce, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_DOMAIN), ZEND_ACC_PRIVATE);
	zend_declare_property_null(tf_request_ce, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_URI), ZEND_ACC_PRIVATE);
	zend_declare_property_null(tf_request_ce, ZEND_STRL(TF_REQUEST_PROPERTY_NAME_URL), ZEND_ACC_PRIVATE);

	return SUCCESS;
}