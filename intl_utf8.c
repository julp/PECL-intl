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
