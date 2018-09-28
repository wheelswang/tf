# 控制器
## 示例
    <?php
    namespace application/web/controller;
    use TF;

    class IndexController extends TF/controller
    {
        public function __construct()
        {

        }

        public function index($arg1, $arg2 = null)
        {

        }
    }

## 构造函数
构造函数中适合一些初始化操作，如权限控制、登录态检测

## 方法
客户端请求会调用控制器中相应的public方法，函数名为路由解析后的action

## 参数
参数为客户端请求参数，变量名与参数名相同  
没有默认值的参数为必选参数，如果请求中没有携带，框架会执行错误处理程序  
参数的获取顺序如下
 * 路由参数
 * GET
 * POST
 * FILES

## 输出
对于非ajax请求，框架会自动调用render函数输出渲染后的view, view的名称与函数名一致  
对于ajax请求，建议调用[ajaxSuccess](#Controller.md#ajaxsuccess)或者[ajaxError](#Controller.md#ajaxerror)方法输出  
错误输出调用内置的[error](#Controller.md#error)函数，如果定义了ErrorController，控制器内置的error函数会自动调用错误处理。否则直接输出
