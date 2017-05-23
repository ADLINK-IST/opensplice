/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include <stdio.h>

#include "vortex_os.h"
#include "os_cfg.h"
#include "os_report.h"
#include "ut_xmlparser.h"
#include "ut_expand_envvars.h"
#include "cfg_parser.h"
#include "cf_data.h"
#include "cf_node.h"
#include "ctype.h"

cfgprs_status cfg_parse_init(void)
{
    return CFGPRS_OK;
}

cfgprs_status cfg_parse_deinit(void)
{
    return CFGPRS_OK;
}

struct parg {
    int error;
    cf_element *toplevel;
    c_iter stack;
};

static int elem_open_cb(void *varg, os_address parentinfo, os_address *eleminfo, const char *name)
{
    struct parg *parg = varg;
    cf_element e = cf_elementNew(name);
    if (parentinfo) {
        cf_element pe = (cf_element) parentinfo;
        cf_elementAddChild(pe, cf_node(e));
    } else {
        *parg->toplevel = e;
    }
    *eleminfo = (os_address) e;
    parg->stack = c_iterInsert(parg->stack, os_strdup(name));
    return 0;
}

static int elem_attr_cb(void *varg, os_address eleminfo, const char *name, const char *value)
{
    cf_element e = (cf_element) eleminfo;
    c_string xvalue = ut_expand_envvars(value);
    OS_UNUSED_ARG(varg);
    cf_elementAddAttribute(e, cf_attributeNew(name, c_stringValue(xvalue)));
    os_free(xvalue);
    return 0;
}

static int elem_data_cb(void *varg, os_address eleminfo, const char *data)
{
    cf_element e = (cf_element) eleminfo;
    c_string xdata = ut_expand_envvars(data);
    OS_UNUSED_ARG(varg);
    cf_elementAddChild(e, cf_node(cf_dataNew(c_stringValue(xdata))));
    os_free(xdata);
    return 0;
}

static int elem_close_cb(void *varg, os_address eleminfo)
{
    struct parg *parg = varg;
    OS_UNUSED_ARG(eleminfo);
    os_free(c_iterTakeFirst(parg->stack));
    return 0;
}

static void build_path1(char **path, size_t *sz, c_iterIter *it)
{
    char *str = c_iterNext(it);
    if (str) {
        int n;
        build_path1(path, sz, it);
        n = snprintf(*path, *sz, "/%s", str);
        assert(n >= 0);
        if (n > (int) *sz) { n = (int) *sz; }
        *path += n;
        *sz -= (size_t) n;
    }
}

static void build_path(char *path, size_t sz, c_iter stack)
{
    c_iterIter it = c_iterIterGet(stack);
    build_path1(&path, &sz, &it);
}

static void error_cb(void *varg, const char *msg, int line)
{
    struct parg *parg = varg;
    char path[512] = "/";
    parg->error = 1;
    build_path(path, sizeof(path), parg->stack);
    OS_REPORT(OS_ERROR, "configuration parser", 0, "line %d, element %s: %s", line, path, msg);
}

static void init_cb_arg(struct ut_xmlpCallbacks *cb, struct parg *arg, cf_element *spliceElement)
{
    *spliceElement = NULL;

    cb->elem_open = elem_open_cb;
    cb->attr = elem_attr_cb;
    cb->elem_data = elem_data_cb;
    cb->elem_close = elem_close_cb;
    cb->error = error_cb;

    arg->error = 0;
    arg->toplevel = spliceElement;
    arg->stack = c_iterNew(NULL);
}

static void fini_arg(struct parg *parg)
{
    char *str;
    while ((str = c_iterTakeFirst(parg->stack)) != NULL) {
        os_free(str);
    }
    c_iterFree(parg->stack);
}

cfgprs_status cfg_parse_ospl(const char *uristr, cf_element *spliceElement)
{
    struct ut_xmlpCallbacks cb;
    struct ut_xmlpState *st;
    struct parg parg;
    cfgprs_status ret;
    os_cfg_handle *cfg;

    if ((cfg = os_cfgRead(uristr)) == NULL) {
        ret = CFGPRS_NO_INPUT;
        goto err_cfgRead;
    }
    init_cb_arg(&cb, &parg, spliceElement);
    if ((st = ut_xmlpNewString(cfg->ptr, &parg, &cb)) == NULL) {
        OS_REPORT(OS_ERROR, "configuration parser", 0, "Failed to initialize configuration parser");
        ret = CFGPRS_ERROR;
        goto err_xmlpNew;
    }
    if (ut_xmlpParse(st) < 0) {
        if (!parg.error) {
            OS_REPORT(OS_ERROR, "configuration parser", 0, "Failed to parse configuration");
        }
        ret = CFGPRS_ERROR;
        goto err_xmlpParse;
    }
    ret = CFGPRS_OK;
err_xmlpParse:
    ut_xmlpFree(st);
err_xmlpNew:
    fini_arg(&parg);
    os_cfgRelease(cfg);
err_cfgRead:
    return ret;
}

cfgprs_status cfg_parse_str(const char *str, cf_element *spliceElement)
{
    struct ut_xmlpCallbacks cb;
    struct ut_xmlpState *st;
    struct parg parg;
    cfgprs_status ret = CFGPRS_ERROR;
    *spliceElement = NULL;
    init_cb_arg(&cb, &parg, spliceElement);
    if ((st = ut_xmlpNewString(str, &parg, &cb)) == NULL) {
        OS_REPORT(OS_ERROR, "configuration parser", 0, "Failed to initialize configuration parser");
        goto err_xmlpNew;
    }
    if (ut_xmlpParse(st) < 0) {
        if (!parg.error) {
            OS_REPORT(OS_ERROR, "configuration parser", 0, "Failed to parse configuration");
        }
        goto err_xmlpParse;
    }
    ret = CFGPRS_OK;
err_xmlpParse:
    ut_xmlpFree(st);
err_xmlpNew:
    fini_arg(&parg);
    if (ret != CFGPRS_OK && *spliceElement) {
        cf_elementFree(*spliceElement);
    }
    return ret;
}

