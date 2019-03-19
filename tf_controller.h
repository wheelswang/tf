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

#ifndef TF_CONTROLLER_H
#define TF_CONTROLLER_H

#define TF_CONTROLLER_PROPERTY_NAME_REQUEST "_request"
#define TF_CONTROLLER_PROPERTY_NAME_ROUTER "_router"
#define TF_CONTROLLER_PROPERTY_NAME_VIEW "_view"
#define TF_CONTROLLER_PROPERTY_NAME_VIEW_EXT "_view_ext"
#define TF_CONTROLLER_PROPERTY_NAME_ACTION "_action"
#define TF_CONTROLLER_PROPERTY_NAME_AJAX_ERR_CODE_KEY "_err_code_key"
#define TF_CONTROLLER_PROPERTY_NAME_AJAX_ERR_MSG_KEY "_err_msg_key"
#define TF_CONTROLLER_PROPERTY_NAME_AJAX_DATA_KEY "_data_key"

void tf_controller_constructor(zval *controller, zval *view_ext, zval *err_code_key, zval *err_msg_key, zval *data_key TSRMLS_DC);

void tf_controller_run_action(zval *controller, zval *action TSRMLS_DC);

void tf_controller_run_error(zval *controller, int error_type, char *error_msg, char *error_file, int error_lineno TSRMLS_DC);

zval * tf_controller_render(zval *controller, char *tpl_name, zval *params TSRMLS_DC);

void tf_controller_display(zval *controller, char *tpl_name, zval *params TSRMLS_DC);

#endif