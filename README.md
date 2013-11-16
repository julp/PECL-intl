# Introduction

This repository is **unofficial** and **experimental**. Its goal is to extend current implementation of PHP/intl extension with some +/- essential, or at least useful, missing functions.

# Features

* ini settings added (only used by utf8_[starts|ends]with for now):
  * intl.turkic_casefolding: i (U+69) = I (U+49) vs i (U+69) = İ (U+130) + ı (U+131) = I (U+49)
* extra stuffs:
  * all matches are "grapheme consistent"
  * expose Unicode version supported by intl/ICU (phpinfo + constant INTL_UNICODE_VERSION)
* Collator, methods added (+ procedural equivalents):
  * replace (str_ireplace replacement) `string Collator::replace(string $string, string $search, string $replacement [, int &count ])`
  * lfind (str(i)str replacement) `mixed Collator::lfind(string $haystack, string $needle [, int $startOffset = 0 [, bool $before ]])`
  * rfind (strr(i)str replacement with a different behavior) `mixed Collator::rfind(string $haystack, string $needle [, int $startOffset = 0 [, bool $before ]])`
  * lindex (str(i)pos replacement) `int Collator::lindex(string $haystack, string $needle [, int $startOffset = 0 ])`
  * rindex (strr(i)pos replacement with a different behavior) `int Collator::rindex(string $haystack, string $needle [, int $startOffset = 0 ])`
  * startswith (locale dependant) `boolean Collator::startswith(string $haystack, string $needle)`
  * endswith (locale dependant) `boolean Collator::endswith(string $haystack, string $needle)`
* Regexp, class added (+ procedural equivalents):
  * __construct `Regexp::__construct(string $pattern [, mixed $flags ])`
  * match `int Regexp::match(string $subject, array &$match [, int $startOffset = 0 [, int $flags = 0 ]])`
  * matchAll `int Regexp::matchAll(string $subject, array &$match [, int $startOffset = 0 [, int $flags = 0 ]])`
  * replace `string Regexp::replace(string $subject, string $replacement [, int $limit = -1 [, int &$count ]])`
  * replaceCallback `string Regexp::replaceCallback(string $subject, callable $replacement [, int $limit = -1 [, int &$count ]])`
  * split `array Regexp::split(string $subject [, int $limit = -1 [, int $flags = 0 ]])`
* String, functions added:
  * full case mapping: functions utf8_to\[upper|lower|title\] \(strtoupper, strtolower, ucwords replacements\) `string utf8_toupper(string $string [, string $locale = ini_get('intl.default_locale') ])`
  * utf8_startswith (locale independant and turkic languages supported) `bool utf8_startswith(string $string, string $prefix [, bool $case_insensitive = FALSE ])`
  * utf8_endswith (locale independant and turkic languages supported) `bool utf8_endswith(string $string, string $suffix [, bool $case_insensitive = FALSE ])`

Note: for locale independant case insensitivity (turkic languages excluded), you can use UCA rules by creating a "root" Collator (`$coll = new Collator('root');`) (do not use an empty string as locale name) and a secondary-level strength (`$coll->setStrength(Collator::SECONDARY);`)

# How to use

## As dynamic extension

1. make sure your current PHP installation does not already include intl as a static extension (if dynamic, backup/move first intl.so or php_intl.dll somewhere else)
2. grab these sources
3. compile and install them as any other extension: `cd path/to/sources && phpize && ./configure && make && make install`
4. load it by adding `extension=intl.so` (replace intl.so by php_intl.dll on Windows) to your php.ini
5. restart your web server if relevant (depends on the SAPI - needed for fpm or apache2handler)

## As static extension

1. get PHP sources, uncompress them and go into their directory
2. move or delete official sources of intl (`rm -fr ext/intl/*`)
3. grab these sources and extract them into the directory ext/intl
4. run buildconf from top directory of PHP (`./buildconf --force`)
5. compile PHP as usual (`./configure ... --enable-intl && make && make install`)
