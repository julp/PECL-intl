--TEST--
INTL_ICU_DATA_VERSION constant
--SKIPIF--
<?php if( !extension_loaded( 'intl' ) || version_compare(INTL_ICU_VERSION, '4.4', '<') ) print 'for ICU >= 4.4'; ?>
--FILE--
<?php
var_dump(defined("INTL_ICU_DATA_VERSION"));
?>
--EXPECT--
bool(true)
