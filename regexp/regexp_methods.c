#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#define WITH_GRAPHEME 1 /* incomplete/experimental */

#include "php_intl.h"
#include "intl_data.h"
#include "intl_convert.h"
#include "intl_utf8.h"
#include "intl_utf16.h"
#include "regexp.h"
#include "regexp_class.h"
#include "regexp_methods.h"
#ifdef WITH_GRAPHEME
# include "grapheme/grapheme_util.h" /* grapheme consistency */
#endif /* WITH_GRAPHEME */
#include "transliterator/transliterator.h" /* parse error handling */

#include <zend_exceptions.h>

#define REGEXP_PARSE_VOID_ARGS(reset)                                                                                     \
    do {                                                                                                                  \
        if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Regexp_ce_ptr)) { \
            intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);                                 \
            RETURN_FALSE;                                                                                                 \
        }                                                                                                                 \
        ro = zend_object_store_get_object(object TSRMLS_CC);                                                              \
        if (NULL == ro) {                                                                                                 \
            RETURN_FALSE;                                                                                                 \
        }                                                                                                                 \
        REGEXP_METHOD_FETCH_OBJECT(reset);                                                                                \
    } while(0);

#define REGEXP_RESET(ro)                                                           \
    do {                                                                           \
        UErrorCode status = U_ZERO_ERROR;                                          \
        uregex_setText(ro->uregex, UREGEXP_FAKE_USTR, &status);                    \
        if (U_FAILURE(status)) {                                                   \
            intl_error_set(NULL, status, "internal resetting error", 0 TSRMLS_CC); \
        }                                                                          \
    } while (0);

#define UTF8_TO_UTF16(ro, to, to_len, from, from_len)                                      \
    do {                                                                                   \
        to = NULL;                                                                         \
        to_len = 0;                                                                        \
        intl_convert_utf8_to_utf16(&to, &to_len, from, from_len, REGEXP_ERROR_CODE_P(ro)); \
        REGEXP_CHECK_STATUS(ro, "string conversion of " #from " to UTF-16 failed");        \
    } while (0);

#define UTF16_TO_UTF8(ro, to, to_len, from, from_len)                                      \
    do {                                                                                   \
        to = NULL;                                                                         \
        to_len = 0;                                                                        \
        intl_convert_utf16_to_utf8(&to, &to_len, from, from_len, REGEXP_ERROR_CODE_P(ro)); \
        REGEXP_CHECK_STATUS(ro, "string conversion of " #from " to UTF-8 failed");         \
    } while (0);

#define REGEXP_SET_UTF16_SUBJECT(ro, usubject, usubject_len)

#define REGEXP_SET_UTF8_SUBJECT(ro, usubject, usubject_len) /* we already have done the translation */

#define REGEXP_GROUP_START(ro, group, l)                                                                                     \
    do {                                                                                                                     \
        l = uregex_start(ro->uregex, group, REGEXP_ERROR_CODE_P(ro));                                                        \
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);                                                          \
        if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {                                                                              \
            intl_errors_setf_custom_msg(REGEXP_ERROR_P(ro) TSRMLS_CC, "error extracting start of group capture #%d", group); \
            goto end;                                                                                                        \
        }                                                                                                                    \
    } while (0);

#define REGEXP_GROUP_END(ro, group, u)                                                                                     \
    do {                                                                                                                   \
        u = uregex_end(ro->uregex, group, REGEXP_ERROR_CODE_P(ro));                                                        \
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);                                                        \
        if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {                                                                            \
            intl_errors_setf_custom_msg(REGEXP_ERROR_P(ro) TSRMLS_CC, "error extracting end of group capture #%d", group); \
            goto end;                                                                                                      \
        }                                                                                                                  \
    } while (0);

static const UChar _UREGEXP_FAKE_USTR[] = { 0 };
#define UREGEXP_FAKE_USTR _UREGEXP_FAKE_USTR, 0

