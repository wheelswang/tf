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
#include "SAPI.h"
#include "ext/json/php_json.h"
#include "ext/standard/php_string.h"
#include "tf.h"
#include "tf_controller.h"
#include "tf_application.h"
#include "tf_web_application.h"
#include "tf_request.h"
#include "tf_router.h"
#include "tf_loader.h"
#include "tf_view.h"
#include "tf_common.h"
#include "tf_error.h"

zend_class_entry *tf_controller_ce;
zend_class_entry *reflection_method_ce;
zend_class_entry *reflection_parameter_ce;
extern zend_class_entry *tf_view_ce;

ZEND_BEGIN_ARG_INFO_EX(tf_controller_assign_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_controller_render_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, tpl_name)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_controller_display_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, tpl_name)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_controller_ajaxError_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, error_msg)
    ZEND_ARG_INFO(0, error_code)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_controller_ajaxSuccess_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(tf_controller_error_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, error)
ZEND_END_ARG_INFO()

void tf_controller_constructor(zval *controller, zval *view_ext TSRMLS_DC) {
    if (view_ext) {
        zend_update_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW_EXT), view_ext TSRMLS_CC);
    }
}

zval * tf_controller_init_view(zval *controller TSRMLS_DC) {
    zval *view = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW), 1 TSRMLS_CC);
    if (Z_TYPE_P(view) == IS_NULL) {
        zend_class_entry *controller_ce = Z_OBJCE_P(controller);
        char *tmp_name = strrchr(controller_ce->name, '\\');
        tmp_name++;
        int tmp_name_len = strlen(tmp_name);
        char *controller_name = emalloc(tmp_name_len - 9);
        strncpy(controller_name, tmp_name, tmp_name_len - 10);
        *(controller_name + tmp_name_len - 10) = '\0';
        *controller_name = tolower(*controller_name);
        char *tpl_dir;
        zval *module = tf_web_application_get_module(tf_get_application(TSRMLS_CC) TSRMLS_CC);
        zval *app_root = tf_application_get_root_path(tf_get_application(TSRMLS_CC) TSRMLS_CC);
        if (Z_TYPE_P(module) == IS_NULL) {
            spprintf(&tpl_dir, 0, "%s/application/web/view/%s", Z_STRVAL_P(app_root), controller_name);
        } else {
            spprintf(&tpl_dir, 0, "%s/application/web/%s/view/%s", Z_STRVAL_P(app_root), Z_STRVAL_P(module), controller_name);
        }
        zval *view_ext = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW_EXT), 1 TSRMLS_CC);
        zval *tpl_dir_pzval;
        MAKE_STD_ZVAL(tpl_dir_pzval);
        ZVAL_STRING(tpl_dir_pzval, tpl_dir, 0);
        view = tf_view_constructor(NULL, controller, tpl_dir_pzval, view_ext TSRMLS_CC);
        zend_update_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW), view TSRMLS_CC);

        efree(controller_name);
        zval_ptr_dtor(&view);
        zval_ptr_dtor(&tpl_dir_pzval);
    }

    return view;
} 

