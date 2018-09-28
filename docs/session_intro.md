# 会话
## 介绍
session数据写入redis，使用PHP内置session

## 配置
application.ini中作相关配置

    session.use=1
    session.server="127.0.0.1:6379"
    #可选配置
    session.password=password
    session.index=0
    session.serialize=1
    session.prefix=prefix

## 示例
    TF::getSession()->get('id');
    TF::getSession()->set('id', 1);
    TF::getSession()->del('id');

    //another way
    $_SESSION['id'];
    unset($_SSSSION['id']);

    session_set_cookie_params(365 * 86400);
    ini_set('session.gc_maxlifetime', 7 * 86400);
