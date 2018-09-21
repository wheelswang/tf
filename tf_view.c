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
#include "tf.h"
#include "tf_view.h"
#include "tf_loader.h"

zend_class_entry *tf_view_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_view___construct_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, tpl_dir)
    ZEND_ARG_INFO(0, tpl_ext)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_view_setTplDir_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, tpl_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_view_assign_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_view_render_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, tpl_name)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_view_display_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, tpl_name)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_view_get_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

zval * tf_view_constructor(zval *view, zval *controller, zval *tpl_dir, zval *tpl_ext TSRMLS_DC) {
    if (!view) {
        MAKE_STD_ZVAL(view);
        object_init_ex(view, tf_view_ce);
    }

    if (controller) {
        zend_update_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_CONTROLLER), controller TSRMLS_CC);
    }

    zval *tpl_vars;
    MAKE_STD_ZVAL(tpl_vars);
    array_init(tpl_vars);
    zend_update_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_VARS), tpl_vars TSRMLS_CC);
    zval_ptr_dtor(&tpl_vars);

    zend_update_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_DIR), tpl_dir TSRMLS_CC);
    if (tpl_ext) {
        zend_update_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_EXT), tpl_ext TSRMLS_CC);
    }

    return view;
}

void tf_view_assign_single(zval *view, char *name, uint name_len, zval *value TSRMLS_DC) {
    zval *tpl_vars = zend_read_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_VARS), 1 TSRMLS_CC);
    add_assoc_zval_ex(tpl_vars, name, name_len + 1, value);
    Z_ADDREF_P(value);
}

void tf_view_assign_multi(zval *view, zval *params TSRMLS_DC) {
    if (Z_TYPE_P(params) != IS_ARRAY) {
        return;
    }
    
    zval *tpl_vars = zend_read_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_VARS), 1 TSRMLS_CC);
    zend_hash_copy(Z_ARRVAL_P(tpl_vars), Z_ARRVAL_P(params), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));
}

