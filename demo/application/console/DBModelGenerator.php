<?php

namespace application\console;

use \TF;

/**
 * useage:
 * ./run ModelGenerator tableName
 */
class DBModelGenerator
{
    public static function run($args)
    {
        $table = $args[0];
        if (!$table) {
            return;
        }

        $classname = '';
        $arr = explode('_', $table);
        foreach ($arr as $item) {
            $item[0] = strtoupper($item[0]);
            $classname .= $item;
        }
        $classname .= 'Model';

        $fields = '';
        $pk = '';
        $rows = TF::getDB()->execSql('desc ' . $table);
        foreach ($rows as $row) {
            if (strpos($row['Type'], 'int') === 0 ||
                strpos($row['Type'], 'tinyint') === 0 ||
                strpos($row['Type'], 'smallint') === 0 ||
                strpos($row['Type'], 'mediumint') === 0 ||
                strpos($row['Type'], 'bigint') === 0) {
                $type = 'self::INT';
                if ($row['Default']) {
                    $row['Default'] = (int) $row['Default'];
                }
            } else if (strpos($row['Type'], 'boolean') === 0) {
                $type = 'self::BOOL';
                if ($row['Default']) {
                    $row['Default'] = $row['Default'] ? 'true' : 'false';
                }
            } else if (strpos($row['Type'], 'float') === 0 || strpos($row['Type'], 'double') === 0) {
                $type = 'self::DOUBLE';
                if ($row['Default']) {
                    $row['Default'] = (double) $row['Default'];
                }
            } else {
                $type = 'self::STRING';
                if ($row['Default']) {
                    $row['Default'] = '\'' . (string) $row['Default'] . '\'';
                }
            }

            if ($row['Default']) {
                $fields .= '        \'' . $row['Field'] . '\' => array(' . $type . ', ' . $row['Default'] . '),' . "\n";
            } else {
                $fields .= '        \'' . $row['Field'] . '\' => ' . $type . ",\n";
            }

            if ($row['Key'] == 'PRI') {
                $pk = $row['Field'];
            }
        }
        $fields = rtrim($fields);
        if (!$pk) {
            die("table miss primary key\n");
        }

        $code = "<?php

namespace application\common\model;

class $classname extends \TF\DBModel
{
    protected static \$_table = '$table';

    protected static \$_fields = array(
$fields
    );

" . ($pk == 'id' ? '' : "    protected static \$_pk = '$pk';\n\n") . "}\n";

        $file = realpath(ROOT_PATH . '/application/common/model/') . '/' . $classname . '.php';
        if (file_exists($file)) {
            die($file . " exists\n");
        }
        file_put_contents($file, $code);

        echo "success\n";
    }
}
