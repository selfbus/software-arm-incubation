//
// The printf code is courtesey Thomas Wahl
// https://community.nxp.com/thread/56035
// Source: https://www.nxp.com/docs/en/application-note-software/6728_sprintf.c
//
#include "../common.h"
#include <ctype.h>
#include <stdarg.h>
#include <sblib/serial.h>

Serial serial(PIO1_6, PIO1_7);

// the following code is taken from kernel 2.4.17  (lib/vsprintf.c)
static int skip_atoi(const char **s) {
    int i=0;
    while (isdigit(**s)) i = i*10 + *((*s)++) - '0';
    return i;
}

#define ZEROPAD        1                /* pad with zero */
#define SIGN           2                /* unsigned/signed long */
#define PLUS           4                /* show plus */
#define SPACE          8                /* space if plus */
#define LEFT          16                /* left justified */
#define SPECIAL       32                /* 0x */
#define LARGE         64                /* use 'ABCDEF' instead of 'abcdef' */

const char small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
const char large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char * number(char * buf, char * end, long num, int base, int size,
        int precision, int type)
{
    // because the base is restricted to 8<=base<=16 we need the following
    // number of tmp bits:
    //
    // long is of size 32 digits in base 2 => 32/3 digits in base 8 => 11
    // digits for the number plus additional 3 digits (1 sign, 2 hex-modifier)
    // so we need at max 14 digits
    char c,sign,tmp[14];
    const char *digits;
    int i;

    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
        type &= ~ZEROPAD;
    // we only allow a base between 8 and 16
    if (base < 8 || base > 16)
        return 0;
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (type & SIGN) {
        if (num < 0) {
            sign = '-';
            num = -num;
            size--;
        } else if (type & PLUS) {
            sign = '+';
            size--;
        } else if (type & SPACE) {
            sign = ' ';
            size--;
        }
    }
    if (type & SPECIAL) {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }
    i = 0;
    if (num == 0)
        tmp[i++]='0';
    else while (num != 0)
        {
            tmp[i++] = digits[num % base];
            num=num/base;
        }
    if (i > precision)
        precision = i;
    size -= precision;
    if (!(type&(ZEROPAD+LEFT))) {
        while(size-->0) {
            if (buf <= end)
                *buf = ' ';
            ++buf;
        }
    }
    if (sign) {
        if (buf <= end)
            *buf = sign;
        ++buf;
    }
    if (type & SPECIAL) {
        if (base==8) {
            if (buf <= end)
                *buf = '0';
            ++buf;
        } else if (base==16) {
            if (buf <= end)
                *buf = '0';
            ++buf;
            if (buf <= end)
                *buf = digits[33];
            ++buf;
        }
    }
    if (!(type & LEFT)) {
        while (size-- > 0) {
            if (buf <= end)
                *buf = c;
            ++buf;
        }
    }
    while (i < precision--) {
        if (buf <= end)
            *buf = '0';
        ++buf;
    }
    while (i-- > 0) {
        if (buf <= end)
            *buf = tmp[i];
        ++buf;
    }
    while (size-- > 0) {
        if (buf <= end)
            *buf = ' ';
        ++buf;
    }
    return buf;
}

/**
* log_vsnprintf - Format a string and place it in a buffer
* @buf: The buffer to place the result into
* @size: The size of the buffer, including the trailing null space
* @fmt: The format string to use
* @args: Arguments for the format string
*
* Call this function if you are already dealing with a va_list.
* You probably want snprintf instead.
 */
