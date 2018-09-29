<?php
// global init
if (strpos($_SERVER['HTTP_DOMAIN'], 'admin') === 0) {
    $this->getRouter()->setModule('admin');
}
