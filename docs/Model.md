# TF\\Model
## 介绍
模型基类

## 属性
const INT = 1

const DOUBLE = 2

const STRING = 3

const BOOL = 4

protected [$\_fields](#_fields) = null

## 方法
public [__construct](#__construct)(void)

public array [toArray](#toarray)(bool camelCase = false)


## $\_fields
字段定义，如：

    array(
        'field1' => self::INT,
        'field2' => array(self::STRING, "default");
    );

## __construct
### 定义
    public __construct(void)
### 参数
无
### 返回值
无

-----

## toArray
### 定义
    public array toArray(bool camelCase = false)
### 参数
#### camelCase
使用驼峰命名法，默认为false
### 返回值
数组
