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

#ifndef TF_VIEW_H
#define TF_VIEW_H

#define TF_VIEW_PROPERTY_NAME_TPL_VARS "_tpl_vars"
#define TF_VIEW_PROPERTY_NAME_TPL_DIR "_tpl_dir"
#define TF_VIEW_PROPERTY_NAME_TPL_EXT "_tpl_ext"
#define TF_VIEW_PROPERTY_NAME_CONTROLLER "_controller"

zval * tf_view_constructor(zval *view, zval *controller, zval *tpl_dir, zval *tpl_ext TSRMLS_DC);

zval * tf_view_render(zval *view, char *tpl_name, zval *params TSRMLS_DC);

void tf_view_display(zval *view, char *tpl_name, zval *params TSRMLS_DC);

#endif