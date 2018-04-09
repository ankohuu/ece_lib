#include <stdio.h>
#include <ctype.h>

static void print_hex_ascii_line(const unsigned char *payload, int len, int offset)
{
    int i;
    int gap;
    const unsigned char *ch;

    printf("%05d   ", offset);

    ch = payload;
    for(i = 0; i < len; i++)
    {
        printf("%02x ", *ch);
        ch++;
        if (i == 7)
            printf(" ");
    }

    if (len < 8)
        printf(" ");

    if (len < 16)
    {
        gap = 16 - len;
        for (i = 0; i < gap; i++)
        {
            printf("   ");
        }
    }
    printf("   ");

    ch = payload;
    for(i = 0; i < len; i++)
    {
        if (isprint(*ch))
            printf("%c", *ch);
        else
            printf(".");
        ch++;
    }

    printf("\n");
    return;
}

static void print_payload(const unsigned char *payload, int len)
{
    int len_rem = len;
    int line_width = 16;
    int line_len;
    int offset = 0;
    const unsigned char* ch = payload;

    if (len <= 0)
        return;

    if (len <= line_width) {
        print_hex_ascii_line(ch, len, offset);
        return;
    }

    while (1) {
        line_len = line_width % len_rem;
        print_hex_ascii_line(ch, line_len, offset);
        len_rem = len_rem - line_len;
        ch = ch + line_len;
        offset = offset + line_width;
        if (len_rem <= line_width) {
            print_hex_ascii_line(ch, len_rem, offset);
            break;
        }
    }
    return;
}

void print_pkt(unsigned long module, unsigned char *pkt, unsigned long len)
{
    printf("receive a pkt from module:%lu len:%lu\n", module, len);
    print_payload(pkt, len);
    return;
}

int ch_to_hex(const char *s, unsigned char *bits)
{
    int i, n = 0;
    char ch;
    for(i = 0; s[i]; i += 2) {

        ch = toupper(s[i]);
        if (0 == isxdigit(ch))
            return 0;
        if (ch >= 'A' && ch <= 'F')
            bits[n] = ch - 'A' + 10;
        else
            bits[n] = ch - '0';

        ch = toupper(s[i + 1]);
        if (0 == isxdigit(ch))
            return 0;
        if (ch >= 'A' && ch <= 'F')
            bits[n] = (bits[n] << 4) | (ch - 'A' + 10);
        else
            bits[n] = (bits[n] << 4) | (ch - '0');
        ++n;
    }
    return n;
}
