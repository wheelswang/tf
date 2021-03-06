# TF\\DBModel
## 介绍
数据库表模型基类

## 父类
TF\\Model

## 属性
protected [$\_pk](#_pk) = 'id'

protected [$\_table](#_table) = null
### 以下继承自TF\Model
const INT = 1

const DOUBLE = 2

const STRING = 3

const BOOL = 4

protected [$\_fields](Model.md#_fields) = null

## 方法
public static model [query](#query)(mixed condition = null, array params = null, mixed fields = null, bool forUpdate = false)

public static model [queryByPk](#querybypk)(string id, mixed fields = null, bool forUpdate = false)

public static array [queryAll](#queryall)(mixed condition = null, array params = null, mixed fields = null, bool forUpdate = false)

public bool [update](#update)(array fields = null)

public static bool [updateByPk](#updatebypk)(string id, array data)

public bool [insert](#insert)(void)

public bool [delete](#delete)(void)

public static bool [deleteByPk](#deletebypk)(string id)

public static int [count](#count)(mixed condition = null, array params = null)

## $\_pk
主键名称，默认为id


## $\_table
表名

## query
### 定义
    public model query(mixed condition = null, array params = null, mixed fields = null, bool forUpdate = false)
查询单条结果
### 参数
#### condition [optional]
查询条件，如"id=:id" 或者 array("id" => 1)
#### params [optional]
绑定参数，如array(':id' => 1)
#### fields [optional]
查询字段，默认为属性$\_fields定义的所有字段
#### forUpdate [optional]
行锁，默认为false，使用时必须先手动开始事务
### 返回值
模型实例，空结果返回null，出错返回false

-----

## queryByPk
### 定义
    public model queryByPk(string id, mixed fields = null, bool forUpdate)
根据主键查询记录
### 参数
#### id
主键
#### fields [optional]
查询字段，默认为属性$\_fields定义的所有字段
#### forUpdate [optional]
行锁 默认false
### 返回值
模型实例，空结果返回null，出错返回false

-----

## queryAll
### 定义
    public model queryAll(mixed condition = null, array params = null, mixed fields = null, bool forUpdate = false)
查询所有结果
### 参数
#### condition [optional]
查询条件，如"id=:id" 或者 array("id" => 1)
#### params [optional]
绑定参数，如array(':id' => 1)
#### fields [optional]
查询字段，默认为属性$\_fields定义的所有字段
#### forUpdate [optional]
行锁 默认false
### 返回值
模型实例数组，出错返回false

-----

## update
### 定义
    public bool update(array fields = null)
### 参数
#### fields [optional]
需要更新的字段数组，如不传则更新全部字段
### 返回值
成功true 失败false

-----

## updateByPk
### 定义
    public static bool updateByPk(string id, array data)
### 参数
#### id
主键id
#### data
需要更新的数据 如： array('field1' => 1, 'field2' => 2)
### 返回值
成功true 失败false

-----

## insert
### 定义
    public bool insert(void)
### 参数
无
### 返回值
成功true 失败false

-----

## delete
### 定义
    public bool delete(void)
### 参数
无
### 返回值
成功true 失败false

-----

## deleteByPk
### 定义
    public static bool deleteByPk(string id)
### 参数
#### id
主键id
### 返回值
成功true 失败false

-----

## count
### 定义
    public static int count(mixed condition = null, array params = null)
### 参数
#### condition [optional]
查询条件 如：'id=:id' 或者 array('id' => 1)
#### params [optional]
绑定参数 如：array(':id' => 1)
### 返回值
记录数 出错返回false
