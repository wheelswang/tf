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
#include "tf_loader.h"
#include "tf_controller.h"
#include "tf_application.h"
#include "tf_common.h"
#include "ext/standard/php_filestat.h"

zend_class_entry *tf_loader_ce;
extern zend_class_entry *tf_controller_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_loader_construct_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, app_root)
    ZEND_ARG_INFO(0, load_paths)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_loader_autoload_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

zval * tf_loader_constructor(zval *loader, zval *app_root, zval *load_paths TSRMLS_DC) {
    if (!loader) {
        MAKE_STD_ZVAL(loader);
        object_init_ex(loader, tf_loader_ce);
    }

    zend_update_property(tf_loader_ce, loader, ZEND_STRL(TF_LOADER_PROPERTY_NAME_APP_ROOT), app_root TSRMLS_CC);

    zval *load_paths_new;
    MAKE_STD_ZVAL(load_paths_new);
    array_init(load_paths_new);
    add_next_index_zval(load_paths_new, app_root);
    Z_ADDREF_P(app_root);
    if (load_paths) {
        char realpath[MAXPATHLEN];
        if (Z_TYPE_P(load_paths) == IS_STRING && VCWD_REALPATH(Z_STRVAL_P(load_paths), realpath)) {
            add_next_index_zval(load_paths_new, load_paths);
            Z_ADDREF_P(load_paths);
        } else if (Z_TYPE_P(load_paths) == IS_ARRAY) {
            zval **ppzval;
            for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(load_paths));
                 zend_hash_get_current_data(Z_ARRVAL_P(load_paths), (void **)&ppzval) == SUCCESS;
                 zend_hash_move_forward(Z_ARRVAL_P(load_paths))) {
                if (Z_TYPE_PP(ppzval) == IS_STRING && VCWD_REALPATH(Z_STRVAL_PP(ppzval), realpath)) {
                    add_next_index_zval(load_paths_new, *ppzval);
                    Z_ADDREF_PP(ppzval);
                }
            }
        }
    }

    zend_update_property(tf_loader_ce, loader, ZEND_STRL(TF_LOADER_PROPERTY_NAME_LOAD_PATHS), load_paths_new TSRMLS_DC);
    zval_ptr_dtor(&load_paths_new);

    zval *autoload, *method, *function, *ret;
    MAKE_STD_ZVAL(autoload);
    array_init(autoload);
    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "autoload", 1);
    add_next_index_zval(autoload, loader);
    Z_ADDREF_P(loader);
    add_next_index_zval(autoload, method);
    zval *params[1];
    params[0] = autoload;

    MAKE_STD_ZVAL(function)
    ZVAL_STRING(function, "spl_autoload_register", 1);
    MAKE_STD_ZVAL(ret);

    call_user_function(EG(function_table), NULL, function, ret, 1, params TSRMLS_CC);

    zval_ptr_dtor(&ret);
    zval_ptr_dtor(&autoload);
    zval_ptr_dtor(&function);

    return loader;
}

zend_class_entry * tf_loader_load_class(zval *loader, char *classname, int classname_len TSRMLS_DC) {
    char *class_path = emalloc(classname_len + 1);
    strcpy(class_path, classname);
    int i = 0;
    for (; i < classname_len; i++) {
        if (class_path[i] == '\\') {
            class_path[i] = '/';
        }
    }

    zval *import_paths = zend_read_property(tf_loader_ce, loader, ZEND_STRL(TF_LOADER_PROPERTY_NAME_LOAD_PATHS), 1 TSRMLS_CC);
    HashTable *import_paths_ht = Z_ARRVAL_P(import_paths);
    zval **ppzval;
    for (zend_hash_internal_pointer_reset(import_paths_ht);
        zend_hash_has_more_elements(import_paths_ht) == SUCCESS;
        zend_hash_move_forward(import_paths_ht)) {
        if (zend_hash_get_current_data(import_paths_ht, (void **) &ppzval) == FAILURE) {
            continue;
        }

        char *path;
        spprintf(&path, 0, "%s/%s.php", Z_STRVAL_PP(ppzval), class_path);
        if (!tf_loader_load_file(path)) {
            efree(path);
            continue;
        }

        zend_class_entry **ce = NULL;
        char *class_lowercase = zend_str_tolower_dup(classname, classname_len);
        if (zend_hash_find(EG(class_table), class_lowercase, classname_len + 1, (void **) &ce) != SUCCESS) {
            efree(path);
            efree(class_path);
            efree(class_lowercase);
            return NULL;
        }

        efree(class_lowercase);
        efree(path);
        efree(class_path);

        return *ce;
    }

    efree(class_path);

    return NULL;
}

