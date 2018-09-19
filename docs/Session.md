# TF\Session

# 介绍
会话类

# 方法
public void function [__construct](#__construct)(array config)

public void function [start](#start)(void)

public mixed function [get](#get)(string name)

public void function [set](#set)(string name, mixed value)

public void function [del](#del)(string name)

public string function [getSessionId](#getsessionid)(void)

public void function [setSessionId](#setsessionid)(string sessionId)

public void function [destroy](#destroy)(void)

## __construct
### 定义
    public void function __construct(array config)
### 参数
#### config
redis配置，如：array('server' => '127.0.0.1:6379', 'index' => 1)
### 返回值
无

-----

## start
### 定义
    public void function start(void)
开始会话
### 参数
无
### 返回值
无

-----

## get
### 定义
    public mixed function get(string name)
### 参数
#### name
名称
### 返回值
变量值，不存在则返回null

-----

## set
### 定义
    public void function set(string name, mixed value)
### 参数
#### name
名称
#### value
变量值
### 返回值
无

-----

## del
### 定义
    public void function del(string name)
### 参数
#### name
名称
### 返回值
无

-----

## getSessionId
### 定义
    public string function getSessionId(void)
### 参数
无
### 返回值
会话ID

-----

## setSessionId
### 定义
    public void function setSessionId(string sessionId)
### 参数
#### sessionId
会话ID
### 返回值
无

-----

## destroy
### 定义
    public void function destroy(void)
销毁会话
### 参数
无
### 返回值
无
