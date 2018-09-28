# TF\\WebApplication

## 介绍
web应用类

## 方法

public function [\__construct](#__construct)(string configFile, string configSection = null)

public void function [run](#run)(void)

public void function [setAutoDisplay](#setautodisplay)(bool autoDisplay)

public TF\\Request function [getRequest](#getrequest)(void)

public TF\\Router function [getRouter](#getrouter)(void)

以下方法继承自TF\\Application

public TF\\Config function [getConfig](Application.md#getconfig)(void)

public TF\\DB function [getDB](Application.md#getdb)(void)

public TF\\Redis function [getRedis](Application.md#getredis)(void)

public TF\\Session function [getSession](Application.md#getsession)(void)

public TF\\Logger function [getLogger](Application.md#getlogger)(void)

## __construct
### 定义
    public function __construct(string configFile, string configSection = null)
### 参数
#### configFile
配置文件路径
#### configSection [optional]
选取的配置文件段，默认为全部
### 返回值
无

-----

## run
### 定义
    public void function run(void)
### 参数
无
### 返回值
无

-----

## setAutoDisplay
### 定义
    public void function setAutoDisplay(bool autoDisplay)
### 参数
#### autoDisplay
自动渲染并输出视图模板
### 返回值
无

-----

## getRequest
### 定义
    public TF\Request function getRequest(void)
### 参数
无
### 返回值
TF\Request实例

-----

## getRouter
### 定义
    public TF\\Router function getRouter(void)
### 参数
无
### 返回值
TF\Router实例
