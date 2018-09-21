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

#ifndef COMMON_H
#define COMMON_H

#include "php.h"

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif

#define TF_ZVAL_DTOR(zval) {\
        zval_dtor(zval);\
        ZVAL_NULL(zval);\
    }

int tf_common_get_num_len(int num);

void tf_common_persistent_zval_dtor(zval **value);

HashTable * tf_common_zend_hash_copy(HashTable *src, zend_bool persistent);

zval * tf_common_zval_copy(zval *src, zend_bool persistent);

zend_bool tf_common_in_array(zval *pzval, zval *arr);

#endif