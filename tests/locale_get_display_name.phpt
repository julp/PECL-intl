--TEST--
locale_get_display_name()
--INI--
unicode.runtime_encoding="utf-8"
--SKIPIF--
<?php if( !extension_loaded( 'intl' ) ) print 'skip'; ?>
--FILE--
<?php

/*
 * Try getting the display_name for different locales
 * with Procedural and Object methods.
 */

function ut_main()
{
    $res_str='';

	$disp_locales=array('en','fr','de');

    $locales = array(
        'sl_IT_nedis_KIRTI',
        'sl_IT_nedis-a-kirti-x-xyz',
        'sl_IT_rozaj',
        'sl_IT_NEDIS_ROJAZ_1901',
        'i-enochian',
        'zh-hakka',
        'zh-wuu',
        'i-tay',
        'sgn-BE-nl',
        'sgn-CH-de',
        'sl_IT_rozaj@currency=EUR',
        'uk-ua_CALIFORNIA@currency=;currency=GRN',
        'root',
        'uk@currency=EURO',
        'Hindi',
//Simple language subtag
        'de',
        'fr',
        'ja',
        'i-enochian', //(example of a grandfathered tag)
//Language subtag plus Script subtag:
        'zh-Hant',
        'zh-Hans',
        'sr-Cyrl',
        'sr-Latn',
//Language-Script-Region
        'zh-Hans-CN',
        'sr-Latn-CS',
//Language-Variant
        'sl-rozaj',
        'sl-nedis',
//Language-Region-Variant
        'de-CH-1901',
        'sl-IT-nedis',
//Language-Script-Region-Variant
        'sl-Latn-IT-nedis',
//Language-Region:
        'de-DE',
        'en-US',
        'es-419',
//Private use subtags:
        'de-CH-x-phonebk',
        'az-Arab-x-AZE-derbend',
//Extended language subtags
        'zh-min',
        'zh-min-nan-Hant-CN',
//Private use registry values
        'x-whatever',
        'qaa-Qaaa-QM-x-southern',
        'sr-Latn-QM',
        'sr-Qaaa-CS',
/*Tags that use extensions (examples ONLY: extensions MUST be defined
   by revision or update to this document or by RFC): */
        'en-US-u-islamCal',
        'zh-CN-a-myExt-x-private',
        'en-a-myExt-b-another',
//Some Invalid Tags:
        'de-419-DE',
        'a-DE',
        'ar-a-aaa-b-bbb-a-ccc'
    );


    $res_str = '';

   	foreach( $locales as $locale )
    {
       	$res_str .= "locale='$locale'\n";
   		foreach( $disp_locales as $disp_locale )
    	{
        	$scr = ut_loc_get_display_name( $locale ,$disp_locale );
        	$res_str .= "disp_locale=$disp_locale :  display_name=$scr";
        	$res_str .= "\n";
		} 
        $res_str .= "-----------------\n";
    }

    return $res_str;

}

include_once( 'ut_common.php' );
ut_run();