static void regexp_ctor(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *object;
    Regexp_object *ro;
    char *pattern;
    int32_t pattern_len;
    UChar *upattern = NULL;
    int32_t upattern_len = 0;
    zval *zflags = NULL;
    uint32_t flags = 0;
    UParseError pe = { -1, -1, {0}, {0} };

    intl_error_reset(NULL TSRMLS_CC);
    object = return_value;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &pattern, &pattern_len, &zflags)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        zval_dtor(object);
        RETURN_NULL();
    }
    if (NULL != zflags) {
        switch (Z_TYPE_P(zflags)) {
            case IS_LONG:
                flags = (uint32_t) Z_LVAL_P(zflags);
                if (0 != (flags & ~(UREGEX_CASE_INSENSITIVE|UREGEX_MULTILINE|UREGEX_DOTALL|UREGEX_COMMENTS|UREGEX_UWORD))) {
                    intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "invalid flag", 0 TSRMLS_CC);
                    zval_dtor(object);
                    RETURN_NULL();
                }
                break;
            case IS_STRING:
            {
                const char *p;

                for (p = Z_STRVAL_P(zflags); '\0' != *p; p++) {
                    switch (*p) {
                        case 'i': flags |= UREGEX_CASE_INSENSITIVE; break;
                        case 'm': flags |= UREGEX_MULTILINE;        break;
                        case 's': flags |= UREGEX_DOTALL;           break;
                        case 'x': flags |= UREGEX_COMMENTS;         break;
                        case 'w': flags |= UREGEX_UWORD;            break;
                        default:
                            intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "invalid modifier", 0 TSRMLS_CC);
                            zval_dtor(object);
                            RETURN_NULL();
                    }
                }
                break;
            }
            default:
                intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
                zval_dtor(object);
                RETURN_NULL();
        }
    }
    ro = (Regexp_object *) zend_object_store_get_object(object TSRMLS_CC);
    intl_convert_utf8_to_utf16(&upattern, &upattern_len, pattern, pattern_len, REGEXP_ERROR_CODE_P(ro));
    INTL_CTOR_CHECK_STATUS(ro, "string conversion of pattern to UTF-16 failed");
    ro->uregex = uregex_open(upattern, upattern_len, flags, &pe, REGEXP_ERROR_CODE_P(ro));
    efree(upattern);
    if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);
        if (-1 != pe.line) {
            smart_str parse_error_str;

            parse_error_str = transliterator_parse_error_to_string(&pe);
            intl_errors_setf_custom_msg(NULL, TSRMLS_CC "unable to compile ICU regular expression, %s", parse_error_str.c);
            smart_str_free(&parse_error_str);
        } else {
            intl_error_set_custom_msg(NULL, "unable to compile ICU regular expression", 0 TSRMLS_CC);
        }
        zval_dtor(object);
        RETURN_NULL();
    }
}

PHP_FUNCTION(regexp_create)
{
    object_init_ex(return_value, Regexp_ce_ptr);
    regexp_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}


