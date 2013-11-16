#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "php_intl.h"
#include "collator.h"
#include "collator_class.h"
#include "collator_string.h"
#include "intl_convert.h"
#include "intl_utf8.h"
#include "grapheme/grapheme_util.h"

#include <unicode/usearch.h>

#undef COLLATOR_CHECK_STATUS
#define COLLATOR_CHECK_STATUS(ro, msg)                                          \
    do {                                                                        \
        intl_error_set_code(NULL, COLLATOR_ERROR_CODE(ro) TSRMLS_CC);           \
        if (U_FAILURE(COLLATOR_ERROR_CODE(ro))) {                               \
            intl_errors_set_custom_msg(COLLATOR_ERROR_P(ro), msg, 0 TSRMLS_CC); \
            goto end;                                                           \
        }                                                                       \
    } while (0);

#define UTF8_TO_UTF16(ro, to, to_len, from, from_len)                                        \
    do {                                                                                     \
        to = NULL;                                                                           \
        to_len = 0;                                                                          \
        intl_convert_utf8_to_utf16(&to, &to_len, from, from_len, COLLATOR_ERROR_CODE_P(ro)); \
        COLLATOR_CHECK_STATUS(ro, "string conversion of " #from " to UTF-16 failed");        \
    } while (0);

#define UTF16_TO_UTF8(ro, to, to_len, from, from_len)                                        \
    do {                                                                                     \
        to = NULL;                                                                           \
        to_len = 0;                                                                          \
        intl_convert_utf16_to_utf8(&to, &to_len, from, from_len, COLLATOR_ERROR_CODE_P(ro)); \
        COLLATOR_CHECK_STATUS(ro, "string conversion of " #from " to UTF-8 failed");         \
    } while (0);

static void starts_or_ends_with(INTERNAL_FUNCTION_PARAMETERS, int first)
{
    char *needle = NULL;
    int needle_len = 0;
    UChar *uneedle = NULL;
    int32_t uneedle_len = 0;
    char *haystack = NULL;
    int haystack_len = 0;
    UChar *uhaystack = NULL;
    int32_t uhaystack_len = 0;
    UStringSearch *uss = NULL;
    UBreakIterator *ubrk = NULL;
    unsigned char u_break_iterator_buffer[U_BRK_SAFECLONE_BUFFERSIZE];

    COLLATOR_METHOD_INIT_VARS
    if ( FAILURE == zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss", &object, Collator_ce_ptr, &haystack, &haystack_len, &needle, &needle_len )) {
        intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR, "collator_replace: unable to parse input params", 0 TSRMLS_CC );

        RETURN_FALSE;
    }
    COLLATOR_METHOD_FETCH_OBJECT;
    if (!co || !co->ucoll) {
        intl_error_set_code( NULL, COLLATOR_ERROR_CODE( co ) TSRMLS_CC );
        intl_errors_set_custom_msg( COLLATOR_ERROR_P( co ), "Object not initialized", 0 TSRMLS_CC );
        php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "Object not initialized");

        RETURN_FALSE;
    }

    UTF8_TO_UTF16(co, uhaystack, uhaystack_len, haystack, haystack_len);
    UTF8_TO_UTF16(co, uneedle, uneedle_len, needle, needle_len);
    uss = usearch_openFromCollator(uneedle, uneedle_len, uhaystack, uhaystack_len, co->ucoll, NULL, COLLATOR_ERROR_CODE_P( co ));
    COLLATOR_CHECK_STATUS(co, "failed creating UStringSearch");

    if (first) {
        RETVAL_BOOL(0 == usearch_first(uss, COLLATOR_ERROR_CODE_P( co )));
        COLLATOR_CHECK_STATUS(co, "failed while searching");
    } else {
        int32_t lastMatch;

        lastMatch = usearch_last(uss, COLLATOR_ERROR_CODE_P( co ));
        COLLATOR_CHECK_STATUS(co, "failed while searching");
        RETVAL_BOOL(uhaystack_len == lastMatch + usearch_getMatchedLength(uss));
    }
    if (FALSE) {
end:
        RETVAL_FALSE;
    }
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
    if (NULL != uneedle) {
        efree(uneedle);
    }
    if (NULL != uhaystack) {
        efree(uhaystack);
    }
    if (NULL != uss) {
        usearch_close(uss);
    }
}

