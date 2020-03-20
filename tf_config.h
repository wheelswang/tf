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

#ifndef TF_CONFIG_H
#define TF_CONFIG_H

#define TF_CONFIG_INI_PARSING_START   0
#define TF_CONFIG_INI_PARSING_PROCESS 1
#define TF_CONFIG_INI_PARSING_END     2

#define TF_CONFIG_PROPERTY_NAME_DATA "_data"
#define TF_CONFIG_PROPERTY_NAME_FILE "_file"
#define TF_CONFIG_PROPERTY_NAME_SECTION "_section"

ZEND_BEGIN_MODULE_GLOBALS(tf_config)
    zval        *config_data_tmp;
    char        *config_section;
    uint        config_parsing_flag;
    HashTable   *configs_cache;
ZEND_END_MODULE_GLOBALS(tf_config)

ZEND_DECLARE_MODULE_GLOBALS(tf_config)

#ifdef ZTS
#define TF_CONFIG_G(v) TSRMG(tf_config_globals_id, zend_tf_config_globals *, v)
#else
#define TF_CONFIG_G(v) (tf_config_globals.v)
#endif

zval * tf_config_constructor(zval *config, char *file, int file_len, char *section, int section_len TSRMLS_DC);
zval * tf_config_get(zval *config, char *key, int key_len TSRMLS_DC);

#endif