PHP_METHOD(Regexp, __construct)
{
    return_value = getThis();
    regexp_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static void add_offset_pair(zval *result, char *string, int string_len, int offset)
{
    zval *match_pair;

    ALLOC_ZVAL(match_pair);
    array_init(match_pair);
    INIT_PZVAL(match_pair);

    add_next_index_stringl(match_pair, string, string_len, FALSE);
    add_next_index_long(match_pair, offset);
    zend_hash_next_index_insert(Z_ARRVAL_P(result), &match_pair, sizeof(zval *), NULL);
}

PHP_FUNCTION(regexp_match)
{
    UBool res = FALSE;
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    zval *subpats = NULL;
    int32_t start_cu_offset = 0;
    long start_offset = 0;
    long flags = 0;
#ifdef WITH_GRAPHEME
    UBreakIterator *ubrk = NULL;
    unsigned char u_break_iterator_buffer[U_BRK_SAFECLONE_BUFFERSIZE];
#endif /* WITH_GRAPHEME */

    REGEXP_METHOD_INIT_VARS

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|zll", &object, Regexp_ce_ptr, &subject, &subject_len, &subpats, &flags, &start_offset)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if (NULL != subpats) {
        zval_dtor(subpats);
        array_init(subpats);
    }
    if (0 != (flags & ~(OFFSET_CAPTURE))) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "invalid flag(s)", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
#ifdef WITH_GRAPHEME
    ubrk = grapheme_get_break_iterator((void*) u_break_iterator_buffer, REGEXP_ERROR_CODE_P(ro) TSRMLS_CC);
    REGEXP_CHECK_STATUS(ro, "failed cloning UBreakIterator");
    ubrk_setText(ubrk, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "failed binding text to UBreakIterator");
    if (0 != start_offset) {
        if (UBRK_DONE == (start_cu_offset = utf16_grapheme_to_cu(ubrk, start_offset))) {
            intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "offset is out of bounds", 0 TSRMLS_CC);
            goto end;
        }
    }
#else
    UTF16_CP_TO_CU(usubject, usubject_len, start_offset, start_cu_offset);
#endif /* WITH_GRAPHEME */
    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");
#ifdef WITH_GRAPHEME
    if (0 != start_offset) {
        uregex_reset(ro->uregex, start_cu_offset, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error setting start region");
    }
    while (!res && uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro))) {
        int32_t l, u;

        REGEXP_GROUP_START(ro, 0, l);
        REGEXP_GROUP_END(ro, 0, u);
        if (ubrk_isBoundary(ubrk, l) && ubrk_isBoundary(ubrk, u)) {
            res = TRUE;
        }
    }
#else
    res = uregex_find(ro->uregex, start_cu_offset, REGEXP_ERROR_CODE_P(ro));
#endif /* WITH_GRAPHEME */
    REGEXP_CHECK_STATUS(ro, "error finding pattern");
    if (res && NULL != subpats) {
        int i;
        char *group;
        int group_len;
        int32_t group_count;
        int32_t l, u;

        group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error counting groups");
        for (i = 0; i <= group_count; i++) {
            REGEXP_GROUP_START(ro, i, l);
            REGEXP_GROUP_END(ro, i, u);
            UTF16_TO_UTF8(ro, group, group_len, usubject + l, u - l);
            if (!(flags & OFFSET_CAPTURE)) {
                add_index_stringl(subpats, i, group, group_len, FALSE);
            } else {
#ifdef WITH_GRAPHEME
                add_offset_pair(subpats, group, group_len, utf16_cu_to_grapheme(ubrk, l));
#else
                add_offset_pair(subpats, group, group_len, u_countChar32(usubject, l));
#endif /* WITH_GRAPHEME */
            }
        }
    }

    if (FALSE) {
end:
        if (NULL != subpats) {
            zval_dtor(subpats);
            array_init(subpats);
        }
    }
#ifdef WITH_GRAPHEME
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
#endif /* WITH_GRAPHEME */
    if (NULL != usubject) {
        REGEXP_RESET(ro);
        efree(usubject);
    }
    RETURN_BOOL(res);
}

PHP_FUNCTION(regexp_match_all)
{
    int i;
    int match_count = 0;
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    zval *subpats = NULL;
    zval **Zres = NULL;
    int32_t start_cu_offset = 0;
    long start_offset = 0;
    int32_t group_count = 0;
    long flags = 0;
#ifdef WITH_GRAPHEME
    UBreakIterator *ubrk = NULL;
    unsigned char u_break_iterator_buffer[U_BRK_SAFECLONE_BUFFERSIZE];
#endif /* WITH_GRAPHEME */
    REGEXP_METHOD_INIT_VARS

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|zll", &object, Regexp_ce_ptr, &subject, &subject_len, &subpats, &flags, &start_offset)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if (0 != (flags & ~(OFFSET_CAPTURE|MATCH_ALL_PATTERN_ORDER|MATCH_ALL_SET_ORDER))) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "invalid flag(s)", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
#ifdef WITH_GRAPHEME
    ubrk = grapheme_get_break_iterator((void*) u_break_iterator_buffer, REGEXP_ERROR_CODE_P(ro) TSRMLS_CC);
    REGEXP_CHECK_STATUS(ro, "failed cloning UBreakIterator");
    ubrk_setText(ubrk, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "failed binding text to UBreakIterator");
    if (0 != start_offset) {
        if (UBRK_DONE == (start_cu_offset = utf16_grapheme_to_cu(ubrk, start_offset))) {
            intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "offset is out of bounds", 0 TSRMLS_CC);
            goto end;
        }
    }
