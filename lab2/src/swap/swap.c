#include "swap.h"

void Swap(char *left, char *right)
{
    char temp_left = *left;
	char temp_right = *right;
    *left = temp_right;
    *right = temp_left;
}