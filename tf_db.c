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
#include "ext/pdo/php_pdo_driver.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_smart_str.h"
#include "tf_db.h"
#include "tf_application.h"
#include "tf_common.h"

zend_class_entry *tf_db_ce;
zend_class_entry *pdo_ce;
zend_class_entry *pdo_statement_ce;
zend_class_entry *exception_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_db___construct_arginfo, 0, 0, 7)
    ZEND_ARG_INFO(0, server)
    ZEND_ARG_INFO(0, user)
    ZEND_ARG_INFO(0, password)
    ZEND_ARG_INFO(0, dbname)
    ZEND_ARG_INFO(0, charset)
    ZEND_ARG_INFO(0, persistent)
    ZEND_ARG_INFO(0, slave_configs)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_query_arginfo, 0, 0, 5)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, condition)
    ZEND_ARG_INFO(0, params)
    ZEND_ARG_INFO(0, fields)
    ZEND_ARG_INFO(0, forUpdate)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_queryAll_arginfo, 0, 0, 5)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, condition)
    ZEND_ARG_INFO(0, params)
    ZEND_ARG_INFO(0, fields)
    ZEND_ARG_INFO(0, forUpdate)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_update_arginfo, 0, 0, 4)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, condition)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_insert_arginfo, 0, 0, 4)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_delete_arginfo, 0, 0, 3)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, condition)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_count_arginfo, 0, 0, 3)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, condition)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_execSql_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, sql)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_db_setSlaveEnabled_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, slaveEnabled)
ZEND_END_ARG_INFO()

zval * tf_db_constructor(zval *db, zval *server, zval *user, zval *password, zval *dbname, zval *charset, zval *persistent, zval *slave_configs TSRMLS_DC) {
    if (!db) {
        MAKE_STD_ZVAL(db);
        object_init_ex(db, tf_db_ce);
    }

    zend_update_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_SERVER), server TSRMLS_CC);
    zend_update_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_USER), user TSRMLS_CC);
    zend_update_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_PASSWORD), password TSRMLS_CC);
    zend_update_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_DBNAME), dbname TSRMLS_CC);
    // 这里赋值避免后面直接操作原始属性
    zend_update_property_long(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_TRANSACTION_LEVEL), 0 TSRMLS_CC);
    if (charset) {
        zend_update_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_CHARSET), charset TSRMLS_CC);
    }
    if (persistent) {
        zend_update_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_PERSISTENT), persistent TSRMLS_CC);
    }

    do {
        if (!slave_configs) {
            break;
        }

        zval *slave_configs_zval;
        MAKE_STD_ZVAL(slave_configs_zval);
        array_init(slave_configs_zval);

        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(slave_configs));
             zend_hash_has_more_elements(Z_ARRVAL_P(slave_configs)) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(slave_configs))) {
            zval **ppzval;
            zend_hash_get_current_data(Z_ARRVAL_P(slave_configs), (void **)&ppzval);
            if (Z_TYPE_PP(ppzval) != IS_ARRAY) {
                continue;
            }

            zval **item;
            if (zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("server"), (void **)&item) != SUCCESS) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "slave config miss server");
                continue;
            }
            convert_to_string(*item);

            if (zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("user"), (void **)&item) != SUCCESS) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "slave config miss user");
                continue;
            }
            convert_to_string(*item);

            if (zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("password"), (void **)&item) != SUCCESS) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "slave config miss password");
                continue;
            }
            convert_to_string(*item);

            if (zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("dbname"), (void **)&item) != SUCCESS) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "slave config miss dbname");
                continue;
            }
            convert_to_string(*item);

            if (zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("charset"), (void **)&item) != SUCCESS) {
                add_assoc_string(*ppzval, "charset", "utf8", 1);
            } else {
                convert_to_string(*item);
            }

            if (zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("persistent"), (void **)&item) != SUCCESS) {
                add_assoc_bool(*ppzval, "persistent", 0);
            } else {
                convert_to_boolean(*item);
            }

            add_next_index_zval(slave_configs_zval, *ppzval);
            Z_ADDREF_PP(ppzval);
        }

        zend_update_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_SLAVE_CONFIGS), slave_configs_zval TSRMLS_CC);
        zval_ptr_dtor(&slave_configs_zval);
    } while (0);

    return db;
}

