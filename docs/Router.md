# TF\\Router

## 介绍
路由类，控制请求对应的module、controller和action

## 方法
public function [__construct](#__construct)(string url)

public string function [getModule](#getmodule)(void)

public string function [getController](#getcontroller)(void)

public string function [getAction](#getaction)(void)

public string function [getParam](#getparam)(string name)

public array function [getParams](#getparams)(void)

public bool function [addRule](#addrule)(string search, string replace)

public bool function [addRules](#addrules)(array rules)

public array function [getRules](#getrules)(void)

## <span id="__construct">__construct</span>
### 定义
    public function __construct(string url)
### 参数
#### url
链接地址
### 返回值
无

-----

## <span id="getmodule">getModule</span>
### 定义
    public string function getModule(void)
### 参数
无
### 返回值
模块名 不存在模块则返回null

-----

## <span id="getcontroller">getController</span>
### 定义
    public string function getController(void)
### 参数
无
### 返回值
控制器名称，默认控制器为index

-----

## <span id="getaction">getAction</span>
### 定义
    public string function getAction(void)
### 参数
无
### 返回值
方法名，默认方法为index

-----

## <span id="getparam">getparam</span>
### 定义
    public string function getParam(string name)
获取uri携带的参数，注意不是$_GET参数，而是由路由解析后得到的参数 如：
/controller/action/param1/value1/param2/value2，这里可以得到param1和param2两个参数
### 参数
#### name
参数名
### 返回值
参数值，不存在则返回null

-----

## <span id="getparams">getParams</span>
### 定义
    public array function getParams(void)
### 参数
无
### 返回值
全部参数数组

-----

## <span id="addrule">addRule</span>
### 定义
    public bool function addRule(string search, string replace)
### 参数
#### search
用来匹配url的正则表达式，如：/\/room\/(\d+)\/i
#### replace
替换字符串，如：/room/index/id/$1
#### 返回值
成功true 失败false

-----

## <span id="addrules">addRules</span>
### 定义
    public bool function addRules(array rules)
用来添加一组规则
### 参数
#### rules
key=>value形式的规则 如：array('/\/room\/(\d+)\/i' => '/room/index/id/$1')
#### 返回值
成功true 失败false

-----

## <span id="getrules">getRules</span>
### 定义
    public array function getRules(void)
### 参数
无
### 返回值
规则数组
