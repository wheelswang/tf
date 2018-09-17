# TF\\DB
## 介绍
数据库类，封装常用数据库操作，支持事务嵌套、主从配置
## 函数列表
public [__construct](#__construct)(string server, string user, string password, string dbname, string charset = "utf8", string persistent = 0, string slaveConfigs = null);

public array [query](#query)(string table, mixed condition = null, array params, mixed fields = "\*", bool forUpdate = false);

public array [queryAll](#queryAll)(string table, mixed condition = null, array params = null, mixed fields = "\*", bool forUpdate = false);

public bool [update](#update)(string table, array data, mixed condition, array params = null);

public bool [insert](#insert)(string table, array data);

public bool [delete](#delete)(string table, mixed condition, array params = null);

public int [count](#count)(string table, mixed condition = null, array params = null);

public mixed [execSql](#execSql)(string sql, array params = null);

public int [getRowCount](#getRowCount)(void);

public string [getLastInsertId](#getLastInsertId)(void);

public bool [begin](#begin)(void);

public bool [commit](#commit)(void);

public bool [rollback](#rollback)(void);

public int [getTransactionLevel](#getTransactionLevel)(void);

public void [close](#close)(void);


## <span id="__construct">__construct</span>
### 定义
    public void __construct(string server, string user, string password, string dbname, string charset = "utf8", string persistent = 0, string slaveConfigs = null);
### 参数
#### server
数据库地址， 如："127.0.0.1:3306"
### user
数据库用户名
### password
数据库密码
### dbname
数据库名称
### charset [optional]
编码，默认utf8
### persistent [optional]
持久链接，默认0
### slaveConfigs [optional]
从库配置，如:

    array(
        array(
            'server' => '127.0.0.1:3306',
            'user' => 'readonly',
            'password' => 'readonly',
            'dbname' => 'test',
            'charset' => 'utf8',
            'persistent' => 0,
        ),
        ...
    )
### 返回值
无

## <span id="query">query</span>
### 定义
    public array query(string table, mixed condition = null, array params = null, mixed fields = "*", bool forUpdate = false)
### 参数
#### table
表名
#### condition [optional]
查询条件，如："id=:id" 或者 array("id" => 1)
#### params [optional]
绑定参数，如：array(":id" => 1)
#### fields [optional]
查询字段，默认为"*"，如："\`field1\`,\`field2\`" 或者 array("field1", "field2")
#### forUpdate [optional]
行锁，默认为false，使用时必须先手动开始事务
### 返回值
查询结果数组，结果为空时返回null， 出错返回false

## <span id="queryAll">queryAll</span>
### 定义
    public array queryAll(string table, mixed condition = null, array params = null, mixed fields = "*", bool forUpdate = false);
### 参数
#### table
表名
#### condition [optional]
查询条件，如："id=:id" 或者 array("id" => 1)
#### params [optional]
绑定参数，如：array(":id" => 1)
#### fields [optional]
查询字段，默认为"*"，如："\`field1\`,\`field2\`" 或者 array("field1", "field2")
#### forUpdate [optional]
行锁，默认为false，使用时必须先手动开始事务
### 返回值
查询结果数组, 结果为空时返回空数组，出错返回false

## <span id="update">update</h2>
### 定义
    public bool update(string table, array data, mixed condition, array params = null)
### 参数
#### table
表名
#### data
数据，如：array('field1' => 1, 'field2' => 2)
#### condition
更新条件 如："id=:id" 或者 array("id" => 1) <br/>
安全考虑，此参数为必传参数
#### params [optional]
绑定参数，如：array(":id" => 1)
### 返回值
成功true 失败false

## <span id="insert">insert</span>
### 定义
    public bool insert(string table, array data)
### 参数
#### table
表名
#### data
数据，如：array('field1' => 1, 'field2' => 2)
### 返回值
成功true 失败false

## <span id="delete">delete</span>
### 定义
    public bool delete(string table, mixed condition, array params = null)
### 参数
#### table
表名
#### condition
删除条件 如："id=:id" 或者 array("id" => 1) <br/>
安全考虑，此参数为必传参数
#### params [optional]
绑定参数，如：array(":id" => 1)
### 返回值
成功true 失败false

## <span id="count">count</span>
### 定义
    public int count(string table, mixed condition = null, array params = null)
### 参数
#### table
表名
#### condition [optional]
查询条件 如："id=:id" 或者 array("id" => 1)
#### params [optional]
绑定参数，如：array(":id" => 1)
### 返回值
查询结果数量，或者失败false

## <span id="execSql">execSql</span>
### 定义
    public mixed execSql(string sql, array params = null)
### 参数
#### sql
SQL语句
#### params [optional]
绑定参数，如：array(":id" => 1)
### 返回值
查询结果数组，或者操作布尔结果

## <span id="getRowCount">getRowCount</span>
### 定义
    public int getRowCount(void)
### 参数
无
### 返回值
update或者delete影响的行数

## <span id="getLastInsertId">getLastInsertId</span>
### 定义
    public string getLastInsertId(void)
### 参数
无
### 返回值
上一次insert产生的主键Id

## <span id="begin">begin</span>
### 定义
    public bool begin(void)
开启事务
### 参数
无
### 返回值
成功true 失败false

## <span id="commit">commit</span>
### 定义
    public bool commit(void)
提交事务
### 参数
无
### 返回值
成功true 失败false

## <span id="rollback">rollback</span>
### 定义
    public bool rollback(void)
回滚事务
### 参数
无
### 返回值
成功true 失败false

## <span id="getTransactionLevel">getTransactionLevel</span>
### 定义
    public int getTransactionLevel(void)
获取事务嵌套等级
### 参数
无
### 返回值
事务嵌套等级

## <span id="close">close</span>
### 定义
    public void close(void)
关闭数据库连接
### 参数
无
### 返回值
无
