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

#ifndef TF_SESSION_H
#define TF_SESSION_H

#define TF_SESSION_PROPERTY_NAME_REDIS "_redis"
#define TF_SESSION_PROPERTY_NAME_HANDLER "_handler"
#define TF_SESSION_PROPERTY_NAME_STARTED "_started"

zval * tf_session_constructor(zval *session, zval *config TSRMLS_DC);

#endif