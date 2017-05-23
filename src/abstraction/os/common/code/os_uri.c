#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "os_defs.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_string.h"
#include "os_uri.h"

OS_STRUCT(os_uri) {
    os_uriStrict strict;
    os_uriRelative relative;
    os_char *scheme;
    os_char *userinfo;
    os_char *host;
    os_char *port;
    os_char *path;
    os_char *query;
    os_char *fragment;
};

static void
os__uriInit(
    os_uri uri);

static void
os__uriDeinit(
    os_uri uri);

static os_char *
os__uriScanSubDelim(
    const os_char *str)
{
    const os_char *end;

    assert (str != NULL);

    end = str;
    if (str[0] == '!' || str[0] == '$' ||
        str[0] == '&' || str[0] == '(' ||
        str[0] == ')' || str[0] == '*' ||
        str[0] == '+' || str[0] == ',' ||
        str[0] == ';' || str[0] == '=' ||
        str[0] == '\'')
    {
        end = str + 1;
    }

    return (os_char *)end;
}

static os_char *
os__uriScanUnreserved(
    const os_char *str)
{
    const os_char *end;

    assert (str != NULL);

    end = str;
    if (isalnum ((unsigned char)str[0]) || str[0] == '-' || str[0] == '.'
                                        || str[0] == '_' || str[0] == '~')
    {
        end = str + 1;
    }

    return (os_char *)end;
}

static os_char *
os__uriScanPctEncoded(
    const os_char *str)
{
    const os_char *end;

    assert (str != NULL);

    end = str;
    if (str[0] == '%' && isxdigit((unsigned char)str[1]) &&
                         isxdigit((unsigned char)str[2]))
    {
        end = str + 3;
    }

    return (os_char *)end;
}

static os_char *
os__uriScanPchar(
    const os_char *str)
{
    const os_char *end;

    assert (str != NULL);

    if ((end = os__uriScanUnreserved(str)) == str &&
        (end = os__uriScanSubDelim(str)) == str &&
        (end = os__uriScanPctEncoded(str)) == str)
    {
        if (str[0] == ':' || str[0] == '@') {
            end = str + 1;
        }
    }

    return (os_char *)end;
}

#define OS__URI_SCAN_SEGMENT_NONE 0
#define OS__URI_SCAN_SEGMENT_NO_STRICT (1<<0)
#define OS__URI_SCAN_SEGMENT_NO_COLON (1<<1)

static os_char *
os__uriScanSegment(
    const os_char *str,
    os_uint flags)
{
    const os_char *end, *ptr, *quit, *spc;
    os_int strict;

    assert (str != NULL);

    strict = flags & OS__URI_SCAN_SEGMENT_NO_STRICT ? 0 : 1;

    end = ptr = quit = spc = str;
    do {
        if (quit > spc) {
            end = ptr;
        }
        quit = ptr;
        if (!strict && ptr > str && (*ptr == ' ' || *ptr == '\t')) {
            spc = ptr++;
        } else if (*ptr != ':' || !(flags & OS__URI_SCAN_SEGMENT_NO_COLON)) {
            ptr = os__uriScanPchar (ptr);
        }
    } while (ptr > quit);

    return (os_char *)end;
}

static os_char *
os__uriScanWinDrive(
    const os_char *str)
{
    const os_char *end;

    assert (str != NULL);

    end = str;
    if (isalpha((unsigned char)str[0]) && str[1] == ':' &&
            (str[2] == '\\' || str[2] == '/'))
    {
        end = str + 3;
    }

    return (os_char *)end;
}

static os_char *
os__uriParseFragment(
    os_uri uri,
    const os_char *str)
{
    const os_char *end, *ptr;

    assert (uri != NULL);
    assert (str != NULL);

    ptr = str;
    do {
        end = ptr;
        if (*ptr == '/' || *ptr == '?') {
            ptr++;
        } else {
            ptr = os__uriScanPchar (ptr);
        }
    } while (ptr != end);

    if (end != str) {
        uri->fragment = os_strndup (str, (os_size_t)(end - str));
        if (uri->fragment == NULL) {
            end = NULL;
        }
    }

    return (os_char *)end;
}

static os_char *
os__uriParseQuery(
    os_uri uri,
    const os_char *str)
{
    const os_char *end, *ptr;

    assert (uri != NULL);
    assert (str != NULL);

    ptr = str;
    do {
        end = ptr;
        if (*ptr == '/' || *ptr == '?') {
            ptr++;
        } else {
            ptr = os__uriScanPchar (ptr);
        }
    } while (ptr != end);

    if (str != end) {
        uri->query = os_strndup (str, (os_size_t)(end - str));
        if (uri->query == NULL) {
            end = NULL;
        }
    }

    return (os_char *)end;
}

