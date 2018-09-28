# 数据库
## 配置
application.ini中作相关配置

    db.server="127.0.0.1:3306"
    db.user="user"
    db.password="password"
    db.dbname="dbname"
    db.charset="utf8mb4"
    db.persistent=0

    #从库0(可选)
    db.slaves.0.server="127.0.0.1:3306"
    db.slaves.0.user="user"
    db.slaves.0.password="password"
    db.slaves.0.dbname="dbname"
    db.slaves.0.charset="utf8mb4"
    db.slaves.0.persistent=0
    #从库1(可选)
    db.slaves.1.server="127.0.0.1:3306"
    db.slaves.1.user="user"
    db.slaves.1.password="password"
    db.slaves.1.dbname="dbname"
    db.slaves.1.charset="utf8mb4"
    db.slaves.1.persistent=0

## 主从
如果配置了从库，框架自动对select语句（不带for update）使用从库，其余使用主库

## 查询
    TF::getDB()->query('user', array('id' => 1));
    TF::getDB()->query('user', 'id=:id', array(':id' => 1));
    TF::getDB()->queryAll('user');

## 查询指定字段
    TF::getDB()->query('user', array('id' => 1), null, array('id', 'nickname'));
    TF::getDB()->query('user', 'id=:id', array(':id' => 1), array('id', 'nickname'));
    TF::getDB()->queryAll('user', array('id' => 1), null, array('id', 'nickname'));

## 更新
    TF::getDB()->update('user', array('nickname' => 'somebody'), array('id' => 1));
    TF::getDB()->update('user', array('nickname' => 'somebody'), 'id=:id', array(':id' => 1));

## 插入
    TF::getDB()->insert('user', array('nickname' => 'somebody', 'avatar' => 'avatar.jpg'));

## 删除
    TF::getDB()->delete('user', array('id' => 1));
    TF::getDB()->delete('user', 'id=:id', array(':id' => 1));

## 行锁
    TF::getDB()->begin();
    TF::getDB()->query('user', array('id' => 1), null, null, true);
    TF::getDB()->query('user', 'id=:id', array(':id' => 1), null, true);
    TF::getDB()->queryAll('user', null, null, null, true);

## 强制使用主库
在某些实时性要求高的场景下业务可能需要查询主库数据

    TF::getDB()->setSlaveEnabled(false);
