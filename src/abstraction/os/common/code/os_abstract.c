pa_endianNess
pa_getEndianNess (
    void)
{
#ifdef PA_LITTLE_ENDIAN
    return pa_endianLittle;
#else
    return pa_endianBig;
#endif
}
