#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <php.h>
#include "php_intl.h"
#include "intl_error.h"
#include <unicode/ustring.h>
#include "intl_utf16.h"

typedef int32_t (*ubrk_start_t)(UBreakIterator *); // ubrk_first or ubrk_last
typedef int32_t (*ubrk_move_t)(UBreakIterator *);  // ubrk_next or ubrk_previous

int32_t utf16_grapheme_to_cu(UBreakIterator *ubrk, int32_t offset)
{
    int32_t i, pos;
    ubrk_start_t s;
    ubrk_move_t m;

    if (0 == offset) {
        return 0;
    }
    if (offset < 0) {
        s = ubrk_last;
        m = ubrk_previous;
        offset = -offset;
    } else {
        s = ubrk_first;
        m = ubrk_next;
    }
    if (UBRK_DONE != (pos = s(ubrk))) {
        for (i = 0; i < offset && UBRK_DONE != (pos = m(ubrk)); i++) {
            ;
        }
    }

    return pos;
}

int32_t utf16_cu_to_grapheme(UBreakIterator *ubrk, int32_t pos)
{
    int32_t l, u, offset;

    offset = 0;
    if (UBRK_DONE != (l = ubrk_first(ubrk))) {
        while (l < pos && UBRK_DONE != (u = ubrk_next(ubrk))) {
            l = u;
            ++offset;
        }
    }

    return offset;
}

void utf16_replace_len(
    UChar **ucopy, int32_t *ucopy_len,
    UChar *ureplacement, int32_t ureplacement_len,
    UChar *ustring /* UNUSED */, int32_t ustring_len, /* Used by ICU, don't alter it ! So, we work on a copy. */
    int32_t start_match_offset, int32_t match_cu_length,
    ReplacementDirection direction
) {
    int32_t diff_len;
    int32_t real_offset;

    if (REPLACE_REVERSE == direction) {
        real_offset = start_match_offset;
    } else {
        real_offset = *ucopy_len - (ustring_len - start_match_offset);
    }
    diff_len = ureplacement_len - match_cu_length;
    if (diff_len > 0) {
        *ucopy = mem_renew(*ucopy, **ucopy, *ucopy_len + diff_len + 1); // reference may no longer be valid from this point
    }
    if (ureplacement_len != match_cu_length) {
        u_memmove(*ucopy + real_offset + match_cu_length + diff_len, *ucopy + real_offset + match_cu_length, *ucopy_len - real_offset - match_cu_length);
    }
    u_memcpy(*ucopy + real_offset, ureplacement, ureplacement_len);
    *ucopy_len += diff_len;
}

int utf16_cp_to_cu(const UChar *ustring, int32_t ustring_len, long cp_offset, int32_t *cu_offset, UErrorCode *status)
{
    if (0 != cp_offset) {
        int32_t _cp_count = u_countChar32(ustring, ustring_len);
        if (cp_offset < 0) {
            if (cp_offset < -_cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FAILURE;
            }
            *cu_offset = ustring_len;
            U16_BACK_N(ustring, 0, *cu_offset, -cp_offset);
        } else {
            if (cp_offset >= _cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FAILURE;
            }
            U16_FWD_N(ustring, *cu_offset, ustring_len, cp_offset);
        }
    }

    return SUCCESS;
}