#else
    UTF16_CP_TO_CU(usubject, usubject_len, start_offset, start_cu_offset);
#endif /* WITH_GRAPHEME */
    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");
    if (NULL != subpats) {
        zval_dtor(subpats);
        array_init(subpats);
        group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error counting groups");

        if ((flags & MATCH_ALL_PATTERN_ORDER)) {
            Zres = mem_new_n(*Zres, group_count + 1);
            for (i = 0; i <= group_count; i++) {
                ALLOC_ZVAL(Zres[i]);
                array_init(Zres[i]);
                INIT_PZVAL(Zres[i]);
            }
        }
    }
    if (0 != start_offset) {
        uregex_reset(ro->uregex, start_cu_offset, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error setting start region");
    }
    while (uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro))) {
#ifdef WITH_GRAPHEME
        int32_t l, u;

        i = 0;
        REGEXP_GROUP_START(ro, i, l);
        REGEXP_GROUP_END(ro, i, u);
        if (!ubrk_isBoundary(ubrk, l) || !ubrk_isBoundary(ubrk, u)) {
            continue;
        }
#endif /* WITH_GRAPHEME */
        match_count++;
        if (NULL != subpats) {
            char *group;
            int group_len;
#ifndef WITH_GRAPHEME
            int32_t l, u;
#endif /* !WITH_GRAPHEME */
            zval *match_groups;

            if (!(flags & MATCH_ALL_PATTERN_ORDER)) {
                ALLOC_ZVAL(match_groups);
                array_init(match_groups);
                INIT_PZVAL(match_groups);
            }
            for (i = 0; i <= group_count; i++) {
                REGEXP_GROUP_START(ro, i, l);
                REGEXP_GROUP_END(ro, i, u);
                UTF16_TO_UTF8(ro, group, group_len, usubject + l, u - l);
                if ((flags & MATCH_ALL_PATTERN_ORDER)) {
                    if (!(flags & OFFSET_CAPTURE)) {
                        add_next_index_stringl(Zres[i], group, group_len, FALSE);
                    } else {
#ifdef WITH_GRAPHEME
                        add_offset_pair(Zres[i], group, group_len, utf16_cu_to_grapheme(ubrk, l));
#else
                        add_offset_pair(Zres[i], group, group_len, u_countChar32(usubject, l));
#endif /* WITH_GRAPHEME */
                    }
                } else {
                    if (!(flags & OFFSET_CAPTURE)) {
                        add_index_stringl(match_groups, i, group, group_len, FALSE);
                    } else {
#ifdef WITH_GRAPHEME
                        add_offset_pair(match_groups, group, group_len, utf16_cu_to_grapheme(ubrk, l));
#else
                        add_offset_pair(match_groups, group, group_len, u_countChar32(usubject, l));
#endif /* WITH_GRAPHEME */
                    }
                }
            }
            if (!(flags & MATCH_ALL_PATTERN_ORDER)) {
                zend_hash_next_index_insert(Z_ARRVAL_P(subpats), &match_groups, sizeof(zval *), NULL);
            }
        }
    }
    REGEXP_CHECK_STATUS(ro, "error finding pattern");
    if ((flags & MATCH_ALL_PATTERN_ORDER)) {
        for (i = 0; i <= group_count; i++) {
            zend_hash_next_index_insert(Z_ARRVAL_P(subpats), &Zres[i], sizeof(zval *), NULL);
        }
        efree(Zres);
    }
    RETVAL_LONG(match_count);

    if (FALSE) {
end:
        if (NULL != subpats) {
            zval_dtor(subpats);
            array_init(subpats);
        }
        RETVAL_FALSE;
    }
