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

#ifndef TF_ROUTER_H
#define TF_ROUTER_H

#define TF_ROUTER_PROPERTY_NAME_URL "_url"
#define TF_ROUTER_PROPERTY_NAME_URI "_uri"
#define TF_ROUTER_PROPERTY_NAME_MODULE "_module"
#define TF_ROUTER_PROPERTY_NAME_CONTROLLER "_controller"
#define TF_ROUTER_PROPERTY_NAME_ACTION "_action"
#define TF_ROUTER_PROPERTY_NAME_RULES "_rules"
#define TF_ROUTER_PROPERTY_NAME_PARAMS "_params"
#define TF_ROUTER_PROPERTY_NAME_PARSED "_parsed"


zval * tf_router_constructor(zval *router, char *url TSRMLS_DC);

void tf_router_add_rule(zval *router, zval *search, zval *replace TSRMLS_DC);

zval * tf_router_get_module(zval *router TSRMLS_DC);

zval * tf_router_get_controller(zval *router TSRMLS_DC);

zval * tf_router_get_action(zval *router TSRMLS_DC);

zval * tf_router_get_param(zval *router, char *name, uint name_len TSRMLS_DC);

#endif