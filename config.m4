PHP_ARG_ENABLE(tf,
    [Whether to enable the "tf" extension],
    [  enable-tf        Enable "tf" extension support])

if test $PHP_TF != "no"; then
    PHP_SUBST(TF_SHARED_LIBADD)
    PHP_NEW_EXTENSION(tf, tf_common.c tf.c tf_application.c tf_console_application.c tf_web_application.c tf_request.c tf_router.c tf_config.c tf_loader.c tf_controller.c tf_view.c tf_model.c tf_error.c tf_db.c tf_db_model.c tf_redis.c tf_session_handler.c tf_session.c tf_logger.c, $ext_shared)
fi
