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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "standard/php_filestat.h"
#include "Zend/zend_interfaces.h"
#include "tf.h"
#include "tf_common.h"

zend_class_entry *tf_config_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_config_construct_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, file)
    ZEND_ARG_INFO(0, section)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_config_get_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static void tf_config_deep_copy_section(zval *dst, zval *src TSRMLS_DC) {
    zval **ppzval, **dstppzval, *value;
    HashTable *ht;
    ulong idx;
    char *key;
    uint key_len;

    ht = Z_ARRVAL_P(src);
    for (zend_hash_internal_pointer_reset(ht);
         zend_hash_has_more_elements(ht) == SUCCESS;
         zend_hash_move_forward(ht)) {
        if (zend_hash_get_current_data(ht, (void **)&ppzval) == FAILURE) {
            continue;
        }

        switch (zend_hash_get_current_key_ex(ht, &key, &key_len, &idx, 0, NULL)) {
            case HASH_KEY_IS_STRING:
                if (Z_TYPE_PP(ppzval) == IS_ARRAY &&
                	zend_hash_find(Z_ARRVAL_P(dst), key, key_len, (void **)&dstppzval) == SUCCESS &&
                	Z_TYPE_PP(dstppzval) == IS_ARRAY) {
                	MAKE_STD_ZVAL(value);
                    array_init(value);
                    tf_config_deep_copy_section(value, *dstppzval TSRMLS_CC);
                    tf_config_deep_copy_section(value, *ppzval TSRMLS_CC);
                } else {
                    value = *ppzval;
                    Z_ADDREF_P(value);
                }
                zend_hash_update(Z_ARRVAL_P(dst), key, key_len, (void *)&value, sizeof(zval *), NULL);
                break;
            case HASH_KEY_IS_LONG:
                if (Z_TYPE_PP(ppzval) == IS_ARRAY &&
                	zend_hash_index_find(Z_ARRVAL_P(dst), idx, (void **)&dstppzval) == SUCCESS &&
                	Z_TYPE_PP(dstppzval) == IS_ARRAY) {
                    MAKE_STD_ZVAL(value);
                    array_init(value);
                    tf_config_deep_copy_section(value, *dstppzval TSRMLS_CC);
                    tf_config_deep_copy_section(value, *ppzval TSRMLS_CC);
                } else {
                    value = *ppzval;
                    Z_ADDREF_P(value);
                }
                zend_hash_index_update(Z_ARRVAL_P(dst), idx, (void *)&value, sizeof(zval *), NULL);
                break;
            case HASH_KEY_NON_EXISTANT:
                break;
        }
    }
}

