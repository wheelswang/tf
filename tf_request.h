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

#ifndef TF_REQUEST_H
#define TF_REQUEST_H

#define TF_REQUEST_PROPERTY_NAME_SCHEMA "_schema"
#define TF_REQUEST_PROPERTY_NAME_DOMAIN "_domain"
#define TF_REQUEST_PROPERTY_NAME_URI "_uri"
#define TF_REQUEST_PROPERTY_NAME_URL "_url"

zval * tf_request_get_url(zval *request TSRMLS_DC);

zval * tf_request_constructor(zval *request TSRMLS_DC);

zval * tf_request_query(uint type, char * name, uint len TSRMLS_DC);

zend_bool tf_request_is_ajax(TSRMLS_DC);

#endif