zend_bool tf_db_connect(zval *db, int db_type, zend_bool reconnect TSRMLS_DC) {
    char *pdo_property = db_type == 0 ? TF_DB_PROPERTY_NAME_PDO : TF_DB_PROPERTY_NAME_SLAVE_PDO;
    zval *pdo = zend_read_property(tf_db_ce, db, pdo_property, strlen(pdo_property), 1 TSRMLS_CC);
    if (!reconnect && Z_TYPE_P(pdo) != IS_NULL) {
        return TRUE;
    }

    MAKE_STD_ZVAL(pdo);
    object_init_ex(pdo, pdo_ce);
    zend_update_property(tf_db_ce, db, pdo_property, strlen(pdo_property), pdo TSRMLS_CC);
    zval_ptr_dtor(&pdo);

    zval *ret_zval, *method_construct;
    MAKE_STD_ZVAL(ret_zval);
    MAKE_STD_ZVAL(method_construct);
    ZVAL_STRING(method_construct, "__construct", 1);

    zval *server, *user, *password, *dbname, *charset, *persistent;
    if (db_type == 0) {
        server = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_SERVER), 1 TSRMLS_CC);
        user = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_USER), 1 TSRMLS_CC);
        password = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_PASSWORD), 1 TSRMLS_CC);
        dbname = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_DBNAME), 1 TSRMLS_CC);
        charset = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_CHARSET), 1 TSRMLS_CC);
        persistent = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_PERSISTENT), 1 TSRMLS_CC);
    } else {
        zval *slave_configs = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_SLAVE_CONFIGS), 1 TSRMLS_CC);
        int config_num = zend_hash_num_elements(Z_ARRVAL_P(slave_configs));
        if (config_num == 0) {
            return FALSE;
        }

        int index = php_rand(TSRMLS_CC) % config_num;
        zval **ppzval;
        zend_hash_index_find(Z_ARRVAL_P(slave_configs), index, (void **)&ppzval);
        zval **item;
        zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("server"), (void **)&item);
        server = *item;
        zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("user"), (void **)&item);
        user = *item;
        zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("password"), (void **)&item);
        password = *item;
        zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("dbname"), (void **)&item);
        dbname = *item;
        zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("charset"), (void **)&item);
        charset = *item;
        zend_hash_find(Z_ARRVAL_PP(ppzval), ZEND_STRS("persistent"), (void **)&item);
        persistent = *item;
    }

    char *dsn;
    zval *dsn_zval;
    MAKE_STD_ZVAL(dsn_zval);
    char *p = strchr(Z_STRVAL_P(server), ':');
    if (p) {
        *p = '\0';
        char *host = Z_STRVAL_P(server);
        char *port = p + 1;
        spprintf(&dsn, 0, "mysql:host=%s;port=%s;dbname=%s;charset=%s", host, port, Z_STRVAL_P(dbname), Z_STRVAL_P(charset));
        *p = ':';
    } else {
        spprintf(&dsn, 0, "mysql:host=%s;dbname=%s;charset=%s", Z_STRVAL_P(server), Z_STRVAL_P(dbname), Z_STRVAL_P(charset));
    }
    ZVAL_STRING(dsn_zval, dsn, 0);

    zval *attr;
    MAKE_STD_ZVAL(attr);
    array_init(attr);
    add_index_long(attr, PDO_ATTR_ERRMODE, PDO_ERRMODE_EXCEPTION);
    if (Z_BVAL_P(persistent)) {
        add_index_bool(attr, PDO_ATTR_PERSISTENT, 1);
    }

    zval *params[4] = {
        dsn_zval,
        user,
        password,
        attr
    };
    call_user_function(&pdo_ce->function_table, &pdo, method_construct, ret_zval, 4, params TSRMLS_CC);

    zval_ptr_dtor(&dsn_zval);
    zval_ptr_dtor(&attr);
    zval_ptr_dtor(&method_construct);
    zval_ptr_dtor(&ret_zval);

    if (EG(exception)) {
        zval *error_msg = zend_read_property(exception_ce, EG(exception), ZEND_STRL("message"), 1 TSRMLS_CC);
        tf_set_error_msg(Z_STRVAL_P(error_msg), Z_STRLEN_P(error_msg), NULL, 0 TSRMLS_CC);
        php_error_docref(NULL TSRMLS_CC, E_WARNING, Z_STRVAL_P(error_msg));
        zend_clear_exception(TSRMLS_CC);

        zend_update_property_null(tf_db_ce, db, pdo_property, strlen(pdo_property) TSRMLS_CC);

        return FALSE;
    }

    // 重连时恢复事务 避免后面rollback时异常
    zval *transaction_level = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_TRANSACTION_LEVEL), 1 TSRMLS_CC);
    if (Z_LVAL_P(transaction_level) > 0) {
        zval *method, *ret;
        MAKE_STD_ZVAL(method);
        ZVAL_STRING(method, "beginTransaction", 1);
        MAKE_STD_ZVAL(ret);
        call_user_function(&pdo_ce->function_table, &pdo, method, ret, 0, NULL TSRMLS_CC);
        zval_ptr_dtor(&method);
        zval_ptr_dtor(&ret);
    }

    return TRUE;
}

void * tf_db_build_condition_str(zval *condition, char **return_condition, zval *params TSRMLS_DC) {
    smart_str condition_str = { 0 };
    if (Z_TYPE_P(condition) == IS_ARRAY) {
        zval **ppzval;
        char *str_index;
        uint str_index_len;
        ulong num_index;
        int elem_num = zend_hash_num_elements(Z_ARRVAL_P(condition));
        int i = 0;
        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(condition));
             zend_hash_has_more_elements(Z_ARRVAL_P(condition)) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(condition))) {
            i++;
            char *pdo_key;
            smart_str_appendc(&condition_str, '`');
            switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(condition), &str_index, &str_index_len, &num_index, 0, NULL)) {
                case HASH_KEY_IS_STRING:
                    spprintf(&pdo_key, 0, ":%s", str_index);
                    smart_str_appendl(&condition_str, str_index, str_index_len - 1);
                    break;
                case HASH_KEY_IS_LONG:
                    spprintf(&pdo_key, 0, ":%lu", num_index);
                    smart_str_append_long(&condition_str, num_index);
                    break;
            }
            smart_str_appendl(&condition_str, "`=", 2);
            smart_str_appendl(&condition_str, pdo_key, strlen(pdo_key));
            if (i < elem_num) {
                smart_str_appendl(&condition_str, " AND ", 5);
            }

            zend_hash_get_current_data(Z_ARRVAL_P(condition), (void **)&ppzval);
            // 数组参数传递是浅拷贝，必须分离变量后进行更改操作
            SEPARATE_ZVAL(ppzval);
            convert_to_string(*ppzval);
            add_assoc_zval(params, pdo_key, *ppzval);
            Z_ADDREF_PP(ppzval);

            efree(pdo_key);
        }
    } else {
        convert_to_string(condition);
        if (Z_STRLEN_P(condition) > 0) {
            smart_str_appendl(&condition_str, Z_STRVAL_P(condition), Z_STRLEN_P(condition));
        }
    }

    smart_str_0(&condition_str);
    *return_condition = condition_str.c;
}