zend_class_entry * tf_loader_load_controller_class(zval *loader, zval *controller, zval *module TSRMLS_DC) {
    char *classname;
    *Z_STRVAL_P(controller) = toupper(*Z_STRVAL_P(controller));
    zend_bool is_controller_name = 0;
    char *p = strstr(Z_STRVAL_P(controller), "Controller");
    if (p && strcmp(p, "Controller") == 0) {
        is_controller_name = 1;
    }
    if (Z_TYPE_P(module) == IS_NULL) {
        if (is_controller_name) {
            spprintf(&classname, 0, "application\\web\\controller\\%s", Z_STRVAL_P(controller));
        } else {
            spprintf(&classname, 0, "application\\web\\controller\\%sController", Z_STRVAL_P(controller));
        }
    } else {
        if (is_controller_name) {
            spprintf(&classname, 0, "application\\web\\%s\\controller\\%s", Z_STRVAL_P(module), Z_STRVAL_P(controller));
        } else {
            spprintf(&classname, 0, "application\\web\\%s\\controller\\%sController", Z_STRVAL_P(module), Z_STRVAL_P(controller));
        }
    }
    *Z_STRVAL_P(controller) = tolower(*Z_STRVAL_P(controller));
    zend_class_entry *ce = tf_loader_load_class(loader, classname, strlen(classname) TSRMLS_CC);
    efree(classname);

    return ce;
}

zend_bool tf_loader_load_file(char *path TSRMLS_DC) {
    zval *file_exists;
    MAKE_STD_ZVAL(file_exists);
    php_stat(path, strlen(path), FS_EXISTS, file_exists TSRMLS_CC);
    if (!Z_BVAL_P(file_exists)) {
        zval_ptr_dtor(&file_exists);
        return FALSE;
    }
    zval_ptr_dtor(&file_exists);

    zend_file_handle file_handle;
    file_handle.filename = path;
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = NULL;
    file_handle.handle.fp = NULL;

    zend_op_array *op_array = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);
    if (op_array) {
        if (!file_handle.opened_path) {
            file_handle.opened_path = path;
        }
        int dummy = 1;
        zend_hash_add(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path) + 1, (void *)&dummy, sizeof(int), NULL);

        TF_STORE_EG_ENVIRON();

        zval *result = NULL;
        EG(return_value_ptr_ptr) = &result;
        EG(active_op_array)      = op_array;

        if (!EG(active_symbol_table)) {
#if PHP_MINOR_VERSION < 5
            zval *orig_this = EG(This);
            EG(This) = NULL;
            zend_rebuild_symbol_table(TSRMLS_C);
            EG(This) = orig_this;
#else
            zend_rebuild_symbol_table(TSRMLS_C);
#endif
        }

        zend_execute(op_array TSRMLS_CC);

        destroy_op_array(op_array TSRMLS_CC);
        efree(op_array);
        if (!EG(exception)) {
            if (EG(return_value_ptr_ptr) && *EG(return_value_ptr_ptr)) {
                zval_ptr_dtor(EG(return_value_ptr_ptr));
            }
        }

        TF_RESTORE_EG_ENVIRON();

        return TRUE;
    }

    zend_destroy_file_handle(&file_handle TSRMLS_CC);

    return FALSE;
}

PHP_METHOD(tf_loader, __construct) {
    zval *app_root;
    zval *import_paths = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &app_root, &import_paths) == FAILURE) {
        return;
    }

    convert_to_string(app_root);
    tf_loader_constructor(getThis(), app_root, import_paths TSRMLS_CC);
}

PHP_METHOD(tf_loader, autoload) {
    zval *classname;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &classname) == FAILURE) {
        RETURN_FALSE;
    }

    if (tf_loader_load_class(getThis(), Z_STRVAL_P(classname), Z_STRLEN_P(classname) TSRMLS_CC)) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

zend_function_entry tf_loader_methods[] = {
    PHP_ME(tf_loader, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(tf_loader, autoload, tf_loader_autoload_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_loader) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Loader", tf_loader_methods);

    tf_loader_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

    zend_declare_property_null(tf_loader_ce, ZEND_STRL(TF_LOADER_PROPERTY_NAME_APP_ROOT), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_loader_ce, ZEND_STRL(TF_LOADER_PROPERTY_NAME_LOAD_PATHS), ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}