static int tf_view_valid_var_name(char *var_name, int len) /* {{{ */
{
    int i, ch;

    if (!var_name)
        return 0;

    /* These are allowed as first char: [a-zA-Z_\x7f-\xff] */
    ch = (int)((unsigned char *)var_name)[0];
    if (var_name[0] != '_' &&
            (ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
            (ch < 97  /* a    */ || /* z    */ ch > 122) &&
            (ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
       ) {
        return 0;
    }

    /* And these as the rest: [a-zA-Z0-9_\x7f-\xff] */
    if (len > 1) {
        for (i = 1; i < len; i++) {
            ch = (int)((unsigned char *)var_name)[i];
            if (var_name[i] != '_' &&
                    (ch < 48  /* 0    */ || /* 9    */ ch > 57)  &&
                    (ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
                    (ch < 97  /* a    */ || /* z    */ ch > 122) &&
                    (ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
               ) {
                return 0;
            }
        }
    }

    return 1;
}

void tf_view_extract_vars(zval *view TSRMLS_DC) {
    zval *tpl_vars = zend_read_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_VARS), 1 TSRMLS_CC);
    HashTable *ht = Z_ARRVAL_P(tpl_vars);
    zval **value;
    char *name;
    uint name_len;
    ulong idx;
    for (zend_hash_internal_pointer_reset(ht);
        zend_hash_has_more_elements(ht) == SUCCESS;
        zend_hash_move_forward(ht)) {
        zend_hash_get_current_data(ht, (void **)&value);
        if (zend_hash_get_current_key_ex(ht, &name, &name_len, &idx, 0, NULL) != HASH_KEY_IS_STRING) {
            continue;
        }

        // GLOBALS protection
        if (name_len == sizeof("GLOBALS") && !strcmp(name, "GLOBALS")) {
            continue;
        }

        if (!tf_view_valid_var_name(name, name_len - 1)) {
            continue;
        }

        ZEND_SET_SYMBOL_WITH_LENGTH(EG(active_symbol_table), name, name_len,
                        *value, Z_REFCOUNT_P(*value) + 1, PZVAL_IS_REF(*value));
    }
}

zval * tf_view_render(zval *view, char *tpl_name, zval *params TSRMLS_DC) {
    if (params) {
        tf_view_assign_multi(view, params);
    }

    char *path;
    zval *tpl_dir = zend_read_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_DIR), 1 TSRMLS_CC);
    zval *tpl_ext = zend_read_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_EXT), 1 TSRMLS_CC);
    if (Z_TYPE_P(tpl_dir) == IS_NULL) {
        spprintf(&path, 0, "%s.%s", tpl_name, Z_STRVAL_P(tpl_ext));
    } else {
        spprintf(&path, 0, "%s/%s.%s", Z_STRVAL_P(tpl_dir), tpl_name, Z_STRVAL_P(tpl_ext));
    }

    HashTable *calling_symbol_table;
    if (EG(active_symbol_table)) {
        calling_symbol_table = EG(active_symbol_table);
    } else {
        calling_symbol_table = NULL;
    }
    ALLOC_HASHTABLE(EG(active_symbol_table));
    zend_hash_init(EG(active_symbol_table), 0, NULL, ZVAL_PTR_DTOR, 0);

    tf_view_extract_vars(view);

    if (php_output_start_user(NULL, 0, PHP_OUTPUT_HANDLER_STDFLAGS TSRMLS_CC) == FAILURE) {
        efree(path);
        php_error_docref("ref.outcontrol" TSRMLS_CC, E_WARNING, "failed to create buffer");
        return NULL;
    }
    zval *me_old  = EG(This);
    EG(This) = view;

    if (!tf_loader_load_file(path)) {
        EG(This) = me_old;
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to load file:%s", path);
        efree(path);
        php_output_end(TSRMLS_C);
        return NULL;
    }

    EG(This) = me_old;

    if (calling_symbol_table) {
        zend_hash_destroy(EG(active_symbol_table));
        FREE_HASHTABLE(EG(active_symbol_table));
        EG(active_symbol_table) = calling_symbol_table;
    }

    zval *ret;
    MAKE_STD_ZVAL(ret);
    if (php_output_get_contents(ret TSRMLS_CC) == FAILURE) {
        php_output_end(TSRMLS_C);
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to fetch ob content");
        efree(path);
        return NULL;
    }

    if (php_output_discard(TSRMLS_C) != SUCCESS ) {
        efree(path);
        return NULL;
    }

    efree(path);

    return ret;
}

void tf_view_display(zval *view, char *tpl_name, zval *params TSRMLS_DC) {
    if (params) {
        tf_view_assign_multi(view, params);
    }

    char *path;
    zval *tpl_dir = zend_read_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_DIR), 1 TSRMLS_CC);
    zval *tpl_ext = zend_read_property(tf_view_ce, view, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_EXT), 1 TSRMLS_CC);
    if (Z_TYPE_P(tpl_dir) == IS_NULL) {
        spprintf(&path, 0, "%s.%s", tpl_name, Z_STRVAL_P(tpl_ext));
    } else {
        spprintf(&path, 0, "%s/%s.%s", Z_STRVAL_P(tpl_dir), tpl_name, Z_STRVAL_P(tpl_ext));
    }

    HashTable *calling_symbol_table;
    if (EG(active_symbol_table)) {
        calling_symbol_table = EG(active_symbol_table);
    } else {
        calling_symbol_table = NULL;
    }
    ALLOC_HASHTABLE(EG(active_symbol_table));
    zend_hash_init(EG(active_symbol_table), 0, NULL, ZVAL_PTR_DTOR, 0);

    tf_view_extract_vars(view);

    zval *me_old  = EG(This);
    EG(This) = view;

    if (!tf_loader_load_file(path)) {
        EG(This) = me_old;
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to load file:%s", path);
    }

    EG(This) = me_old;

    if (calling_symbol_table) {
        zend_hash_destroy(EG(active_symbol_table));
        FREE_HASHTABLE(EG(active_symbol_table));
        EG(active_symbol_table) = calling_symbol_table;
    }

    efree(path);
}

