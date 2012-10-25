#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

int
_utf8_get_char_extended(const char *s,
                             int max_len);
int _utf8_get_char_validated(const char *p,
                                  int max_len);
char *
_utf8_get_char(const char *i, uint32_t *chr);
int _utf8_check_string(const char *s);


#endif // UTILS_H