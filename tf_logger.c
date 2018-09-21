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
#include "ext/standard/flock_compat.h"
#include "ext/standard/php_filestat.h"
#include "ext/date/php_date.h"
#include "tf_logger.h"

ZEND_BEGIN_ARG_INFO_EX(tf_logger___constructor_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, max_size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_logger_setPath_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_logger_common_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, msg)
ZEND_END_ARG_INFO()

zend_class_entry *tf_logger_ce;

zval * tf_logger_set_path(zval *logger, char *path, int path_len TSRMLS_DC) {
	zend_update_property_stringl(tf_logger_ce, logger, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_PATH), path, path_len TSRMLS_CC);
	zval *file_name = zend_read_property(tf_logger_ce, logger, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_FILE_NAME), 1 TSRMLS_CC);
	char *file;
	spprintf(&file, 0, "%s/%s", path, Z_STRVAL_P(file_name));
	zend_update_property_stringl(tf_logger_ce, logger, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_FILE), file, strlen(file) TSRMLS_CC);
	efree(file);
}

zval * tf_logger_constructor(zval *logger, char *path, int path_len, int max_size TSRMLS_DC) {
	if (!logger) {
		MAKE_STD_ZVAL(logger);
		object_init_ex(logger, tf_logger_ce);
	}

	tf_logger_set_path(logger, path, path_len TSRMLS_CC);

	if (max_size) {
		zval *max_size_z;
		MAKE_STD_ZVAL(max_size_z);
		ZVAL_LONG(max_size_z, max_size);
		zend_update_property(tf_logger_ce, logger, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_MAX_SIZE), max_size_z TSRMLS_CC);
		zval_ptr_dtor(&max_size_z);
	}

	return logger;
}

void tf_logger_write(zval *logger, char *msg, int msg_len, int type TSRMLS_DC) {
	zval *file = zend_read_property(tf_logger_ce, logger, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_FILE), 1 TSRMLS_CC);
	FILE *fp = fopen(Z_STRVAL_P(file), "a");
	if (!fp) {
		zval *path = zend_read_property(tf_logger_ce, logger, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_PATH), 1 TSRMLS_CC);
		zval *exist;
		MAKE_STD_ZVAL(exist);
		php_stat(Z_STRVAL_P(path), Z_STRLEN_P(path), FS_EXISTS, exist TSRMLS_CC);
		if (Z_BVAL_P(exist)) {
			zval_ptr_dtor(&exist);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "permission deny in directory: %s", Z_STRVAL_P(path));
			return;
		}
		zval_ptr_dtor(&exist);

		if (php_mkdir(Z_STRVAL_P(path), 0755) != SUCCESS) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "mkdir %s error", Z_STRVAL_P(path));
			return;
		}

		fp = fopen(Z_STRVAL_P(file), "a");
		if (!fp) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "fopen %s error", Z_STRVAL_P(file));
			return;
		}
	}

	if (php_flock(fileno(fp), PHP_LOCK_EX) != SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "flock %s error(1)", Z_STRVAL_P(file));
		return;
	}

	zval *max_size = zend_read_property(tf_logger_ce, logger, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_MAX_SIZE), 1 TSRMLS_CC);

	zval *size;
	MAKE_STD_ZVAL(size);
	php_clear_stat_cache(0, Z_STRVAL_P(file), Z_STRLEN_P(file) TSRMLS_CC);
	php_stat(Z_STRVAL_P(file), Z_STRLEN_P(file), FS_SIZE, size TSRMLS_CC);
	if (Z_TYPE_P(size) != IS_LONG) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed get file:%s size", Z_STRVAL_P(file));
	} else if (Z_LVAL_P(size) >= Z_LVAL_P(max_size)) {
		char *new_file;
		char *date = php_format_date(ZEND_STRL("YmdHis"), time(NULL), 1 TSRMLS_CC);
		spprintf(&new_file, 0, "%s.%s", Z_STRVAL_P(file), date);
		if (rename(Z_STRVAL_P(file), new_file) != 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "rename file %s error", Z_STRVAL_P(file));
		}

		efree(new_file);
		efree(date);

		php_flock(fileno(fp), PHP_LOCK_UN);
		fclose(fp);

		fp = fopen(Z_STRVAL_P(file), "a");
		if (php_flock(fileno(fp), PHP_LOCK_EX) != 0) {
			zval_ptr_dtor(&size);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "flock %s error(2)", Z_STRVAL_P(file));
			return;
		}
	}
	zval_ptr_dtor(&size);

	char *new_msg, *type_str;
	struct timeval tp = {0};
	gettimeofday(&tp, NULL);
	char *date = php_format_date(ZEND_STRL("Y-m-d H:i:s"), tp.tv_sec, 1 TSRMLS_CC);
	if (type == TF_LOGGER_TYPE_INFO) {
		type_str = "info";
	} else if (type == TF_LOGGER_TYPE_WARN) {
		type_str = "warn";
	} else {
		type_str = "error";
	}

	spprintf(&new_msg, 0, "[%s.%d][%s]%s\n", date, tp.tv_usec, type_str, msg);

	// may be not completely write
	fwrite(new_msg, strlen(new_msg), 1, fp);

	efree(new_msg);
	efree(date);
	php_flock(fileno(fp), PHP_LOCK_UN);
	fclose(fp);
}

PHP_METHOD(tf_logger, __construct) {
	char *path;
	int path_len;
	long max_size = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &path, &path_len, &max_size) != SUCCESS) {
		return;
	}

	tf_logger_constructor(getThis(), path, path_len, max_size TSRMLS_CC);
}

PHP_METHOD(tf_logger, setPath) {
	char *path;
	int path_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) != SUCCESS) {
		return;
	}

	tf_logger_set_path(getThis(), path, path_len TSRMLS_CC);
}

PHP_METHOD(tf_logger, info) {
	char *msg;
	int msg_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &msg, &msg_len) != SUCCESS) {
		return;
	}

	tf_logger_write(getThis(), msg, msg_len, TF_LOGGER_TYPE_INFO TSRMLS_CC);
}

PHP_METHOD(tf_logger, warn) {
	char *msg;
	int msg_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &msg, &msg_len) != SUCCESS) {
		return;
	}

	tf_logger_write(getThis(), msg, msg_len, TF_LOGGER_TYPE_WARN TSRMLS_CC);
}

PHP_METHOD(tf_logger, error) {
	char *msg;
	int msg_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &msg, &msg_len) != SUCCESS) {
		return;
	}

	tf_logger_write(getThis(), msg, msg_len, TF_LOGGER_TYPE_ERROR TSRMLS_CC);
}

zend_function_entry tf_logger_methods[] = {
	PHP_ME(tf_logger, __construct, tf_logger___constructor_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(tf_logger, setPath, tf_logger_setPath_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_logger, info, tf_logger_common_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_logger, warn, tf_logger_common_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_logger, error, tf_logger_common_arginfo, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_logger) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "TF\\Logger", tf_logger_methods);
	tf_logger_ce = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(tf_logger_ce, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_PATH), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(tf_logger_ce, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_FILE_NAME), "runtime.log", ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(tf_logger_ce, ZEND_STRL(TF_LOGGER_PROPERTY_NAME_MAX_SIZE), 50 * 1024 * 1024, ZEND_ACC_PRIVATE TSRMLS_CC);
}
