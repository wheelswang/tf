# TF\\Controller

## 介绍
控制器基类

## 方法
public TF\\Request function [getRequest](#getrequest)(void)

public TF\\Router function [getRouter](#getrouter)(void)

public void function [assign](#assign)(mixed arg1, mixed arg2)

public string function [render](#render)(string tplName = null, array params = array())

public void function [display](#display)(string tplName = null, array params = array())

public void function [ajaxError](#ajaxerror)(string errMsg, int errCode = 100)

public void function [ajaxSuccess](#ajaxsuccess)(array data = {})

public void function [error](#error)(string errMsg)

## <span id="getrequest">getRequest<span>
### 定义
    public TF\Request function getRequest(void)
### 参数
无
### 返回值
TF\Request实例

-----

## <span id="getrouter">getRouter</span>
### 定义
    public TF\Router function getRouter(void)
### 参数
无
### 返回值
TF\Router实例

-----

## <span id="assign">assign</span>
### 定义
    public void function assign(mixed arg1, mixed arg2)
此函数支持两种传参方式
### 参数
#### arg1
只传arg1，arg1必须为数组形式，如：array('param1' => 1, 'param2' => 2)，否则arg1为变量名
#### arg2
变量值
### 返回值
无

-----

## <span id="render">render</span>
### 定义
    public string function render(string tplName = null, array params = array())
### 参数
#### tplName [optional]
模板名 默认为方法名
#### params [optional]
变量数组
### 返回值
渲染后的视图字符串，模板文件不存在返回null

-----

## <span id="display">display</span>
### 定义
    public void function display(string tplName = null, array params = array())
输出渲染后的模板
### 参数
#### tplName [optional]
模板名 默认为方法名
#### params [optional]
变量数组
### 返回值
无

-----

## <span id="ajaxerror">ajaxError</span>
### 定义
    public void function ajaxError(string errMsg, int errCode = 100)
输出错误，格式为{"errCode": 100, "errMsg": "", "data": {}}
### 参数
#### errMsg
错误提示信息
#### errCode [optional]
错误码
### 返回值
无

-----

## <span id="ajaxsuccess">ajaxSuccess</span>
### 定义
    public void function ajaxSuccess(array data = {})
输出成功信息，格式为{"errCode": 0, "errMsg": "", "data": {}}
### 参数
#### data
需要返回的数组
### 返回值
无

-----

## <span id="error">error</span>
### 定义
    public void function error(string errMsg)
函数会初始化ErrorController，并且调用其index方法，
如果ErrorController不存在则直接输出data变量
### 参数
#### errMsg
错误信息
### 返回值
无