static os_int
os__uriRequireStrict(
    os_uri uri)
{
    os_int strict = 1;

    assert (uri != NULL);

    if (uri->strict == OS_URI_STRICT_FALSE) {
        strict = 0;
    } else if (uri->strict == OS_URI_STRICT_AUTO) {
        strict = (uri->scheme && os_strcasecmp (uri->scheme, "file") == 0);
    }

    return strict;
}

/* os__uriParsePath function must called by os_uriParsePath* functions only */
static os_char *
os__uriParsePath(
    os_uri uri,
    const os_char *str)
{
    const os_char *end, *ptr;
    os_int strict = 1;
    os_uint flags = OS__URI_SCAN_SEGMENT_NONE;

    assert (uri != NULL);
    assert (str != NULL);

    strict = os__uriRequireStrict (uri);
    if (!strict) {
        flags = OS__URI_SCAN_SEGMENT_NO_STRICT;
    }

    ptr = os__uriScanSegment (str, flags);
    do {
        end = ptr;
        if (*ptr == '/' || (!strict && *ptr == '\\')) {
            ptr = os__uriScanSegment (++ptr, flags);
        }
    } while (ptr > end);

    if (end != str) {
        uri->path = os_strndup (str, (os_size_t)(end - str));
        if (uri->path == NULL) {
            end = NULL;
        }
    }

    return (os_char *)end;
}

static os_char *
os__uriParsePathRootless(
    os_uri uri,
    const os_char *str)
{
    const os_char *end;
    os_uint flags = OS__URI_SCAN_SEGMENT_NONE;

    assert (uri != NULL);
    assert (str != NULL);

    if (!os__uriRequireStrict (uri)) {
        flags = OS__URI_SCAN_SEGMENT_NO_STRICT;
    }

    end = os__uriScanSegment (str, flags);
    if (end != str) {
        end = os__uriParsePath (uri, str);
    }

    return (os_char *)end;
}

static os_char *
os__uriParsePathNoScheme(
    os_uri uri,
    const os_char *str)
{
    const os_char *end;
    os_uint flags = OS__URI_SCAN_SEGMENT_NONE;

    assert (uri != NULL);
    assert (str != NULL);

    if (!os__uriRequireStrict (uri)) {
        flags = OS__URI_SCAN_SEGMENT_NO_STRICT;
    }

    end = os__uriScanSegment (str, flags | OS__URI_SCAN_SEGMENT_NO_COLON);
    if (end != str) {
        end = os__uriParsePath (uri, str);
    }

    return (os_char *)end;
}

static os_char *
os__uriParsePathAbsolute(
    os_uri uri,
    const os_char *str)
{
    const os_char *end;

    assert (uri != NULL);
    assert (str != NULL);

    end = str;
    if (*end == '/') {
        end = os__uriParsePath (uri, str);
    }

    return (os_char *)end;
}

static os_char *
os__uriParsePort(
    os_uri uri,
    const os_char *str)
{
    const os_char *end, *ptr;

    assert (uri != NULL);
    assert (str != NULL);

    end = ptr = str;
    while (isdigit((unsigned char)*ptr)) {
        ptr++;
    }

    if (ptr != str) {
        uri->port = os_strndup (str, (os_size_t)(ptr - str));
        if (uri->port != NULL) {
            end = ptr;
        } else {
            end = NULL;
        }
    }

    return (os_char *)end;
}

static os_char *
os__uriParseRegName(
    os_uri uri,
    const os_char *str)
{
    const os_char *end;

    assert (uri != NULL);
    assert (str != NULL);

    end = os_scanRegName (str);
    if (end != str && (uri->host = os_strndup (str, (os_size_t)(end - str))) == NULL) {
        end = NULL;
    }

    return (os_char *)end;
}

static os_char *
os__uriParseIPv4Address(
    os_uri uri,
    const os_char *str)
{
    const os_char *end;

    assert (uri != NULL);
    assert (str != NULL);

    end = os_scanIPv4Address (str);
    if (end != str && (uri->host = os_strndup (str, (os_size_t)(end - str))) == NULL) {
        end = NULL;
    }

    return (os_char *)end;
}

