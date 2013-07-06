static const char version[] = "$Id$";

/* 
 * Copyright 2002, Meiosys SA (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

/* domnode - simple XML DOM-like interface
 * Copyright (c) 2002 Michael B. Allen <mballen@erols.com>
 *
 * The MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sd/domnode.h>
#include <sd/stack.h>
#include <sd/malloc.h>
#include <sd/error.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_LANGINFO_H
#   include <langinfo.h>
#endif

#include <assert.h>
#include <expat.h>

struct udata {
    char 	  cdata[2048];
    size_t	  wptr;
    sd_stack_t*	  elements;
    sd_domnode_t* root;
};

/******************************************************************************/
static struct udata* udata_new(void)
{
    struct udata* this;

    this = sd_calloc(1, sizeof(*this));

    this->cdata[0]	= 0;
    this->wptr		= 0;
    this->elements	= sd_stack_new(0);
    this->root		= NULL;

    return this;
}

/******************************************************************************/
static void udata_delete(struct udata* this)
{
    if (!this)
	return;

    sd_stack_delete(this->elements, NULL);
    free(this);
}

/******************************************************************************/
static void udata_pop_cdata(struct udata* this)
{
    this->wptr = 0;
    memset(this->cdata, 0, sizeof(this->cdata));
}

/******************************************************************************/
static int udata_push_cdata(struct udata* this, const XML_Char* s, int len)
{
    XML_Char *str;
    
    if (!this || !s || !len)
	return -1;

    str = (XML_Char *)s + len;
    while (s < str && isspace(*s)) {
	s++;
	len--;
    }

    if (s == str) return -1;

    str = (XML_Char *)s + len - 1;
    while (str > s && isspace(*str)) {
	str--;
	len--;
    }

    if ((this->wptr + len) >= sizeof(this->cdata) - 1){
	sd_error("cdata buffer exceeded (maximum %d bytes)\n", 
		 sizeof(this->cdata));
	return -1;	
    }

    strncpy(&this->cdata[this->wptr], s, len);
    this->wptr += len;
    this->cdata[this->wptr] = 0;

    return 0;
}

/******************************************************************************/
static int foreach_delete(sd_domnode_t* anode, void* unused)
{
    sd_domnode_delete(anode);
    return 0;
}

/******************************************************************************/
extern sd_domnode_t* 
__sd_domnode_new(const char* name, const char* value, int is_elem)
{
    sd_domnode_t* this;

    this = sd_calloc(1, sizeof(*this));

    this->name     = name    ? sd_strdup(name) : NULL;
    this->value    = value   ? sd_strdup(value): NULL;
    this->children = is_elem ? sd_list_new(10) : NULL;
    this->attrs    = is_elem ? sd_list_new(10) : NULL;

    return this;
}

/******************************************************************************/
extern sd_domnode_t* sd_domnode_new(const char* name, const char* value)
{
    return __sd_domnode_new(name, value, 1);
}

/******************************************************************************/
extern void sd_domnode_delete(sd_domnode_t* this)
{
    if (!this)
	return;

    free((void*) this->name);
    free((void*) this->value);

    sd_list_foreach(this->children, (sd_list_func_t) foreach_delete, NULL);
    sd_list_delete(this->children);

    sd_list_foreach(this->attrs, (sd_list_func_t) foreach_delete, NULL);
    sd_list_delete(this->attrs);

    free(this);
}

/******************************************************************************/
static void 
start_handler(struct udata* udata, const XML_Char* name, const XML_Char** atts)
{
    sd_domnode_t* parent;
    sd_domnode_t* child;
    int i;

    if (!udata || !name || !atts)
	return;

    child = __sd_domnode_new(name, NULL, 1);

    for (i = 0; atts[i]; i += 2)
	sd_list_add(child->attrs, __sd_domnode_new(atts[i], atts[i + 1], 0));
    
    udata_pop_cdata(udata);

    /* root node has no parent. save it. */
    if ( (parent = sd_stack_peek(udata->elements)) == NULL)
	udata->root = child;
    else
	sd_list_add(parent->children, child);

    sd_stack_push(udata->elements, child);
}

/******************************************************************************/
static void end_handler(struct udata* udata, const XML_Char* name)
{
    udata_pop_cdata(udata);
    sd_stack_pop(udata->elements);
}

/******************************************************************************/
static void cdata_handler(struct udata* udata, const XML_Char* s, int len)
{
    sd_domnode_t* parent = sd_stack_peek(udata->elements);

    assert(parent != NULL);

    if (udata_push_cdata(udata, s, len) == -1)
	return;

    free((void*) parent->value);
    parent->value = strdup(udata->cdata);
}

