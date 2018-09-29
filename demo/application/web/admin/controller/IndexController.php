<?php

namespace application\web\admin\controller;

use TF;

class IndexController extends TF\Controller
{
    public function __construct()
    {

    }

    public function index($id)
    {
        $this->assign(array('id' => $id));
    }
}