#ifdef WITH_GRAPHEME
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
#endif /* WITH_GRAPHEME */
    if (NULL != usubject) {
        REGEXP_RESET(ro);
        efree(usubject);
    }
}

PHP_FUNCTION(regexp_get_pattern)
{
    char *pattern = NULL;
    int pattern_len = 0;
    const UChar *upattern;
    int32_t upattern_len = 0;
    REGEXP_METHOD_INIT_VARS

    REGEXP_PARSE_VOID_ARGS(TRUE);

    upattern = uregex_pattern(ro->uregex, &upattern_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error retrieving pattern");

    UTF16_TO_UTF8(ro, pattern, pattern_len, upattern, upattern_len);
    RETURN_STRINGL(pattern, pattern_len, 0);

end:
    RETURN_FALSE;
}

PHP_FUNCTION(regexp_get_flags)
{
    int32_t flags = 0;
    REGEXP_METHOD_INIT_VARS

    REGEXP_PARSE_VOID_ARGS(TRUE);

    flags = uregex_flags(ro->uregex, REGEXP_ERROR_CODE_P(ro));

    RETURN_LONG((long) flags);

end:
    RETURN_FALSE;
}

PHP_FUNCTION(regexp_replace)
{
    UErrorCode status = U_ZERO_ERROR; /* preflighting */

    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;

    char *replacement = NULL;
    int replacement_len = 0;
    UChar *ureplacement = NULL;
    int32_t ureplacement_len = 0;

    char *result = NULL;
    int result_len = 0;
    UChar *uresult_p = NULL;
    const UChar *uresult = NULL;
    int32_t uresult_len = 0;
    int32_t uresult_size = 0;

    zval **zcount = NULL;
    long limit = INT_MAX;
    long match_count = 0;
#ifdef WITH_GRAPHEME
    UBreakIterator *ubrk = NULL;
    unsigned char u_break_iterator_buffer[U_BRK_SAFECLONE_BUFFERSIZE];
#endif /* WITH_GRAPHEME */
    REGEXP_METHOD_INIT_VARS

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss|lZ", &object, Regexp_ce_ptr, &subject, &subject_len, &replacement, &replacement_len, &limit, &zcount)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
    UTF8_TO_UTF16(ro, ureplacement, ureplacement_len, replacement, replacement_len);

    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");
#ifdef WITH_GRAPHEME
    ubrk = grapheme_get_break_iterator((void*) u_break_iterator_buffer, REGEXP_ERROR_CODE_P(ro) TSRMLS_CC);
    REGEXP_CHECK_STATUS(ro, "failed cloning UBreakIterator");
    ubrk_setText(ubrk, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "failed binding text to UBreakIterator");
#endif /* WITH_GRAPHEME */
    while (match_count < limit && uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro))) {
        int32_t l0, u0;

        REGEXP_GROUP_START(ro, 0, l0);
        REGEXP_GROUP_END(ro, 0, u0);
#ifdef WITH_GRAPHEME
        if (!ubrk_isBoundary(ubrk, l0) || !ubrk_isBoundary(ubrk, u0)) {
            continue;
        }
#endif /* WITH_GRAPHEME */
        ++match_count;
        /* Note: for preflighting we need a specific UErrorCode */
        uresult_size += uregex_appendReplacement(ro->uregex, ureplacement, ureplacement_len, &uresult_p, &uresult_len, &status);
    }
    if (0 == match_count) {
        RETVAL_STRINGL(subject, subject_len, TRUE);
    } else {
        /* Note: for preflighting we need a specific UErrorCode */
        uresult_size += uregex_appendTail(ro->uregex, &uresult_p, &uresult_len, &status);
        if (U_BUFFER_OVERFLOW_ERROR != status) {
            intl_errors_set_custom_msg(REGEXP_ERROR_P(ro), "preflighting failed", 0 TSRMLS_CC);
            goto end;
        }
        match_count = 0;
        intl_error_reset(REGEXP_ERROR_P(ro) TSRMLS_CC);
        uresult = uresult_p = mem_new_n(*uresult, uresult_size + 1);
        uregex_reset(ro->uregex, 0, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error while resetting internal offset");
        while (match_count < limit && U_SUCCESS(REGEXP_ERROR_CODE(ro)) && uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro))) {
            int32_t l0, u0;

            REGEXP_GROUP_START(ro, 0, l0);
            REGEXP_GROUP_END(ro, 0, u0);
#ifdef WITH_GRAPHEME
            if (!ubrk_isBoundary(ubrk, l0) || !ubrk_isBoundary(ubrk, u0)) {
                continue;
            }
#endif /* WITH_GRAPHEME */
            ++match_count;
            uresult_len += uregex_appendReplacement(ro->uregex, ureplacement, ureplacement_len, &uresult_p, &uresult_size, REGEXP_ERROR_CODE_P(ro));
        }
        REGEXP_CHECK_STATUS(ro, "error while replacing");
        uresult_len += uregex_appendTail(ro->uregex, &uresult_p, &uresult_size, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error while replacing (last step)");
        UTF16_TO_UTF8(ro, result, result_len, uresult, uresult_len);
        RETVAL_STRINGL(result, result_len, FALSE);
    }

    if (ZEND_NUM_ARGS() == 4) {
        zval_dtor(*zcount);
        ZVAL_LONG(*zcount, match_count);
    }
    if (FALSE) {
end:
        if (NULL != result) {
            efree(result);
        }
        RETVAL_FALSE;
    }