static void tf_config_simple_parser_cb(zval *key, zval *value, zval *index, int callback_type, zval *arr TSRMLS_DC) {
    zval *element;
    switch (callback_type) {
        case ZEND_INI_PARSER_ENTRY: {
            char *skey, *seg, *ptr;
            zval **ppzval, *dst;

            if (!value) {
                break;
            }

            dst = arr;
            skey = estrndup(Z_STRVAL_P(key), Z_STRLEN_P(key));
            if ((seg = php_strtok_r(skey, ".", &ptr))) {
                do {
                    char *real_key = seg;
                    seg = php_strtok_r(NULL, ".", &ptr);
                    if (zend_symtable_find(Z_ARRVAL_P(dst), real_key, strlen(real_key) + 1, (void **) &ppzval) == FAILURE) {
                        if (seg) {
                            zval *tmp;
                            MAKE_STD_ZVAL(tmp);   
                            array_init(tmp);
                            zend_symtable_update(Z_ARRVAL_P(dst), real_key, strlen(real_key) + 1, (void **)&tmp, sizeof(zval *), (void **)&ppzval);
                        } else {
                            MAKE_STD_ZVAL(element);
                            ZVAL_ZVAL(element, value, 1, 0);
                            zend_symtable_update(Z_ARRVAL_P(dst), real_key, strlen(real_key) + 1, (void **)&element, sizeof(zval *), NULL);
                            break;
                        }
                    } else {
                        SEPARATE_ZVAL(ppzval);
                        if (IS_ARRAY != Z_TYPE_PP(ppzval)) {
                            if (seg) {
                                zval *tmp;
                                MAKE_STD_ZVAL(tmp);   
                                array_init(tmp);
                                zend_symtable_update(Z_ARRVAL_P(dst), real_key, strlen(real_key) + 1, (void **)&tmp, sizeof(zval *), (void **)&ppzval);
                            } else {
                                MAKE_STD_ZVAL(element);
                                ZVAL_ZVAL(element, value, 1, 0);
                                zend_symtable_update(Z_ARRVAL_P(dst), real_key, strlen(real_key) + 1, (void **)&element, sizeof(zval *), NULL);
                            }
                        } 
                    }
                    dst = *ppzval;
                } while (seg);
            }
            efree(skey);
            break;
        }

        case ZEND_INI_PARSER_POP_ENTRY: {
            zval *hash, **find_hash, *dst;

            if (!value) {
                break;
            }

            if (!(Z_STRLEN_P(key) > 1 && Z_STRVAL_P(key)[0] == '0') &&
            	is_numeric_string(Z_STRVAL_P(key), Z_STRLEN_P(key), NULL, NULL, 0) == IS_LONG) {
                ulong skey = (ulong)zend_atol(Z_STRVAL_P(key), Z_STRLEN_P(key));
                if (zend_hash_index_find(Z_ARRVAL_P(arr), skey, (void **) &find_hash) == FAILURE) {
                    MAKE_STD_ZVAL(hash);
                    array_init(hash);
                    zend_hash_index_update(Z_ARRVAL_P(arr), skey, &hash, sizeof(zval *), NULL);
                } else {
                    hash = *find_hash;
                }
            } else {
                char *seg, *ptr;
                char *skey = estrndup(Z_STRVAL_P(key), Z_STRLEN_P(key));

                dst = arr;
                if ((seg = php_strtok_r(skey, ".", &ptr))) {
                    while (seg) {
                        if (zend_symtable_find(Z_ARRVAL_P(dst), seg, strlen(seg) + 1, (void **) &find_hash) == FAILURE) {
                            MAKE_STD_ZVAL(hash);
                            array_init(hash);
                            zend_symtable_update(Z_ARRVAL_P(dst), seg, strlen(seg) + 1, (void **)&hash, sizeof(zval *), (void **)&find_hash);
                        }
                        dst = *find_hash;
                        seg = php_strtok_r(NULL, ".", &ptr);
                    }
                    hash = dst;
                } else {
                    if (zend_symtable_find(Z_ARRVAL_P(dst), seg, strlen(seg) + 1, (void **)&find_hash) == FAILURE) {
                        MAKE_STD_ZVAL(hash);
                        array_init(hash);
                        zend_symtable_update(Z_ARRVAL_P(dst), seg, strlen(seg) + 1, (void **)&hash, sizeof(zval *), NULL);
                    } else {
                        hash = *find_hash;
                    }
                }
                efree(skey);
            }

            if (Z_TYPE_P(hash) != IS_ARRAY) {
                zval_dtor(hash);
                INIT_PZVAL(hash);
                array_init(hash);
            }

            MAKE_STD_ZVAL(element);
            ZVAL_ZVAL(element, value, 1, 0);

            if (index && Z_STRLEN_P(index) > 0) {
                add_assoc_zval_ex(hash, Z_STRVAL_P(index), Z_STRLEN_P(index) + 1, element);
            } else {
                add_next_index_zval(hash, element);
            }
            break;
        }

        case ZEND_INI_PARSER_SECTION:
            break;
    }
}