zval * tf_db_build_query_sql_data(char *table, zval *condition, zval *params, zval *fields TSRMLS_DC) {
    smart_str field_str = { 0 };
    if (fields) {
        if (Z_TYPE_P(fields) == IS_ARRAY) {
            zval **ppzval;
            int elem_num = zend_hash_num_elements(Z_ARRVAL_P(fields));
            int i = 0;
            for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(fields));
                 zend_hash_has_more_elements(Z_ARRVAL_P(fields)) == SUCCESS;
                 zend_hash_move_forward(Z_ARRVAL_P(fields))) {
                i++;
                zend_hash_get_current_data(Z_ARRVAL_P(fields), (void **)&ppzval);
                convert_to_string(*ppzval);
                smart_str_appendc(&field_str, '`');
                smart_str_appendl(&field_str, Z_STRVAL_PP(ppzval), Z_STRLEN_PP(ppzval));
                if (i == elem_num) {
                    smart_str_appendc(&field_str, '`');
                } else {
                    smart_str_appendl(&field_str, "`,", 2);
                }
            }
        } else {
            convert_to_string(fields);
            smart_str_appendl(&field_str, Z_STRVAL_P(fields), Z_STRLEN_P(fields));
        }
    }

    if (!field_str.len) {
        smart_str_appendc(&field_str, '*');
    }

    smart_str_0(&field_str);

    char *condition_str = NULL;
    zval *new_params;
    MAKE_STD_ZVAL(new_params);
    array_init(new_params);
    if (condition) {
        tf_db_build_condition_str(condition, &condition_str, new_params TSRMLS_CC);
    }
    
    if (params && Z_TYPE_P(params) == IS_ARRAY) {
        zend_hash_copy(Z_ARRVAL_P(new_params), Z_ARRVAL_P(params), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));
    }

    char *sql;
    if (condition_str) {
        spprintf(&sql, 0, "SELECT %s FROM `%s` WHERE %s", field_str.c, table, condition_str);
    } else {
        spprintf(&sql, 0, "SELECT %s FROM `%s`", field_str.c, table);
    }

    efree(field_str.c);
    if (condition_str) {
        efree(condition_str);
    }

    zval *ret;
    MAKE_STD_ZVAL(ret);
    array_init(ret);
    add_assoc_string(ret, "sql", sql, 0);
    add_assoc_zval(ret, "params", new_params);

    return ret;
}

zval * tf_db_build_update_sql_data(char *table, zval *data, zval *condition, zval *params TSRMLS_DC) {
    convert_to_array(data);

    smart_str data_str = { 0 };
    zval **ppzval;
    char *str_index;
    uint str_index_len;
    ulong num_index;
    int elem_num = zend_hash_num_elements(Z_ARRVAL_P(data));
    int i = 0;
    zval *new_params;
    MAKE_STD_ZVAL(new_params);
    array_init(new_params);
    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
         zend_hash_has_more_elements(Z_ARRVAL_P(data)) == SUCCESS;
         zend_hash_move_forward(Z_ARRVAL_P(data))) {
        i++;

        char *pdo_key = NULL;
        switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(data), &str_index, &str_index_len, &num_index, 0, NULL)) {
            case HASH_KEY_IS_STRING:
                spprintf(&pdo_key, 0, ":%s", str_index);
                smart_str_appendc(&data_str, '`');
                smart_str_appendl(&data_str, str_index, str_index_len - 1);
                break;
            case HASH_KEY_IS_LONG:
                spprintf(&pdo_key, 0, ":%lu", num_index);
                smart_str_appendc(&data_str, '`');
                smart_str_append_long(&data_str, num_index);
                break;
        }
        smart_str_appendl(&data_str, "`=", 2);
        smart_str_appendl(&data_str, pdo_key, strlen(pdo_key));
        if (i < elem_num) {
            smart_str_appendc(&data_str, ',');
        }

        zend_hash_get_current_data(Z_ARRVAL_P(data), (void **)&ppzval);
        convert_to_string(*ppzval);
        add_assoc_zval(new_params, pdo_key, *ppzval);
        Z_ADDREF_PP(ppzval);

        efree(pdo_key);
    }
    smart_str_0(&data_str);

    char *condition_str = NULL;
    if (condition) {
        tf_db_build_condition_str(condition, &condition_str, new_params TSRMLS_CC);
    }
    
    char *sql;
    if (condition_str) {
        spprintf(&sql, 0, "UPDATE `%s` set %s WHERE %s", table, data_str.c, condition_str);
    } else {
        spprintf(&sql, 0, "UPDATE `%s` set %s", table, data_str.c);
    }

    efree(data_str.c);
    if (condition_str) {
        efree(condition_str);
    }

    if (params && Z_TYPE_P(params) == IS_ARRAY) {
        zend_hash_copy(Z_ARRVAL_P(new_params), Z_ARRVAL_P(params), (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval *));
    }
    
    zval *ret;
    MAKE_STD_ZVAL(ret);
    array_init(ret);
    add_assoc_string(ret, "sql", sql, 0);
    add_assoc_zval(ret, "params", new_params);

    return ret;
}