void tf_controller_run_action(zval *controller, zval *action TSRMLS_CC) {
    zval *ref_method, *method, *ref_method_ctr_params[2], *ret;
    MAKE_STD_ZVAL(ref_method);
    object_init_ex(ref_method, reflection_method_ce);
    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "__construct", 1);
    ref_method_ctr_params[0] = controller;
    ref_method_ctr_params[1] = action;
    MAKE_STD_ZVAL(ret);
    call_user_function(&reflection_method_ce->function_table, &ref_method, method, ret, 2, ref_method_ctr_params TSRMLS_CC);
    TF_ZVAL_DTOR(method);
    TF_ZVAL_DTOR(ret);
    if (EG(exception)) {
        return;
    }

    zval *ret_parameters;
    ZVAL_STRING(method, "getParameters", 1);
    MAKE_STD_ZVAL(ret_parameters);
    call_user_function(&reflection_method_ce->function_table, &ref_method, method, ret_parameters, 0, NULL TSRMLS_CC);
    TF_ZVAL_DTOR(method);
    int param_count = zend_hash_num_elements(Z_ARRVAL_P(ret_parameters));
    zval **params = NULL;
    if (param_count) {
        params = emalloc(sizeof(zval *) * param_count);
    }
    zval **ppzval, *method_get_default_value, *ret_default_value = NULL;
    ZVAL_STRING(method, "getName", 1);
    MAKE_STD_ZVAL(method_get_default_value);
    ZVAL_STRING(method_get_default_value, "getDefaultValue", 1);
    int i = 0;
    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(ret_parameters));
         zend_hash_get_current_data(Z_ARRVAL_P(ret_parameters), (void **)&ppzval) == SUCCESS;
         zend_hash_move_forward(Z_ARRVAL_P(ret_parameters))) {
        call_user_function(&reflection_parameter_ce->function_table, ppzval, method, ret, 0, NULL TSRMLS_CC);
        zval *param = tf_web_application_get_param(tf_get_application(TSRMLS_CC), Z_STRVAL_P(ret), Z_STRLEN_P(ret) TSRMLS_CC);
        if (!param) {
            MAKE_STD_ZVAL(ret_default_value);
            call_user_function(&reflection_parameter_ce->function_table, ppzval, method_get_default_value, ret_default_value, 0, NULL TSRMLS_CC);
            if (EG(exception)) {
                zend_clear_exception(TSRMLS_CC);
                
                char *error;
                spprintf(&error, 0, "miss param: %s", Z_STRVAL_P(ret));
                tf_web_application_run_error_controller(tf_get_application(TSRMLS_CC), E_USER_ERROR, error, NULL, 0 TSRMLS_CC);
                efree(error);

                zval_ptr_dtor(&method);
                zval_ptr_dtor(&method_get_default_value);
                zval_ptr_dtor(&ret_default_value);
                zval_ptr_dtor(&ret);
                zval_ptr_dtor(&ret_parameters);
                zval_ptr_dtor(&ref_method);
                if (param_count) {
                    efree(params);
                }

                return;
            }
            *(params + i) = ret_default_value;
        } else {
            if (Z_TYPE_P(param) == IS_STRING) {
                zval *trim_param;
                MAKE_STD_ZVAL(trim_param);
                php_trim(Z_STRVAL_P(param), Z_STRLEN_P(param), NULL, 0, trim_param, 3 TSRMLS_CC);
                param = trim_param;
            } else if (Z_TYPE_P(param) == IS_ARRAY) {
                if (zend_hash_find(Z_ARRVAL_P(param), ZEND_STRS("size"), (void **)&ppzval) == SUCCESS && Z_TYPE_PP(ppzval) != IS_STRING) {
                    // param is $_FILES elem
                    Z_ADDREF_P(param);
                } else {
                    zval *trim_param_arr;
                    MAKE_STD_ZVAL(trim_param_arr);
                    array_init(trim_param_arr);
                    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(param));
                         zend_hash_get_current_data(Z_ARRVAL_P(param), (void **)&ppzval) == SUCCESS;
                         zend_hash_move_forward(Z_ARRVAL_P(param))) {
                        char *param_trim_str = php_trim(Z_STRVAL_PP(ppzval), Z_STRLEN_PP(ppzval), NULL, 0, NULL, 3 TSRMLS_CC);
                        add_next_index_string(trim_param_arr, param_trim_str, 0);
                    }
                    param = trim_param_arr;
                }
            } else {
                Z_ADDREF_P(param);
            }
            *(params + i) = param;
        }
        i++;
        TF_ZVAL_DTOR(ret);
    }
    zval_ptr_dtor(&ret_parameters);
    zval_ptr_dtor(&method_get_default_value);
    TF_ZVAL_DTOR(method);
    zval_ptr_dtor(&ref_method);

    ZVAL_STRING(method, "__construct", 1);
    zend_class_entry *controller_ce = Z_OBJCE_P(controller);
    call_user_function(&controller_ce->function_table, &controller, method, ret, 0, NULL TSRMLS_CC);
    TF_ZVAL_DTOR(ret);

    call_user_function(&controller_ce->function_table, &controller, action, ret, param_count, params TSRMLS_CC);
    TF_ZVAL_DTOR(method);
    TF_ZVAL_DTOR(ret);
    if (param_count) {
        for (i = 0; i < param_count; i++) {
            zval_ptr_dtor(params + i);
        }
        efree(params);
    }

    zend_update_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ACTION), action TSRMLS_CC);

    zval *auto_display = tf_web_application_get_auto_display(tf_get_application(TSRMLS_CC) TSRMLS_CC);
    if (Z_BVAL_P(auto_display) && !tf_request_is_ajax(TSRMLS_DC)) {
        ZVAL_STRING(method, "display", 1);
        call_user_function(&controller_ce->function_table, &controller, method, ret, 0, NULL TSRMLS_CC);
    }

    zval_ptr_dtor(&ret);
    zval_ptr_dtor(&method);
}

