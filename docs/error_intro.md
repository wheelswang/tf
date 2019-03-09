# 错误处理
## 介绍
触发ErrorController的错误：
 * E_ERROR类型
 * E_USER_ERROR类型
 * 调用controller->error函数

如果没有定义ErrorController，则会直接输出errMsg

## 示例
    <?php

    namespace application\web\controller;

    use TF;

    class ErrorController extends TF\Controller
    {
        public function index($errType, $errMsg, $errFile, $errLineNo)
        {
            if (TF::getApp()->getRequest()->isAjax()) {
                $this->ajaxError($errMsg);
            } else {
                $this->assign(array(
                    'errMsg' => $errMsg
                ));
            }
        }
    }