/******************************************************************************/
static void comment_handler(struct udata* udata, const XML_Char* s)
{
    sd_domnode_t* parent = sd_stack_peek(udata->elements);

    assert(parent != NULL);

    sd_list_add(parent->children, __sd_domnode_new("#comment", s, 0));
}

/******************************************************************************/
extern int sd_domnode_fread(sd_domnode_t* this, FILE* stream)
{
    XML_Parser   p;
    struct udata* udata;
    int           ret  = 0;

    if (!this || !stream)
	return -1;

    if ((p = XML_ParserCreate(NULL)) == NULL)
	return -1;

    udata = udata_new();

    XML_SetStartElementHandler  (p, (XML_StartElementHandler)  start_handler);
    XML_SetEndElementHandler    (p, (XML_EndElementHandler)    end_handler);
    XML_SetCharacterDataHandler (p, (XML_CharacterDataHandler) cdata_handler);
    XML_SetCommentHandler       (p, (XML_CommentHandler)       comment_handler);
    XML_SetUserData             (p, udata);

    for ( ;; ) {
	size_t n;
	void* buf;
	int done;

	if ((buf = XML_GetBuffer(p, BUFSIZ)) == NULL) {
	    ret = -1;
	    break;
	}

	if ((n = fread(buf, 1, BUFSIZ, stream)) == 0 && ferror(stream)) {
	    ret = -1;
	    break;
	}

	if (!XML_ParseBuffer(p, n, (done = feof(stream)))) {
	    sd_error("XML error: %s [%d:%d - %ld]\n",
		     XML_ErrorString(XML_GetErrorCode(p)),
		     XML_GetCurrentLineNumber(p),
		     XML_GetCurrentColumnNumber(p),
		     XML_GetCurrentByteIndex(p));
	    ret = -1;
	    break;
	}

	if (done)
	    break;
    }

    if (udata->root)
    {
	free((void*) this->name);
	free((void*) this->value);
	sd_list_foreach(this->children, (sd_list_func_t) foreach_delete, NULL);
	sd_list_delete(this->children);
	sd_list_foreach(this->attrs, (sd_list_func_t) foreach_delete, NULL);
	sd_list_delete(this->attrs);

	this->name     = udata->root->name;
	this->value    = udata->root->value;
	this->children = udata->root->children;
	this->attrs    = udata->root->attrs;

	free(udata->root);
	udata->root = NULL;
    }

    udata_delete(udata);
    XML_ParserFree(p);
    return ret;
}

/******************************************************************************/
extern int 
sd_domnode_read(sd_domnode_t* this, const char* abuffer, size_t asize)
{
    XML_Parser  p;
    struct udata* udata;
    int           ret = 0;

    if (!this)
	return -1;

    if ((p = XML_ParserCreate(NULL)) == NULL)
	return -1;

    udata = udata_new();

    XML_SetStartElementHandler  (p, (XML_StartElementHandler)  start_handler);
    XML_SetEndElementHandler    (p, (XML_EndElementHandler)    end_handler);
    XML_SetCharacterDataHandler (p, (XML_CharacterDataHandler) cdata_handler);
    XML_SetCommentHandler       (p, (XML_CommentHandler)       comment_handler);
    XML_SetUserData             (p, udata);

    if (!XML_Parse(p, abuffer, asize, 1)) {
	sd_error("XML error: %s [%d:%d - %ld]\n", 
		 XML_ErrorString(XML_GetErrorCode(p)),
		 XML_GetCurrentLineNumber(p),
		 XML_GetCurrentColumnNumber(p),
		 XML_GetCurrentByteIndex(p));
	ret = -1;
    }
    
    if (udata->root)
    {
	free((void*) this->name);
	free((void*) this->value);
	sd_list_foreach(this->children, (sd_list_func_t) foreach_delete, NULL);
	sd_list_delete(this->children);
	sd_list_foreach(this->attrs, (sd_list_func_t) foreach_delete, NULL);
	sd_list_delete(this->attrs);

	this->name     = udata->root->name;
	this->value    = udata->root->value;
	this->children = udata->root->children;
	this->attrs    = udata->root->attrs;

	free(udata->root);
	udata->root = NULL;
    }

    udata_delete(udata);
    XML_ParserFree(p);
    return ret;
}

