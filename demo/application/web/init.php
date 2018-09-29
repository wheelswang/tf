<?php
// 全局初始化代码
if (strpos($_SERVER['HTTP_DOMAIN'], 'admin') === 0) {
    $this->getRouter()->setModule('admin');
}
