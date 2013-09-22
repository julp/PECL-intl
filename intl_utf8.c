#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <php.h>
#include "php_intl.h"
#include "intl_error.h"
#include <unicode/ustring.h>
#include "intl_utf8.h"

int utf8_cp_to_cu(const char *string, int string_len, int32_t cp_offset, int32_t *cu_offset, UErrorCode *status)
{
    if (0 != cp_offset) {
        int32_t _cp_count = utf8_countChar32((const uint8_t *) string, string_len);
        if (cp_offset < 0) {
            if (cp_offset < -_cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FALSE;
            }
            *cu_offset = string_len;
            U8_BACK_N((const uint8_t *) string, 0, *cu_offset, -cp_offset);
        } else {
            if (cp_offset >= _cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FALSE;
            }
            U8_FWD_N(string, *cu_offset, string_len, cp_offset);
        }
    }

    return TRUE;
}

void utf8_replace_len_from_utf16(
    char **string, int *string_len,
    char *replacement, int replacement_len,
    UChar *ustring, int32_t utf16_cu_start_match_offset, int32_t utf16_cu_length,
    int32_t utf16_cp_length,
    ReplacementDirection direction
) {
    int32_t diff_len;
    int32_t utf8_match_cu_length = 0;
    int32_t utf8_cu_start_match_offset = 0;
    int32_t utf16_cp_start_match_offset = u_countChar32(ustring, utf16_cu_start_match_offset);

    if (0 == utf16_cu_length) {
        utf8_match_cu_length = 0;
    } else {
        int32_t utf16_cu_end_match_offset = utf16_cu_start_match_offset + utf16_cu_length;

        while (utf16_cu_start_match_offset < utf16_cu_end_match_offset) {
            UChar32 c;

            U16_NEXT(ustring, utf16_cu_start_match_offset, utf16_cu_end_match_offset, c);
            utf8_match_cu_length += U8_LENGTH(c); // TODO: handle surrogates(forbidden)?
        }
    }
    /* <NOTE> */
    /**
     * String is altered from start to end
     * But a replacement of a different length, create a gap. We should consider it, when there is multiple replacements
     * So, we should consider the part of string we haven't yet modified: we should recalculate offsets from end of string
     * instead its start
     * (there is many way: but it'll introduce, at least, a new parameter: an int or a pointer - last one may be "dangerous" because of realloc)
     **/
    if (REPLACE_REVERSE == direction) {
        utf8_cu_start_match_offset = 0;
        U8_FWD_N(*string, utf8_cu_start_match_offset, *string_len, utf16_cp_start_match_offset);
    } else {
        utf8_cu_start_match_offset = *string_len;
        U8_BACK_N(*string, 0, utf8_cu_start_match_offset, utf16_cp_length - utf16_cp_start_match_offset);
    }
    /* </NOTE> */
    diff_len = replacement_len - utf8_match_cu_length;
    if (diff_len > 0) {
        *string = mem_renew(*string, **string, *string_len + diff_len + 1); // reference may no longer be valid from this point
    }
    if (replacement_len != utf8_match_cu_length) {
        memmove(*string + utf8_cu_start_match_offset + utf8_match_cu_length + diff_len, *string + utf8_cu_start_match_offset + utf8_match_cu_length, *string_len - utf8_cu_start_match_offset - utf8_match_cu_length);
    }
    memcpy(*string + utf8_cu_start_match_offset, replacement, replacement_len);
    *string_len += diff_len;
}
