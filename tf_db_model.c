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
#include "ext/standard/php_smart_str.h"
#include "tf.h"
#include "tf_db.h"
#include "tf_model.h"
#include "tf_db_model.h"
#include "tf_application.h"
#include "tf_config.h"
#include "tf_common.h"

zend_class_entry *tf_db_model_ce;
extern zend_class_entry *tf_model_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_db_model_query_arginfo, 0, 0, 4)
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, for_update)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_model_queryByPk_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, id)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, for_update)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_model_queryAll_arginfo, 0, 0, 4)
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, for_update)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_model_update_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_model_updateByPk_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, id)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_model_insert_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_model_delete_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_model_deleteByPk_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_model_count_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

void tf_db_model_init(TSRMLS_CC) {
	zval *db = zend_read_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), 1 TSRMLS_CC);
	if (Z_TYPE_P(db) != IS_NULL) {
		return;
	}

	zval *application = tf_get_application(TSRMLS_CC);
	if (Z_TYPE_P(application) == IS_NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "application not init");
	}

	db = tf_application_get_db(application TSRMLS_CC);
	// 这里引用计数没有增加 可能是因为db已经是application的属性
	zend_update_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), db TSRMLS_CC);
}

zval * tf_db_model_get_default_fields(zval *db_model TSRMLS_DC) {
	zval *fields = zend_read_static_property(EG(called_scope), ZEND_STRL(TF_MODEL_PROPERTY_NAME_FIELDS), 1 TSRMLS_CC);
	if (Z_TYPE_P(fields) != IS_ARRAY) {
		return NULL;
	}

	smart_str field_str = { 0 };
	zval **ppzval;
	int elem_num = zend_hash_num_elements(Z_ARRVAL_P(fields));
	int i = 0;
	for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(fields));
		 zend_hash_has_more_elements(Z_ARRVAL_P(fields)) == SUCCESS;
		 zend_hash_move_forward(Z_ARRVAL_P(fields))) {
		i++;
		smart_str_appendc(&field_str, '`');
		char *str_index;
		uint str_index_len;
		ulong num_index;
		switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(fields), &str_index, &str_index_len, &num_index, 0, NULL)) {
			case HASH_KEY_IS_STRING:
				smart_str_appendl(&field_str, str_index, str_index_len - 1);
				break;
			case HASH_KEY_IS_LONG:
				smart_str_append_long(&field_str, num_index);
				break;
		}

		if (i == elem_num) {
			smart_str_appendc(&field_str, '`');
		} else {
			smart_str_appendl(&field_str, "`,", 2);
		}
	}

	if (!field_str.len) {
		smart_str_appendc(&field_str, '*');
	}

	smart_str_0(&field_str);

	zval *ret;
	MAKE_STD_ZVAL(ret);
	ZVAL_STRINGL(ret, field_str.c, field_str.len, 0);

	return ret;
}

inline zval * tf_db_model_get_data(zval *db_model TSRMLS_DC) {
	zval *data = zend_read_property(tf_db_model_ce, db_model, ZEND_STRL(TF_MODEL_PROPERTY_NAME_DATA), 1 TSRMLS_CC);
	convert_to_array(data);

	return data;
}

inline zval * tf_db_model_get_pk(TSRMLS_DC) {
	zval *pk = zend_read_static_property(EG(called_scope), ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_TABLE_PK), 1 TSRMLS_CC);
	convert_to_string(pk);

	return pk;
}

inline zval * tf_db_model_get_table(TSRMLS_DC) {
	zval *table = zend_read_static_property(EG(called_scope), ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_TABLE), 1 TSRMLS_CC);
	convert_to_string(table);

	return table;
}