PHP_FUNCTION(collator_startswith)
{
    starts_or_ends_with(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE);
}

PHP_FUNCTION(collator_endswith)
{
    starts_or_ends_with(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE);
}

static void collator_index(INTERNAL_FUNCTION_PARAMETERS, int search_first, int want_only_pos)
{
    int ret;
    int32_t cuoffset; /* Unit: UTF-16 CU */
    long startoffset = 0; /* Unit: grapheme */
    char *needle = NULL;
    int needle_len = 0;
    UChar *uneedle = NULL;
    int32_t uneedle_len = 0;
    char *haystack = NULL;
    int haystack_len = 0;
    UChar *uhaystack = NULL;
    int32_t uhaystack_len = 0;
    UStringSearch *uss = NULL;
    int32_t match_start = -1; /* Unit: UTF-16 CU */
    zend_bool before = FALSE;
    UBreakIterator *ubrk = NULL;
    unsigned char u_break_iterator_buffer[U_BRK_SAFECLONE_BUFFERSIZE];

    COLLATOR_METHOD_INIT_VARS
    if (want_only_pos) {
        ret = zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss|l", &object, Collator_ce_ptr, &haystack, &haystack_len, &needle, &needle_len, &startoffset);
    } else {
        ret = zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss|lb", &object, Collator_ce_ptr, &haystack, &haystack_len, &needle, &needle_len, &startoffset, &before);
    }
    if (FAILURE == ret) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "unable to parse input params", 0 TSRMLS_CC);

        RETURN_FALSE;
    }
    COLLATOR_METHOD_FETCH_OBJECT;
    if (!co || !co->ucoll) {
        intl_error_set_code( NULL, COLLATOR_ERROR_CODE( co ) TSRMLS_CC );
        intl_errors_set_custom_msg( COLLATOR_ERROR_P( co ), "Object not initialized", 0 TSRMLS_CC );
        php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "Object not initialized");

        RETURN_FALSE;
    }

    UTF8_TO_UTF16(co, uhaystack, uhaystack_len, haystack, haystack_len);
    UTF8_TO_UTF16(co, uneedle, uneedle_len, needle, needle_len);
    ubrk = grapheme_get_break_iterator((void*) u_break_iterator_buffer, COLLATOR_ERROR_CODE_P( co ) TSRMLS_CC);
    COLLATOR_CHECK_STATUS(co, "failed cloning UBreakIterator");
    ubrk_setText(ubrk, uhaystack, uhaystack_len, COLLATOR_ERROR_CODE_P( co ));
    COLLATOR_CHECK_STATUS(co, "failed binding text to UBreakIterator");
    if (0 != startoffset) {
        if (UBRK_DONE == (cuoffset = utf16_grapheme_to_cu(ubrk, startoffset))) {
            intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "offset is out of bounds", 0 TSRMLS_CC);
            goto end;
        }
    } else {
        if (search_first) {
            cuoffset = 0;
        } else {
            cuoffset = uhaystack_len;
        }
    }
    uss = usearch_openFromCollator(uneedle, uneedle_len, uhaystack, uhaystack_len, co->ucoll, ubrk, COLLATOR_ERROR_CODE_P( co ));
    COLLATOR_CHECK_STATUS(co, "failed creating UStringSearch");

    if (search_first) {
        match_start = usearch_following(uss, cuoffset, COLLATOR_ERROR_CODE_P( co ));
    } else {
        usearch_setAttribute(uss, USEARCH_OVERLAP, USEARCH_ON, COLLATOR_ERROR_CODE_P( co ));
        COLLATOR_CHECK_STATUS(co, "failed switching overlap attribute to on");
        match_start = usearch_preceding(uss, cuoffset/* + 1*/, COLLATOR_ERROR_CODE_P( co ));
    }
    COLLATOR_CHECK_STATUS(co, "failed while searching");
    if (USEARCH_DONE != match_start) {
        if (want_only_pos) {
            RETVAL_LONG((long) utf16_cu_to_grapheme(ubrk, match_start));
        } else {
            char *result = NULL;
            int32_t result_len = 0;

            if (before) {
                UTF16_TO_UTF8(co, result, result_len, uhaystack, match_start);
            } else {
                UTF16_TO_UTF8(co, result, result_len, uhaystack + match_start, uhaystack_len - match_start);
            }
            RETVAL_STRINGL(result, result_len, FALSE);
        }
    } else {
        if (want_only_pos) {
            RETVAL_LONG((long) -1);
        } else {
            RETVAL_FALSE;
        }
    }

    if (FALSE) {
end:
        RETVAL_FALSE;
    }
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
    if (NULL != uneedle) {
        efree(uneedle);
    }
    if (NULL != uhaystack) {
        efree(uhaystack);
    }
    if (NULL != uss) {
        usearch_close(uss);
    }
}

