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

#ifndef TF_ERROR_H
#define TF_ERROR_H

#define TF_ERROR_PROPERTY_NAME_ERROR_CODE "_error_code"
#define TF_ERROR_PROPERTY_NAME_ERROR_MSG "_error_msg"
#define TF_ERROR_PROPERTY_NAME_ERROR_DETAIL "_error_detail"

void tf_error_set_error(zval *error, zval *error_code, zval *error_msg, zval *error_detail TSRMLS_DC);

void tf_error_set_error_msg(zval *error, zval *error_msg, zval *error_detail TSRMLS_DC);

void tf_error_clear(zval *error TSRMLS_DC);

inline zval * tf_error_get_error_code(zval *error TSRMLS_CC);

inline zval * tf_error_get_error_msg(zval *error TSRMLS_CC);

inline zval * tf_error_get_error_detail(zval *error TSRMLS_CC);

#endif