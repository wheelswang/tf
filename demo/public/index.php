<?php

define('ROOT_PATH', __DIR__ . '/..');

$app = new TF\WebApplication(ROOT_PATH . '/config/application.ini', TF::getEnv());
$app->run();