static void tf_config_parser_cb(zval *key, zval *value, zval *index, int callback_type, zval *arr) {
    if (TF_G(config_parsing_flag) == TF_CONFIG_INI_PARSING_END) {
        return;
    }

    if (callback_type == ZEND_INI_PARSER_SECTION) {
        zval **parent;
        char *section, *skey, *next_skey, *skey_orig;
        uint section_len, skey_len;

        if (TF_G(config_parsing_flag) == TF_CONFIG_INI_PARSING_PROCESS) {
            TF_G(config_parsing_flag) = TF_CONFIG_INI_PARSING_END;
            return;
        }

        MAKE_STD_ZVAL(TF_G(config_data_tmp));
        array_init(TF_G(config_data_tmp));

        skey_orig = skey = estrndup(Z_STRVAL_P(key), Z_STRLEN_P(key));
        skey_len = Z_STRLEN_P(key);

        int i = 0;
        while (1) {
            next_skey = strchr(skey, ':');
            skey_len = next_skey ? next_skey - skey : strlen(skey);
            while (*skey == ' ') {
                skey++;
                skey_len--;
            }
            while (*(skey + skey_len - 1) == ' ') {
                skey_len--;
            }

            skey[skey_len] = '\0';
            
            if (i == 0) {
                section = skey;
                section_len = skey_len;
            } else if (zend_symtable_find(Z_ARRVAL_P(arr), skey, skey_len + 1, (void **)&parent) == SUCCESS) {
                tf_config_deep_copy_section(TF_G(config_data_tmp), *parent TSRMLS_CC);
            }

            if (!next_skey) {
                break;
            }

            next_skey++;
            skey = next_skey;
            i++;
        };

        zend_symtable_update(Z_ARRVAL_P(arr), section, section_len + 1, &TF_G(config_data_tmp), sizeof(zval *), NULL);
        if (TF_G(config_section) && strlen(TF_G(config_section)) == section_len
                && !strncasecmp(TF_G(config_section), section, section_len)) {
            TF_G(config_parsing_flag) = TF_CONFIG_INI_PARSING_PROCESS;
        }
        efree(skey_orig);
    } else if (value) {
        zval *active_arr;
        if (TF_G(config_data_tmp)) {
            active_arr = TF_G(config_data_tmp);
        } else {
            active_arr = arr;
        }
        tf_config_simple_parser_cb(key, value, index, callback_type, active_arr TSRMLS_CC);
    }
}

zval * tf_config_constructor(zval *config, char *file, int file_len, char *section, int section_len TSRMLS_DC) {
    if (!config) {
        MAKE_STD_ZVAL(config);
        object_init_ex(config, tf_config_ce);
    }

    char *cache_key;
    spprintf(&cache_key, 0, "%s|%s", file, section ? section : "");

    do {
        if (!TF_G(configs_cache)) {
            break;
        }

        zval **config_cache, **cache_ctime;
        if (zend_symtable_find(TF_G(configs_cache), cache_key, strlen(cache_key) + 1, (void **)&config_cache) == FAILURE) {
            break;
        }

        if (zend_symtable_find(Z_ARRVAL_PP(config_cache), "ctime", 6, (void **)&cache_ctime) == FAILURE) {
            break;
        }
        zval ctime;
        php_stat(file, file_len, 7 /* FS_CTIME */ , &ctime TSRMLS_CC);
        if (Z_LVAL(ctime) > Z_LVAL_PP(cache_ctime)) {
            break;
        }

        zval **config_cache_data;
        if (zend_symtable_find(Z_ARRVAL_PP(config_cache), "data", 5, (void **)&config_cache_data) == FAILURE) {
            break;
        }

        zval *config_data = tf_common_zval_copy(*config_cache_data, 0);
        // persistent的变量不能存入类属性 不然会出内存问题
        zend_update_property(tf_config_ce, config, ZEND_STRL(TF_CONFIG_PROPERTY_NAME_DATA), config_data TSRMLS_CC);

        zval_ptr_dtor(&config_data);
        efree(cache_key);

        return config;
    } while (0);

    zend_file_handle fh = {0};
    fh.filename = file;
    fh.handle.fp = VCWD_FOPEN(file, "r");
    fh.type = ZEND_HANDLE_FP;

    TF_G(config_parsing_flag) = TF_CONFIG_INI_PARSING_START;

    if (section && section_len) {
        TF_G(config_section) = estrndup(section, section_len);
    }
    
    zval *config_data;
    MAKE_STD_ZVAL(config_data);
    array_init(config_data);

    if (zend_parse_ini_file(&fh, 0, 0, (zend_ini_parser_cb_t)tf_config_parser_cb, config_data TSRMLS_CC)  == FAILURE) {
        zval_ptr_dtor(&config_data);
        efree(cache_key);
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to parse config file");
    }

    if (section && section_len) {
        zval **section_config;
        if (zend_symtable_find(Z_ARRVAL_P(config_data), section, section_len + 1, (void **)&section_config) == FAILURE) {
            zval_ptr_dtor(&config_data);
            efree(cache_key);
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to find section:%s from config", section);
        }

        // zval *tmp;
        // MAKE_STD_ZVAL(tmp);
        // ZVAL_ZVAL(tmp, *section_config, 1, 0);
        // zval_ptr_dtor(&config_data);
        // config_data = tmp;
        zval *tmp = *section_config;
        Z_ADDREF_PP(section_config);
        zval_ptr_dtor(&config_data);
        config_data = tmp;
    }

    zend_update_property(tf_config_ce, config, ZEND_STRL(TF_CONFIG_PROPERTY_NAME_DATA), config_data TSRMLS_CC);
    zval_ptr_dtor(&config_data);

    if (!TF_G(configs_cache)) {
        TF_G(configs_cache) = (HashTable *)pemalloc(sizeof(HashTable), 1);
        zend_hash_init(TF_G(configs_cache), 0, NULL, (dtor_func_t)tf_common_persistent_zval_dtor, 1);
    }

    zval *config_cache, *ctime;
    MAKE_STD_ZVAL(config_cache);
    array_init(config_cache);
    MAKE_STD_ZVAL(ctime);

    php_stat(file, file_len, 7 /* FS_CTIME */ , ctime TSRMLS_CC);
    add_assoc_zval(config_cache, "ctime", ctime);
    add_assoc_zval(config_cache, "data", config_data);
    Z_ADDREF_P(config_data);

    zval *config_cache_persistent = tf_common_zval_copy(config_cache, 1);
    zend_symtable_update(TF_G(configs_cache), cache_key, strlen(cache_key) + 1, &config_cache_persistent, sizeof(zval *), NULL);

    zval_ptr_dtor(&config_cache);
    efree(cache_key);

    return config;
}

