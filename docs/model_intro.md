# 模型
## 示例
    <?php
    namespace application\common\model;
    use TF;

    class UserModel extends TF\DBModel
    {
        protected $_table = 'user';

        protected $_feilds = array(
            'id' => self::INT,
            'username' => self::STRING,
            'avatar' => array(self::STRING, 'avatar.jpg')
        );

        protected $_pk = 'id';
    }

## 字段
fields属性用于声明字段类型和默认值，支持四种类型如下：
 * STRING 字符串
 * INT 数字
 * BOOL 布尔值
 * DOUBLE 浮点型数字

## 查询
    UserModel::query(array('id' => 1));
    UserModel::query('id=:id', array(':id' => 1));
    UserModel::queryByPk(1);
    UserModel::queryAll(array('id' => 1));

## 复杂查询
    UserModel::query('id>:id or (id=10 and username=:username)', array(':id' => 1, 'nickname' => 'xx'));

## 查询指定字段
    UserModel::query(array('id' => 1), null, array('id', 'nickname'));
    UserModel::queryByPk(1, array('id', 'nickname'));
    UserModel::queryAll(array('id' => 1), null, array('id', 'nickname'));

## 更新
    $user = UserModel::queryByPk(1);
    $user->nickname = 'somebody';
    $user->update(array('nickname'));

    //other ways
    UserModel::update(array('nickname' => 'somebody'), array('id' => 1));
    UserModel::update(array('nickname' => 'somebody'), 'id=:id', array(':id' => 1));
    UserModel::updateByPk(1, array('nickname' => 'somebody'));

## 插入
    $user = new UserModel;
    $user->username = 'somebody';
    $user->insert();

## 删除
    $user = UserModel::queryByPk(1);
    $user->delete();

## 行锁
    TF::getDB()->begin();
    UserModel::query(array('id' => 1), null, null, true);
    UserModel::queryByPk(1, null, true);
    UserModel::queryAll(array('id' => 1), null, null, true);
