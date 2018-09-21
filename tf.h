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

#ifndef TF_H
#define TF_H

/* zend_compiler_globals CG
   zend_executor_globals EG
**/

#include "tf_config.h"

ZEND_BEGIN_MODULE_GLOBALS(tf)
    char        *environ;
    zend_bool   use_namespace;
    zval        *config_data_tmp;
    char        *config_section;
    uint        config_parsing_flag;
    HashTable   *configs_cache;
ZEND_END_MODULE_GLOBALS(tf)

#ifdef ZTS
#include "TSRM.h"
#endif

extern ZEND_DECLARE_MODULE_GLOBALS(tf);

#ifdef ZTS
#define TF_G(v) TSRMG(tf_globals_id, zend_tf_globals *, v)
#else
#define TF_G(v) (tf_globals.v)
#endif

#define TF_PROPERTY_NAME_APPLICATION "_application"
#define TF_PROPERTY_NAME_ERROR "_error"

inline zval * tf_set_application(zval *application TSRMLS_DC);

inline zval * tf_get_application(TSRMLS_DC);

void tf_set_error(int error_code, char *error_msg, int error_msg_len, char *error_detail, int error_detail_len TSRMLS_DC);

inline void tf_set_error_msg(char *error_msg, int error_msg_len, char *error_detail, int error_detail_len TSRMLS_DC);

inline void tf_set_error_simple_msg(char *error_msg, int error_msg_len TSRMLS_DC);

inline void tf_clear_error(TSRMLS_DC);

#endif