#ifdef WITH_GRAPHEME
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
#endif /* WITH_GRAPHEME */
    if (NULL != ureplacement) {
        efree(ureplacement);
    }
    if (NULL != usubject) {
        REGEXP_RESET(ro);
        efree(usubject);
    }
    if (NULL != uresult) {
        efree((void *) uresult);
    }
}

PHP_FUNCTION(regexp_replace_callback)
{
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    int32_t usubject_cp_len = 0;
    zval **Zcallback;
    char *callback = NULL;
    zval *match_groups = NULL;
    zval **zargs[1];
    zval *retval_ptr;
    zval **zcount = NULL;
    long limit = INT_MAX;
    char *result = NULL;
    int result_len = 0;
    int32_t group_count = 0;
    int match_count = 0;
#ifdef WITH_GRAPHEME
    UBreakIterator *ubrk = NULL;
    unsigned char u_break_iterator_buffer[U_BRK_SAFECLONE_BUFFERSIZE];
#endif /* WITH_GRAPHEME */
    REGEXP_METHOD_INIT_VARS

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OsZ|lZ", &object, Regexp_ce_ptr, &subject, &subject_len, &Zcallback, &limit, &zcount)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    REGEXP_METHOD_FETCH_OBJECT(TRUE);
    if (!zend_is_callable(*Zcallback, 0, &callback TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "valid callback required", callback);
        //intl_errors_set_custom_msg(REGEXP_ERROR_P(ro), "regexp_replace_callback: requires a valid callback", 0 TSRMLS_CC);
        efree(callback);
        RETURN_FALSE;
    }
    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
    usubject_cp_len = u_countChar32(usubject, usubject_len);
    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");
    group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error counting groups");
#ifdef WITH_GRAPHEME
    ubrk = grapheme_get_break_iterator((void*) u_break_iterator_buffer, REGEXP_ERROR_CODE_P(ro) TSRMLS_CC);
    REGEXP_CHECK_STATUS(ro, "failed cloning UBreakIterator");
    ubrk_setText(ubrk, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "failed binding text to UBreakIterator");
#endif /* WITH_GRAPHEME */
    MAKE_STD_ZVAL(match_groups);
    array_init(match_groups);
    zargs[0] = &match_groups;
    result = estrndup(subject, result_len = subject_len);
    while (match_count < limit && uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro))) {
        int i;
        char *group;
        int group_len;
        int32_t l, u, l0, u0;

        REGEXP_GROUP_START(ro, 0, l0);
        REGEXP_GROUP_END(ro, 0, u0);
#ifdef WITH_GRAPHEME
        if (!ubrk_isBoundary(ubrk, l0) || !ubrk_isBoundary(ubrk, u0)) {
            continue;
        }
#endif /* WITH_GRAPHEME */
        match_count++;

        UTF16_TO_UTF8(ro, group, group_len, usubject + l0, u0 - l0);
        add_index_stringl(match_groups, 0, group, group_len, FALSE);
        for (i = 1; i <= group_count; i++) {
            REGEXP_GROUP_START(ro, i, l);
            REGEXP_GROUP_END(ro, i, u);
            UTF16_TO_UTF8(ro, group, group_len, usubject + l, u - l);
            add_index_stringl(match_groups, i, group, group_len, FALSE);
        }
        if (SUCCESS == call_user_function_ex(EG(function_table), NULL, *Zcallback, &retval_ptr, 1, zargs, 0, NULL TSRMLS_CC) && NULL != retval_ptr) {
            convert_to_string_ex(&retval_ptr);
            utf8_replace_len_from_utf16(&result, &result_len, Z_STRVAL_P(retval_ptr), Z_STRLEN_P(retval_ptr), usubject, l0, u0 - l0, usubject_cp_len, REPLACE_FORWARD);
            zval_ptr_dtor(&retval_ptr);
        } else {
            if (!EG(exception)) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to call custom replacement callback");
            }
            goto end; // is it appropriate?
        }
        zend_hash_clean(Z_ARRVAL_P(match_groups)); // is it better/faster than overwrite/s?
    }
    result[result_len] = '\0';
    RETVAL_STRINGL(result, result_len, FALSE);

    if (ZEND_NUM_ARGS() == 4) {
        zval_dtor(*zcount);
        ZVAL_LONG(*zcount, match_count);
    }
    if (FALSE) {
end:
        if (NULL != result) {
            efree(result);
        }
        RETVAL_FALSE;
    }
