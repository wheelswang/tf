<?php

namespace application\web\pc\controller;

use TF;

class ErrorController extends TF\Controller
{
    public function index($errType, $errMsg, $errFile, $errLineNo)
    {
        TF::logError('[' . $errType . ']' . $errMsg . ' in ' . $errFile . ':' . $errLineNo);
        if (TF::getApp()->getRequest()->isAjax()) {
            $this->ajaxError($errMsg);
        } else {
            $this->assign(array('errMsg' => $errMsg));
        }
    }
}