static os_char *
os__uriParseIPv6Address(
    os_uri uri,
    const os_char *str)
{
    const os_char *end;

    assert (uri != NULL);
    assert (str != NULL);

    end = os_scanIPv6Address (str);
    if (end != str && (uri->host = os_strndup (str, (os_size_t)(end - str))) == NULL) {
        end = NULL;
    }

    return (os_char *)end;
}

static os_char *
os__uriParseHost(
    os_uri uri,
    const os_char *str)
{
    const os_char *end, *ptr;

    assert (uri != NULL && uri->host == NULL);
    assert (str != NULL);

    end = str;
    if (*str == '[') {
        ptr = os__uriParseIPv6Address (uri, str + 1);
        if (ptr == NULL) {
            end = ptr;
        } else if (ptr > (str + 1) && *ptr == ']') {
            /* Termination by ] required. */
            end = ++ptr;
        }
    } else {
        end = os__uriParseIPv4Address (uri, str);
        if (end == str) {
            end = os__uriParseRegName (uri, str);
        }
    }

    return (os_char *)end;
}

static os_char *
os__uriParseUserinfo(
    os_uri uri,
    const os_char *str)
{
    const os_char *end, *ptr;

    assert (uri != NULL && uri->userinfo == NULL);
    assert (str != NULL);

    ptr = str;
    do {
        end = ptr;
        if ((ptr = os__uriScanUnreserved (ptr)) == end &&
            (ptr = os__uriScanSubDelim (ptr)) == end)
        {
            if (*ptr == ':') {
                ptr++;
            }
        }
    } while (end != ptr);

    if (end != str) {
        uri->userinfo = os_strndup (str, (os_size_t)(end - str));
        if (uri->userinfo == NULL) {
            end = NULL;
        }
    }

    return (os_char *)end;
}

static os_char *
os__uriParseAuthority(
    os_uri uri,
    const os_char *str)
{
    const os_char *end, *ptr;

    assert (uri != NULL);
    assert (str != NULL);

    end = str;
    ptr = os__uriParseUserinfo (uri, str);
    if (ptr != NULL && ptr != str) {
        assert (uri->userinfo != NULL);
        if (*ptr == '@') {
            ptr++;
        } else {
            /* No error, userinfo is optional. */
            os_free (uri->userinfo);
            uri->userinfo = NULL;
            ptr = str;
        }
    }

    if (ptr != NULL) {
        end = os__uriParseHost (uri, ptr);
        if (end == ptr) {
            /* Host is not optional, reset entirely. */
            end = str;
        } else if (end != NULL) {
            assert (uri->host != NULL);
            if (*end == ':') {
                ptr = os__uriParsePort (uri, ++end);
                if (ptr == end) {
                    /* Port is optional, but invalid port invalidates uri. */
                    end = str;
                } else {
                    end = ptr;
                }
            }
        }
    }

    return (os_char *)end;
}

static os_char *
os__uriParseHierPart(
    os_uri uri,
    const os_char *str)
{
    const os_char *end, *ptr;
    os_int strict = 1;

    assert (uri != NULL);
    assert (str != NULL);

    strict = os__uriRequireStrict (uri);

    end = ptr = str;
    if (str[0] == '/' && str[1] == '/') {
        ptr = str + 2;
        if (!strict) {
            /* Backwards compatibility. */
            if (*ptr == '/') {
                end = os__uriParsePathAbsolute (uri, ptr);
            } else if (os__uriScanWinDrive (ptr) != ptr) {
                end = os__uriParsePathRootless (uri, ptr);
            }
        }

        if (end == str) {
            end = os__uriParseAuthority (uri, ptr);
            if (end == ptr) {
                end = str;
            } else if (end != NULL) {
                end = os__uriParsePathAbsolute (uri, end);
            }
        }
    } else {
        end = os__uriParsePathAbsolute (uri, str);
        if (end == str) {
            end = os__uriParsePathRootless (uri, str);
        }
    }

    return (os_char *)end;
}

static os_char *
os__uriParseRelativePart(
    os_uri uri,
    const os_char *str)
{
    os_char *end;
    os_int strict = 1;

    assert (uri != NULL);
    assert (str != NULL);

    if (uri->strict != OS_URI_STRICT_TRUE) {
        strict = 0;
    }

    if (str[0] == '/' && str[1] == '/') {
        end = os__uriParseAuthority(uri, str + 2);
        if (end != NULL && end == (str + 2)) {
            end = os__uriParsePathAbsolute(uri, str + 2);
        }
    } else {
        end = os__uriParsePathAbsolute(uri, str);
        if (!strict && end == str && os__uriScanWinDrive (str) != str) {
            end = os__uriParsePathRootless (uri, str);
        }
        if (end == str) {
            end = os__uriParsePathNoScheme(uri, str);
        }
    }

    return (os_char *)end;
}