#ifdef WITH_GRAPHEME
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
#endif /* WITH_GRAPHEME */
    if (NULL != usubject) {
        REGEXP_RESET(ro);
        efree(usubject);
    }
    if (NULL != match_groups) {
        zval_ptr_dtor(&match_groups);
    }
    if (NULL != callback) {
        efree(callback);
    }
}

PHP_FUNCTION(regexp_split)
{
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    long limit = INT_MAX;
    int32_t last = 0;
    char *group = NULL;
    int group_len = 0;
    long flags = 0;
    int32_t group_count = 0;
    int i;
#ifdef WITH_GRAPHEME
    UBreakIterator *ubrk = NULL;
    unsigned char u_break_iterator_buffer[U_BRK_SAFECLONE_BUFFERSIZE];
#endif /* WITH_GRAPHEME */
    REGEXP_METHOD_INIT_VARS

    array_init(return_value);

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|ll", &object, Regexp_ce_ptr, &subject, &subject_len, &limit, &flags)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        goto end;
    }
    if (0 != (flags & ~(OFFSET_CAPTURE|SPLIT_NO_EMPTY|SPLIT_DELIM_CAPTURE))) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "invalid flag(s)", 0 TSRMLS_CC);
        goto end;
    }
    if (limit <= 0) {
        limit = INT_MAX;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);

    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");
    if (0 != (flags & SPLIT_DELIM_CAPTURE)) {
        group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error counting groups");
    }
