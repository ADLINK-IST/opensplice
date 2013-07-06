static const char version[] = "$Id$";

/*
* factory.c
*
* Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
* See the COPYING file for the terms of usage and distribution.
*/

#include <sd/factory.h>
#include <stdlib.h>
#include <string.h>
#include <sd/malloc.h>
#include <sd/hash.h>
#include <sd/error.h>

struct __sd_factory
{
  char*			fac_name;
  const sd_factory_ops_t*	fac_ops;
  sd_hash_t*			fac_hash;
};

/* xxx FIXME: unsafe hand made polymorphism ...
* This is used to access the name field of objects created by
* factories.  For example instances of appenders, layouts or
* rollingpolicies.
* So there's a supposition that the name field is always first--this is true
* but should enforce that probably by extending a base 'name' struct.
*/
typedef struct {
  char* pr_name;
} sd_factory_product_t;

/*******************************************************************************/
extern sd_factory_t* sd_factory_new(const char* a_name,
  const sd_factory_ops_t* a_ops)
{
  sd_factory_t* this;
  
  if (!a_name || !a_ops)
    return NULL;
  
  this           = sd_calloc(1, sizeof(*this));
  this->fac_name = sd_strdup(a_name);
  this->fac_ops  = a_ops;
  this->fac_hash = sd_hash_new(20, NULL);
  return this;
}

/*******************************************************************************/
extern void sd_factory_delete(sd_factory_t* this)
{
  sd_hash_iter_t* i;
  
  sd_debug("sd_factory_delete['%s',",
    (this && (this->fac_name) ? this->fac_name: "(no name)"));
  
  if (!this){
    goto sd_factory_delete_exit;
  }
  
  if (this->fac_ops->fac_delete){
    for (i = sd_hash_begin(this->fac_hash); i != sd_hash_end(this->fac_hash); 
      i = sd_hash_iter_next(i)) {
      this->fac_ops->fac_delete(i->data);
    }
  }
  
  sd_hash_delete(this->fac_hash);
  free(this->fac_name);
  free(this);
  
  sd_factory_delete_exit:
  sd_debug("]");
}

/*******************************************************************************/
extern void* sd_factory_get(sd_factory_t* this, const char* a_name)
{
  sd_hash_iter_t*		i;
  sd_factory_product_t*	pr;	
  
  if ( (i = sd_hash_lookup(this->fac_hash, a_name)) != NULL)
    return i->data;
  
  if (!this->fac_ops->fac_new)
    return NULL;
  
  if ( (pr = this->fac_ops->fac_new(a_name)) == NULL)
    return NULL;
  
  sd_hash_add(this->fac_hash, pr->pr_name, pr);
  return pr;
  
}

/*******************************************************************************/
extern void sd_factory_destroy(sd_factory_t* this, void* a_pr)
{
  sd_factory_product_t* pr = (sd_factory_product_t*) a_pr;
  
  sd_hash_del(this->fac_hash, pr->pr_name);
  if (this->fac_ops->fac_delete)
    this->fac_ops->fac_delete(pr);
}

/*******************************************************************************/
extern void sd_factory_print(const sd_factory_t* this, FILE* a_stream)
{
  sd_hash_iter_t* i;
  
  if (!this)
    return;
  
  if (!this->fac_ops->fac_print)
    return;
  
  fprintf(a_stream, "factory[%s]:\n", this->fac_name);
  for (i = sd_hash_begin(this->fac_hash); i != sd_hash_end(this->fac_hash); 
    i = sd_hash_iter_next(i)) 
  {
    this->fac_ops->fac_print(i->data, a_stream);
    fprintf(a_stream, "\n");
  }
}

/******************************************************************************/
extern int sd_factory_list(const sd_factory_t* this, void** a_items,
  int a_nitems)
{
  sd_hash_iter_t* i;
  int j;
  
  if (!this || !a_items || a_nitems <= 0)
    return -1;
  
    for (i = sd_hash_begin(this->fac_hash), j = 0;
      i != sd_hash_end(this->fac_hash);
      i = sd_hash_iter_next(i), j++)
    {
      if (j < a_nitems)
        a_items[j] = i->data;
    }
    
    return j;
}
