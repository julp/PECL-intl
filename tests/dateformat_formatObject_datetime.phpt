--TEST--
IntlDateFormatter::formatObject(): DateTime tests
--SKIPIF--
<?php
if (!extension_loaded('intl'))
	die('skip intl extension not enabled');
--FILE--
<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", "Europe/Lisbon");

$dt = new DateTime('2012-01-01 00:00:00'); //Europe/Lisbon
echo IntlDateFormatter::formatObject($dt), "\n";
echo IntlDateFormatter::formatObject($dt, IntlDateFormatter::FULL), "\n";
echo IntlDateFormatter::formatObject($dt, null, "en-US"), "\n";
echo IntlDateFormatter::formatObject($dt, array(IntlDateFormatter::SHORT, IntlDateFormatter::FULL), "en-US"), "\n";
echo IntlDateFormatter::formatObject($dt, 'E y-MM-d HH,mm,ss.SSS v', "en-US"), "\n";

$dt = new DateTime('2012-01-01 05:00:00+03:00');
echo IntlDateFormatter::formatObject($dt, IntlDateFormatter::FULL), "\n";

?>
==DONE==

--EXPECTF--
01/01/2012%S 00:00:00
Domingo, 1 de Janeiro de 2012 %S0:00:00 Hora %Sda Europa Ocidental
Jan 1, 2012%S 12:00:00 AM
1/1/12%S 12:00:00 AM Western European %STime
Sun 2012-01-1 00,00,00.000 %SLisbon%S
Domingo, 1 de Janeiro de 2012 %S5:00:00 GMT+03:00
==DONE==