PHP_METHOD(tf_view, __construct) {
    zval *tpl_dir, *tpl_ext = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &tpl_ext) != SUCCESS) {
        return;
    }

    convert_to_string(tpl_dir);
    if (tpl_ext) {
        convert_to_string(tpl_ext);
    }
    tf_view_constructor(getThis(), NULL, tpl_dir, tpl_ext TSRMLS_CC);
}

PHP_METHOD(tf_view, setTplDir) {
    zval *tpl_dir;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &tpl_dir) != SUCCESS) {
        RETURN_FALSE;
    }

    convert_to_string(tpl_dir);
    zend_update_property(tf_view_ce, getThis(), ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_DIR), tpl_dir TSRMLS_CC);
}

PHP_METHOD(tf_view, assign) {
    uint argc = ZEND_NUM_ARGS();
    if (argc == 1) {
        zval *params;
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &params) == FAILURE) {
            return;
        }
        tf_view_assign_multi(getThis(), params TSRMLS_CC);
    } else if (argc == 2) {
        zval *value;
        char *name;
        uint name_len;
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &value) == FAILURE) {
            return;
        }

        tf_view_assign_single(getThis(), name, name_len, value TSRMLS_CC);
    } else {
        WRONG_PARAM_COUNT;
    }
}

PHP_METHOD(tf_view, render) {
    zval *tpl_name;
    zval *params = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &tpl_name, &params) == FAILURE) {
        return;
    }

    zval *ret = tf_view_render(getThis(), Z_STRVAL_P(tpl_name), params TSRMLS_CC);
    if (ret) {
        RETURN_ZVAL(ret, 0, 0);
    }
}

PHP_METHOD(tf_view, display) {
    zval *tpl_name;
    zval *params = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &tpl_name, &params) == FAILURE) {
        return;
    }

    tf_view_display(getThis(), Z_STRVAL_P(tpl_name), params TSRMLS_CC);
}

PHP_METHOD(tf_view, getVar) {
    zval *name;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &name) == FAILURE) {
        return;
    }

    zval **var;
    zval *tpl_vars = zend_read_property(tf_view_ce, getThis(), ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_VARS), 1 TSRMLS_CC);
    if (zend_hash_find(Z_ARRVAL_P(tpl_vars), Z_STRVAL_P(name), Z_STRLEN_P(name) + 1, (void **)&var) == FAILURE) {
        return;
    }

    RETURN_ZVAL(*var, 1, 0);
}

PHP_METHOD(tf_view, getVars) {
    zval *tpl_vars = zend_read_property(tf_view_ce, getThis(), ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_VARS), 1 TSRMLS_CC);
    RETURN_ZVAL(tpl_vars, 1, 0);
}

PHP_METHOD(tf_view, getController) {
    zval *controller = zend_read_property(tf_view_ce, getThis(), ZEND_STRL(TF_VIEW_PROPERTY_NAME_CONTROLLER), 1 TSRMLS_CC);
    RETURN_ZVAL(controller, 1, 0);
}

zend_function_entry tf_view_method[] = {
    PHP_ME(tf_view, __construct, tf_view___construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_view, setTplDir, tf_view_setTplDir_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_view, assign, tf_view_assign_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_view, render, tf_view_render_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_view, display, tf_view_display_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_view, getVar, tf_view_get_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_view, getVars, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(tf_view, getController, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_view) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\View", tf_view_method);
    tf_view_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

    zend_declare_property_null(tf_view_ce, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_VARS), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_view_ce, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_DIR), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_string(tf_view_ce, ZEND_STRL(TF_VIEW_PROPERTY_NAME_TPL_EXT), "php", ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_view_ce, ZEND_STRL(TF_VIEW_PROPERTY_NAME_CONTROLLER), ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}