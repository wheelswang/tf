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
#include "ext/date/php_date.h"
#include "tf_model.h"

zend_class_entry *tf_model_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_model_toArray_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, camel_case)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_model___set_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_model___get_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

zval * tf_model_constructor(zval *model TSRMLS_DC) {
    zval *data;
    MAKE_STD_ZVAL(data);
    array_init(data);
    zend_update_property(tf_model_ce, model, ZEND_STRL(TF_MODEL_PROPERTY_NAME_DATA), data TSRMLS_CC);
    zval_ptr_dtor(&data);
    
    //init default value
    zval *fields = zend_read_static_property(EG(called_scope), ZEND_STRL(TF_MODEL_PROPERTY_NAME_FIELDS), 1 TSRMLS_CC);
    if (Z_TYPE_P(fields) != IS_ARRAY) {
        return;
    }

    zval **ppzval;
    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(fields));
         zend_hash_has_more_elements(Z_ARRVAL_P(fields)) == SUCCESS;
         zend_hash_move_forward(Z_ARRVAL_P(fields))) {
        zend_hash_get_current_data(Z_ARRVAL_P(fields), (void **)&ppzval);

        if (Z_TYPE_PP(ppzval) != IS_LONG && Z_TYPE_PP(ppzval) != IS_ARRAY) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "model field define error");
            continue;
        }

        char *str_index = NULL;
        ulong num_index;
        zend_hash_get_current_key(Z_ARRVAL_P(fields), &str_index, &num_index, 0);

        zval *type, *default_value = NULL;
        zend_bool has_default = 0;
        if (Z_TYPE_PP(ppzval) == IS_LONG) {
            type = *ppzval;
        } else if (Z_TYPE_PP(ppzval) == IS_ARRAY) {
            zval **type_tmp, **default_tmp;
            if (zend_hash_index_find(Z_ARRVAL_PP(ppzval), 0, (void **)&type_tmp) != SUCCESS) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "model field miss type");
                continue;
            }
            if (Z_TYPE_PP(type_tmp) != IS_LONG) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "model field type must be int");
                continue;
            }
            type = *type_tmp;

            if (zend_hash_index_find(Z_ARRVAL_PP(ppzval), 1, (void **)&default_tmp) == SUCCESS) {
                default_value = *default_tmp;
                has_default = 1;
            }
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "model field error");
            continue;
        }

        if (Z_LVAL_P(type) == TF_MODEL_VAR_TYPE_INT) {
            if (has_default) {
                convert_to_long(default_value);
            } else {
                MAKE_STD_ZVAL(default_value);
                ZVAL_LONG(default_value, 0);
            }
        } else if (Z_LVAL_P(type) == TF_MODEL_VAR_TYPE_DOUBLE) {
            if (has_default) {
                convert_to_double(default_value);
            } else {
                MAKE_STD_ZVAL(default_value);
                ZVAL_DOUBLE(default_value, 0);
            }
        } else if (Z_LVAL_P(type) == TF_MODEL_VAR_TYPE_STRING) {
            if (has_default) {
                if (Z_TYPE_P(default_value) == IS_LONG && Z_LVAL_P(default_value) == TF_MODEL_DEFAULT_VALUE_CURRENT_TIMESTAMP) {
                    MAKE_STD_ZVAL(default_value);
                    char *currentDateTime = php_format_date(ZEND_STRL("Y-m-d H:i:s"), time(NULL), 1);
                    ZVAL_STRING(default_value, currentDateTime, 0);
                } else {
                    convert_to_string(default_value);
                }
            } else {
                MAKE_STD_ZVAL(default_value);
                ZVAL_STRING(default_value, "", 1);
            }
        } else if (Z_LVAL_P(type) == TF_MODEL_VAR_TYPE_BOOL) {
            if (has_default) {
                convert_to_boolean(default_value);
            } else {
                MAKE_STD_ZVAL(default_value);
                ZVAL_BOOL(default_value, 0);
            }
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "model field type error");
            continue;
        }

        if (str_index) {
            add_assoc_zval(data, str_index, default_value);
        } else {
            add_index_zval(data, num_index, default_value);
        }
        if (has_default) {
            Z_ADDREF_P(default_value);
        }
    }
}

PHP_METHOD(tf_model, __construct) {
    tf_model_constructor(getThis() TSRMLS_CC);
}

