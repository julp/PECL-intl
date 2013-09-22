#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "php.h"
#include "php_intl.h"
#include "intl_data.h"
#include <unicode/utypes.h>
#include <unicode/uchar.h>
#include <unicode/ucasemap.h>
#include "intl_case.h"

#define mem_new(type) \
    emalloc((sizeof(type)))

#define mem_new_n(type, n) \
    emalloc((sizeof(type) * (n)))

ZEND_EXTERN_MODULE_GLOBALS(intl)

typedef int32_t (*u16_func_full_case_mapping_t)(UChar *, int32_t, const UChar *, int32_t, const char *, UErrorCode *);
typedef int32_t (*u8_func_full_case_mapping_t)(UCaseMap *, char *, int32_t, const char *, int32_t, UErrorCode *);

static int32_t u_strToTitleWithoutBI(UChar *dest, int32_t destCapacity, const UChar *src, int32_t srcLength, const char *locale, UErrorCode *status)
{
    return u_strToTitle(dest, destCapacity, src, srcLength, NULL, locale, status);
}

static int32_t _u_strFoldCase(UChar *dest, int32_t destCapacity, const UChar *src, int32_t srcLength, const char *locale, UErrorCode *status)
{
    return u_strFoldCase(dest, destCapacity, src, srcLength, INTL_G(turkic_casefolding) ? U_FOLD_CASE_EXCLUDE_SPECIAL_I : U_FOLD_CASE_DEFAULT, status);
}

struct case_mapping_t {
    u8_func_full_case_mapping_t u8_func;
    u16_func_full_case_mapping_t u16_func;
};

static const struct case_mapping_t unicode_case_mapping[UCASE_COUNT] = {
    { NULL,                                                NULL },
    { (u8_func_full_case_mapping_t) ucasemap_utf8FoldCase, _u_strFoldCase },
    { (u8_func_full_case_mapping_t) ucasemap_utf8ToLower,  u_strToLower },
    { (u8_func_full_case_mapping_t) ucasemap_utf8ToUpper,  u_strToUpper },
    { ucasemap_utf8ToTitle,                                u_strToTitleWithoutBI }
};

void utf8_fullcase(
    char **target, int32_t *target_len,
    const char *src, int src_len,
    const char *locale,
    UCaseType ct,
    UErrorCode *status
) {
    UCaseMap *cm;

    *status = U_ZERO_ERROR;
    if (ct < UCASE_NONE || ct >= UCASE_COUNT) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    if (UCASE_NONE == ct) {
        *target = (char *) src;
        *target_len = src_len;
        return;
    }
    if (0 == src_len) {
        *target = mem_new(**target);
        **target = '\0';
        *target_len = 0;
        return;
    }
    cm = ucasemap_open(locale, INTL_G(turkic_casefolding) ? U_FOLD_CASE_EXCLUDE_SPECIAL_I : U_FOLD_CASE_DEFAULT, status);
    if (U_FAILURE(*status)) {
        return;
    }
    *target_len = unicode_case_mapping[ct].u8_func(cm, NULL, 0, src, src_len, status);
    if (U_BUFFER_OVERFLOW_ERROR != *status) {
        ucasemap_close(cm);
        return;
    }
    *status = U_ZERO_ERROR;
    *target = mem_new_n(**target, *target_len + 1);
    /* *target_len = */unicode_case_mapping[ct].u8_func(cm, *target, *target_len + 1, src, src_len, status);
    if (U_FAILURE(*status)) {
        efree(*target);
        *target = NULL;
        *target_len = 0;
    } else {
        *(*target + *target_len) = '\0';
        assert(U_ZERO_ERROR == *status);
    }
    ucasemap_close(cm);
}

void utf16_fullcase(
    UChar **target, int32_t *target_len,
    const UChar *src, int src_len,
    const char *locale,
    UCaseType ct,
    UErrorCode *status
) {
    int32_t target_size;
    int tries = 0;

    *target = NULL;
    *target_len = 0;
    *status = U_ZERO_ERROR;
    if (ct < UCASE_NONE || ct >= UCASE_COUNT) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    if (UCASE_NONE == ct) {
        *target = (UChar *) src;
        *target_len = src_len;
        return;
    }
    *target_len = unicode_case_mapping[ct].u16_func(NULL, 0, src, src_len, locale, status);
    if (U_BUFFER_OVERFLOW_ERROR != *status && U_STRING_NOT_TERMINATED_WARNING != *status) {
        return;
    }
    *status = U_ZERO_ERROR;
    *target = mem_new_n(**target, *target_len + 1);
    /* *target_len = */unicode_case_mapping[ct].u16_func(*target, *target_len + 1, src, src_len, locale, status);
    if (U_FAILURE(*status)) {
        efree(*target);
        *target = NULL;
    } else {
        *(*target + *target_len) = 0;
        assert(U_ZERO_ERROR == *status);
    }
}
