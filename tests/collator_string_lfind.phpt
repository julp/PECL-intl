--TEST--
Collator::lfind
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--INI--
intl.use_exceptions=0
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

$grave = "\xCC\x80";
$acute = "\xCC\x81";

$e_acute_nfc = "\xC3\xA9";
$e_acute_nfd = "e${acute}";

$e_grave_nfc = "\xC3\xA8";
$e_grave_nfd = "\x65${grave}";

$coll = new Collator('fr_FR');
// $coll->setAttribute(Collator::NORMALIZATION_MODE, Collator::ON);

$haystack1 = "Ce verre est {$e_acute_nfd}br{$e_acute_nfd}ch{$e_acute_nfd}.";
$haystack2 = str_replace($e_acute_nfd, $e_acute_nfc, $haystack1);

var_dump($coll->lfind($haystack1, $e_acute_nfc) === "{$e_acute_nfd}br{$e_acute_nfd}ch{$e_acute_nfd}.");
var_dump($coll->lfind($haystack2, $e_acute_nfd) === "{$e_acute_nfc}br{$e_acute_nfc}ch{$e_acute_nfc}.");

var_dump($coll->lfind($haystack1, $e_acute_nfc, 0, TRUE) === "Ce verre est ");
var_dump($coll->lfind($haystack2, $e_acute_nfd, 0, TRUE) === "Ce verre est ");

var_dump($coll->lfind($haystack1, $e_acute_nfc, 14) === "{$e_acute_nfd}ch{$e_acute_nfd}.");
var_dump($coll->lfind($haystack2, $e_acute_nfd, 14) === "{$e_acute_nfc}ch{$e_acute_nfc}.");


var_dump($coll->lindex($haystack1, $e_acute_nfc, 14) === 16);
var_dump($coll->lindex($haystack2, $e_acute_nfd, 14) === 16);

var_dump($coll->lindex($haystack1, $e_acute_nfc, -6) === 16);
var_dump($coll->lindex($haystack2, $e_acute_nfd, -6) === 16);



var_dump($coll->rfind($haystack1, $e_acute_nfc) === "{$e_acute_nfd}.");
var_dump($coll->rfind($haystack2, $e_acute_nfd) === "{$e_acute_nfc}.");

var_dump($coll->rfind($haystack1, $e_acute_nfc, 0, TRUE) === "Ce verre est {$e_acute_nfd}br{$e_acute_nfd}ch");
var_dump($coll->rfind($haystack2, $e_acute_nfd, 0, TRUE) === "Ce verre est {$e_acute_nfc}br{$e_acute_nfc}ch");

var_dump($coll->rfind($haystack1, $e_acute_nfc, 14) === "{$e_acute_nfd}br{$e_acute_nfd}ch{$e_acute_nfd}.");
var_dump($coll->rfind($haystack2, $e_acute_nfd, 14) === "{$e_acute_nfc}br{$e_acute_nfc}ch{$e_acute_nfc}."); // fail


var_dump($coll->rindex($haystack1, $e_acute_nfc, 14) === 13);
var_dump($coll->rindex($haystack2, $e_acute_nfd, 14) === 13); // fail

/*
+---+---+---+---+---+
| a | a | a | a | a |
+---+---+---+---+---+
| 0 | 1 | 2 | 3 | 4 |
+---+---+---+---+---+
              # ^     => search on left to offset 4 (represented by ^), match is at offset 3 (represented by #)
  # ^                 => search on left to offset 1 (represented by ^), match is at offset 0 (represented by #)
*/

var_dump($coll->rfind('aaaaa', 'a', 1, FALSE) === 'aaaaa');
var_dump($coll->rfind('aaaaa', 'a', 1, TRUE) === '');

var_dump($coll->rfind('aaaaa', 'a', 4, FALSE) === 'aa');
var_dump($coll->rfind('aaaaa', 'a', 4, TRUE) === 'aaa');

$X307 = "\xCC\x87"; # 307 (COMBINING DOT ABOVE)
$X323 = "\xCC\xA3"; # 323 (COMBINING DOT BELOW)
$S = "S{$X307}{$X323}";
$string = str_repeat($S, 5);
var_dump($coll->rfind($string, $S, 1, FALSE) === str_repeat($S, 5));
var_dump($coll->rfind($string, $S, 1, TRUE) === '');

var_dump($coll->rfind($string, $S, 4, FALSE) === str_repeat($S, 2));
var_dump($coll->rfind($string, $S, 4, TRUE) === str_repeat($S, 3));

/*
var_dump($coll->rindex($string, $S, 4));
var_dump($coll->rindex($string, $S, 4));
var_dump($coll->rfind($string, $S, 4, FALSE));
var_dump($coll->rfind($string, $S, 4, TRUE));
var_dump($coll->rfind('aaaaa', 'a', 1, FALSE)); // string (5) "aaaaa" match on [0;1[
var_dump($coll->rfind('aaaaa', 'a', 1, TRUE)); // string (0) "" match on [0;1[
var_dump($coll->rfind('aaaaa', 'a', 4, FALSE)); // string (2) "aa" match on [3;4[
var_dump($coll->rfind('aaaaa', 'a', 4, TRUE)); // string (3) "aaa" match on [3;4[
*/

/*

NFD ($haystack1):
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| C | e | - | v | e | r | r | e | - | e | s | t | - | e | ' | b | r | ...
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | 1 | 2 |   3   | 4 | 5 | Grapheme offsets
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | CU offsets
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
                                                            ^ (search offset)

NFC ($haystack2):
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| C | e | - | v | e | r | r | e | - | e | s | t | - | é | b | r | é | c | h | é | . | ...
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | Grapheme offsets
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| 1 | 0 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | Negative grapheme offsets (without minus sign)
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | CU offsets
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
                                                        ^ (search offset)
*/

var_dump($coll->rindex($haystack1, $e_acute_nfc, -7) === 13);
var_dump($coll->rindex($haystack2, $e_acute_nfd, -7) === 13); // fail

# Grapheme consistency
var_dump($coll->rindex($haystack1, 'e') === 9);

# Overlap test (r[find|index] only)
var_dump($coll->rindex($haystack1, $e_acute_nfc, 13) === -1);
var_dump($coll->rindex($haystack2, $e_acute_nfd, 13) === -1);

exit;
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