zval * tf_db_model_consturctor(zval *row TSRMLS_DC) {
	zval *db_model;
	MAKE_STD_ZVAL(db_model);
	object_init_ex(db_model, EG(called_scope));

	tf_model_constructor(db_model TSRMLS_CC);

	zval *data = tf_db_model_get_data(db_model TSRMLS_CC);
	for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(row));
		 zend_hash_has_more_elements(Z_ARRVAL_P(row)) == SUCCESS;
		 zend_hash_move_forward(Z_ARRVAL_P(row))) {
		char *field;
		uint field_len;
		ulong num_index;
		int hash_key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(row), &field, &field_len, &num_index, 0, NULL);
		switch (hash_key_type) {
			case HASH_KEY_IS_STRING:
				field_len--;
				break;
			case HASH_KEY_IS_LONG:
				spprintf(&field, 0, "%lu", num_index);
				field_len = strlen(field);
				break;
		}

		zval **ppzval;
		if (zend_hash_find(Z_ARRVAL_P(data), field, field_len + 1, (void **)&ppzval) != SUCCESS) {
			continue;
		}

		zend_hash_get_current_data(Z_ARRVAL_P(row), (void **)&ppzval);
		add_assoc_zval(data, field, *ppzval);
		Z_ADDREF_PP(ppzval);
		if (hash_key_type == HASH_KEY_IS_LONG) {
			efree(field);
		}
	}

	return db_model;
}

zval * tf_db_model_update(zval *db_model, zval *fields, zval *condition, zval *params TSRMLS_DC) {
	zval *table_fields = zend_read_static_property(EG(called_scope), ZEND_STRL(TF_MODEL_PROPERTY_NAME_FIELDS), 1 TSRMLS_CC);
	zval *data;
	if (!db_model) {
		//condition is required for safe
		if (!fields || Z_TYPE_P(fields) != IS_ARRAY || !zend_hash_num_elements(Z_ARRVAL_P(fields))) {
			tf_set_error_simple_msg(ZEND_STRL("update must be called by instance if fields is NULL") TSRMLS_CC);
			return NULL;
		}

		if (!condition) {
			tf_set_error_simple_msg(ZEND_STRL("update must be called by instance if condition is NULL") TSRMLS_CC);
			return NULL;
		} else if (Z_TYPE_P(condition) == IS_ARRAY) {
			if (!zend_hash_num_elements(Z_ARRVAL_P(condition))) {
				tf_set_error_simple_msg(ZEND_STRL("update must be called by instance if condition is empty") TSRMLS_CC);
				return NULL;
			}
		} else {
			convert_to_string(condition);
			if (!Z_STRLEN_P(condition)) {
				tf_set_error_simple_msg(ZEND_STRL("update must be called by instance if condition is empty") TSRMLS_CC);
				return NULL;
			}
		}

		MAKE_STD_ZVAL(data);
		array_init(data);
		for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(fields));
			 zend_hash_has_more_elements(Z_ARRVAL_P(fields)) == SUCCESS;
			 zend_hash_move_forward(Z_ARRVAL_P(fields))) {
			char *str_index;
			uint str_index_len;
			ulong num_index;
			if (zend_hash_get_current_key_ex(Z_ARRVAL_P(fields), &str_index, &str_index_len, &num_index, 0, NULL) != HASH_KEY_IS_STRING) {
				continue;
			}

			if (Z_TYPE_P(table_fields) != IS_ARRAY) {
				break;
			}

			if (!zend_hash_exists(Z_ARRVAL_P(table_fields), str_index, str_index_len)) {
				continue;
			}
			
			zval **ppzval;
			zend_hash_get_current_data(Z_ARRVAL_P(fields), (void **)&ppzval);
			add_assoc_zval(data, str_index, *ppzval);
			Z_ADDREF_PP(ppzval);
		}

		// fields are all num index or invalid
		if (!zend_hash_num_elements(Z_ARRVAL_P(data))) {
			tf_set_error_simple_msg(ZEND_STRL("update must be called by instance if fields are all num indexes") TSRMLS_CC);
			return NULL;
		}
	} else {
		zval *pk = tf_db_model_get_pk(TSRMLS_CC);

		zval *model_data = tf_db_model_get_data(db_model TSRMLS_CC);
		if (fields && Z_TYPE_P(fields) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(fields)) > 0) {
			MAKE_STD_ZVAL(data);
			array_init(data);
			zval **field_ppzval, **value_ppzval;
			for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(fields));
				 zend_hash_get_current_data(Z_ARRVAL_P(fields), (void **)&field_ppzval) == SUCCESS;
				 zend_hash_move_forward(Z_ARRVAL_P(fields))) {
				if (Z_TYPE_PP(field_ppzval) != IS_STRING) {
					continue;
				}
				if (zend_hash_find(Z_ARRVAL_P(model_data), Z_STRVAL_PP(field_ppzval), Z_STRLEN_PP(field_ppzval) + 1, (void **)&value_ppzval) != SUCCESS) {
					continue;
				}
				add_assoc_zval_ex(data, Z_STRVAL_PP(field_ppzval), Z_STRLEN_PP(field_ppzval) + 1, *value_ppzval);
				Z_ADDREF_PP(value_ppzval);
			}
			if (zend_hash_num_elements(Z_ARRVAL_P(data)) == 0) {
				zval_ptr_dtor(&data);
				data = model_data;
				Z_ADDREF_P(model_data);
			}
		} else {
			data = model_data;
			Z_ADDREF_P(model_data);
		}

		zval **ppzval;
		if (zend_hash_find(Z_ARRVAL_P(model_data), Z_STRVAL_P(pk), Z_STRLEN_P(pk) + 1, (void **)&ppzval) != SUCCESS) {
			zval_ptr_dtor(&data);
			tf_set_error_simple_msg(ZEND_STRL("pk value is missing") TSRMLS_CC);
			return NULL;
		}

		MAKE_STD_ZVAL(condition);
		array_init(condition);
		add_assoc_zval(condition, Z_STRVAL_P(pk), *ppzval);
		Z_ADDREF_PP(ppzval);
	}

	tf_db_model_init(TSRMLS_CC);
	zval *db = zend_read_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), 1 TSRMLS_CC);
	zval *table = tf_db_model_get_table(TSRMLS_CC);
	zval *ret = tf_db_update(db, Z_STRVAL_P(table), data, condition, params TSRMLS_DC);

	zval_ptr_dtor(&data);
	if (db_model) {
		zval_ptr_dtor(&condition);
	}

	return ret;
}