static os_char *
os__uriParseScheme(
    os_uri uri,
    const os_char *str)
{
    const os_char *end;

    assert (uri != NULL);
    assert (str != NULL);

    end = str;
    for (; isalnum((unsigned char)*end) || *end == '+' || *end == '-' || *end == '.'; end++) {
        /* do nothing */
    }

    if (end != str) {
        uri->scheme = os_strndup (str, (os_size_t)(end - str));
        if (uri->scheme == NULL) {
            end = NULL;
        }
    }

    return (os_char *)end;
}

static void
os__uriInit (
    os_uri uri)
{
    assert (uri != NULL);
    memset (uri, 0, OS_SIZEOF (os_uri));
}

os_uri
os_uriNew (
    void)
{
    os_uri uri;
    uri = os_malloc (OS_SIZEOF (os_uri));
    os__uriInit (uri);
    return uri;
}

static void
os__uriDeinit(
    os_uri uri)
{
    assert (uri != NULL);

    os_free (uri->scheme);
    os_free (uri->userinfo);
    os_free (uri->host);
    os_free (uri->port);
    os_free (uri->path);
    os_free (uri->query);
    os_free (uri->fragment);
    memset (uri, 0, OS_SIZEOF (os_uri));
}

void
os_uriFree (
    os_uri uri)
{
    if (uri != NULL) {
        os__uriDeinit (uri);
        os_free (uri);
    }
}

os_char *
os_uriParse(
    os_uri uri,
    const os_char *str)
{
    OS_STRUCT(os_uri) buf;
    const os_char *end, *ptr;

    assert (uri != NULL);
    assert (str != NULL);

    end = str;

    os__uriInit (&buf);
    buf.strict = uri->strict;
    buf.relative = uri->relative;

    if (buf.relative != OS_URI_RELATIVE_TRUE) {
        ptr = os__uriParseScheme (&buf, end);
        if (ptr != NULL && ptr > str && *ptr == ':') {
            end = ++ptr;
            ptr = os__uriParseHierPart (&buf, end);
            if (ptr == end) {
                end = str;
            } else {
                end = ptr;
            }
        }
    }

    if (end == str) {
        os__uriDeinit (&buf);
        buf.strict = uri->strict;
        buf.relative = uri->relative;
        if (buf.relative != OS_URI_RELATIVE_FALSE) {
            end = os__uriParseRelativePart (&buf, end);
        }
    }

    /* query */
    if (end != NULL && end > str && *end == '?') {
        end = os__uriParseQuery (&buf, end + 1);
    }

    /* fragment */
    if (end != NULL && end > str && *end == '#') {
        end = os__uriParseFragment (&buf, end + 1);
    }

    if (end != NULL && end > str) {
        os__uriDeinit (uri);
        (void)memcpy (uri, &buf, sizeof (buf));
    } else {
        os__uriDeinit (&buf);
    }

    return (os_char *)end;
}

os_ssize_t
os_uriPrint(
    os_char *str,
    os_size_t len,
    const os_uri uri)
{
    os_int idx, max;
    os_size_t pos = 0;
    os_ssize_t cnt, tot = 0;

    const os_char *parts[][2] = {
        { "%s:", NULL }, /* scheme */
        { "%s",  NULL }, /* // */
        { "%s@", NULL }, /* userinfo */
        { "%s",  NULL }, /* host */
        { ":%s", NULL }, /* port */
        { "%s",  NULL }, /* path */
        { "?%s", NULL }, /* query */
        { "#%s", NULL }  /* fragment */
    };

    assert (str != NULL);
    assert (uri != NULL);

    if (uri->scheme != NULL && uri->relative != OS_URI_RELATIVE_TRUE) {
        parts[0][1] = uri->scheme;
    }

    if (uri->host != NULL) {
        parts[1][1] = "//";
        parts[2][1] = uri->userinfo;
        parts[3][1] = uri->host;
        parts[4][1] = uri->port;
    }

    parts[5][1] = uri->path;
    parts[6][1] = uri->query;
    parts[7][1] = uri->fragment;

    for (idx = 0, max = (sizeof(parts)/sizeof(os_char *[2]));
         idx < max && tot >= 0;
         idx++)
    {
        if (parts[idx][1] != NULL) {
            cnt = snprintf (str + pos, len - pos, parts[idx][0], parts[idx][1]);
            if (cnt >= 0) {
                if ((os_size_t)cnt >= (len - pos)) {
                    pos = len;
                } else {
                    pos += (os_size_t)cnt;
                }
                tot += cnt;
            } else {
                tot = -1;
            }
        }
    }

    return tot;
}

