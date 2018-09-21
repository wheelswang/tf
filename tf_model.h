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

#ifndef TF_MODEL_H
#define TF_MODEL_H

#define TF_MODEL_PROPERTY_NAME_FIELDS "_fields"

#define TF_MODEL_PROPERTY_NAME_DATA "_data"

#define TF_MODEL_VAR_TYPE_INT 1
#define TF_MODEL_VAR_TYPE_DOUBLE 2
#define TF_MODEL_VAR_TYPE_STRING 3
#define TF_MODEL_VAR_TYPE_BOOL 4

#define TF_MODEL_PROPERTY_NAME_VAR_TYPE_INT "INT"
#define TF_MODEL_PROPERTY_NAME_VAR_TYPE_DOUBLE "DOUBLE"
#define TF_MODEL_PROPERTY_NAME_VAR_TYPE_STRING "STRING"
#define TF_MODEL_PROPERTY_NAME_VAR_TYPE_BOOL "BOOL"

zval * tf_model_constructor(zval *model TSRMLS_DC);

#endif