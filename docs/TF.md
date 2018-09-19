# TF

## 介绍
封装一些常用的静态方法

## 方法
public static string function [getEnv](#getenv)(void)

public static TF\Application function [getApp](#getapp)(void)

public static TF\Config function [getConfig](#getconfig)(void)

public static int function [getErrorCode](#geterrorcode)(void)

public static string function [getErrorMsg](#geterrormsg)(void)

public static string function [getErrorDetail](#geterrordetail)(void)

public static void function [setError](#seterror)(int errorCode, string errorMsg = '', string errorDetail = '')

public static void function [setErrorMsg](#seterrormsg)(string errorMsg, string errorDetail = '')

public static TF\DB function [getDB](#getdb)(void)

public static TF\Redis function [getRedis](#getredis)(void)

public static TF\Session function [getSession](#getsession)(void)

public static void function [logInfo](#loginfo)(string msg)

public static void function [logWarn](#logwarn)(string msg);

public static void function [logError](#logerror)(string msg);

## <span id="getenv">getEnv</span>
### 定义
    public static string function getEnv()
### 参数
无
### 返回值
当前环境，取自php.ini中的tf.environ，默认为product

## <span id="getapp">getApp</span>
### 定义
    public static TF\Application function getApp();
### 参数
无
### 返回值
当前Application实例

## <span id="getconfig">getConfig</span>
### 定义
    public static TF\Config function getConfig();
TF::getApp()->getConfig的别名
### 参数
无
### 返回值
TF\Config实例

## <span id="geterrorcode">getErrorCode</span>
### 定义
    public static int function getErrorCode()
### 参数
无
### 返回值
错误码

## <span id="geterrormsg">getErrorMsg</span>
### 定义
    public static string function getErrorMsg()
### 参数
无
### 返回值
错误信息

## <span id="geterrordetail">getErrorDetail</span>
### 定义
    public static string function getErrorDetail()
### 参数
无
### 返回值
错误详情

## <span id="seterror">setError</span>
### 定义
    public static void function setError(int errorCode, string errorMsg = '', string errorDetail = '')
### 参数
#### errorCode
错误码
#### errorMsg [optional]
错误信息
#### errorDetail [optional]
错误详情
### 返回值
无

## <span id="seterrormsg">setErrorMsg</span>
### 定义
    public static void function setErrorMsg(string errorMsg, string errorDetail = '')
### 参数
#### errorMsg
错误信息
#### errorDetail [optional]
错误详情
### 返回值
无

## <span id="getdb">getDB</span>
### 定义
    public static TF\DB function getDB()
此函数是TF::getApp()->getDB的别名
### 参数
无
### 返回值
TF\DB实例

## <span id="getredis">getRedis</span>
### 定义
    public static TF\Redis function getRedis()
此函数是TF::getApp()->getRedis的别名
### 参数
无
### 返回值
TF\Redis实例

## <span id="getsession">getSession</span>
### 定义
    public static TF\Session function getSession()
此函数是TF::getApp()->getSession的别名
### 参数
无
### 返回值
TF\Session实例

## <span id="loginfo">logInfo</span>
### 定义
    public static void function logInfo(string msg)
此函数是TF::getApp()->getLogger()->info的别名
### 参数
#### msg
内容
### 返回值
无

## <span id="logwarn">logWarn</span>
### 定义
    public static void function logWarn(string msg)
此函数是TF::getApp()->getLogger()->warn的别名
### 参数
#### msg
内容
### 返回值
无

## <span id="logerror">logError</span>
### 定义
    public static void function logError(string msg)
此函数是TF::getApp()->getLogger()->error的别名
### 参数
#### msg
内容
### 返回值
无