os_uriStrict
os_uriGetStrict(
    const os_uri uri)
{
    assert (uri != NULL);
    return uri->strict;
}

os_int
os_uriSetStrict(
    os_uri uri,
    os_uriStrict strict)
{
    assert (uri != NULL);

    switch (strict) {
        case OS_URI_STRICT_AUTO:
        case OS_URI_STRICT_FALSE:
            uri->strict = strict;
            break;
        default:
            assert (strict == OS_URI_STRICT_TRUE);
            if (uri->path == NULL || os_uriIsStrict (uri)) {
                uri->strict = strict;
            }
            break;
    }

    return strict == uri->strict ? OS_RETCODE_OK : OS_RETCODE_BAD_PARAMETER;
}

os_boolean
os_uriIsStrict(
    const os_uri uri)
{
    const os_char *end, *ptr;
    os_boolean strict = OS_FALSE;

    assert (uri != NULL);

    if (uri->path != NULL) {
        ptr = uri->path;
        do {
            end = ptr;
            if (*ptr == '/') {
                ptr++;
            } else {
                ptr = os__uriScanPchar (ptr);
            }
        } while (ptr > end);

        if (*ptr == '\0') {
            strict = OS_TRUE;
        }
    }

    return strict;
}

os_uriRelative
os_uriGetRelative(
    const os_uri uri)
{
    assert (uri != NULL);
    return uri->relative;
}

os_int
os_uriSetRelative(
    os_uri uri,
    os_uriRelative relative)
{
    assert (uri != NULL);

    switch (relative) {
        case OS_URI_RELATIVE_AUTO:
            uri->relative = relative;
            break;
        case OS_URI_RELATIVE_TRUE:
            if (uri->scheme == NULL) {
                uri->relative = OS_URI_RELATIVE_TRUE;
            }
            break;
        default:
            assert (relative == OS_URI_RELATIVE_FALSE);
            uri->relative = OS_URI_RELATIVE_FALSE;
            break;
    }

    return relative == uri->relative ? OS_RETCODE_OK : OS_RETCODE_BAD_PARAMETER;
}

os_boolean
os_uriIsRelative(
    const os_uri uri)
{
    os_boolean relative = OS_FALSE;

    assert (uri != NULL);

    if (uri->scheme == NULL) {
        relative = OS_TRUE;
    }

    return relative;
}

const os_char *
os_uriGetScheme(
    const os_uri uri)
{
    assert (uri != NULL);
    return (const os_char *)uri->scheme;
}

os_int
os_uriSetScheme(
    os_uri uri,
    const os_char *str)
{
    os_char *ptr, *scheme;
    os_int retcode = OS_RETCODE_OK;

    assert (uri != NULL);

    if (str == NULL) {
        os_free (uri->scheme);
        uri->scheme = NULL;
    } else if (uri->relative == OS_URI_RELATIVE_TRUE) {
        retcode = OS_RETCODE_BAD_PARAMETER;
    } else {
        scheme = uri->scheme;
        ptr = os__uriParseScheme (uri, str);
        if (ptr == NULL) {
            uri->scheme = scheme;
            retcode = OS_RETCODE_OUT_OF_RESOURCES;
        } else if (ptr != str && *ptr == '\0') {
            os_free (scheme);
        } else {
            if (ptr != str) {
                os_free (uri->scheme);
            }
            uri->scheme = scheme;
            retcode = OS_RETCODE_BAD_PARAMETER;
        }
    }

    return retcode;
}

const os_char *
os_uriGetUserinfo(
    const os_uri uri)
{
    assert (uri != NULL);
    return (const os_char *)uri->userinfo;
}

