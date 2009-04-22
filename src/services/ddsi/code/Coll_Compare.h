#ifndef COLL_COMPARE_H
#define COLL_COMPARE_H

#if defined (__cplusplus)
extern "C" {
#endif

int
stringIsLessThen(
    void *left,
    void *right);

int
pointerIsLessThen(
    void *left,
    void *right);

#if defined (__cplusplus)
}
#endif

#endif /* COLL_COMPARE_H */
