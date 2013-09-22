--TEST--
Regexp: assume graphemes consistency
--SKIPIF--
<?php if (!extension_loaded('intl') || !extension_loaded('mbstring') || version_compare(PHP_VERSION, '5.3.0', '<')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

ini_set('intl.error_level', 0); // will be sufficient

$grave = "\xCC\x80";
$acute = "\xCC\x81";

$e_acute_nfc = "\xC3\xA9";
$e_acute_nfd = "e${acute}";

$e_grave_nfc = "\xC3\xA8";
$e_grave_nfd = "\x65${grave}";

echo "matchAll:\n";

$input = "e${e_acute_nfd}${e_grave_nfd}";

$re = new Regexp('e');
var_dump($re->matchAll($input, $matches, Regexp::OFFSET_CAPTURE), $matches);

$re = new Regexp("E$grave", 'i');
var_dump($re->matchAll($input, $matches, Regexp::OFFSET_CAPTURE), $matches);


$input = "${e_acute_nfd}e${e_grave_nfd}";

echo "\nreplaceCallback:\n";
$re = new Regexp('E', 'i');
var_dump(
    mb_convert_encoding(
        $re->replaceCallback(
            $input,
            function ($matches) {
                return '<replaced>';
            }
        ),
        'HTML-ENTITIES',
        'UTF-8'
    )
);

echo "\nsplit:\n";
var_dump($re->split($input, -1, Regexp::OFFSET_CAPTURE));
?>
--EXPECTF--
matchAll:
int(1)
array(1) {
  [0]=>
  array(1) {
    [0]=>
    array(2) {
      [0]=>
      string(1) "e"
      [1]=>
      int(0)
    }
  }
}
int(1)
array(1) {
  [0]=>
  array(1) {
    [0]=>
    array(2) {
      [0]=>
      string(3) "e%c%c"
      [1]=>
      int(2)
    }
  }
}

replaceCallback:
string(%d) "e&#769;<replaced>e&#768;"

split:
array(2) {
  [0]=>
  string(3) "e%c%c"
  [2]=>
  string(3) "e%c%c"
}
