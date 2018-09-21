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

#ifndef TF_LOGGER_H
#define TF_LOGGER_H

#define TF_LOGGER_PROPERTY_NAME_PATH "_path"
#define TF_LOGGER_PROPERTY_NAME_FILE_NAME "_file_name"
#define TF_LOGGER_PROPERTY_NAME_FILE "_file"
#define TF_LOGGER_PROPERTY_NAME_MAX_SIZE "_max_size"

#define TF_LOGGER_TYPE_INFO 0
#define TF_LOGGER_TYPE_WARN 1
#define TF_LOGGER_TYPE_ERROR 2

zval * tf_logger_constructor(zval *logger, char *path, int path_len, int max_size TSRMLS_DC);

void tf_logger_write(zval *logger, char *msg, int msg_len, int type TSRMLS_DC);

#endif