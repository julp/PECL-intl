#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
// #include <unicode/ubrk.h>
// #include <unicode/uset.h>
// #include <unicode/unorm.h>

#include "php.h"
#include "php_intl.h"
#include "intl_data.h"
#include "intl_convert.h"
// #include "intl_utf8.h"
// #include "intl_utf16.h"
#include "intl_case.h"
#include "string/string.h"
#include "ext/standard/php_smart_str.h"
#include "grapheme/grapheme_util.h"

#define CHECK_STATUS(status, msg)                           \
    intl_error_set_code(NULL, status TSRMLS_CC);            \
    if (U_FAILURE(status)) {                                \
        intl_errors_set_custom_msg(NULL, msg, 0 TSRMLS_CC); \
        RETVAL_FALSE;                                       \
        goto end;                                           \
    }

#define UTF8_TO_UTF16(status, to, to_len, from, from_len)                        \
    do {                                                                         \
        to = NULL;                                                               \
        to_len = 0;                                                              \
        intl_convert_utf8_to_utf16(&to, &to_len, from, from_len, &status);       \
        CHECK_STATUS(status, "String conversion of " #from " to UTF-16 failed"); \
    } while (0);

#define UTF16_TO_UTF8(status, to, to_len, from, from_len)                       \
    do {                                                                        \
        to = NULL;                                                              \
        to_len = 0;                                                             \
        intl_convert_utf16_to_utf8(&to, &to_len, from, from_len, &status);      \
        CHECK_STATUS(status, "String conversion of " #from " to UTF-8 failed"); \
    } while (0);

#define UTF8_CASE_FOLD(status, to, to_len, from, from_len)                                        \
    do {                                                                                          \
        to = NULL;                                                                                \
        to_len = 0;                                                                               \
        utf8_fullcase(&to, &to_len, from, from_len, INTL_G(default_locale), UCASE_FOLD, &status); \
        CHECK_STATUS(status, "UTF-8 full case folding of " #from " failed");                      \
    } while (0);

/* case functions */

static void fullcasemapping(INTERNAL_FUNCTION_PARAMETERS, UCaseType ct)
{
    UErrorCode status;
    int locale_len = 0;
    char *locale = NULL;
    int string_len = 0;
    char *string = NULL;
    int32_t result_len = 0;
    char *result = NULL;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &string, &string_len, &locale, &locale_len)) {
        return;
    }
    if (0 == string_len) {
        RETURN_EMPTY_STRING();
    }
    if (0 == locale_len) {
        locale = INTL_G(default_locale);
    }
    status = U_ZERO_ERROR;
    utf8_fullcase(&result, &result_len, string, string_len, locale, ct, &status);
    CHECK_STATUS(status, "full case mapping failed");
    RETVAL_STRINGL(result, result_len, FALSE);

    if (FALSE) {
end:
        if (NULL != result) {
            efree(result);
        }
        RETVAL_FALSE;
    }
}

PHP_FUNCTION(utf8_toupper)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, UCASE_UPPER);
}

PHP_FUNCTION(utf8_tolower)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, UCASE_LOWER);
}

PHP_FUNCTION(utf8_totitle)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, UCASE_TITLE);
}

/* locale independant functions (full case folding based) */

static void starts_or_ends_with(INTERNAL_FUNCTION_PARAMETERS, int end)
{
    char *haystack = NULL;
    int haystack_len = 0;
    char *needle = NULL;
    int needle_len = 0;
    char *haystack_cf = NULL; /* cf = (full) case folded */
    int32_t haystack_cf_len = 0;
    char *needle_cf = NULL;
    int32_t needle_cf_len = 0;
    UErrorCode status = U_ZERO_ERROR;
    zend_bool case_insensitive = FALSE;
    UBreakIterator *ubrk = NULL;
    UText ut = UTEXT_INITIALIZER;
    unsigned char u_break_iterator_buffer[U_BRK_SAFECLONE_BUFFERSIZE];

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &haystack, &haystack_len, &needle, &needle_len, &case_insensitive)) {
        return;
    }
    RETVAL_FALSE;
    intl_error_reset(NULL TSRMLS_CC);
    if (case_insensitive) {
        UTF8_CASE_FOLD(status, haystack_cf, haystack_cf_len, haystack, haystack_len);
        UTF8_CASE_FOLD(status, needle_cf, needle_cf_len, needle, needle_len);
        haystack = haystack_cf;
        haystack_len = haystack_cf_len;
        needle = needle_cf;
        needle_len = needle_cf_len;
    }
    if (needle_len <= haystack_len && 0 == memcmp(haystack + (end ? haystack_len - needle_len : 0), needle, needle_len)) {
        ubrk = grapheme_get_break_iterator((void*) u_break_iterator_buffer, &status TSRMLS_CC);
        CHECK_STATUS(status, "cloning global UBreakIterator failed");
        utext_openUTF8(&ut, haystack, haystack_len, &status);
        CHECK_STATUS(status, "utext_open failed");
        ubrk_setUText(ubrk, &ut, &status);
        CHECK_STATUS(status, "failed binding text to UBreakIterator");
        if (ubrk_isBoundary(ubrk, (end ? haystack_len - needle_len : needle_len))) {
            RETVAL_TRUE;
        }
        utext_close(&ut);
    }
end:
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
    if (NULL != haystack_cf) {
        efree(haystack_cf);
    }
    if (NULL != needle_cf) {
        efree(needle_cf);
    }
}

PHP_FUNCTION(utf8_startswith)
{
    starts_or_ends_with(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE);
}

PHP_FUNCTION(utf8_endswith)
{
    starts_or_ends_with(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE);
}
