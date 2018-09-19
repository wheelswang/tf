# TF\\View

## 介绍
视图类

## 方法
public void function [__construct](#__construct)(string tplDir, string tplExt = 'php')

public void function [assign](#assign)(mixed arg1, mixed arg2)

public string function [render](#render)(string tplName, array params = array())

public void function [display](#display)(string tplName, array params = array())

public mixed function [getVar](#getvar)(string name)

public array function [getVars](#getvars)(void)

public TF\Controller function [getController](#getcontroller)(void)

## __construct
### 定义
    public void function __construct(string tplDir, string tplExt = 'php')
### 参数
#### tplDir
模板目录
#### tplExt [optional]
模板文件名后缀
### 返回值
无

-----

## assign
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

## render
### 定义
    public string function render(string tplName, array params = array())
渲染模板
### 参数
#### tplName
模板名称
#### params [optional]
变量数组，如：array('param1' => 1, 'param2' => 2)
### 返回值
渲染后的模板字符串 模板文件不存在返回null

-----

## display
### 定义
    public void function display(string tplName, array params = array())
渲染模板并输出
### 参数
#### tplName
模板名称
#### params [optional]
变量数组，如：array('param1' => 1, 'param2' => 2)
### 返回值
无

-----

## getVar
### 定义
    public mixed function getVar(string name)
获取模板变量
### 参数
#### name
变量名
### 返回值
变量值 如不存在返回null

-----

## getVars
### 定义
    public array function getVars(void)
获取所有变量
### 参数
无
### 返回值
变量数组

-----

## getController
### 定义
    public TF\Controller function getController(void)
获取控制器实例
### 参数
无
### 返回值
控制器实例，不存在则返回null
