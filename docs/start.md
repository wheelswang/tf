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
|  文件    | 说明     |
| :------------- | :------------- |
| application\\common       | - |
| application\\common\\library | 公共类库 |
| application\\common\\model | 公共model |
| application\\console | 命令行脚本 |
| application\\console\\run | 命令行执行程序 |
| application\\web | web模块目录 |
| application\\web\\init.php | 模块初始化执行脚本 |
| application\\web\\controller | 控制器目录 |
| application\\web\\view | 视图目录 |
| application\\log\\web | web日志目录 |
| application\\log\\console | 命令行日志目录 |
| application\\config | 配置文件目录 |
| application\\config\\application.ini | 配置文件 |
| application\\public\\index.php | web入口文件 |


## 多模块项目结构
|  文件    | 说明     |
| :------------- | :------------- |
| application\\common       | - |
| application\\common\\library | 公共类库 |
| application\\common\\model | 公共model |
| application\\console | 命令行脚本 |
| application\\console\\run | 命令行执行程序 |
| application\\web | web模块目录 |
| application\\web\\模块1\\init.php | 模块1初始化执行脚本 |
| application\\web\\模块1\\controller | 模块1控制器目录 |
| application\\web\\模块1\\view | 模块1视图目录 |
| application\\web\\模块2\\init.php | 模块2初始化执行脚本 |
| application\\web\\模块2\\controller | 模块2控制器目录 |
| application\\web\\模块2\\view | 模块2视图目录 |
| application\\log\\web | web日志目录 |
| application\\log\\console | 命令行日志目录 |
| application\\config | 配置文件目录 |
| application\\config\\application.ini | 配置文件 |
| application\\public\\index.php | web入口文件 |

## 配置文件
application\\conf\\application.ini

    [dev]
    root=ROOT_PATH
    module.default="www"
    module.availables="www,pay"
    #类加载目录（可选）
    loader.paths.0=ROOT_PATH "/common/library"
    #uri重定向（可选）
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
    $app = new TF\\WebApplication(ROOT_PATH . '/config/application.ini', TF::getEnv());
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