os_int
os_uriSetUserinfo(
    os_uri uri,
    const os_char *str)
{
    os_char *ptr, *userinfo;
    os_int retcode = OS_RETCODE_BAD_PARAMETER;

    assert (uri != NULL);

    if (str == NULL) {
        os_free (uri->userinfo);
        uri->userinfo = NULL;
        retcode = OS_RETCODE_OK;
    } else if (uri->host != NULL) {
        userinfo = uri->userinfo;
        ptr = os__uriParseUserinfo (uri, str);
        if (ptr == NULL) {
            uri->userinfo = userinfo;
            retcode = OS_RETCODE_OUT_OF_RESOURCES;
        } else if (ptr != str && *ptr == '\0') {
            os_free (userinfo);
            retcode = OS_RETCODE_OK;
        } else {
            if (ptr != str) {
                os_free (uri->userinfo);
            }
            uri->userinfo = userinfo;
        }
    }

    return retcode;
}

const os_char *
os_uriGetHost(
    const os_uri uri)
{
    assert (uri != NULL);
    return (const os_char *)uri->host;
}

os_int
os_uriSetHost(
    os_uri uri,
    const os_char *str)
{
    os_char *ptr;
    os_int retcode = OS_RETCODE_BAD_PARAMETER;

    assert (uri != NULL);

    if (str == NULL) {
        os_free (uri->host);
        uri->host = NULL;
        retcode = OS_RETCODE_OK;
    } else if (uri->path == NULL || *uri->path == '/') {
        ptr = os_scanHost (str);
        if (ptr > str && *ptr == '\0') {
            ptr = os_strndup (str, (os_size_t)(ptr - str));
            if (ptr != NULL) {
                os_free (uri->host);
                uri->host = ptr;
                retcode = OS_RETCODE_OK;
            } else {
                retcode = OS_RETCODE_OUT_OF_RESOURCES;
            }
        }
    }

    return retcode;
}

const os_char *
os_uriGetPort(
    const os_uri uri)
{
    assert (uri != NULL);
    return (const os_char *)uri->port;
}

os_int
os_uriSetPort(
    os_uri uri,
    const os_char *port)
{
    const os_char *ptr;
    os_char *cpy;
    os_int retcode = OS_RETCODE_BAD_PARAMETER;

    assert (uri != NULL);

    if (port == NULL) {
        os_free (uri->port);
        uri->port = NULL;
        retcode = OS_RETCODE_OK;
    } else {
        ptr = port;
        while (isdigit (*ptr)) {
            ptr++;
        }

        if (*ptr == '\0') {
            cpy = os_strdup (port);
            os_free (uri->port);
            uri->port = cpy;
            retcode = OS_RETCODE_OK;
        }
    }

    return retcode;
}

const os_char *
os_uriGetPath(
    const os_uri uri)
{
    assert (uri != NULL);
    return (const os_char *)uri->path;
}

os_int
os_uriSetPath(
    os_uri uri,
    const os_char *str)
{
    os_char *path, *ptr;
    os_int retcode = OS_RETCODE_OK;

    assert (uri != NULL);

    if (str == NULL) {
        os_free (uri->path);
        uri->path = NULL;
    } else {
        path = uri->path;
        if (uri->userinfo != NULL || uri->host != NULL || uri->port != 0) {
            ptr = os__uriParsePathAbsolute (uri, str);
        } else {
            ptr = os__uriParsePathRootless (uri, str);
        }

        if (ptr == NULL) {
            uri->path = path;
            retcode = OS_RETCODE_OUT_OF_RESOURCES;
        } else if (ptr != str && *ptr == '\0') {
            os_free (path);
        } else {
            if (ptr != str) {
                os_free (uri->path);
            }
            uri->path = path;
            retcode = OS_RETCODE_BAD_PARAMETER;
        }
    }

    return retcode;
}

const os_char *
os_uriGetQuery(
    const os_uri uri)
{
    assert (uri != NULL);
    return (const os_char *)uri->query;
}

os_int
os_uriSetQuery(
    os_uri uri,
    const os_char *str)
{
    os_char *ptr, *query;
    os_int retcode = OS_RETCODE_OK;

    assert (uri != NULL);

    if (str == NULL) {
        os_free (uri->query);
        uri->query = NULL;
    } else {
        query = uri->query;
        ptr = os__uriParseQuery (uri, str);
        if (ptr == NULL) {
            uri->query = query;
            retcode = OS_RETCODE_OUT_OF_RESOURCES;
        } else if (ptr != str && *ptr == '\0') {
            os_free (query);
        } else {
            if (ptr != str) {
                os_free (uri->query);
            }
            uri->query = query;
            retcode = OS_RETCODE_BAD_PARAMETER;
        }
    }

    return retcode;
}

