# 缓存
## 介绍
框架使用redis作为数据缓存，使用phpredis扩展作为连接客户端

## 配置
application.ini中作相关配置

    redis.server="127.0.0.1:6379"
    #可选配置
    redis.password=password
    redis.index=0
    redis.serialize=1
    redis.prefix=prefix

## 示例
    TF::getRedis()->get('key');
    TF::getRedis()->set('key', 'test');
支持phpredis所有函数，具体可以参考https://github.com/phpredis/phpredis
