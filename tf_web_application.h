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

#ifndef TF_WEB_APPLICATION_H
#define TF_WEB_APPLICATION_H

#define TF_WEB_APPLICATION_PROPERTY_NAME_AUTO_DISPLAY "_auto_display"
#define TF_WEB_APPLICATION_PROPERTY_NAME_MODULES "_modules"
#define TF_WEB_APPLICATION_PROPERTY_NAME_DEFAULT_MODULE "_default_module"
#define TF_WEB_APPLICATION_PROPERTY_NAME_REQUEST "_request"
#define TF_WEB_APPLICATION_PROPERTY_NAME_ROUTER "_router"
#define TF_WEB_APPLICATION_PROPERTY_NAME_ERROR_CONTROLLER "_error_controller"

void tf_web_application_set_module(zval *web_application, zval *module TSRMLS_DC);

zval * tf_web_application_get_modules(zval *web_application TSRMLS_DC);

zval * tf_web_application_get_default_module(zval *web_application TSRMLS_DC);

zval * tf_web_application_get_auto_display(zval *web_application TSRMLS_DC);

zval * tf_web_application_get_param(zval *web_application, char *name, int name_len TSRMLS_DC);

zval * tf_web_application_get_module(zval *web_application TSRMLS_DC);
#endif