const os_char *
os_uriGetFragment(
    const os_uri uri)
{
    assert (uri != NULL);
    return (const os_char *)uri->fragment;
}

os_int
os_uriSetFragment(
    os_uri uri,
    const os_char *str)
{
    os_char *ptr, *fragment;
    os_int retcode = OS_RETCODE_OK;

    assert (uri != NULL);

    if (str == NULL) {
        os_free (uri->fragment);
        uri->fragment = NULL;
    } else {
        fragment = uri->fragment;
        ptr = os__uriParseFragment (uri, str);
        if (ptr == NULL) {
            uri->query = fragment;
            retcode = OS_RETCODE_OUT_OF_RESOURCES;
        } else if (ptr != str && *ptr == '\0') {
            os_free (fragment);
        } else {
            if (ptr != str) {
                os_free (uri->fragment);
            }
            uri->fragment = fragment;
            retcode = OS_RETCODE_BAD_PARAMETER;
        }
    }

    return retcode;
}

os_int
os_uriGetUser(
    const os_uri uri,
    os_char **user)
{
    os_char *ptr;
    os_int retcode = OS_RETCODE_BAD_PARAMETER;

    assert (uri != NULL);
    assert (user != NULL);

    if (uri->userinfo != NULL) {
        ptr = os_index (uri->userinfo, ':');
        if (ptr != NULL) {
            ptr = os_strndup (uri->userinfo, (os_size_t)(ptr - uri->userinfo));
            if (ptr != NULL) {
                *user = ptr;
                retcode = OS_RETCODE_OK;
            } else {
                retcode = OS_RETCODE_OUT_OF_RESOURCES;
            }
        }
    }

    return retcode;
}

os_int
os_uriGetPassword(
    const os_uri uri,
    os_char **password)
{
    os_char *ptr;
    os_int retcode = OS_RETCODE_BAD_PARAMETER;

    assert (uri != NULL);
    assert (password != NULL);

    if (uri->userinfo != NULL) {
        ptr = os_index (uri->userinfo, ':');
        if (ptr != NULL) {
            ptr = os_strdup (++ptr);
            *password = ptr;
            retcode = OS_RETCODE_OK;
        }
    }

    return retcode;
}

os_int
os_uriGetQueryField(
    const os_uri uri,
    const os_char *field,
    os_char **value)
{
    const os_char *ptr, *qry, *sep;
    os_char *cpy = NULL;
    os_int retcode = OS_RETCODE_BAD_PARAMETER;

    assert (uri != NULL);
    assert (field != NULL);
    assert (value != NULL);

    if (strlen (field) > 0 && uri->query != NULL) {
        qry = uri->query;
        do {
            ptr = field;
            while (*ptr != '\0' && tolower(*ptr) == tolower(*qry)) {
                ptr++;
                qry++;
            }

            sep = qry;
            while (*qry != '\0' && *qry != '&' && *qry != ';') {
                qry++;
            }

            if (*ptr == '\0') {
                if ((qry - sep) <= 1) {
                    retcode = OS_RETCODE_OK;
                } else if (*sep == '=') {
                    cpy = os_strndup ((sep + 1), (os_size_t)(qry - (sep + 1)));
                    if (cpy != NULL) {
                        retcode = OS_RETCODE_OK;
                    } else {
                        retcode = OS_RETCODE_OUT_OF_RESOURCES;
                    }
                }
            }

            if (*qry != '\0') {
                qry++;
            }
        } while (*qry != '\0' && retcode == OS_RETCODE_BAD_PARAMETER);
    }


    if (retcode == OS_RETCODE_OK) {
        *value = cpy;
    }

    return retcode;
}

#if 0
os_result
os_uriEncode (
    os_char **pct_encoded,
    const os_char *plain);

os_result
os_uriDecode (
    os_char **plain,
    const os_char *pct_encoded);
#endif

os_char *
os_scanRegName(
    const os_char *str)
{
    const os_char *dot, *end, *ptr, *punct;

    assert (str != NULL);

    punct = dot = ptr = str;
    do {
        end = ptr;
        if ((end - str) < 255 && (end - dot) < 64) {
            if (isalnum ((unsigned char)*ptr)) {
                ptr++;
            } else if ((ptr - punct) > 1) {
                if (*ptr == '.') {
                    punct = dot = ptr;
                    ptr++;
                } else if (*ptr == '-') {
                    punct = ptr;
                    ptr++;
                }
            }
        }
    } while (end != ptr);

    if (end > str && (isalnum((unsigned char)*end) || *end == '.' || *end == '-')) {
        end = str;
    }

    return (os_char *)end;
}