#ifdef WITH_GRAPHEME
    ubrk = grapheme_get_break_iterator((void*) u_break_iterator_buffer, REGEXP_ERROR_CODE_P(ro) TSRMLS_CC);
    REGEXP_CHECK_STATUS(ro, "failed cloning UBreakIterator");
    ubrk_setText(ubrk, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "failed binding text to UBreakIterator");
#endif /* WITH_GRAPHEME */
    /* We don't use uregex_split, it has few "limitations" */
    for (i = 1; i < limit && uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro)); /* NOP */) {
        int32_t l, u;

        REGEXP_GROUP_START(ro, 0, l);
        REGEXP_GROUP_END(ro, 0, u);
#ifdef WITH_GRAPHEME
        if (!ubrk_isBoundary(ubrk, l) || !ubrk_isBoundary(ubrk, u)) {
            continue;
        }
#endif /* WITH_GRAPHEME */
        ++i;
        if (!(flags & SPLIT_NO_EMPTY) || last < l) {
            UTF16_TO_UTF8(ro, group, group_len, usubject + last, l - last);
            if (!(flags & OFFSET_CAPTURE)) {
                add_next_index_stringl(return_value, group, group_len, FALSE);
            } else {
#ifdef WITH_GRAPHEME
                add_index_stringl(return_value, utf16_cu_to_grapheme(ubrk, last), group, group_len, FALSE);
#else
                add_index_stringl(return_value, u_countChar32(usubject, last), group, group_len, FALSE);
#endif /* WITH_GRAPHEME */
            }
        }
        if (0 != (flags & SPLIT_DELIM_CAPTURE)) {
            int j;
            int32_t gu, gl;

            for (j = 1; j <= group_count; j++) {
                REGEXP_GROUP_START(ro, j, gl);
                REGEXP_GROUP_END(ro, j, gu);
                UTF16_TO_UTF8(ro, group, group_len, usubject + gl, gu - gl);
                if (!(flags & OFFSET_CAPTURE)) {
                    add_next_index_stringl(return_value, group, group_len, FALSE);
                } else {
#ifdef WITH_GRAPHEME
                    add_index_stringl(return_value, utf16_cu_to_grapheme(ubrk, gl), group, group_len, FALSE);
#else
                    add_index_stringl(return_value, u_countChar32(usubject, gl), group, group_len, FALSE);
#endif /* WITH_GRAPHEME */
                }
            }
        }
        last = u;
    }
    if (!(flags & SPLIT_NO_EMPTY) || last < usubject_len) {
        UTF16_TO_UTF8(ro, group, group_len, usubject + last, usubject_len - last);
        if (!(flags & OFFSET_CAPTURE)) {
            add_next_index_stringl(return_value, group, group_len, FALSE);
        } else {
#ifdef WITH_GRAPHEME
            add_index_stringl(return_value, utf16_cu_to_grapheme(ubrk, last), group, group_len, FALSE);
#else
            add_index_stringl(return_value, u_countChar32(usubject, last), group, group_len, FALSE);
#endif /* WITH_GRAPHEME */
        }
    }

    if (FALSE) {
end:
        zval_dtor(return_value);
        RETVAL_FALSE;
    }
#ifdef WITH_GRAPHEME
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
#endif /* WITH_GRAPHEME */
    if (NULL != usubject) {
        REGEXP_RESET(ro);
        efree(usubject);
    }
}

PHP_FUNCTION(regexp_get_error_code)
{
    REGEXP_METHOD_INIT_VARS
    REGEXP_PARSE_VOID_ARGS(FALSE);

    RETURN_LONG((long) REGEXP_ERROR_CODE(ro));
}

PHP_FUNCTION(regexp_get_error_message)
{
    const char *message = NULL;

    REGEXP_METHOD_INIT_VARS
    REGEXP_PARSE_VOID_ARGS(FALSE);

    message = intl_error_get_message(REGEXP_ERROR_P(ro) TSRMLS_CC);
    RETURN_STRING(message, 0);
}
