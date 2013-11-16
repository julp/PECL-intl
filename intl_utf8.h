#ifndef INTL_UTF8_H

# define INTL_UTF8_H

# define UTF8_CP_TO_CU(string, string_len, cp_offset, cu_offset)                                              \
    do {                                                                                                      \
        if (0 != cp_offset) {                                                                                 \
            int32_t count_cp = utf8_countChar32((const uint8_t *) string, string_len);                        \
            if (cp_offset < 0) {                                                                              \
                if (cp_offset < -count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    goto end;                                                                                 \
                }                                                                                             \
                cu_offset = string_len;                                                                       \
                U8_BACK_N((const uint8_t *) string, 0, cu_offset, -cp_offset);                                \
            } else {                                                                                          \
                if (cp_offset >= count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    goto end;                                                                                 \
                }                                                                                             \
                U8_FWD_N((const uint8_t *) string, cu_offset, string_len, cp_offset);                         \
            }                                                                                                 \
        }                                                                                                     \
    } while (0);

int utf8_cp_to_cu(const char *, int, int32_t, int32_t *, UErrorCode *status);

#endif /* !INTL_UTF8_H */