PHP_FUNCTION(collator_rindex)
{
    collator_index(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE, TRUE);
}

PHP_FUNCTION(collator_lindex)
{
    collator_index(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE, TRUE);
}

PHP_FUNCTION(collator_rfind)
{
    collator_index(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE, FALSE);
}

PHP_FUNCTION(collator_lfind)
{
    collator_index(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE, FALSE);
}

PHP_FUNCTION(collator_replace)
{
    int32_t l;
    char *result = NULL;
    int result_len = 0;
    char *search = NULL;
    int search_len = 0;
    char *replace = NULL;
    int replace_len = 0;
    char *subject = NULL;
    int subject_len = 0;
    zval *zcount = NULL;
    long count = 0;
    UChar *usearch = NULL;
    int32_t usearch_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    UStringSearch *uss = NULL;
    UChar *ureplace = NULL;
    int32_t ureplace_len = 0;
    UChar *uresult = NULL;
    int32_t uresult_len = 0;

    COLLATOR_METHOD_INIT_VARS
    if ( FAILURE == zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osss|z", &object, Collator_ce_ptr, &subject, &subject_len, &search, &search_len, &replace, &replace_len, &zcount )) {
        intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR, "collator_replace: unable to parse input params", 0 TSRMLS_CC );

        RETURN_FALSE;
    }
    COLLATOR_METHOD_FETCH_OBJECT;
    if (!co || !co->ucoll) {
        intl_error_set_code( NULL, COLLATOR_ERROR_CODE( co ) TSRMLS_CC );
        intl_errors_set_custom_msg( COLLATOR_ERROR_P( co ), "Object not initialized", 0 TSRMLS_CC );
        php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "Object not initialized");

        RETURN_FALSE;
    }

    UTF8_TO_UTF16(co, usearch, usearch_len, search, search_len);
    UTF8_TO_UTF16(co, usubject, usubject_len, subject, subject_len);
    uss = usearch_openFromCollator(usearch, usearch_len, usubject, usubject_len, co->ucoll, NULL, COLLATOR_ERROR_CODE_P( co ));
    COLLATOR_CHECK_STATUS(co, "failed creating UStringSearch");
    UTF8_TO_UTF16(co, ureplace, ureplace_len, replace, replace_len);
    uresult_len = usubject_len;
    uresult = mem_new_n(*usubject, usubject_len + 1);
    u_memcpy(uresult, usubject, usubject_len);
    uresult[uresult_len] = 0;
    for (l = usearch_first(uss, COLLATOR_ERROR_CODE_P( co )); U_SUCCESS(COLLATOR_ERROR_CODE( co )) && USEARCH_DONE != l; l = usearch_next(uss, COLLATOR_ERROR_CODE_P( co )), count++) {
        utf16_replace_len(&uresult, &uresult_len, ureplace, ureplace_len, usubject, usubject_len, l, usearch_getMatchedLength(uss), REPLACE_FORWARD);
    }
    COLLATOR_CHECK_STATUS(co, "failed while searching");
    UTF16_TO_UTF8(co, result, result_len, uresult, uresult_len);
    RETVAL_STRINGL(result, result_len, FALSE);

    if (FALSE) {
end:
        if (NULL != result) {
            efree(result);
        }
        RETVAL_FALSE;
    }
    if (NULL != uss) {
        usearch_close(uss);
    }
    if (NULL != uresult) {
        efree(uresult);
    }
    if (NULL != ureplace) {
        efree(ureplace);
    }
    if (NULL != usearch) {
        efree(usearch);
    }
    if (NULL != usubject) {
        efree(usubject);
    }
    if (ZEND_NUM_ARGS() > 3) {
        zval_dtor(zcount);
        ZVAL_LONG(zcount, count);
    }
}
