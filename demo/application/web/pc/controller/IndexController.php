<?php
namespace application\web\pc\controller;

use application\common\model\UserModel;
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

    public function add()
    {
        $user = new UserModel;
        $user->nickname = 'somebody';
        $user->insert();
    }

    public function myError()
    {
        $this->error('myError');
    }
}
