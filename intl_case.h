#ifndef INTL_CASE_H

# define INTL_CASE_H

typedef enum {
    UCASE_NONE,
    UCASE_FOLD,
    UCASE_LOWER,
    UCASE_UPPER,
    UCASE_TITLE,
    UCASE_COUNT
} UCaseType;

void utf8_fullcase(char **, int32_t *, const char *, int, const char *, UCaseType, UErrorCode *);
void utf16_fullcase(UChar **, int32_t *, const UChar *, int, const char *, UCaseType, UErrorCode *);

#endif /* INTL_CASE_H */
