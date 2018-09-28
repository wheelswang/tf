# 路由
## 配置
可在application.ini中作相关配置

    router.rules.0.search="/(service\.domain\.com)/i"
    router.rules.0.replace="$1/service"
使用正则表达式替换，等同于preg_replace  
替换对象是当前url，框架解析url规则为:  
 * 如果模块存在并且第一项是有效的模块：/模块/控制器/方法/参数1/参数1值/参数2/参数2值  
 * 否则：/控制器/方法/参数1/参数1值/参数2/参数2值

## 代码中进行设置
可以在init.php初始化脚本中改变路由

    <?php
    $this->getRouter()->setController('index');
    $this->getRouter()->setAction('index');