zval * tf_db_model_delete(zval *db_model, zval *condition, zval *params TSRMLS_DC) {
	if (!db_model) {
		if (!condition) {
			tf_set_error_simple_msg(ZEND_STRL("delete must be called by instance if condition is NULL") TSRMLS_CC);
			return NULL;
		} else if (Z_TYPE_P(condition) == IS_ARRAY) {
			if (!zend_hash_num_elements(Z_ARRVAL_P(condition))) {
				tf_set_error_simple_msg(ZEND_STRL("delete must be called by instance if condition is empty") TSRMLS_CC);
				return NULL;
			}
		} else {
			convert_to_string(condition);
			if (!Z_STRLEN_P(condition)) {
				tf_set_error_simple_msg(ZEND_STRL("delete must be called by instance if condition is empty") TSRMLS_CC);
				return NULL;
			}
		}
	} else {
		MAKE_STD_ZVAL(condition);
		array_init(condition);

		zval *pk = tf_db_model_get_pk(TSRMLS_CC);

		zval *model_data = tf_db_model_get_data(db_model TSRMLS_CC);
		zval **ppzval;
		if (zend_hash_find(Z_ARRVAL_P(model_data), Z_STRVAL_P(pk), Z_STRLEN_P(pk) + 1, (void **)&ppzval) != SUCCESS) {
			tf_set_error_simple_msg(ZEND_STRL("pk value is missing") TSRMLS_CC);
			return NULL;
		}

		add_assoc_zval(condition, Z_STRVAL_P(pk), *ppzval);
		Z_ADDREF_PP(ppzval);
	}

	tf_db_model_init(TSRMLS_CC);
	zval *db = zend_read_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), 1 TSRMLS_CC);
	zval *table = tf_db_model_get_table(TSRMLS_CC);
	zval *ret = tf_db_delete(db, Z_STRVAL_P(table), condition, params TSRMLS_CC);

	if (db_model) {
		zval_ptr_dtor(&condition);
	}

	return ret;
}

PHP_METHOD(tf_db_model, query) {
	zval *condition = NULL, *params = NULL, *fields = NULL;
	zend_bool for_update = 0 ; 
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zzzb", &condition, &params, &fields, &for_update) != SUCCESS) {
		RETURN_FALSE;
	}

	zval *default_fields = NULL;
	if (!fields) {
		default_fields = tf_db_model_get_default_fields(getThis() TSRMLS_CC);
	}

	tf_db_model_init(TSRMLS_CC);
	zval *db = zend_read_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), 1 TSRMLS_CC);
	zval *table = tf_db_model_get_table(TSRMLS_CC);
	zval *row = tf_db_query(db, Z_STRVAL_P(table), condition, params, fields ? fields : default_fields, for_update TSRMLS_DC);

	if (default_fields) {
		zval_ptr_dtor(&default_fields);
	}

	if (Z_TYPE_P(row) != IS_ARRAY) {
		RETURN_ZVAL(row, 0, 1);
	}

	zval *model = tf_db_model_consturctor(row TSRMLS_CC);
	zval_ptr_dtor(&row);

	RETURN_ZVAL(model, 0, 1);
}

