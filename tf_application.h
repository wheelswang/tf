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

#ifndef TF_APPLICATION_H
#define TF_APPLICATION_H

#define TF_APPLICATION_PROPERTY_NAME_ROOT_PATH "_root_path"
#define TF_APPLICATION_PROPERTY_NAME_CONFIG "_config"
#define TF_APPLICATION_PROPERTY_NAME_DB "_db"
#define TF_APPLICATION_PROPERTY_NAME_REDIS_ARRAY "_redis_array"
#define TF_APPLICATION_PROPERTY_NAME_SESSION "_session"
#define TF_APPLICATION_PROPERTY_NAME_LOGGER "_logger"
#define TF_APPLICATION_PROPERTY_NAME_LOADER "_loader"

zval * tf_application_constructor(zval *application, char *config_file, int config_file_len, char *config_section, int config_section_len TSRMLS_DC);

zval * tf_application_get_root_path(zval *application TSRMLS_DC);

zval * tf_application_get_config(zval *application TSRMLS_DC);

zval * tf_application_get_db(zval *application TSRMLS_DC);

zval * tf_application_get_redis(zval *application, char *name, int name_len TSRMLS_DC);

zval * tf_application_get_session(zval *application TSRMLS_DC);

zval * tf_application_get_logger(zval *application TSRMLS_DC);

inline zval * tf_application_get_loader(zval *application TSRMLS_DC);

#endif