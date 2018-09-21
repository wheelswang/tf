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

#ifndef TF_DB_H
#define TF_DB_H

#define TF_DB_PROPERTY_NAME_SERVER "_server"
#define TF_DB_PROPERTY_NAME_USER "_user"
#define TF_DB_PROPERTY_NAME_PASSWORD "_password"
#define TF_DB_PROPERTY_NAME_DBNAME "_dbname"
#define TF_DB_PROPERTY_NAME_CHARSET "_charset"
#define TF_DB_PROPERTY_NAME_PERSISTENT "_persistent"
#define TF_DB_PROPERTY_NAME_SLAVE_CONFIGS "_slaveConfigs"
#define TF_DB_PROPERTY_NAME_PDO "_pdo"
#define TF_DB_PROPERTY_NAME_SLAVE_PDO "_slave_pdo"
#define TF_DB_PROPERTY_NAME_SLAVE_ENABLED "_slave_enabled"
#define TF_DB_PROPERTY_NAME_LAST_STATEMENT "_last_statement"
#define TF_DB_PROPERTY_NAME_TRANSACTION_LEVEL "_transaction_level"

zval * tf_db_constructor(zval *db, zval *server, zval *user, zval *password, zval *dbname, zval *charset, zval *persistent, zval *slave_configs TSRMLS_DC);

zval * tf_db_query(zval *db, char *table, zval *condition, zval *params, zval *fields, zend_bool for_update TSRMLS_DC);

zval * tf_db_query_all(zval *db, char *table, zval *condition, zval *params, zval *fields, zend_bool for_update TSRMLS_DC);

zval * tf_db_update(zval *db, char *table, zval *data, zval *condition, zval *params TSRMLS_DC);

zval * tf_db_insert(zval *db, char *table, zval *data TSRMLS_DC);

zval * tf_db_delete(zval *db, char *table, zval *condition, zval *params TSRMLS_DC);

zval * tf_db_count(zval *db, char *table, zval *condition, zval *params TSRMLS_DC);

zval * tf_db_get_last_insert_id(zval *db TSRMLS_DC);

zval * tf_db_exec_sql(zval *db, char *sql, int sql_len, zval *params TSRMLS_DC);

#endif