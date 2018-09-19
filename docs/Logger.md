# TF\\Logger

## 简介
日志类

## 方法
public function [__construct](#__construct)(string path, int maxSize = 50 x 1024 x 1024)

public void function [setPath](#setpath)(string path)

public void function [info](#info)(string msg)

public void function [warn](#info)(string msg)

public void function [error](#error)(string msg)

## __construct
### 定义
    public function __construct(string path, int maxSize = 50 x 1024 x 1024)
### 参数
#### path
日志目录
#### maxSize
日志文件大小
### 返回值
无

-----

## setPath
### 定义
    public void function setPath(string path)
### 参数
#### path
日志目录
### 返回值
无

-----

## info
### 定义
    public void function info(string msg)
### 参数
#### msg
内容
### 返回值
无

-----

## warn
### 定义
    public void function warn(string msg)
### 参数
#### msg
内容
### 返回值
无

-----

## error
### 定义
    public void function error(string msg)
### 参数
#### msg
内容
### 返回值
无
