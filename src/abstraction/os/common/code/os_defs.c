/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

os_char *
os_scopeAttrImage(
    os_scopeAttr _this)
{
    os_char *image;
    switch (_this) {
    case OS_SCOPE_SHARED      : image = "OS_SCOPE_SHARED";      break;
    case OS_SCOPE_PRIVATE     : image = "OS_SCOPE_PRIVATE";     break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_lockPolicyImage(
    os_lockPolicy _this)
{
    os_char *image;
    switch (_this) {
    case OS_LOCK_DEFAULT      : image = "OS_LOCK_DEFAULT";      break;
    case OS_LOCKED            : image = "OS_LOCKED";            break;
    case OS_UNLOCKED          : image = "OS_UNLOCKED";          break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_schedClassImage(
    os_schedClass _this)
{
    os_char *image;
    switch (_this) {
    case OS_SCHED_DEFAULT     : image = "OS_SCHED_DEFAULT";     break;
    case OS_SCHED_REALTIME    : image = "OS_SCHED_REALTIME";    break;
    case OS_SCHED_TIMESHARE   : image = "OS_SCHED_TIMESHARE";   break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_compareImage(
    os_compare _this)
{
    os_char *image;
    switch (_this) {
    case OS_LESS              : image = "OS_LESS";              break;
    case OS_EQUAL             : image = "OS_EQUAL";             break;
    case OS_MORE              : image = "OS_MORE";              break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_resultImage(
    os_result _this)
{
    os_char *image;
    switch (_this) {
    case os_resultSuccess     : image = "os_resultSuccess";     break;
    case os_resultUnavailable : image = "os_resultUnavailable"; break;
    case os_resultTimeout     : image = "os_resultTimeout";     break;
    case os_resultBusy        : image = "os_resultBusy";        break;
    case os_resultInvalid     : image = "os_resultInvalid";     break;
    case os_resultFail        : image = "os_resultFail";        break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_booleanImage(
    os_boolean _this)
{
    os_char *image;
    switch (_this) {
    case OS_FALSE             : image = "OS_FALSE";             break;
    case OS_TRUE              : image = "OS_TRUE";              break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

union ptr_to_fptr {
    os_fptr fptr;
    void* ptr;
};

os_fptr
os_fptr(
    void* ptr)
{
    union ptr_to_fptr u;
    u.ptr = ptr;
    return u.fptr;
}