zval * tf_db_build_insert_sql_data(char *table, zval *data TSRMLS_CC) {
    convert_to_array(data);

    smart_str fields_str = { 0 };
    smart_str data_str = { 0 };
    zval **ppzval, *params;
    MAKE_STD_ZVAL(params);
    array_init(params);
    char *str_index;
    uint str_index_len;
    ulong num_index;
    int elem_num = zend_hash_num_elements(Z_ARRVAL_P(data));
    int i = 0;
    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
         zend_hash_has_more_elements(Z_ARRVAL_P(data)) == SUCCESS;
         zend_hash_move_forward(Z_ARRVAL_P(data))) {
        i++;
        char *pdo_key;
        smart_str_appendc(&fields_str, '`');
        switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(data), &str_index, &str_index_len, &num_index, 0, NULL)) {
            case HASH_KEY_IS_STRING:
                spprintf(&pdo_key, 0, ":%s", str_index);
                smart_str_appendl(&fields_str, str_index, str_index_len - 1);
                break;
            case HASH_KEY_IS_LONG:
                spprintf(&pdo_key, 0, ":%lu", num_index);
                smart_str_append_long(&fields_str, num_index);
                break;
        }
        smart_str_appendc(&fields_str, '`');
        if (i < elem_num) {
            smart_str_appendc(&fields_str, ',');
        }

        smart_str_appendl(&data_str, pdo_key, strlen(pdo_key));
        if (i < elem_num) {
            smart_str_appendc(&data_str, ',');
        }

        zend_hash_get_current_data(Z_ARRVAL_P(data), (void **)&ppzval);
        convert_to_string(*ppzval);
        add_assoc_zval(params, pdo_key, *ppzval);
        Z_ADDREF_PP(ppzval);

        efree(pdo_key);
    }

    smart_str_0(&fields_str);
    smart_str_0(&data_str);

    char *sql_str;
    spprintf(&sql_str, 0, "INSERT `%s`(%s) VALUES(%s)", table, fields_str.c, data_str.c);

    efree(fields_str.c);
    efree(data_str.c);

    zval *ret;
    MAKE_STD_ZVAL(ret);
    array_init(ret);
    add_assoc_string(ret, "sql", sql_str, 0);
    add_assoc_zval(ret, "params", params);

    return ret;
}

zval * tf_db_build_delete_sql_data(char *table, zval *condition, zval *params TSRMLS_DC) {
    char *condition_str = NULL;
    zval *new_params;
    MAKE_STD_ZVAL(new_params);
    array_init(new_params);
    if (condition) {
        tf_db_build_condition_str(condition, &condition_str, new_params TSRMLS_CC);
    }

    char *sql;
    if (condition_str) {
        spprintf(&sql, 0, "DELETE FROM `%s` WHERE %s", table, condition_str);
    } else {
        spprintf(&sql, 0, "DELETE FROM `%s`", table);
    }

    if (condition_str) {
        efree(condition_str);
    }

    if (params && Z_TYPE_P(params) == IS_ARRAY) {
        zend_hash_copy(Z_ARRVAL_P(new_params), Z_ARRVAL_P(params), (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval *));
    }

    zval *ret;
    MAKE_STD_ZVAL(ret);
    array_init(ret);
    add_assoc_string(ret, "sql", sql, 0);
    add_assoc_zval(ret, "params", new_params);

    return ret;
}

