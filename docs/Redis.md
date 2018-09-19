# TF\Redis

# 介绍
redis类，支持自动重连

# 方法
public function __construct(string server, string password = '', int index = 0, bool serialize = false, string prefix = '')

内部使用了__call方法, 公共方法与PhpRedis一致，具体方法参见https://github.com/phpredis/phpredis

## __construct
### 定义
    public function __construct(string server, string password = '', int index = 0, bool serialize = false, string prefix = '')
### 参数
#### server
服务器地址 如："127.0.0.1:6379"
#### password [optional]
服务器密码
#### index [optional]
数据库编号
#### serialize [optional]
序列化
#### prefix [optional]
key前缀