void tf_controller_run_error(zval *controller, int error_type, char *error_msg, char *error_file, int error_lineno TSRMLS_DC) {
    zval *method, *error_type_pzval, *error_msg_pzval, *error_file_pzval, *error_lineno_pzval, *ret;
    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, "index", 1);
    MAKE_STD_ZVAL(error_type_pzval);
    ZVAL_LONG(error_type_pzval, error_type);
    MAKE_STD_ZVAL(error_msg_pzval);
    ZVAL_STRING(error_msg_pzval, error_msg, 1);
    MAKE_STD_ZVAL(error_file_pzval);
    ZVAL_STRING(error_file_pzval, error_file ? error_file : "-", 1);
    MAKE_STD_ZVAL(error_lineno_pzval);
    ZVAL_LONG(error_lineno_pzval, error_lineno);
    MAKE_STD_ZVAL(ret);
    zval *params[4] = {error_type_pzval, error_msg_pzval, error_file_pzval, error_lineno_pzval};
    zend_class_entry *controller_ce = Z_OBJCE_P(controller);
    if (call_user_function(&controller_ce->function_table, &controller, method, ret, 4, params TSRMLS_CC) != SUCCESS) {
        php_printf("call ErrorController->index error\n");
    }
    
    zval_ptr_dtor(&error_type_pzval);
    zval_ptr_dtor(&error_msg_pzval);
    zval_ptr_dtor(&error_file_pzval);
    zval_ptr_dtor(&error_lineno_pzval);
    TF_ZVAL_DTOR(method);
    TF_ZVAL_DTOR(ret);

    zend_update_property_string(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ACTION), "index" TSRMLS_CC);

    zval *auto_display = tf_web_application_get_auto_display(tf_get_application(TSRMLS_CC) TSRMLS_CC);
    if (Z_BVAL_P(auto_display) && !tf_request_is_ajax(TSRMLS_DC)) {
        ZVAL_STRING(method, "display", 1);
        call_user_function(&controller_ce->function_table, &controller, method, ret, 0, NULL TSRMLS_CC);
    }

    zval_ptr_dtor(&method);
    zval_ptr_dtor(&ret);
}

zval * tf_controller_render(zval *controller, char *tpl_name, zval *params TSRMLS_DC) {
    zval *action = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ACTION), 1 TSRMLS_CC);
    zval *view = tf_controller_init_view(controller TSRMLS_CC);
    
    return tf_view_render(view, tpl_name ? tpl_name : Z_STRVAL_P(action), params TSRMLS_CC);
}

void tf_controller_display(zval *controller, char *tpl_name, zval *params TSRMLS_DC) {
    zval *action = zend_read_property(tf_controller_ce, controller, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ACTION), 1 TSRMLS_CC);
    zval *view = tf_controller_init_view(controller TSRMLS_CC);
    
    tf_view_display(view, tpl_name ? tpl_name : Z_STRVAL_P(action), params TSRMLS_CC);
}

void tf_controller_ajax_out(int error_code, char *error_msg, int error_msg_len, zval *data) {
    zval *arr;
    MAKE_STD_ZVAL(arr);
    array_init(arr);
    add_assoc_long(arr, "errCode", error_code);
    add_assoc_stringl(arr, "errMsg", error_msg, error_msg_len, 1);
    if (!data) {
        MAKE_STD_ZVAL(data);
        object_init(data);
        add_assoc_zval(arr, "data", data);
    } else {
        add_assoc_zval(arr, "data", data);
        Z_ADDREF_P(data);
    }

    smart_str buf = {0};
    php_json_encode(&buf, arr, PHP_JSON_UNESCAPED_UNICODE TSRMLS_CC);
    smart_str_0(&buf);

    sapi_header_line ctr = {ZEND_STRL("Content-Type: application/json; charset=utf-8"), 200};
    sapi_header_op(SAPI_HEADER_REPLACE, &ctr TSRMLS_CC);
    
    php_write(buf.c, buf.len);

    efree(buf.c);
    zval_ptr_dtor(&arr);
}

PHP_METHOD(tf_controller, assign) {
    zval *view = tf_controller_init_view(getThis() TSRMLS_CC);
    uint argc = ZEND_NUM_ARGS();
    if (argc == 1) {
        zval *arr;
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &arr) == FAILURE) {
            return;
        }
        tf_view_assign_multi(view, arr TSRMLS_CC);
    } else if (argc == 2) {
        zval *value;
        char *name;
        uint name_len;
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &value) == FAILURE) {
            return;
        }

        tf_view_assign_single(view, name, name_len, value TSRMLS_CC);
    } else {
        WRONG_PARAM_COUNT;
    }
}