zval * tf_db_exec_sql(zval *db, char *sql, int sql_len, zval *params TSRMLS_DC) {
    char *trim_sql = php_trim(sql, sql_len, NULL, 0, NULL, 3 TSRMLS_CC);
    int i = 0;
    for (; i < 7; i++) {
        *(trim_sql + i) = toupper(*(trim_sql + i));
    }
    int type, db_type;
    if (strncmp(trim_sql, "SELECT", 6) == 0) {
        // search for "FOR UPDATE"
        do {
            db_type = 1;
            type = 0;
            char *blank_chars = " \n\r\t\v";
            int trim_sql_len = strlen(trim_sql);
            for (i = trim_sql_len - 6; i < trim_sql_len; i++) {
                *(trim_sql + i) = toupper(*(trim_sql + i));
            }
            if (strncmp(trim_sql + trim_sql_len - 6, "UPDATE", 6) == 0) {
                int j;
                zend_bool b; //blank char
                for (i = trim_sql_len - 7; i >= 2; i--) {
                    b = 0;
                    for (j = 0; j < 6; j++) {
                        if (*(blank_chars + j) == *(trim_sql + i)) {
                            b = 1;
                            break;
                        }
                    }
                    if (!b) {
                        break;
                    }
                }

                if (b || i == trim_sql_len - 7) {
                    break;
                }

                for (j = 0; j < 3; j++) {
                    *(trim_sql + i) = toupper(*(trim_sql + i));
                    i--;
                }
                i++;

                if (strncmp(trim_sql + i, "FOR", 3) == 0) {
                    db_type = 0;
                }

            }
        } while(0);
    } else if (strncmp(trim_sql, "SHOW", 4) == 0 || strncmp(trim_sql, "DESC", 4) == 0 || strncmp(trim_sql, "EXPLAIN", 7) == 0) {
        db_type = 1;
        type = 0;
    } else {
        db_type = 0;
        type = 1;
    }
    efree(trim_sql);

    zval *slave_configs = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_SLAVE_CONFIGS), 1 TSRMLS_CC);
    zval *slave_enabled = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_SLAVE_ENABLED), 1 TSRMLS_CC);
    if (Z_TYPE_P(slave_configs) == IS_NULL || zend_hash_num_elements(Z_ARRVAL_P(slave_configs)) == 0 || !Z_BVAL_P(slave_enabled)) {
        db_type = 0;
    }
    if (!tf_db_connect(db, db_type, 0 TSRMLS_CC)) {
        zval *ret;
        MAKE_STD_ZVAL(ret);
        ZVAL_BOOL(ret, 0);
        return ret;
    }

    char *pdo_property = db_type == 0 ? TF_DB_PROPERTY_NAME_PDO : TF_DB_PROPERTY_NAME_SLAVE_PDO;
    zval *pdo = zend_read_property(tf_db_ce, db, pdo_property, strlen(pdo_property), 1 TSRMLS_CC);
    for (i = 0; i < 2; i++) {
        zval *method_prepare, *statement, *sql_zval;
        MAKE_STD_ZVAL(method_prepare);
        ZVAL_STRING(method_prepare, "prepare", 1);
        MAKE_STD_ZVAL(statement);
        MAKE_STD_ZVAL(sql_zval);
        ZVAL_STRINGL(sql_zval, sql, sql_len, 1);
        zval *prepare_params[1] = { sql_zval };
        call_user_function(&pdo_ce->function_table, &pdo, method_prepare, statement, 1, prepare_params TSRMLS_CC);
        zval_ptr_dtor(&method_prepare);

        zval *method, *ret;
        MAKE_STD_ZVAL(method);
        ZVAL_STRING(method, "execute", 1);
        MAKE_STD_ZVAL(ret);
        if (params) {
            zval *statement_params[1] = { params };
            call_user_function(&pdo_statement_ce->function_table, &statement, method, ret, 1, statement_params TSRMLS_CC);
        } else {
            call_user_function(&pdo_statement_ce->function_table, &statement, method, ret, 0, NULL TSRMLS_CC);
        }
        zval_ptr_dtor(&method);
        zval_ptr_dtor(&sql_zval);

        zend_update_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_LAST_STATEMENT), statement TSRMLS_CC);
        zval_ptr_dtor(&statement);

        if (EG(exception)) {
            ZVAL_FALSE(ret);
            zend_bool reconnected = 0;
            do {
                if (i > 0) {
                    break;
                }

                zval *errorInfo = zend_read_property(exception_ce, EG(exception), ZEND_STRL("errorInfo"), 1 TSRMLS_CC);
                zval **errorCode;
                if (zend_hash_index_find(Z_ARRVAL_P(errorInfo), 1, (void **)&errorCode) == FAILURE) {
                    break;
                }

                if (Z_LVAL_PP(errorCode) != 2006 && Z_LVAL_PP(errorCode) != 2013) {
                    break;
                }

                php_error_docref(NULL TSRMLS_CC, E_WARNING, "mysql error:%d, try to reconnect", Z_LVAL_PP(errorCode));

                zend_clear_exception(TSRMLS_CC);
                if (!tf_db_connect(db, db_type, 1 TSRMLS_CC)) {
                    return ret;
                }
                
                zval_ptr_dtor(&ret);
                pdo = zend_read_property(tf_db_ce, db, pdo_property, strlen(pdo_property), 1 TSRMLS_CC);
                reconnected = 1;
            } while (0);

            if (reconnected) {
                continue;
            }

            zval *error_msg = zend_read_property(exception_ce, EG(exception), ZEND_STRL("message"), 1 TSRMLS_CC);
            tf_set_error_simple_msg(Z_STRVAL_P(error_msg), Z_STRLEN_P(error_msg) TSRMLS_CC);
            zend_clear_exception(TSRMLS_CC);

            return ret;
        }

        if (type == 1) {
            return ret;
        }

        zval_ptr_dtor(&ret);

        zval *fetch_all_method, *param_assoc, *fetch_all_ret;
        MAKE_STD_ZVAL(fetch_all_method);
        ZVAL_STRING(fetch_all_method, "fetchAll", 1);
        MAKE_STD_ZVAL(fetch_all_ret);
        MAKE_STD_ZVAL(param_assoc);
        ZVAL_LONG(param_assoc, PDO_FETCH_ASSOC);
        zval *fetch_all_params[1] = { param_assoc };

        call_user_function(&pdo_statement_ce->function_table, &statement, fetch_all_method, fetch_all_ret, 1, fetch_all_params TSRMLS_CC);

        zval_ptr_dtor(&fetch_all_method);
        zval_ptr_dtor(&param_assoc);

        return fetch_all_ret;
    }
}

