--TEST--
Collator::replace
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--INI--
intl.error_level=0
intl.use_exceptions=0
intl.default_locale=fr
--FILE--
<?php
# charset: UTF-8

define('TIMES', 30);

$coll = new Collator(ini_get('intl.default_locale'));
$coll->setStrength(Collator::PRIMARY);

function assert_single($subject, $search, $replacement, $expected) {
    global $coll;

    return $coll->replace($subject, $search, $replacement) === $expected;
}

function assert_multiple($subject, $search, $replacement, $expected) {
    global $coll;

    return $coll->replace(str_repeat($subject, TIMES), $search, $replacement) === str_repeat($expected, TIMES);
}

ini_set('intl.error_level', 0);

var_dump($coll->replace('subject', 'search'));
var_dump($coll->replace('subject', 'search', 'replacement', $count, 'FAIL'));

ini_set('intl.error_level', E_WARNING);

echo 'Check count parameter', "\n";
$coll->replace('oui', 'non', 'maybe', $c1);
var_dump($c1);
$coll->replace('la valeur a déjà été modifiée', 'É', 'x', $c2);
var_dump($c2);

echo 'Delete (replace by empty string)', "\n";
var_dump(assert_single("Vous eûtes 23 ÉlèVes", ' élèves', '', "Vous eûtes 23"));
var_dump(assert_multiple("Vous eûtes 23 ÉlèVes", ' élèves', '', "Vous eûtes 23"));

echo 'Replace by a shorter string', "\n";
var_dump(assert_single("Vous eûtes 23 ÉlèVes", 'élèves', 'cats', "Vous eûtes 23 cats"));
var_dump(assert_multiple("Vous eûtes 23 ÉlèVes", 'élèves', 'cats', "Vous eûtes 23 cats"));

echo 'ß <=> SS', "\n";
var_dump(assert_single('Petersburger Straße', 'SS', '<eszett>', 'Petersburger Stra<eszett>e'));
var_dump(assert_multiple('Petersburger Straße', 'SS', '<eszett>', 'Petersburger Stra<eszett>e'));

echo 'DZ digraph', "\n";
var_dump(assert_single('ǲwon hráǳa', 'Ǳ', '<DZ>', '<DZ>won hrá<DZ>a'));
var_dump(assert_multiple('ǲwon hráǳa', 'Ǳ', '<DZ>', '<DZ>won hrá<DZ>a'));


$coll = new Collator('tr');
$coll->setStrength(Collator::PRIMARY);

echo 'Turkish dotted I', "\n";
var_dump(assert_single('İyi akşamlar', 'İ', '<dotted i>', '<dotted i>y<dotted i> akşamlar'));
var_dump(assert_multiple('İyi akşamlar', 'İ', '<dotted i>', '<dotted i>y<dotted i> akşamlar'));

echo 'Turkish non dotted I', "\n";
var_dump(assert_single('Hayır', 'I', '<non dotted i>', 'Hay<non dotted i>r'));
var_dump(assert_multiple('Hayır', 'I', '<non dotted i>', 'Hay<non dotted i>r'));
?>
--EXPECTF--

Warning: Collator::replace() expects at least 3 parameters, 2 given in %s on line %d
bool(false)

Warning: Collator::replace() expects at most 4 parameters, 5 given in %s on line %d
bool(false)
Check count parameter
int(0)
int(6)
Delete (replace by empty string)
bool(true)
bool(true)
Replace by a shorter string
bool(true)
bool(true)
ß <=> SS
bool(true)
bool(true)
DZ digraph
bool(true)
bool(true)
Turkish dotted I
bool(true)
bool(true)
Turkish non dotted I
bool(true)
bool(true)
