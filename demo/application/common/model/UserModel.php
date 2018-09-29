<?php

namespace application\common\model;

class UserModel extends \TF\DBModel
{
    protected static $_table = 'user';

    protected static $_fields = array(
        'id' => self::INT,
        'nickname' => self::STRING,
    );

}
