--TEST--
Regexp: bad inheritance
--SKIPIF--
<?php
if (!extension_loaded('intl')) { die('skip intl extension not available'); }
?>
--INI--
intl.use_exceptions=0
intl.error_level=E_WARNING
--FILE--
<?php
class Regexp2 extends Regexp {
    public function __construct() {
        // ommitting parent::__construct($pattern);
    }
}

$c = new Regexp2();
$a = $c->match('h');
?>
--EXPECTF--
Warning: Regexp::match(): Found unconstructed regular expression in %s on line %d
