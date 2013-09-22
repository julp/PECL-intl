--TEST--
Regexp::replace()
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

// WARNING: take care of reference (count) if(when) use(d)
function ut_regexp_replace($oo, $ro)
{
    $args = func_get_args();
    array_splice($args, 0, $oo ? 2 : 1);
    return call_user_func_array($oo ? array($ro, 'replace') : 'regexp_replace', $args);
}

$oo = FALSE;
$subject = "0${A}1${B}2${C}3";
define('TIMES', 100);

start_test_suite:

$ro = ut_regexp_create($oo, '[^\p{L}]');
var_dump(
    $x = ut_regexp_replace(
        $oo,
        $ro,
        $subject,
        ''
    )
    ===
    "$A$B$C"
);

// For leak
$ro = ut_regexp_create($oo, '\p{Nd}');
$io = implode('', array_merge(range('a', 'z'), range('A', 'Z')));
var_dump(
    $x = ut_regexp_replace(
        $oo,
        $ro,
        $io,
        ''
    )
    ===
    $io
);

// Try overflow
$ro = ut_regexp_create($oo, '\p{L}');
var_dump(
    $x = ut_regexp_replace(
        $oo,
        $ro,
        $subject,
        str_repeat('$0', TIMES)
    )
    ===
    implode(
        '',
        array(
            '0',
            str_repeat($A, TIMES),
            '1',
            str_repeat($B, TIMES),
            '2',
            str_repeat($C, TIMES),
            '3'
        )
    )
);

if (!$oo) {
    echo "\n";
    $oo = TRUE;
    goto start_test_suite;
}
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)

bool(true)
bool(true)
bool(true)
