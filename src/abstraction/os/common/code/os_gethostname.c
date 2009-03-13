os_result
os_gethostname(
    char *hostname,
    os_uint32 buffersize)
{
    os_result result;
    char hostnamebuf[MAXHOSTNAMELEN];

    if (gethostname (hostnamebuf, MAXHOSTNAMELEN) == 0) {
        if ((strlen(hostnamebuf)+1) > (size_t)buffersize) {
            result = os_resultFail;
        } else {
            strncpy (hostname, hostnamebuf, (size_t)buffersize);
            result = os_resultSuccess;
	    }
    } else {
        result = os_resultFail;
    }

    return result;
}