PHP_METHOD(tf_controller, render) {
    char *tpl_name = NULL;
    int tpl_name_len;
    zval *params = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sa", &tpl_name, &tpl_name_len, &params) != SUCCESS) {
        return;
    }

    zval *ret = tf_controller_render(getThis(), tpl_name, params TSRMLS_CC);
    if (ret) {
        RETURN_ZVAL(ret, 0, 1);
    }
}

PHP_METHOD(tf_controller, display) {
    char *tpl_name = NULL;
    int tpl_name_len;
    zval *params = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sa", &tpl_name, &tpl_name_len, &params) != SUCCESS) {
        return;
    }

    tf_controller_display(getThis(), tpl_name, params TSRMLS_CC);
}

PHP_METHOD(tf_controller, ajaxError) {
    char *error_msg = NULL;
    int error_msg_len;
    long error_code = 100;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &error_msg, &error_msg_len, &error_code) != SUCCESS) {
        return;
    }

    if (error_msg == NULL) {
        zval *error_msg_zval = tf_error_get_error_msg(tf_get_error(TSRMLS_CC) TSRMLS_CC);
        error_msg = Z_STRVAL_P(error_msg_zval);
        error_msg_len = Z_STRLEN_P(error_msg_zval);
    }

    tf_controller_ajax_out(error_code, error_msg, error_msg_len, NULL);

    // https://grokbase.com/t/php/php-internals/04727wymqn/zend-api-correct-way-to-terminate-entire-request-from-within-function
    zend_bailout();
}

PHP_METHOD(tf_controller, ajaxSuccess) {
    zval *data = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_DC, "|a", &data) != SUCCESS) {
        return;
    }

    tf_controller_ajax_out(0, "", 0, data);

    zend_bailout();
}

PHP_METHOD(tf_controller, error) {
    char *error_msg = NULL;
    int error_msg_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_DC, "|s", &error_msg, &error_msg_len) != SUCCESS) {
        return;
    }

    if (error_msg == NULL) {
        error_msg = Z_STRVAL_P(tf_error_get_error_msg(tf_get_error(TSRMLS_CC) TSRMLS_CC));
    }

    tf_web_application_run_error_controller(tf_get_application(TSRMLS_CC), E_USER_ERROR, error_msg, NULL, 0 TSRMLS_CC);

    zend_bailout();
}

PHP_METHOD(tf_controller, getView) {
    zval *view = tf_controller_init_view(getThis() TSRMLS_CC);
    RETURN_ZVAL(view, 1, 0);
}

zend_function_entry tf_controller_methods[] = {
    PHP_ME(tf_controller, assign, tf_controller_assign_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_controller, render, tf_controller_render_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_controller, display, tf_controller_display_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_controller, ajaxError, tf_controller_ajaxError_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_controller, ajaxSuccess, tf_controller_ajaxSuccess_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(tf_controller, error, tf_controller_error_arginfo, ZEND_ACC_PROTECTED)
    PHP_ME(tf_controller, getView, NULL, ZEND_ACC_PROTECTED)
    {NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(tf_controller) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "TF\\Controller", tf_controller_methods);
    tf_controller_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
    
    zend_declare_property_null(tf_controller_ce, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_REQUEST), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_controller_ce, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_controller_ce, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ROUTER), ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_string(tf_controller_ce, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_VIEW_EXT), "php", ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(tf_controller_ce, ZEND_STRL(TF_CONTROLLER_PROPERTY_NAME_ACTION), ZEND_ACC_PRIVATE TSRMLS_DC);

    zend_class_entry **reflection_method_tmp_ce;
    if (zend_hash_find(CG(class_table), ZEND_STRS("reflectionmethod"), (void **)&reflection_method_tmp_ce) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "fail to find ReflectionMethod class");
        return FAILURE;
    }
    reflection_method_ce = *reflection_method_tmp_ce;

    zend_class_entry **reflection_parameter_tmp_ce;
    if (zend_hash_find(CG(class_table), ZEND_STRS("reflectionparameter"), (void **)&reflection_parameter_tmp_ce) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "fail to find ReflectionParameter class");
        return FAILURE;
    }
    reflection_parameter_ce = *reflection_parameter_tmp_ce;

    return SUCCESS;
}
