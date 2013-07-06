#if OS_SOCKET_USE_FCNTL == 1
os_result
os_sockSetNonBlocking(
    os_socket s,
    os_boolean nonblock)
{
    int oldflags;
    os_result r;

    assert(nonblock == OS_FALSE || nonblock == OS_TRUE);

    oldflags = fcntl(s, F_GETFL, 0);
    if(oldflags >= 0){
        if (nonblock == OS_TRUE){
            oldflags |= O_NONBLOCK;
        } else {
            oldflags &= ~O_NONBLOCK;
        }
        if(fcntl (s, F_SETFL, oldflags) == 0){
            r = os_resultSuccess;
        } else {
            r = os_resultFail;
        }
    } else {
        switch(errno){
            case EAGAIN:
                r = os_resultBusy;
                break;
            case EBADF:
                r = os_resultInvalid;
                break;
            default:
                r = os_resultFail;
                break;
        }
    }

    return r;
}
#endif

#if OS_SOCKET_USE_IOCTL == 1
os_result
os_sockSetNonBlocking(
    os_socket s,
    os_boolean nonblock)
{
    os_result r = os_resultSuccess;

    int nonblockingvalue = (nonblock != OS_FALSE);

    assert(nonblock == OS_FALSE || nonblock == OS_TRUE);

    if ( ioctl(s, FIONBIO, &nonblockingvalue) == -1 )
    {
        switch(errno)
        {
            case EAGAIN:
                r = os_resultBusy;
                break;
            case EBADF:
                r = os_resultInvalid;
                break;
            default:
                r = os_resultFail;
                break;
        }
    }

    return r;
}
#endif