PHP_METHOD(tf_db_model, queryByPk) {
	zval *id, *fields = NULL;
	zend_bool for_update = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|zb", &id, &fields, &for_update) != SUCCESS) {
		RETURN_FALSE;
	}

	zval *default_fields = NULL;
	if (!fields) {
		default_fields = tf_db_model_get_default_fields(getThis() TSRMLS_CC);
	}

	zval *condition;
	MAKE_STD_ZVAL(condition);
	array_init(condition);
	zval *pk = tf_db_model_get_pk(TSRMLS_CC);
	add_assoc_zval(condition, Z_STRVAL_P(pk), id);
	Z_ADDREF_P(id);

	tf_db_model_init(TSRMLS_CC);
	zval *db = zend_read_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), 1 TSRMLS_CC);
	zval *table = tf_db_model_get_table(TSRMLS_CC);
	zval *row = tf_db_query(db, Z_STRVAL_P(table), condition, NULL, fields ? fields : default_fields, for_update TSRMLS_DC);

	zval_ptr_dtor(&condition);
	if (default_fields) {
		zval_ptr_dtor(&default_fields);
	}

	if (Z_TYPE_P(row) != IS_ARRAY) {
		RETURN_ZVAL(row, 0, 1);
	}

	zval *model = tf_db_model_consturctor(row TSRMLS_CC);
	zval_ptr_dtor(&row);

	RETURN_ZVAL(model, 0, 1);
}

PHP_METHOD(tf_db_model, queryAll) {
	zval *condition = NULL, *params = NULL, *fields = NULL;
	zend_bool for_update = 0 ; 
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zzzb", &condition, &params, &fields, &for_update) != SUCCESS) {
		RETURN_FALSE;
	}

	zval *default_fields = NULL;
	if (!fields) {
		default_fields = tf_db_model_get_default_fields(getThis() TSRMLS_CC);
	}

	tf_db_model_init(TSRMLS_CC);
	zval *db = zend_read_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), 1 TSRMLS_CC);
	zval *table = tf_db_model_get_table(TSRMLS_CC);
	zval *rows = tf_db_query_all(db, Z_STRVAL_P(table), condition, params, fields ? fields : default_fields, for_update TSRMLS_DC);
	if (default_fields) {
		zval_ptr_dtor(&default_fields);
	}
	if (Z_TYPE_P(rows) != IS_ARRAY) {
		RETURN_ZVAL(rows, 0, 1);
	}

	zval *models;
	MAKE_STD_ZVAL(models);
	array_init(models);
	for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(rows));
		 zend_hash_has_more_elements(Z_ARRVAL_P(rows)) == SUCCESS;
		 zend_hash_move_forward(Z_ARRVAL_P(rows))) {
		zval **row;
		zend_hash_get_current_data(Z_ARRVAL_P(rows), (void **)&row);
		zval *model = tf_db_model_consturctor(*row TSRMLS_CC);
		add_next_index_zval(models, model);
	}
	zval_ptr_dtor(&rows);

	RETURN_ZVAL(models, 0, 1);
}

