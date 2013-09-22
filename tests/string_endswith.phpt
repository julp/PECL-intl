--TEST--
utf8_endswith()
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
var_dump(utf8_endswith('eSSSS', 'ßß', TRUE));
var_dump(utf8_endswith('eßß', 'SSSS', TRUE));

echo "\nGrapheme consistency\n";
var_dump(utf8_endswith($e_acute_nfd, $acute));

echo "\nintl.turkic_casefolding = 0\n";
ini_set('intl.turkic_casefolding', FALSE);
var_dump(utf8_endswith('İYİ', 'i', TRUE));
var_dump(utf8_endswith('IYI', 'i', TRUE));
var_dump(utf8_endswith('Hayır', 'YIR', TRUE));
var_dump(utf8_endswith('Hayir', 'YIR', TRUE));

echo "\nintl.turkic_casefolding = 1\n";
ini_set('intl.turkic_casefolding', TRUE);
var_dump(utf8_endswith('İYİ', 'i', TRUE));
var_dump(utf8_endswith('IYI', 'i', TRUE));
var_dump(utf8_endswith('Hayır', 'YIR', TRUE));
var_dump(utf8_endswith('Hayir', 'YIR', TRUE));
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
