#include <stdio.h>
#include <ctype.h>

int main(void)
{
    int c;
    while ((c = getchar()) != EOF) {
        if (c != '\n' && isspace(c))
            putchar('_');
        else
            putchar(c);
    }
    return 0;
}