zval * tf_db_query(zval *db, char *table, zval *condition, zval *params, zval *fields, zend_bool for_update TSRMLS_DC) {
    zval *sql_data = tf_db_build_query_sql_data(table, condition, params, fields TSRMLS_CC);
    zval **sql_ppzval, **new_params;
    zend_hash_find(Z_ARRVAL_P(sql_data), ZEND_STRS("sql"), (void **)&sql_ppzval);
    zend_hash_find(Z_ARRVAL_P(sql_data), ZEND_STRS("params"), (void **)&new_params);
    char *sql_str;
    if (for_update) {
        spprintf(&sql_str, 0, "%s LIMIT 1 FOR UPDATE", Z_STRVAL_PP(sql_ppzval));
    } else {
        spprintf(&sql_str, 0, "%s LIMIT 1", Z_STRVAL_PP(sql_ppzval));
    }

    zval *ret = tf_db_exec_sql(db, sql_str, strlen(sql_str), *new_params TSRMLS_CC);

    zval_ptr_dtor(&sql_data);
    efree(sql_str);

    if (Z_TYPE_P(ret) == IS_BOOL || Z_TYPE_P(ret) == IS_NULL) {
        convert_to_boolean(ret);
        return ret;
    }

    zval **tmp_row;
    if (zend_hash_index_find(Z_ARRVAL_P(ret), 0, (void **)&tmp_row) != SUCCESS) {
        convert_to_null(ret);
        return ret;
    }

    zval *row = *tmp_row;
    Z_ADDREF_P(row);
    zval_ptr_dtor(&ret); //tmp_row指针指向的内存地址会被回收

    return row;
}

zval * tf_db_query_all(zval *db, char *table, zval *condition, zval *params, zval *fields, zend_bool for_update TSRMLS_DC) {
    zval *sql_data = tf_db_build_query_sql_data(table, condition, params, fields TSRMLS_CC);
    zval **sql_ppzval, **new_params;
    zend_hash_find(Z_ARRVAL_P(sql_data), ZEND_STRS("sql"), (void **)&sql_ppzval);
    zend_hash_find(Z_ARRVAL_P(sql_data), ZEND_STRS("params"), (void **)&new_params);
    char *sql_str;
    if (for_update) {
        spprintf(&sql_str, 0, "%s FOR UPDATE", Z_STRVAL_PP(sql_ppzval));
    } else {
        sql_str = Z_STRVAL_PP(sql_ppzval);
    }

    zval *ret = tf_db_exec_sql(db, sql_str, strlen(sql_str), *new_params TSRMLS_CC);

    zval_ptr_dtor(&sql_data);
    if (for_update) {
        efree(sql_str);
    }

    return ret;
}

zval * tf_db_update(zval *db, char *table, zval *data, zval *condition, zval *params TSRMLS_DC) {
    zval *sql_data = tf_db_build_update_sql_data(table, data, condition, params TSRMLS_CC);
    zval **sql, **new_params;
    zend_hash_find(Z_ARRVAL_P(sql_data), "sql", 4, (void **)&sql);
    zend_hash_find(Z_ARRVAL_P(sql_data), "params", 7, (void **)&new_params);

    zval *ret = tf_db_exec_sql(db, Z_STRVAL_PP(sql), Z_STRLEN_PP(sql), *new_params TSRMLS_CC);

    zval_ptr_dtor(&sql_data);

    return ret;
}

zval * tf_db_insert(zval *db, char *table, zval *data TSRMLS_DC) {
    zval *sql_data = tf_db_build_insert_sql_data(table, data TSRMLS_CC);
    zval **sql, **new_params;
    zend_hash_find(Z_ARRVAL_P(sql_data), "sql", 4, (void **)&sql);
    zend_hash_find(Z_ARRVAL_P(sql_data), "params", 7, (void **)&new_params);

    zval *ret = tf_db_exec_sql(db, Z_STRVAL_PP(sql), Z_STRLEN_PP(sql), *new_params TSRMLS_CC);

    zval_ptr_dtor(&sql_data);

    return ret;
}

zval * tf_db_delete(zval *db, char *table, zval *condition, zval *params TSRMLS_DC) {
    zval *sql_data = tf_db_build_delete_sql_data(table, condition, params TSRMLS_CC);
    zval **sql, **new_params;
    zend_hash_find(Z_ARRVAL_P(sql_data), "sql", 4, (void **)&sql);
    zend_hash_find(Z_ARRVAL_P(sql_data), "params", 7, (void **)&new_params);

    zval *ret = tf_db_exec_sql(db, Z_STRVAL_PP(sql), Z_STRLEN_PP(sql), *new_params TSRMLS_CC);

    zval_ptr_dtor(&sql_data);

    return ret;
}

zval * tf_db_count(zval *db, char *table, zval *condition, zval *params TSRMLS_DC) {
    zval *fields;
    MAKE_STD_ZVAL(fields);
    ZVAL_STRING(fields, "count(*) as num", 1);
    zval *ret = tf_db_query(db, table, condition, params, fields, 0 TSRMLS_CC);
    zval_ptr_dtor(&fields);
    if (Z_TYPE_P(ret) == IS_BOOL) {
        return ret;
    }

    zval **tmp;
    if (zend_hash_find(Z_ARRVAL_P(ret), "num", 4, (void **)&tmp) != SUCCESS) {
        tf_set_error_msg(ZEND_STRL("failed to get num"), NULL, 0 TSRMLS_CC);
        convert_to_boolean(ret);
        ZVAL_FALSE(ret);
        return ret;
    }

    zval *pzval = *tmp;
    Z_ADDREF_P(pzval);
    zval_ptr_dtor(&ret);

    return pzval;
}

