#include <stdio.h>


int *add_nonnegative(int a, int b)
{
    int c = a + b;
    int *p = &c;
    if (a < 0 || b < 0) {
        p = NULL;
    }
    return p;
}

int main(int argc, char **argv)
{
    int *result_1 = add_nonnegative(1, 3);
    int *result_2 = add_nonnegative(-1, 1);
    printf("1 + 3 = %d.\n", *result_1);
    printf("-1 + 1 = %d.\n", *result_2);
}