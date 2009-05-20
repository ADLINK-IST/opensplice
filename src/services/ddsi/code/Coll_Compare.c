#include "os.h"
#include "Coll_Compare.h"

int
stringIsLessThen(
    void *left,
    void *right)
{
    char* leftString  =	(char*)left;
    char* rightString =	(char*)right;

    return (strcmp(leftString, rightString) < 0);
}

int
pointerIsLessThen(
    void *left,
    void *right)
{
    return (left < right);
    /* suppress warnings */
}
