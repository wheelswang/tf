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

#ifndef TF_REDIS_H
#define TF_REDIS_H

#define TF_REDIS_PROPERTY_NAME_SERVER "_server"
#define TF_REDIS_PROPERTY_NAME_PASSWORD "_password"
#define TF_REDIS_PROPERTY_NAME_INDEX "_index"
#define TF_REDIS_PROPERTY_NAME_SERIALIZE "_serialize"
#define TF_REDIS_PROPERTY_NAME_PREFIX "_prefix"
#define TF_REDIS_PROPERTY_NAME_CLIENT "_client"

zval * tf_redis_constructor(zval *redis, zval *server, zval *password, zval *index, zval *serialize, zval *prefix TSRMLS_DC);

zval * tf_redis_exec(zval *redis, zval *method, zval *args TSRMLS_DC);

#endif