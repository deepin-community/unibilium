#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unibilium.h>
#include <unistd.h>

void encoded_print(const char *str)
{
    for (int c = *str; c != '\0'; c = *(++str)) {
        if (isprint(c)) {
            printf("%c", c);
            continue;
        }

        switch (c) {
            case 0x1b:
                printf("^[");
                break;
            case '\a':
                printf("^G");
                break;
            case '\b':
                printf("^H");
                break;
            case '\n':
                printf("^J");
                break;
            case '\r':
                printf("^M");
                break;
            case '\t':
                printf("^I");
                break;
            case 127:
                printf("^?");
                break;
            default:
                printf("\\%o", c & 0xff);
                break;
        }
    }
}

int main(int argc, const char *argv[])
{
    const char *term = NULL;
    const char **aliases = NULL;
    unibi_term *ut = NULL;

    if (argc < 2) {
        printf("Bail out! Must specify a terminfo file to parse.\n");
        return 0;
    }

    term = strrchr(argv[1], '/');
    if (term == NULL) {
        term = argv[1];
    } else {
        term++;
    }

    if (!(ut = unibi_from_file(argv[1]))) {
        int err = errno;

        printf("not ok - ");
        if (err == EINVAL) {
            printf("db entry for %s does not look like a valid terminfo entry\n", term);
        } else if (err == EFAULT) {
            printf("db entry for %s is too small to be valid\n", term);
        } else {
            printf("unknown error - %s\n", strerror(err));
        }

        return 1;
    }

    printf("ok - Parsed db entry '%s' from TERM=%s\n", unibi_get_name(ut), term);

    aliases = unibi_get_aliases(ut);
    for (int i = 0; aliases[i]; i++) {
        printf("# alias[%d] = %s\n", i, aliases[i]);
    }

    printf("# bool capabilities\n");
    for (int i = unibi_boolean_begin_ + 1; i < unibi_boolean_end_; i++) {
        int b = unibi_get_bool(ut, i);
        if (!b) {
            continue;
        }
        printf("#\t%s (%s) - %d\n", unibi_name_bool(i), unibi_short_name_bool(i), b);
    }
    printf("#\n");

    printf("# num capabilities\n");
    for (int i = unibi_numeric_begin_ + 1; i < unibi_numeric_end_; i++) {
        int num = unibi_get_num(ut, i);
        if (num == -1) {
            continue;
        }
        printf("#\t%s (%s) - %d\n", unibi_name_num(i), unibi_short_name_num(i), num);
    }
    printf("#\n");

    printf("# str capabilities\n");
    for (int i = unibi_string_begin_ + 1; i < unibi_string_end_; i++) {
        const char *str = unibi_get_str(ut, i);
        if (str == NULL) {
            continue;
        }
        printf("#\t%s (%s) - ", unibi_name_str(i), unibi_short_name_str(i));
        encoded_print(str);
        printf("\n");
    }
    printf("#\n");

    printf("# extbool capabilities\n");
    for (size_t i = 0, max = unibi_count_ext_bool(ut); i < max; i++) {
        int b = unibi_get_ext_bool(ut, i);
        if (!b) {
            continue;
        }
        printf("#\t%s - %d\n", unibi_get_ext_bool_name(ut, i), b);
    }
    printf("#\n");

    printf("# extnum capabilities\n");
    for (size_t i = 0, max = unibi_count_ext_num(ut); i < max; i++) {
        int num = unibi_get_ext_num(ut, i);
        if (num == -1) {
            continue;
        }
        printf("#\t%s - %d\n", unibi_get_ext_num_name(ut, i), num);
    }
    printf("#\n");

    printf("# extstr capabilities\n");
    for (size_t i = 0, max = unibi_count_ext_str(ut); i < max; i++) {
        const char *str = unibi_get_ext_str(ut, i);
        if (str == NULL) {
            continue;
        }
        printf("#\t%s - ", unibi_get_ext_str_name(ut, i));
        encoded_print(str);
        printf("\n");
    }

    unibi_destroy(ut);
    return 0;
}
