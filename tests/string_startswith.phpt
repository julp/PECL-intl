--TEST--
utf8_startswith()
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
$grave = "\xCC\x80";
$acute = "\xCC\x81";

$e_acute_nfc = "\xC3\xA9";
$e_acute_nfd = "e${acute}";

$e_grave_nfc = "\xC3\xA8";
$e_grave_nfd = "\x65${grave}";

echo "Full case folding\n";
var_dump(utf8_startswith('SSSSe', 'ßß', TRUE));
var_dump(utf8_startswith('ßße', 'SSSS', TRUE));

echo "\nGrapheme consistency\n";
var_dump(utf8_startswith($e_acute_nfd, 'e'));

echo "\nintl.turkic_casefolding = 0\n";
ini_set('intl.turkic_casefolding', FALSE);
var_dump(utf8_startswith('İyi', 'i', TRUE));
var_dump(utf8_startswith('Iyi', 'i', TRUE));
var_dump(utf8_startswith('Hayır', 'HAYI', TRUE));
var_dump(utf8_startswith('Hayir', 'HAYI', TRUE));

echo "\nintl.turkic_casefolding = 1\n";
ini_set('intl.turkic_casefolding', TRUE);
var_dump(utf8_startswith('İyi', 'i', TRUE));
var_dump(utf8_startswith('Iyi', 'i', TRUE));
var_dump(utf8_startswith('Hayır', 'HAYI', TRUE));
var_dump(utf8_startswith('Hayir', 'HAYI', TRUE));
?>
--EXPECTF--
Full case folding
bool(true)
bool(true)

Grapheme consistency
bool(false)

intl.turkic_casefolding = 0
bool(false)
bool(true)
bool(false)
bool(true)

intl.turkic_casefolding = 1
bool(true)
bool(false)
bool(true)
bool(false)