zval * tf_config_get(zval *config, char *key TSRMLS_DC) {
    zval *config_data = zend_read_property(tf_config_ce, config, ZEND_STRL(TF_CONFIG_PROPERTY_NAME_DATA), 1 TSRMLS_CC);
    if (!config_data) {
        return NULL;
    }

    zval **ppzval, *ret_val = NULL;
    HashTable *hash_table = Z_ARRVAL_P(config_data);
    char *ptr, *orig_ptr, *next;
    ptr = orig_ptr = estrndup(key, strlen(key));
    while (1) {
        next = strchr(ptr, '.');
        if (next) {
            *next = '\0';
        }
        if (zend_symtable_find(hash_table, ptr, strlen(ptr) + 1, (void **)&ppzval) == FAILURE) {
            break;
        }

        if (!next) {
            ret_val = *ppzval;
            break;
        }

        next++;
        ptr = next;
        hash_table = Z_ARRVAL_PP(ppzval);
    };

    efree(orig_ptr);

    return ret_val;
}

PHP_METHOD(tf_config, __construct) {
    zval *file, *section = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &file, &section) == FAILURE) {
        return;
    }

    tf_config_constructor(getThis(), Z_STRVAL_P(file), Z_STRLEN_P(file), section ? Z_STRVAL_P(section) : NULL, section ? Z_STRLEN_P(section) : 0 TSRMLS_CC);
}

PHP_METHOD(tf_config, getAll) {
    zval *config_data = zend_read_property(tf_config_ce, getThis(), ZEND_STRL(TF_CONFIG_PROPERTY_NAME_DATA), 1 TSRMLS_CC);
    if (!config_data) {
        return;
    }

    RETVAL_ZVAL(config_data, 1, 0);
}

PHP_METHOD(tf_config, get) {
    zval *key;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &key) == FAILURE) {
        return;
    }

    zval *pzval = tf_config_get(getThis(), Z_STRVAL_P(key));
    if (pzval) {
        RETVAL_ZVAL(pzval, 1, 0);
    }
}

zend_function_entry tf_config_methods[] = {
    PHP_ME(tf_config, __construct, tf_config_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_config, getAll, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_config, get, tf_config_get_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_config) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Config", tf_config_methods);

    tf_config_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

    zend_declare_property_null(tf_config_ce, ZEND_STRL(TF_CONFIG_PROPERTY_NAME_DATA), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_config_ce, ZEND_STRL(TF_CONFIG_PROPERTY_NAME_SECTION), ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}