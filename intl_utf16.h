#ifndef INTL_UTF16_H

# define INTL_UTF16_H

# define UTF16_CP_TO_CU(ustring, ustring_len, cp_offset, cu_offset)                                           \
    do {                                                                                                      \
        if (0 != cp_offset) {                                                                                 \
            int32_t count_cp = u_countChar32(ustring, ustring_len);                                           \
            if (cp_offset < 0) {                                                                              \
                if (cp_offset < -count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    RETVAL_FALSE;                                                                             \
                    goto end;                                                                                 \
                }                                                                                             \
                cu_offset = ustring_len;                                                                      \
                U16_BACK_N(ustring, 0, cu_offset, -cp_offset);                                                \
            } else {                                                                                          \
                if (cp_offset >= count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    RETVAL_FALSE;                                                                             \
                    goto end;                                                                                 \
                }                                                                                             \
                U16_FWD_N(ustring, cu_offset, ustring_len, cp_offset);                                        \
            }                                                                                                 \
        }                                                                                                     \
    } while (0);

int32_t utf16_grapheme_to_cu(UBreakIterator *, int32_t);
int32_t utf16_cu_to_grapheme(UBreakIterator *, int32_t);
int utf16_cp_to_cu(const UChar *, int32_t, long, int32_t *, UErrorCode *);
void utf16_replace_len(UChar **, int32_t *, UChar *, int32_t, UChar *, int32_t, int32_t, int32_t, ReplacementDirection);

#endif /* !INTL_UTF16_H */