static os_char *
os__scanIPv4AddressOctet(
    const os_char *str)
{
    const os_char *end;

    assert (str != NULL);

    end = str;

    /* 250 - 255 */
    if ((str[0] == '2') &&
        (str[1] == '5') &&
        (str[2] >= '0' && str[2] <= '5'))
    {
        end = str + 3;
    /* 200 - 249 */
    } else if ((str[0] == '2') &&
               (str[1] >= '0' && str[1] <= '4') &&
               (str[2] >= '0' && str[2] <= '9'))
    {
        end = str + 3;
    /* 100 - 199 */
    } else if ((str[0] == '1') &&
               (str[1] >= '0' && str[1] <= '9') &&
               (str[2] >= '0' && str[2] <= '9'))
    {
        end = str + 3;
    /* 10 - 99 */
    } else if ((str[0] >= '1' && str[0] <= '9') &&
               (str[1] >= '0' && str[1] <= '9'))
    {
        end = str + 2;
    /* 0 - 9 */
    } else if ((str[0] >= '0' && str[0] <= '9'))
    {
        end = str + 1;
    }

    return (os_char *)end;
}

os_char *
os_scanIPv4Address(
    const os_char *str)
{
    const os_char *oct, *end, *ptr;
    os_int cnt = 0;

    assert (str != NULL);

    end = oct = str;
    do {
        ptr = os__scanIPv4AddressOctet (oct);
        if (ptr > oct) {
            if (cnt == 3) {
                cnt++;
            } else if (cnt < 3 && *ptr == '.') {
                cnt++;
                oct = ptr + 1;
            }
        }
    } while (ptr < oct && cnt < 4);

    if (cnt == 4 && !isdigit((unsigned char)*ptr) && *ptr != '.') {
        end = ptr;
    }

    return (os_char *)end;
}

static os_char *
os__scanIPv6AddressQuad(
    const os_char *str)
{
    const os_char *end;

    assert (str != NULL);

    end = str;
    for (; isxdigit ((unsigned char)*end) && (end - str) < 4; end++) {
        /* do nothing */
    }

    return (os_char *)end;
}

#define OS__URI_IPV6_MAX_QUADS 8

os_char *
os_scanIPv6Address(
    const os_char *str)
{
    const os_char *end, *ipv4, *ptr, *sep;
    os_int cnt = 0;
    os_int max = OS__URI_IPV6_MAX_QUADS;

    assert (str != NULL);

    end = ptr = str;
    do {
        sep = ptr;
        if (*sep == ':') {
            sep++;
            if (*sep == ':' && max == OS__URI_IPV6_MAX_QUADS) {
                sep++;
                max--;
            }

            ptr = sep;
        }

        if (cnt < max) {
            /* Maximum number of groups may have been reached. Do not scan for
               another if that is the case. */
            ptr = os__scanIPv6AddressQuad (sep);
            if (ptr > sep && ++cnt < max) {
                /* IPv4 address might be embedded in the low-order 32 bits of
                   the address */
                if (*ptr == '.' && ((cnt == (max - 1)) ||
                                    (max != OS__URI_IPV6_MAX_QUADS)))
                {
                    ipv4 = os_scanIPv4Address (sep);
                    if (ipv4 > sep) {
                        ptr = ipv4;
                        max = ++cnt; /* Stop iteration. */
                    }
                }
            }
        }
    } while (ptr > sep && cnt < max);

    if (!isxdigit((unsigned char)*ptr) && *ptr != ':' &&
            (max == cnt || max != OS__URI_IPV6_MAX_QUADS))
    {
        end = ptr;
    }

    return (os_char *)end;
}

#undef OS__URI_IPV6_MAX_QUADS

os_char *
os_scanHost(
    const os_char *str)
{
    const os_char *end, *ptr;

    assert (str != NULL);

    end = (os_char *)str;
    if ((ptr = os_scanIPv6Address (str)) != str ||
        (ptr = os_scanIPv4Address (str)) != str ||
        (ptr = os_scanRegName (str)) != str)
    {
        end = ptr;
    }

    return (os_char *)end;
}

#undef OS__URI_SCAN_SEGMENT_NONE
#undef OS__URI_SCAN_SEGMENT_NO_STRICT
#undef OS__URI_SCAN_SEGMENT_NO_COLON