zval * tf_db_get_last_insert_id(zval *db TSRMLS_DC) {
    zval *pdo = zend_read_property(tf_db_ce, db, ZEND_STRL(TF_DB_PROPERTY_NAME_PDO), 1 TSRMLS_CC);
    if (Z_TYPE_P(pdo) == IS_NULL) {
        tf_set_error_msg(ZEND_STRL("pdo is null"), NULL, 0 TSRMLS_CC);
        zval *ret;
        MAKE_STD_ZVAL(ret);
        ZVAL_FALSE(ret);
        return ret;
    }
    
    zval *ret, *method;
    MAKE_STD_ZVAL(ret);
    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "lastInsertId", 1);
    call_user_function(&pdo_statement_ce->function_table, &pdo, method, ret, 0, NULL TSRMLS_CC);

    zval_ptr_dtor(&method);

    return ret;
}

PHP_METHOD(tf_db, __construct) {
    zval *server, *user, *password, *dbname, *persistent = NULL, *charset = NULL, *slave_configs = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzzz|zza", &server, &user, &password, &dbname, &charset, &persistent, &slave_configs) != SUCCESS) {
        return;
    }

    convert_to_string(server);
    convert_to_string(user);
    convert_to_string(password);
    convert_to_string(dbname);
    if (persistent) {
        convert_to_boolean(persistent);
    }
    if (charset) {
        convert_to_string(charset);
    }
    
    tf_db_constructor(getThis(), server, user, password, dbname, charset, persistent, slave_configs TSRMLS_CC);
}

PHP_METHOD(tf_db, query) {
    char *table;
    int table_len;
    zval *fields = NULL, *condition = NULL, *params = NULL;
    zend_bool for_update = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|zazb", &table, &table_len, &condition, &params, &fields, &for_update) != SUCCESS) {
        RETURN_FALSE;
    }

    zval *ret = tf_db_query(getThis(), table, condition, params, fields, for_update TSRMLS_CC);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, queryAll) {
    char *table;
    int table_len;
    zval *fields = NULL, *condition = NULL, *params = NULL;
    zend_bool for_update = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|zazb", &table, &table_len, &condition, &params, &fields, &for_update) != SUCCESS) {
        RETURN_FALSE;
    }

    zval *ret = tf_db_query_all(getThis(), table, condition, params, fields, for_update TSRMLS_CC);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, update) {
    char *table;
    int table_len;
    zval *data, *condition, *params = NULL;
    //condition is required for safe
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "saz|a", &table, &table_len, &data, &condition, &params) != SUCCESS) {
        RETURN_FALSE;
    }

    if (Z_TYPE_P(condition) == IS_ARRAY) {
        if (!zend_hash_num_elements(Z_ARRVAL_P(condition))) {
            tf_set_error_simple_msg(ZEND_STRL("condition is required") TSRMLS_CC);
            RETURN_FALSE;
        }
    } else {
        convert_to_string(condition);
        if (!Z_STRLEN_P(condition)) {
            tf_set_error_simple_msg(ZEND_STRL("condition is required") TSRMLS_CC);
            RETURN_FALSE;
        }
    }

    zval *ret = tf_db_update(getThis(), table, data, condition, params TSRMLS_CC);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, insert) {
    char *table;
    int table_len;
    zval *data;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &table, &table_len, &data) != SUCCESS) {
        RETURN_FALSE;
    }

    zval *ret = tf_db_insert(getThis(), table, data TSRMLS_CC);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, delete) {
    char *table;
    int table_len;
    zval *condition, *params = NULL;
    //condition is required for safe
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|a", &table, &table_len, &condition, &params) != SUCCESS) {
        RETURN_FALSE;
    }

    if (Z_TYPE_P(condition) == IS_ARRAY) {
        if (!zend_hash_num_elements(Z_ARRVAL_P(condition))) {
            tf_set_error_simple_msg(ZEND_STRL("condition is required") TSRMLS_CC);
            RETURN_FALSE;
        }
    } else {
        convert_to_string(condition);
        if (!Z_STRLEN_P(condition)) {
            tf_set_error_simple_msg(ZEND_STRL("condition is required") TSRMLS_CC);
            RETURN_FALSE;
        }
    }

    zval *ret = tf_db_delete(getThis(), table, condition, params TSRMLS_CC);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, count) {
    char *table;
    int table_len;
    zval *condition = NULL, *params = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|za", &table, &table_len, &condition, &params) != SUCCESS) {
        RETURN_FALSE;
    }

    zval *ret = tf_db_count(getThis(), table, condition, params TSRMLS_CC);

    RETURN_FALSE(ret, 0, 1);
}

