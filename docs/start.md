# 快速开始

## 安装
依赖扩展
* pdo-mysql
* redis

版本要求：PHP 5.4+

## 扩展引入
    extension=tf.so
    #环境配置 默认product
    #tf.environ=product

## 项目结构
* application\\common
* application\\common\\library&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;公共类库
* application\\common\\model&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;公共model
* application\\console&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;命令行脚本
* application\\console\\run&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;命令行执行程序
* application\\web  
* application\\web\\init.php&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;模块初始化执行脚本
* application\\web\\controller&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;控制器
* application\\web\\view&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;视图
* application\\log\\web&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;web日志
* application\\log\\console&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;命令行日志
* application\\conf
* application\\conf\\application.ini&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;配置文件
* application\\public\\index.php&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;web入口文件

## 多模块项目结构
* application\\common
* application\\common\\library
* application\\common\\model
* application\\console
* application\\web
* application\\web\\模块1\init.php
* application\\web\\模块1\controller
* application\\web\\模块1\view
* application\\web\\模块2\init.php
* application\\web\\模块2\controller
* application\\web\\模块2\view
* application\\log\\web
* application\\log\console
* application\\conf
* application\\conf\\application.ini
* application\\public\\index.php

## 配置文件
application\\conf\\application.ini

    [dev]
    root=ROOT_PATH
    module.default="www"
    module.availables="www,pay"
    #uri重定向(可选)
    router.rules.0.search="/\/room\/(\d+)/i"
    router.rules.0.replace="/room/index/id/$1"

    #数据库
    db.server="127.0.0.1:3306"
    db.user="user"
    db.password="password"
    db.dbname="dbname"
    db.charset="utf8mb4"
    db.persistent=0

    #从库0(可选)
    db.slaves.0.server="127.0.0.1:3306"
    db.slaves.0.user="user"
    db.slaves.0.password="password"
    db.slaves.0.dbname="dbname"
    db.slaves.0.charset="utf8mb4"
    db.slaves.0.persistent=0
    #从库1(可选)
    db.slaves.1.server="127.0.0.1:3306"
    db.slaves.1.user="user"
    db.slaves.1.password="password"
    db.slaves.1.dbname="dbname"
    db.slaves.1.charset="utf8mb4"
    db.slaves.1.persistent=0

    #redis配置
    redis.server="127.0.0.1:6379"
    redis.index=0

    #session存入redis缓存
    session.use=1
    session.server="127.0.0.1:6379"
    session.index=1

    #继承配置
    [product:dev]

## 入口文件
application\\public\\index.php

    <?php
    define('ROOT_PATH', __DIR__ . '/..');
    $app = new TF\\WebApplication(ROOT_PATH . '/conf/application.ini', TF::getEnv());
    $app->run();


## 控制器
    <?php
    namespace IndexController extends TF\\Controller
    {
        public function index($arg1, $arg2, $arg3)
        {
            $this->assign(array(
                'arg1' => $arg1,
                'arg2' => $arg2,
                'arg3' => $arg3
            ));
        }
    }
框架会自动加载视图view/index/index.php，并且输出<br/>
可通过调用函数TF::getApp()->setAutoDisplay(false)关闭自动输出<br/>
Ajax请求默认关闭自动输出

## 运行
    http://yourdomain.com/?arg1=1&arg2=2&arg3=3
    http://yourdomain.com/index/index/arg1/1/arg2/2/arg3/3