static int log_vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    int len;
    unsigned long num;
    int i, base;
    char *str, *end, c;
    const char *s;

    int flags;                /* flags to number() */

    int field_width;        /* width of output field */
    int precision;                /* min. # of digits for integers; max
                                   number of chars for from string */
    int qualifier;                /* 'h', 'l', or 'L' for integer fields */
    /* 'z' support added 23/7/1999 S.H.    */
    /* 'z' changed to 'Z' --davidm 1/25/99 */

    str = buf;
    end = buf + size - 1;

    if (end < buf - 1) {
        end = ((char *) -1);
        size = (unsigned int)end - (unsigned int)buf + 1;
    }

    for (; *fmt ; ++fmt) {
        if (*fmt != '%') {
            if (str <= end)
                *str = *fmt;
            ++str;
            continue;
        }

        /* process flags */
        flags = 0;
        repeat:
        ++fmt;                /* this also skips first '%' */
        switch (*fmt) {
            case '-': flags |= LEFT; goto repeat;
            case '+': flags |= PLUS; goto repeat;
            case ' ': flags |= SPACE; goto repeat;
            case '#': flags |= SPECIAL; goto repeat;
            case '0': flags |= ZEROPAD; goto repeat;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (isdigit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*') {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt =='Z') {
            qualifier = *fmt;
            ++fmt;
            if (qualifier == 'l' && *fmt == 'l') {
                qualifier = 'L';
                ++fmt;
            }
        }

        /* default base */
        base = 10;

        switch (*fmt) {
            case 'c':
                if (!(flags & LEFT)) {
                    while (--field_width > 0) {
                        if (str <= end)
                            *str = ' ';
                        ++str;
                    }
                }
                c = (char) va_arg(args, int);
                if (str <= end)
                    *str = c;
                ++str;
                while (--field_width > 0) {
                    if (str <= end)
                        *str = ' ';
                    ++str;
                }
                continue;

            case 's':
                s = va_arg(args, char *);
                if (!s)
                    s = "<NULL>";

                len = (int) strlen(s);

                if (!(flags & LEFT)) {
                    while (len < field_width--) {
                        if (str <= end)
                            *str = ' ';
                        ++str;
                    }
                }
                for (i = 0; i < len; ++i) {
                    if (str <= end)
                        *str = *s;
                    ++str; ++s;
                }
                while (len < field_width--) {
                    if (str <= end)
                        *str = ' ';
                    ++str;
                }
                continue;

            case 'p':
                if (field_width == -1) {
                    field_width = 2*sizeof(void *);
                    flags |= ZEROPAD;
                }
                str = number(str, end,
                             (long) va_arg(args, void *),
                             16, field_width, precision, flags);
                continue;


            case 'n':
                /* FIXME:
                * What does C99 say about the overflow case here? */
                if (qualifier == 'l') {
                    long * ip = va_arg(args, long *);
                    *ip = (str - buf);
                } else if (qualifier == 'Z') {
                    unsigned int * ip = va_arg(args, size_t *);
                    *ip = (str - buf);
                } else {
                    int * ip = va_arg(args, int *);
                    *ip = (str - buf);
                }
                continue;

            case '%':
                if (str <= end)
                    *str = '%';
                ++str;
                continue;

                /* integer number formats - set up the flags and "break" */
            case 'o':
                base = 8;
                break;

            case 'X':
                flags |= LARGE;
            case 'x':
                base = 16;
                break;

            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                break;

            default:
                if (str <= end)
                    *str = '%';
                ++str;
                if (*fmt) {
                    if (str <= end)
                        *str = *fmt;
                    ++str;
                } else {
                    --fmt;
                }
                continue;
        }
        if (qualifier == 'L')
            num = va_arg(args, unsigned long);
        else if (qualifier == 'l') {
            num = va_arg(args, unsigned long);
            if (flags & SIGN)
                num = (signed long) num;
        } else if (qualifier == 'Z') {
            num = va_arg(args, size_t);
        } else if (qualifier == 'h') {
            num = (unsigned short) va_arg(args, int);
            if (flags & SIGN)
                num = (signed short) num;
        } else {
            num = va_arg(args, unsigned int);
            if (flags & SIGN)
                num = (signed int) num;
        }
        str = number(str, end, num, base,
                     field_width, precision, flags);
    }
    if (str <= end)
        *str = '\0';
    else if (size > 0)
        /* don't write out a null byte if the buf size is zero */
        *end = '\0';
    /* the trailing null byte doesn't count towards the total
    * ++str;
    */
    return str-buf;
}

void serPrintf(const char *fmt, ...) {
    va_list args;
    size_t len = 256;
    char buf[len];
    va_start(args, fmt);
    log_vsnprintf(&buf[0], len, fmt, args);
    va_end(args);
    serial.print(millis() % 1000000);
    serial.print(": ");
    serial.println(buf);
}

void initLogger() {
    serial.begin(115200);
    serPrintf("Logging enabled");
}