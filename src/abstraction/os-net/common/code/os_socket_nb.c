/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
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
        switch(os_getErrno()){
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
        switch(os_getErrno())
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