?>
--EXPECT--
locale='sl_IT_nedis_KIRTI'
disp_locale=en :  display_name=Slovenian (Italy, NEDIS_KIRTI)
disp_locale=fr :  display_name=slovène (Italie, NEDIS_KIRTI)
disp_locale=de :  display_name=Slowenisch (Italien, NEDIS_KIRTI)
-----------------
locale='sl_IT_nedis-a-kirti-x-xyz'
disp_locale=en :  display_name=Slovenian (Italy, NEDIS_A_KIRTI_X_XYZ)
disp_locale=fr :  display_name=slovène (Italie, NEDIS_A_KIRTI_X_XYZ)
disp_locale=de :  display_name=Slowenisch (Italien, NEDIS_A_KIRTI_X_XYZ)
-----------------
locale='sl_IT_rozaj'
disp_locale=en :  display_name=Slovenian (Italy, Resian)
disp_locale=fr :  display_name=slovène (Italie, dialecte de Resia)
disp_locale=de :  display_name=Slowenisch (Italien, ROZAJ)
-----------------
locale='sl_IT_NEDIS_ROJAZ_1901'
disp_locale=en :  display_name=Slovenian (Italy, NEDIS_ROJAZ_1901)
disp_locale=fr :  display_name=slovène (Italie, NEDIS_ROJAZ_1901)
disp_locale=de :  display_name=Slowenisch (Italien, NEDIS_ROJAZ_1901)
-----------------
locale='i-enochian'
disp_locale=en :  display_name=i-enochian
disp_locale=fr :  display_name=i-enochian
disp_locale=de :  display_name=i-enochian
-----------------
locale='zh-hakka'
disp_locale=en :  display_name=Chinese (HAKKA)
disp_locale=fr :  display_name=chinois (HAKKA)
disp_locale=de :  display_name=Chinesisch (HAKKA)
-----------------
locale='zh-wuu'
disp_locale=en :  display_name=Chinese (WUU)
disp_locale=fr :  display_name=chinois (WUU)
disp_locale=de :  display_name=Chinesisch (WUU)
-----------------
locale='i-tay'
disp_locale=en :  display_name=i-tay
disp_locale=fr :  display_name=i-tay
disp_locale=de :  display_name=i-tay
-----------------
locale='sgn-BE-nl'
disp_locale=en :  display_name=Sign Languages (Belgium, NL)
disp_locale=fr :  display_name=langues des signes (Belgique, NL)
disp_locale=de :  display_name=Gebärdensprache (Belgien, NL)
-----------------
locale='sgn-CH-de'
disp_locale=en :  display_name=Sign Languages (Switzerland, DE)
disp_locale=fr :  display_name=langues des signes (Suisse, DE)
disp_locale=de :  display_name=Gebärdensprache (Schweiz, DE)
-----------------
locale='sl_IT_rozaj@currency=EUR'
disp_locale=en :  display_name=Slovenian (Italy, Resian, Currency=Euro)
disp_locale=fr :  display_name=slovène (Italie, dialecte de Resia, Devise=euro)
disp_locale=de :  display_name=Slowenisch (Italien, ROZAJ, Währung=Euro)
-----------------
locale='uk-ua_CALIFORNIA@currency=;currency=GRN'
disp_locale=en :  display_name=Ukrainian (Ukraine, CALIFORNIA, Currency)
disp_locale=fr :  display_name=ukrainien (Ukraine, CALIFORNIA, Devise)
disp_locale=de :  display_name=Ukrainisch (Ukraine, CALIFORNIA, Währung)
-----------------
locale='root'
disp_locale=en :  display_name=Root
disp_locale=fr :  display_name=racine
disp_locale=de :  display_name=root
-----------------
locale='uk@currency=EURO'
disp_locale=en :  display_name=Ukrainian (Currency=EURO)
disp_locale=fr :  display_name=ukrainien (Devise=EURO)
disp_locale=de :  display_name=Ukrainisch (Währung=EURO)
-----------------
locale='Hindi'
disp_locale=en :  display_name=hindi
disp_locale=fr :  display_name=hindi
disp_locale=de :  display_name=hindi
-----------------
locale='de'
disp_locale=en :  display_name=German
disp_locale=fr :  display_name=allemand
disp_locale=de :  display_name=Deutsch
-----------------
locale='fr'
disp_locale=en :  display_name=French
disp_locale=fr :  display_name=français
disp_locale=de :  display_name=Französisch
-----------------
locale='ja'
disp_locale=en :  display_name=Japanese
disp_locale=fr :  display_name=japonais
disp_locale=de :  display_name=Japanisch
-----------------
locale='i-enochian'
disp_locale=en :  display_name=i-enochian
disp_locale=fr :  display_name=i-enochian
disp_locale=de :  display_name=i-enochian
-----------------
locale='zh-Hant'
disp_locale=en :  display_name=Chinese (Traditional Han)
disp_locale=fr :  display_name=chinois (idéogrammes han (variante traditionnelle))
disp_locale=de :  display_name=Chinesisch (Traditionelle Chinesische Schrift)
-----------------
locale='zh-Hans'
disp_locale=en :  display_name=Chinese (Simplified Han)
disp_locale=fr :  display_name=chinois (idéogrammes han (variante simplifiée))
disp_locale=de :  display_name=Chinesisch (Vereinfachte Chinesische Schrift)
-----------------
locale='sr-Cyrl'
disp_locale=en :  display_name=Serbian (Cyrillic)
disp_locale=fr :  display_name=serbe (cyrillique)
disp_locale=de :  display_name=Serbisch (Kyrillisch)
-----------------
locale='sr-Latn'
disp_locale=en :  display_name=Serbian (Latin)
disp_locale=fr :  display_name=serbe (latin)
disp_locale=de :  display_name=Serbisch (Lateinisch)
-----------------
locale='zh-Hans-CN'
disp_locale=en :  display_name=Chinese (Simplified Han, China)
disp_locale=fr :  display_name=chinois (idéogrammes han (variante simplifiée), Chine)
disp_locale=de :  display_name=Chinesisch (Vereinfachte Chinesische Schrift, China)
-----------------
locale='sr-Latn-CS'
disp_locale=en :  display_name=Serbian (Latin, Serbia And Montenegro)
disp_locale=fr :  display_name=serbe (latin, Serbie-et-Monténégro)
disp_locale=de :  display_name=Serbisch (Lateinisch, Serbien und Montenegro)
-----------------
locale='sl-rozaj'
disp_locale=en :  display_name=Slovenian (ROZAJ)
disp_locale=fr :  display_name=slovène (ROZAJ)
disp_locale=de :  display_name=Slowenisch (ROZAJ)
-----------------
locale='sl-nedis'
disp_locale=en :  display_name=Slovenian (NEDIS)
disp_locale=fr :  display_name=slovène (NEDIS)
disp_locale=de :  display_name=Slowenisch (NEDIS)
-----------------
locale='de-CH-1901'
disp_locale=en :  display_name=German (Switzerland, Traditional German orthography)
disp_locale=fr :  display_name=allemand (Suisse, orthographe allemande traditionnelle)
disp_locale=de :  display_name=Deutsch (Schweiz, 1901)
-----------------
locale='sl-IT-nedis'
disp_locale=en :  display_name=Slovenian (Italy, Natisone dialect)
disp_locale=fr :  display_name=slovène (Italie, dialecte de Natisone)
disp_locale=de :  display_name=Slowenisch (Italien, NEDIS)
-----------------
locale='sl-Latn-IT-nedis'
disp_locale=en :  display_name=Slovenian (Latin, Italy, Natisone dialect)
disp_locale=fr :  display_name=slovène (latin, Italie, dialecte de Natisone)
disp_locale=de :  display_name=Slowenisch (Lateinisch, Italien, NEDIS)
-----------------
locale='de-DE'
disp_locale=en :  display_name=German (Germany)
disp_locale=fr :  display_name=allemand (Allemagne)
disp_locale=de :  display_name=Deutsch (Deutschland)
-----------------
locale='en-US'
disp_locale=en :  display_name=English (United States)
disp_locale=fr :  display_name=anglais (États-Unis)
disp_locale=de :  display_name=Englisch (Vereinigte Staaten)
-----------------
locale='es-419'
disp_locale=en :  display_name=Spanish (Latin America and the Caribbean)
disp_locale=fr :  display_name=espagnol (Amérique latine et Caraïbes)
disp_locale=de :  display_name=Spanisch (Lateinamerika und Karibik)
-----------------
locale='de-CH-x-phonebk'
disp_locale=en :  display_name=German (Switzerland, X_PHONEBK)
disp_locale=fr :  display_name=allemand (Suisse, X_PHONEBK)
disp_locale=de :  display_name=Deutsch (Schweiz, X_PHONEBK)
-----------------
locale='az-Arab-x-AZE-derbend'
disp_locale=en :  display_name=Azerbaijani (Arabic, X, AZE_DERBEND)
disp_locale=fr :  display_name=azéri (arabe, X, AZE_DERBEND)
disp_locale=de :  display_name=Aserbaidschanisch (Arabisch, X, AZE_DERBEND)
-----------------
locale='zh-min'
disp_locale=en :  display_name=Chinese (MIN)
disp_locale=fr :  display_name=chinois (MIN)
disp_locale=de :  display_name=Chinesisch (MIN)
-----------------
locale='zh-min-nan-Hant-CN'
disp_locale=en :  display_name=Chinese (MIN, NAN_HANT_CN)
disp_locale=fr :  display_name=chinois (MIN, NAN_HANT_CN)
disp_locale=de :  display_name=Chinesisch (MIN, NAN_HANT_CN)
-----------------
locale='x-whatever'
disp_locale=en :  display_name=x-whatever
disp_locale=fr :  display_name=x-whatever
disp_locale=de :  display_name=x-whatever
-----------------
locale='qaa-Qaaa-QM-x-southern'
disp_locale=en :  display_name=qaa (Qaaa, QM, X_SOUTHERN)
disp_locale=fr :  display_name=qaa (Qaaa, QM, X_SOUTHERN)
disp_locale=de :  display_name=qaa (Qaaa, QM, X_SOUTHERN)
-----------------
locale='sr-Latn-QM'
disp_locale=en :  display_name=Serbian (Latin, QM)
disp_locale=fr :  display_name=serbe (latin, QM)
disp_locale=de :  display_name=Serbisch (Lateinisch, QM)
-----------------
locale='sr-Qaaa-CS'
disp_locale=en :  display_name=Serbian (Qaaa, Serbia And Montenegro)
disp_locale=fr :  display_name=serbe (Qaaa, Serbie-et-Monténégro)
disp_locale=de :  display_name=Serbisch (Qaaa, Serbien und Montenegro)
-----------------
locale='en-US-u-islamCal'
disp_locale=en :  display_name=English (United States, U_ISLAMCAL)
disp_locale=fr :  display_name=anglais (États-Unis, U_ISLAMCAL)
disp_locale=de :  display_name=Englisch (Vereinigte Staaten, U_ISLAMCAL)
-----------------
locale='zh-CN-a-myExt-x-private'
disp_locale=en :  display_name=Chinese (China, A_MYEXT_X_PRIVATE)
disp_locale=fr :  display_name=chinois (Chine, A_MYEXT_X_PRIVATE)
disp_locale=de :  display_name=Chinesisch (China, A_MYEXT_X_PRIVATE)
-----------------
locale='en-a-myExt-b-another'
disp_locale=en :  display_name=English (A, MYEXT_B_ANOTHER)
disp_locale=fr :  display_name=anglais (A, MYEXT_B_ANOTHER)
disp_locale=de :  display_name=Englisch (A, MYEXT_B_ANOTHER)
-----------------
locale='de-419-DE'
disp_locale=en :  display_name=German (Latin America and the Caribbean, DE)
disp_locale=fr :  display_name=allemand (Amérique latine et Caraïbes, DE)
disp_locale=de :  display_name=Deutsch (Lateinamerika und Karibik, DE)
-----------------
locale='a-DE'
disp_locale=en :  display_name=a (Germany)
disp_locale=fr :  display_name=a (Allemagne)
disp_locale=de :  display_name=a (Deutschland)
-----------------
locale='ar-a-aaa-b-bbb-a-ccc'
disp_locale=en :  display_name=Arabic (A, AAA_B_BBB_A_CCC)
disp_locale=fr :  display_name=arabe (A, AAA_B_BBB_A_CCC)
disp_locale=de :  display_name=Arabisch (A, AAA_B_BBB_A_CCC)
-----------------
