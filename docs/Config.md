# TF\\Config

## 方法

public function [__construct](#__construct)(string file, string section = null)

public mixed function [get](#get)(string name)

public array function [getAll](#getall)(void)

## <span id="__construct">__construct</span>
### 定义
    public function [__construct](#__construct)(string file, string section = null)
### 参数
#### file
ini配置文件路径
#### section [optional]
使用的配置文件段的名称，默认为全部
### 返回值
无

## <span id="get">get</span>
### 定义
    public mixed function get(string name)
### 参数
#### name
名称
#### 返回值
name对应的配置数据，如不存在返回null

## <span id="getall">getAll</span>
### 定义
    public array function getAll()
### 参数
无
### 返回值
配置数组