PHP_METHOD(tf_model, toArray) {
    zend_bool camel_case = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &camel_case) != SUCCESS) {
        return;
    }

    zval *data = zend_read_property(tf_model_ce, getThis(), ZEND_STRL(TF_MODEL_PROPERTY_NAME_DATA), 1 TSRMLS_CC);
    if (Z_TYPE_P(data) != IS_ARRAY) {
        return;
    }
    zval *ret, **ppzval;
    MAKE_STD_ZVAL(ret);
    array_init(ret);
    char *str_index;
    int str_index_len;
    ulong num_index;
    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
         zend_hash_get_current_data(Z_ARRVAL_P(data), (void *)&ppzval) == SUCCESS;
         zend_hash_move_forward(Z_ARRVAL_P(data))) {
        switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(data), &str_index, &str_index_len, &num_index, 0, NULL)) {
            case HASH_KEY_IS_STRING:
                if (!camel_case || !str_index_len) {
                    add_assoc_zval_ex(ret, str_index, str_index_len, *ppzval);
                    Z_ADDREF_PP(ppzval);
                    break;
                }

                smart_str camel_key = {0};
                char *last_pos = str_index;
                char *pos = strstr(last_pos, "_");
                while (1) {
                    if (!pos) {
                        smart_str_appendl(&camel_key, last_pos, str_index + str_index_len - 1 - last_pos);
                        break;
                    }

                    if (pos == last_pos) {
                        if (pos - str_index == str_index_len - 2) { // last char
                            break;
                        }
                        last_pos = pos + 1;
                        pos = strstr(last_pos, "_");
                        continue;
                    }

                    smart_str_appendl(&camel_key, last_pos, pos - last_pos);
                    if (pos - str_index == str_index_len - 2) { // last char
                        break;
                    }

                    smart_str_appendc(&camel_key, toupper(*(pos + 1)));
                    if (pos - str_index == str_index_len - 3) {
                        break;
                    }

                    last_pos = pos + 2;
                    pos = strstr(last_pos, "_");
                }
                smart_str_0(&camel_key);
                add_assoc_zval_ex(ret, camel_key.c, camel_key.len + 1, *ppzval);
                Z_ADDREF_PP(ppzval);
                efree(camel_key.c);
                break;
            case HASH_KEY_IS_LONG:
                add_index_zval(ret, num_index, *ppzval);
                Z_ADDREF_PP(ppzval);
                break;
        }
    }

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_model, __set) {
    zval *name, *value;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &name, &value) == FAILURE) {
        return;
    }

    zval *fields = zend_read_static_property(EG(called_scope), ZEND_STRL(TF_MODEL_PROPERTY_NAME_FIELDS), 1 TSRMLS_CC);
    if (Z_TYPE_P(fields) != IS_ARRAY) {
        return;
    }

    zval **ppzval;
    if (zend_hash_find(Z_ARRVAL_P(fields), Z_STRVAL_P(name), Z_STRLEN_P(name) + 1, (void **)&ppzval) != SUCCESS) {
        return;
    }

    zval *data = zend_read_property(tf_model_ce, getThis(), ZEND_STRL(TF_MODEL_PROPERTY_NAME_DATA), 1 TSRMLS_CC);
    if (Z_TYPE_P(data) != IS_ARRAY) {
        return;
    }

    zval *var_type;
    if (Z_TYPE_PP(ppzval) == IS_ARRAY) {
        zval **type_ppzval;
        if (zend_hash_index_find(Z_ARRVAL_PP(ppzval), 0, (void **)&type_ppzval) != SUCCESS) {
            return;
        }
        var_type = *type_ppzval;
    } else {
        var_type = *ppzval;
    }

    if (Z_TYPE_P(var_type) != IS_LONG) {
        return;
    }

    if (Z_LVAL_P(var_type) == TF_MODEL_VAR_TYPE_INT) {
        convert_to_long(value);
    }  else if (Z_LVAL_P(var_type) == TF_MODEL_VAR_TYPE_DOUBLE) {
        convert_to_double(value);
    } else if (Z_LVAL_P(var_type) == TF_MODEL_VAR_TYPE_STRING) {
        convert_to_string(value);
    } else if (Z_LVAL_P(var_type) == TF_MODEL_VAR_TYPE_BOOL) {
        convert_to_boolean(value);
    } else {
        return;
    }

    add_assoc_zval_ex(data, Z_STRVAL_P(name), Z_STRLEN_P(name) + 1, value);
    Z_ADDREF_P(value);
}

PHP_METHOD(tf_model, __get) {
    zval *name;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &name) == FAILURE) {
        return;
    }

    zval *data = zend_read_property(tf_model_ce, getThis(), ZEND_STRL(TF_MODEL_PROPERTY_NAME_DATA), 1 TSRMLS_CC);
    if (Z_TYPE_P(data) != IS_ARRAY) {
        return;
    }

    zval **ppzval;
    if (zend_hash_find(Z_ARRVAL_P(data), Z_STRVAL_P(name), Z_STRLEN_P(name) + 1, (void **)&ppzval) != SUCCESS) {
        return;
    }

    RETURN_ZVAL(*ppzval, 1, 0);
}

zend_function_entry tf_model_methods[] = {
    PHP_ME(tf_model, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_model, toArray, tf_model_toArray_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_model, __set, tf_model___set_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_model, __get, tf_model___get_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_model) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Model", tf_model_methods);

    tf_model_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(tf_model_ce, ZEND_STRL(TF_MODEL_PROPERTY_NAME_FIELDS), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC TSRMLS_CC);
    zend_declare_property_null(tf_model_ce, ZEND_STRL(TF_MODEL_PROPERTY_NAME_DATA), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_class_constant_long(tf_model_ce, ZEND_STRL(TF_MODEL_PROPERTY_NAME_VAR_TYPE_INT), TF_MODEL_VAR_TYPE_INT TSRMLS_CC);
    zend_declare_class_constant_long(tf_model_ce, ZEND_STRL(TF_MODEL_PROPERTY_NAME_VAR_TYPE_DOUBLE), TF_MODEL_VAR_TYPE_DOUBLE TSRMLS_CC);
    zend_declare_class_constant_long(tf_model_ce, ZEND_STRL(TF_MODEL_PROPERTY_NAME_VAR_TYPE_STRING), TF_MODEL_VAR_TYPE_STRING TSRMLS_CC);
    zend_declare_class_constant_long(tf_model_ce, ZEND_STRL(TF_MODEL_PROPERTY_NAME_VAR_TYPE_BOOL), TF_MODEL_VAR_TYPE_BOOL TSRMLS_CC);
    zend_declare_class_constant_long(tf_model_ce, ZEND_STRL(TF_MODEL_PROPERTY_NAME_DEFAULT_VALUE_CURRENT_TIMESTAMP), TF_MODEL_DEFAULT_VALUE_CURRENT_TIMESTAMP TSRMLS_CC);

    return SUCCESS;
}