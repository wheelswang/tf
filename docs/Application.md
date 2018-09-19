# TF\\Application
## 介绍
应用抽象基类

## 方法
public function [\__construct](#__construct)(string configFile, string configSection = null)

public TF\\Config function [getConfig](#getconfig)(void)

public TF\\DB function [getDB](#getdb)(void)

public TF\\Redis function [getRedis](#getredis)(void)

public TF\\Session function [getSession](#getsession)(void)

public TF\\Logger function [getLogger](#getlogger)(void)

## <span id="__construct">__construct</span>
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

## getConfig
### 定义
    public TF\Config function getConfig()
### 参数
无
### 返回值
TF\\Config实例

-----

## getDB
### 定义
    public TF\DB function getDB()
### 参数
无
### 返回值
TF\\DB实例

-----

## getRedis
### 定义
    public TF\Redis function getRedis()
### 参数
无
### 返回值
TF\\Redis实例

-----

## getSession
### 定义
    public TF\Session function getSession()
### 参数
无
### 返回值
TF\\Session实例

-----

## getLogger
### 定义
    public TF\\Logger function getLogger()
### 参数
无
### 返回值
TF\\Logger实例