PHP_METHOD(tf_db, execSql) {
    char *sql;
    int sql_len;
    zval *params = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a", &sql, &sql_len, &params) != SUCCESS) {
        RETURN_FALSE;
    }

    zval *ret = tf_db_exec_sql(getThis(), sql, sql_len, params TSRMLS_CC);
    if (!ret) {
        RETURN_FALSE;
    }

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, getRowCount) {
    zval *lastStatement = zend_read_property(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_LAST_STATEMENT), 1 TSRMLS_CC);
    if (Z_TYPE_P(lastStatement) == IS_NULL) {
        tf_set_error_msg(ZEND_STRL("last statement is null"), NULL, 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    zval *ret, *method;
    MAKE_STD_ZVAL(ret);
    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "rowCount", 1);
    call_user_function(&pdo_statement_ce->function_table, &lastStatement, method, ret, 0, NULL TSRMLS_CC);
    zval_ptr_dtor(&method);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, getLastInsertId) {
    zval *ret = tf_db_get_last_insert_id(getThis() TSRMLS_CC);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, begin) {
    zval *transaction_level = zend_read_property(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_TRANSACTION_LEVEL), 1 TSRMLS_CC);
    Z_LVAL_P(transaction_level)++;
    if (Z_LVAL_P(transaction_level) > 1) {
        RETURN_TRUE;
    }

    if (!tf_db_connect(getThis(), 0, 0 TSRMLS_CC)) {
        RETURN_FALSE;
    }

    zval *pdo = zend_read_property(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_PDO), 1 TSRMLS_CC);
    if (Z_TYPE_P(pdo) == IS_NULL) {
        tf_set_error_msg(ZEND_STRL("pdo is null"), NULL, 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    zval *method, *ret;
    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "beginTransaction", 1);
    MAKE_STD_ZVAL(ret);
    call_user_function(&pdo_ce->function_table, &pdo, method, ret, 0, NULL TSRMLS_CC);
    zval_ptr_dtor(&method);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, commit) {
    zval *transaction_level = zend_read_property(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_TRANSACTION_LEVEL), 1 TSRMLS_CC);
    if (Z_LVAL_P(transaction_level) > 0) {
        Z_LVAL_P(transaction_level)--;
    }
    
    if (Z_LVAL_P(transaction_level) > 0) {
        RETURN_TRUE;
    }

    zval *pdo = zend_read_property(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_PDO), 1 TSRMLS_CC);
    if (Z_TYPE_P(pdo) == IS_NULL) {
        tf_set_error_msg(ZEND_STRL("pdo is null"), NULL, 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    zval *method, *ret;
    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "commit", 1);
    MAKE_STD_ZVAL(ret);
    call_user_function(&pdo_ce->function_table, &pdo, method, ret, 0, NULL TSRMLS_CC);
    zval_ptr_dtor(&method);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, rollback) {
    zval *transaction_level = zend_read_property(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_TRANSACTION_LEVEL), 1 TSRMLS_CC);
    if (Z_LVAL_P(transaction_level) > 0) {
        Z_LVAL_P(transaction_level)--;
    }

    if (Z_LVAL_P(transaction_level) > 0) {
        RETURN_TRUE;
    }

    zval *pdo = zend_read_property(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_PDO), 1 TSRMLS_CC);
    if (Z_TYPE_P(pdo) == IS_NULL) {
        tf_set_error_msg(ZEND_STRL("pdo is null"), NULL, 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    zval *method, *ret;
    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "rollback", 1);
    MAKE_STD_ZVAL(ret);
    call_user_function(&pdo_ce->function_table, &pdo, method, ret, 0, NULL TSRMLS_CC);
    zval_ptr_dtor(&method);

    RETURN_ZVAL(ret, 0, 1);
}

PHP_METHOD(tf_db, getTransactionLevel) {
    zval *transaction_level = zend_read_property(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_TRANSACTION_LEVEL), 1 TSRMLS_CC);
    RETURN_ZVAL(transaction_level, 0, 0);
}

PHP_METHOD(tf_db, close) {
    zend_update_property_null(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_PDO) TSRMLS_CC);
}

PHP_METHOD(tf_db, setSlaveEnabled) {
    zval *slaveEnabled;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &slaveEnabled) != SUCCESS) {
        return;
    }

    convert_to_boolean(slaveEnabled);
    zend_update_property(tf_db_ce, getThis(), ZEND_STRL(TF_DB_PROPERTY_NAME_SLAVE_ENABLED), slaveEnabled TSRMLS_CC);
}

zend_function_entry tf_db_methods[] = {
    PHP_ME(tf_db, __construct, tf_db___construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_db, query, tf_db_query_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, queryAll, tf_db_queryAll_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, update, tf_db_update_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, insert, tf_db_insert_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, delete, tf_db_delete_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, count, tf_db_count_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, execSql, tf_db_execSql_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, getRowCount, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, getLastInsertId, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, begin, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, commit, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, rollback, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, getTransactionLevel, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_db, setSlaveEnabled, tf_db_setSlaveEnabled_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_db) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\DB", tf_db_methods);
    tf_db_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
    zend_declare_property_null(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_SERVER), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_USER), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_PASSWORD), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_DBNAME), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_string(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_CHARSET), "utf8", ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_bool(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_PERSISTENT), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_SLAVE_CONFIGS), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_PDO), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_SLAVE_PDO), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_LAST_STATEMENT), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_long(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_TRANSACTION_LEVEL), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_bool(tf_db_ce, ZEND_STRL(TF_DB_PROPERTY_NAME_SLAVE_ENABLED), 1, ZEND_ACC_PRIVATE TSRMLS_CC);

    zend_class_entry **pdo_tmp_ce, **pdo_statement_tmp_ce, **pdo_exception_tmp_ce;
    if (zend_hash_find(CG(class_table), ZEND_STRS("pdo"), (void **) &pdo_tmp_ce) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "fail to find PDO class");
        return FAILURE;
    }
    pdo_ce = *pdo_tmp_ce;

    if (zend_hash_find(CG(class_table), ZEND_STRS("pdostatement"), (void **) &pdo_statement_tmp_ce) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "fail to find PDOStatement class");
        return FAILURE;
    }
    pdo_statement_ce = *pdo_statement_tmp_ce;

    zend_class_entry **exception_tmp_ce;
    if (zend_hash_find(CG(class_table), ZEND_STRS("exception"), (void **)&exception_tmp_ce) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "fail to find exception class");
        return FAILURE;
    }
    exception_ce = *exception_tmp_ce;

    return SUCCESS;
}