/******************************************************************************/
static int _sd_domnode_fwrite(const sd_domnode_t* this, FILE* stream, int indent)
{
    sd_list_iter_t* iter;
    int i;

    if (!this || !this->name || !stream)
	return -1;
    
    for (i = 0; i < indent; i++)
	fprintf(stream, "    ");

    if (this->name && strcmp(this->name, "#comment") == 0) {
	fprintf(stream, "<!-- %s -->\n", this->value);
	return 0;
    } 

    fprintf(stream, "<%s", this->name);

    for (iter = sd_list_begin(this->attrs); iter != sd_list_end(this->attrs);
	 iter = sd_list_iter_next(iter)) {
	sd_domnode_t* node = iter->data;
	
	fprintf(stream, " %s=\"%s\"", node->name, node->value);
    }
    
    if (this->value || sd_list_get_nelem(this->children)) {
	fprintf(stream, ">\n");

	if (this->value) {
	    for (i = 0; i < indent + 1; i++)
		fprintf(stream, "    ");
	    fprintf(stream, "%s\n", this->value);
	}
    
	for (iter = sd_list_begin(this->children); iter != sd_list_end(this->children);
	     iter = sd_list_iter_next(iter)) {
	    sd_domnode_t* node = iter->data;
	    
	    if (_sd_domnode_fwrite(node, stream, indent + 1) == -1)
		return -1;
	}
    
	for (i = 0; i < indent; i++)
	    fprintf(stream, "    ");	
	fprintf(stream, "</%s>\n", this->name);
    } else {
	fprintf(stream, "/>\n");
    }

    return 0;
}

/******************************************************************************/
extern int sd_domnode_fwrite(const sd_domnode_t* this, FILE* stream)
{
#ifdef HAVE_NL_LANGINFO
    fprintf(stream, "<?xml version=\"1.0\" encoding=\"%s\"?>\n\n", 
	    nl_langinfo(CODESET));
#else
    fprintf(stream, "<?xml version=\"1.0\"?>\n\n");
#endif

    return _sd_domnode_fwrite(this, stream, 0);
}

/******************************************************************************/
extern int sd_domnode_load(sd_domnode_t* this, const char* afilename)
{
    FILE* fp;
    int   ret = 0;

    if ( (fp = fopen(afilename, "r")) == NULL)
	return -1;
    
    ret = sd_domnode_fread(this, fp);

    fclose(fp);
    return 0;
}

/******************************************************************************/
extern int sd_domnode_store(const sd_domnode_t* this, const char* afilename)
{
    FILE* fp;
    int   ret = 0;

    if ( (fp = fopen(afilename, "w")) == NULL)
	return -1;
    
    ret = sd_domnode_fwrite(this, fp);

    fclose(fp);
    return 0;
}

/******************************************************************************/
extern sd_domnode_t* 
sd_domnode_search(const sd_domnode_t* this, const char* name)
{
    sd_list_iter_t* i;

    for (i = sd_list_begin(this->children); i != sd_list_end(this->children); 
	 i = sd_list_iter_next(i)) {
	sd_domnode_t* node = i->data;
	
	if (strcmp(node->name, name) == 0)
	    return node;
    }

    for (i = sd_list_begin(this->attrs); i != sd_list_end(this->attrs); 
	 i = sd_list_iter_next(i)) {
	sd_domnode_t* node = i->data;
	
	if (strcmp(node->name, name) == 0)
	    return node;
    }

    for (i = sd_list_begin(this->children); i != sd_list_end(this->children); 
	 i = sd_list_iter_next(i)) {
	sd_domnode_t* node = i->data;
	
	if ((node = sd_domnode_search(node, name)) != NULL)
	    return node;
    }

    return NULL;
}

/******************************************************************************/
extern sd_domnode_t* 
sd_domnode_attrs_put(sd_domnode_t* anode, sd_domnode_t* attr)
{
    sd_list_iter_t* i;
    
    if (!anode || !anode->attrs || !attr || !attr->value)
	return NULL;

    if ( (i = sd_list_lookadd(anode->attrs, attr)) == sd_list_end(anode->attrs))
	return NULL;

    return i->data;
}

/******************************************************************************/
extern sd_domnode_t* 
sd_domnode_attrs_get(const sd_domnode_t* anode, const char* name)
{
    sd_list_iter_t* i;

    if (!anode || !anode->attrs || !name || !*name)
	return NULL;

    for (i = sd_list_begin(anode->attrs); i != sd_list_end(anode->attrs); 
	 i = sd_list_iter_next(i)) {
	sd_domnode_t* node = i->data;
	
	if (strcmp(node->name, name) == 0)
	    return node;
    }

    return NULL;
}

/******************************************************************************/
extern sd_domnode_t* 
sd_domnode_attrs_remove(sd_domnode_t* anode, const char* name)
{
    sd_list_iter_t* i;

    if (!anode || !anode->attrs || !name || !*name)
	return NULL;

    for (i = sd_list_begin(anode->attrs); i != sd_list_end(anode->attrs); 
	 i = sd_list_iter_next(i)) {
	sd_domnode_t* node = i->data;
	
	if (strcmp(node->name, name) == 0) {
	    sd_list_iter_del(i);
	    return node;
	}
    }

    return NULL;
}
