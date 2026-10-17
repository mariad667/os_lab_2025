#include <string.h>
#include "revert_string.h"

void RevertString(char *str)
{
    size_t len = strlen(str);
    if (len < 2) return;   
	// если строка пустая или из одного символа, то делать с ней нечего

    size_t i = 0;
    size_t j = len - 1;

    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}