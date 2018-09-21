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

#include <string.h>
#include "tf_common.h"

int tf_common_get_num_len(int num) {
	int len = num < 0 ? 1 : 0;
	do {
        num /= 10;
        ++len;
	} while (num != 0);

	return len;
}

void tf_common_persistent_zval_dtor(zval **value) {
	if (*value) {
		switch(Z_TYPE_PP(value)) {
			case IS_STRING:
			case IS_CONSTANT:
				CHECK_ZVAL_STRING(*value);
				pefree((*value)->value.str.val, 1);
				pefree(*value, 1);
				break;
#ifdef IS_CONSTANT_ARRAY
			case IS_CONSTANT_ARRAY:
#endif
			case IS_ARRAY: {
				zend_hash_destroy((*value)->value.ht);
				pefree((*value)->value.ht, 1);
				pefree(*value, 1);
			}
			break;
		}
	}
}

zval * tf_common_zval_copy(zval *src, zend_bool persistent) {
	zval *ret;
	// zend_gc.h override
	if (persistent) {
		ALLOC_PERMANENT_ZVAL(ret);
		INIT_PZVAL(ret);
	} else {
		MAKE_STD_ZVAL(ret);
	}
	
	switch (src->type) {
		case IS_RESOURCE:
		case IS_OBJECT:
			break;
		case IS_BOOL:
		case IS_LONG:
		case IS_NULL:
			ret->value.lval = src->value.lval;
			Z_TYPE_P(ret) = Z_TYPE_P(src);
			break;
		case IS_CONSTANT:
		case IS_STRING:
				CHECK_ZVAL_STRING(src);
				Z_TYPE_P(ret) = IS_STRING;
				ret->value.str.val = pestrndup(src->value.str.val, src->value.str.len, persistent);
				ret->value.str.len = src->value.str.len;
			break;
#ifdef IS_CONSTANT_ARRAY
		case IS_CONSTANT_ARRAY:
#endif
		case IS_ARRAY: {
				Z_TYPE_P(ret) = IS_ARRAY;
				ret->value.ht = tf_common_zend_hash_copy(src->value.ht, persistent);
			}
			break;
	}

	return ret;
}

HashTable * tf_common_zend_hash_copy(HashTable *src, zend_bool persistent) {
	HashTable *ret = (HashTable *)pemalloc(sizeof(HashTable), persistent);
	zend_hash_init(ret, zend_hash_num_elements(src), NULL, persistent ? (dtor_func_t)tf_common_persistent_zval_dtor : ZVAL_PTR_DTOR, persistent ? 1 : 0);

	zval **ppzval;
	char *key;
	uint keylen;
	ulong idx;

	for(zend_hash_internal_pointer_reset(src);
			zend_hash_has_more_elements(src) == SUCCESS;
			zend_hash_move_forward(src)) {
		if (zend_hash_get_current_key_ex(src, &key, &keylen, &idx, 0, NULL) == HASH_KEY_IS_LONG) {
			zval *tmp;
			if (zend_hash_get_current_data(src, (void**)&ppzval) == FAILURE) {
				continue;
			}

			tmp = tf_common_zval_copy(*ppzval, persistent);
			if (tmp) {
				zend_hash_index_update(ret, idx, (void **)&tmp, sizeof(zval *), NULL);
			}

		} else {
			zval *tmp;
			if (zend_hash_get_current_data(src, (void**)&ppzval) == FAILURE) {
				continue;
			}

			tmp = tf_common_zval_copy(*ppzval, persistent);
			if (tmp) {
				zend_hash_update(ret, key, keylen, (void **)&tmp, sizeof(zval *), NULL);
			}
		}
	}

	return ret;
}

zend_bool tf_common_in_array(zval *pzval, zval *arr) {
	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return FALSE;
	}

	for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(arr));
		 zend_hash_has_more_elements(Z_ARRVAL_P(arr));
		 zend_hash_move_forward(Z_ARRVAL_P(arr))) {
		zval **ppzval;
		zend_hash_get_current_data(Z_ARRVAL_P(arr), (void **)&ppzval);
		zval *result;
		MAKE_STD_ZVAL(result);
		compare_function(result, pzval, *ppzval TSRMLS_CC);
		if (Z_LVAL_P(result) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}
