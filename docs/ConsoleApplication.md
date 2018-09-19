# TF\\ConsoleApplication

## 介绍
控制台应用类

## 父类
TF\\Application

## 方法

public function [\__construct](#__construct)(string configFile, string configSection = null)

public void function [run](#run)(void)

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
