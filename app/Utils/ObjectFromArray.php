<?php

namespace App\Utils;

use stdClass;

class ObjectFromArray
{
    public function __construct($array)
    {
        return $this->createObject($array);
    }

    public static function createObject($array)
    {
        $object = new stdClass();
        foreach ($array as $key => $value) {
            if (is_array($value)) {
                $object->$key = self::createObject($value);
            } else {
                $object->$key = $value;
            }
        }
        return $object;
    }

    public function get()
    {
        return $this->object;
    }
}