PHP_METHOD(tf_db_model, update) {
	zval *fields = NULL, *condition = NULL, *params = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zzz", &fields, &condition, &params)) {
		RETURN_FALSE;
	}
	
	zval *me = getThis();
	if (me && !instanceof_function(Z_OBJCE_P(me), tf_db_model_ce TSRMLS_CC)) {
		me = NULL;
	}

	zval *ret = tf_db_model_update(me, fields, condition, params TSRMLS_DC);
	if (!ret) {
		RETURN_FALSE;
	}

	RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db_model, updateByPk) {
	zval *id, *data;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "za", &id, &data) != SUCCESS) {
		RETURN_FALSE;
	}

	zval *pk = tf_db_model_get_pk(TSRMLS_CC);
	zval *condition;
	MAKE_STD_ZVAL(condition);
	array_init(condition);
	add_assoc_zval(condition, Z_STRVAL_P(pk), id);
	Z_ADDREF_P(id);

	tf_db_model_init(TSRMLS_CC);
	zval *db = zend_read_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), 1 TSRMLS_CC);
	zval *table = tf_db_model_get_table(TSRMLS_CC);
	zval *ret = tf_db_update(db, Z_STRVAL_P(table), data, condition, NULL TSRMLS_DC);

	zval_ptr_dtor(&condition);

	RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db_model, insert) {
	zval *model_data = tf_db_model_get_data(getThis() TSRMLS_CC);
	zval *pk = tf_db_model_get_pk(TSRMLS_CC);
	zval *data;
	MAKE_STD_ZVAL(data);
	ZVAL_ZVAL(data, model_data, 1, 0);
	zend_hash_del(Z_ARRVAL_P(data), Z_STRVAL_P(pk), Z_STRLEN_P(pk) + 1);

	tf_db_model_init(TSRMLS_CC);
	zval *db = zend_read_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), 1 TSRMLS_CC);
	zval *table = tf_db_model_get_table(TSRMLS_CC);
	zval *ret = tf_db_insert(db, Z_STRVAL_P(table), data TSRMLS_CC);
	if (Z_BVAL_P(ret)) {
		zval *id = tf_db_get_last_insert_id(db TSRMLS_CC);
		zend_hash_update(Z_ARRVAL_P(model_data), Z_STRVAL_P(pk), Z_STRLEN_P(pk) + 1, &id, sizeof(zval *), NULL);
	}
	zval_ptr_dtor(&data);

	RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db_model, delete) {
	zval *condition, *params = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|za", &condition, &params) != SUCCESS) {
		RETURN_FALSE;
	}

	zval *me = getThis();
	if (me && !instanceof_function(Z_OBJCE_P(me), tf_db_model_ce TSRMLS_CC)) {
		me = NULL;
	}

	zval *ret = tf_db_model_delete(me, condition, params TSRMLS_CC);
	if (!ret) {
		RETURN_FALSE;
	}

	RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db_model, deleteByPk) {
	zval *id;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &id) != SUCCESS) {
		RETURN_FALSE;
	}

	zval *pk = tf_db_model_get_pk(TSRMLS_CC);
	zval *condition;
	MAKE_STD_ZVAL(condition);
	array_init(condition);
	add_assoc_zval_ex(condition, Z_STRVAL_P(pk), Z_STRLEN_P(pk) + 1, id);

	zval *ret = tf_db_model_delete(NULL, condition, NULL TSRMLS_CC);
	zval_ptr_dtor(&condition);
	if (!ret) {
		RETURN_FALSE;
	}

	RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db_model, count) {
	zval *condition = NULL, *params = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|za", &condition, &params) != SUCCESS) {
		RETURN_FALSE;
	}

	tf_db_model_init(TSRMLS_CC);
	zval *db = zend_read_static_property(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), 1 TSRMLS_CC);
	zval *table = tf_db_model_get_table(TSRMLS_CC);
	zval *ret = tf_db_count(db, Z_STRVAL_P(table), condition, params TSRMLS_CC);

	RETURN_ZVAL(ret, 0, 1);
}

zend_function_entry tf_db_model_methods[]= {
	PHP_ME(tf_db_model, query, tf_db_model_query_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(tf_db_model, queryByPk, tf_db_model_queryByPk_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(tf_db_model, queryAll, tf_db_model_queryAll_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(tf_db_model, update, tf_db_model_update_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_ALLOW_STATIC)
	PHP_ME(tf_db_model, updateByPk, tf_db_model_updateByPk_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(tf_db_model, insert, tf_db_model_insert_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(tf_db_model, delete, tf_db_model_delete_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_ALLOW_STATIC)
	PHP_ME(tf_db_model, deleteByPk, tf_db_model_deleteByPk_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(tf_db_model, count, tf_db_model_count_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_db_model) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "TF\\DBModel", tf_db_model_methods);
	tf_db_model_ce = zend_register_internal_class_ex(&ce, tf_model_ce, NULL TSRMLS_CC);
	zend_declare_property_null(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_DB), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_TABLE), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(tf_db_model_ce, ZEND_STRL(TF_DB_MODEL_PROPERTY_NAME_TABLE_PK), "id", ZEND_ACC_PROTECTED | ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}