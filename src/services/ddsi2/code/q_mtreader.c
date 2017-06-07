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
#include <stddef.h>
#include <assert.h>

#include "ut_avl.h"
#include "ut_fibheap.h"

#include "c_iterator.h"
#include "c_stringSupport.h"

#include "v_state.h"
#include "v_topic.h"

#include "u_topic.h"
#include "u_observable.h"

#include "q_error.h"
#include "q_config.h"
#include "q_time.h"
#include "q_globals.h"
#include "q_log.h"
#include "q_mtreader.h"

struct mtreader {
  unsigned ntopics;
  struct u_topic_s **topics;
  c_field *gid_field;
  ut_avlTree_t samples;
  os_int64 dur_expire;
  ut_fibheap_t expiry_heap;
};

static int cmp_gid_sysId_localId (const void *va, const void *vb);
static int expiry_time_cmp (const void *va, const void *vb);

static const ut_avlTreedef_t mtr_samples_td = UT_AVL_TREEDEF_INITIALIZER (offsetof (struct mtr_sample, avlnode), offsetof (struct mtr_sample, gid), cmp_gid_sysId_localId, 0);

static const ut_fibheapDef_t mtr_fibheap_def = UT_FIBHEAPDEF_INITIALIZER (offsetof (struct mtr_sample, fhnode), expiry_time_cmp);

static int cmp_gid_sysId_localId (const void *va, const void *vb)
{
  const v_gid *a = va;
  const v_gid *b = vb;
  if (a->systemId < b->systemId)
    return -1;
  else if (a->systemId > b->systemId)
    return 1;
  else if (a->localId < b->localId)
    return -1;
  else if (a->localId > b->localId)
    return 1;
  else
    return 0;
}

static c_field get_gid_field (struct v_topic_s *topic)
{
  static const char *msg = "mtreader: keys do not match expectation";
  const struct c_type_s *utype = v_topicDataType (topic);
  char *ukeys = os_strdup (v_topicKeyExpr (topic));
  c_iter key_iter;
  int saw_systemId = 0, saw_localId = 0;
  char *k, *root = NULL;
  c_field gid_field = NULL;

  /* Expecting "key.systemId,key.localId", or the reverse, and just for kicks we allow
     the "key" to be any name but require it to be of type v_builtinTopicKey = v_gid */
  key_iter = c_splitString (ukeys, ", \t");
  while ((k = c_iterTakeFirst (key_iter)) != NULL)
  {
    char *k2;
    if ((k2 = strrchr (k, '.')) == NULL)
    {
      NN_ERROR2 ("%s (0; %s)\n", msg, k);
      os_free (k);
      goto out;
    }
    *k2++ = 0;
    if (strcmp (k2, "systemId") == 0)
      saw_systemId++;
    else if (strcmp (k2, "localId") == 0)
      saw_localId++;
    else
    {
      NN_ERROR3 ("%s (1; %s.%s)\n", msg, k, k2);
      os_free (k);
      goto out;
    }

    if (root == NULL)
      root = k;
    else
    {
      if (strcmp (k, root) != 0)
      {
        NN_ERROR3 ("%s (2; %s,%s)\n", msg, k, root);
        os_free (k);
        goto out;
      }
      os_free (k);
    }
  }

  /* Expect systemId and localId exactly once */
  if (saw_systemId != 1 || saw_localId != 1)
  {
    NN_ERROR3 ("%s (3; %d,%d)\n", msg, saw_systemId, saw_localId);
    goto out;
  }

  /* "base" key field should have the expected type, and if it has, we keep the field object
     to later use c_fieldBlobCopy to extract it from the v_message we receive */
  {
    c_field fld;
    c_type fld_type, btk_type;
    if ((btk_type = c_resolve (gv.ospl_base, "kernelModule::v_builtinTopicKey")) == NULL)
    {
      NN_ERROR1 ("%s (4; can't find typename)", msg);
      goto out;
    }
    if ((fld = c_fieldNew ((c_type) utype, root)) == NULL)
    {
      NN_ERROR2 ("%s (5; %s)\n", msg, root);
      c_free (btk_type);
      goto out;
    }
    fld_type = c_fieldType (fld);
    if (c_metaCompare ((c_metaObject) btk_type, (c_metaObject) fld_type) != E_EQUAL)
      NN_ERROR1 ("%s (6)\n", msg);
    else
      gid_field = c_keep (fld);
    c_free (fld_type);
    c_free (fld);
    c_free (btk_type);
  }

out:
  while ((k = c_iterTakeFirst (key_iter)) != NULL)
    os_free (k);
  c_iterFree (key_iter);
  if (root)
    os_free (root);
  os_free (ukeys);
  return gid_field;
}

static void get_gid_field_action (v_public vtopic, void *varg)
{
  c_field *arg = varg;
  *arg = get_gid_field (v_topic (vtopic));
}

static int expiry_time_cmp (const void *va, const void *vb)
{
  const struct mtr_sample *a = va;
  const struct mtr_sample *b = vb;
  if (a->texpire.v < b->texpire.v)
    return -1;
  else if (a->texpire.v > b->texpire.v)
    return 1;
  else
    return 0;
}

struct mtreader *new_mtreader (unsigned ntopics, struct u_topic_s **topics)
{
  struct mtreader *mtr;
  unsigned i;
  mtr = os_malloc (sizeof (*mtr));
  mtr->ntopics = ntopics;
  mtr->topics = os_malloc (ntopics * sizeof (*mtr->topics));
  mtr->gid_field = os_malloc (ntopics * sizeof (*mtr->gid_field));
  for (i = 0; i < ntopics; i++)
  {
    c_field fld;
    if (u_observableAction ((u_observable) topics[i], get_gid_field_action, &fld) != U_RESULT_OK || fld == NULL)
    {
      NN_ERROR0 ("new_mtreader: gid not found in topic\n");
      goto gid_not_found;
    }
    mtr->topics[i] = topics[i];
    mtr->gid_field[i] = fld;
  }
  ut_avlInit (&mtr_samples_td, &mtr->samples);
  ut_fibheapInit (&mtr_fibheap_def, &mtr->expiry_heap);
  /* expire known dead entities after very long time (there's also durability
     involved, after all) */
  mtr->dur_expire = 1 * T_SECOND;
  return mtr;

gid_not_found:
  os_free (mtr->gid_field);
  os_free (mtr->topics);
  os_free (mtr);
  return NULL;
}

void free_mtr_sample (struct mtreader *mtr, struct mtr_sample *s)
{
  /* Samples only get deleted/freed via the expiry mechanism */
  unsigned i;
  for (i = 0; i < mtr->ntopics; i++)
    c_free (s->vmsg[i]);
  os_free (s);
}

static void expire_one_mtr_sample (struct mtreader *mtr)
{
  ut_avlDPath_t dp;
  struct mtr_sample *s, *s1;
  s = ut_fibheapExtractMin (&mtr_fibheap_def, &mtr->expiry_heap);
  assert (s != NULL);
  if ((s1 = ut_avlLookupDPath (&mtr_samples_td, &mtr->samples, &s->gid, &dp)) != NULL && s == s1)
  {
    TRACE (("expire_one_mtr_sample: deleting %x:%x:%x\n", s->gid.systemId, s->gid.localId, s->gid.serial));
    ut_avlDeleteDPath (&mtr_samples_td, &mtr->samples, s1, &dp);
  }
  free_mtr_sample (mtr, s);
}

static void sched_expiry (struct mtreader *mtr, struct mtr_sample *s, nn_mtime_t texpire)
{
  if (s->texpire.v == T_NEVER)
  {
    s->texpire = texpire;
    ut_fibheapInsert (&mtr_fibheap_def, &mtr->expiry_heap, s);
  }
  else if (texpire.v < s->texpire.v)
  {
    s->texpire = texpire;
    ut_fibheapDecreaseKey (&mtr_fibheap_def, &mtr->expiry_heap, s);
  }
  else
  {
    /* not supposed to delay expiry (if that's desired, see q_lease.c */
    assert (texpire.v == s->texpire.v);
  }
}

static void sched_expiry_voidwrapper (void *vs, void *vmtr)
{
  static const nn_mtime_t mtime_zero = { 0 };
  sched_expiry (vmtr, vs, mtime_zero);
}

void delete_mtreader (struct mtreader *mtr)
{
  unsigned i;
  ut_avlFreeArg (&mtr_samples_td, &mtr->samples, sched_expiry_voidwrapper, mtr);
  while (ut_fibheapMin (&mtr_fibheap_def, &mtr->expiry_heap))
    expire_one_mtr_sample (mtr);
  for (i = 0; i < mtr->ntopics; i++)
    c_free (mtr->gid_field[i]);
  os_free (mtr->gid_field);
  os_free (mtr->topics);
  os_free (mtr);
}

static struct mtr_sample *new_mtr_sample (struct mtreader *mtr, const v_gid *gid)
{
  struct mtr_sample *s;
#if __STDC_VERSION__ >= 199901L
  if ((s = os_malloc (sizeof (*s) + mtr->ntopics * sizeof (*s->vmsg))) == NULL)
    return NULL;
#else
  if ((s = os_malloc (sizeof (*s) + (mtr->ntopics-1) * sizeof (*s->vmsg))) == NULL)
    return NULL;
#endif
  memset (s->vmsg, 0, mtr->ntopics * sizeof (*s->vmsg));
  s->gid = *gid;
  s->ntopics = 0;
  s->state = MTR_SST_NEW;
  s->texpire.v = T_NEVER;
  s->flag = 0;
  return s;
}

static int update_new (struct mtreader *mtr, const struct mtr_sample *out[2], unsigned idx, struct v_message_s *vmsg, const v_gid *gid)
{
  struct mtr_sample *s;
  int ret = 0;
  if ((s = new_mtr_sample (mtr, gid)) == NULL)
    return ERR_OUT_OF_MEMORY;
  s->vmsg[idx] = c_keep (vmsg);
  s->ntopics++;
  ut_avlInsert (&mtr_samples_td, &mtr->samples, s);
  if (s->ntopics == mtr->ntopics)
    out[ret++] = s;
  return ret;
}

static int update_existing (struct mtreader *mtr, const struct mtr_sample *out[2], unsigned idx, struct v_message_s *vmsg, struct mtr_sample *s, const v_gid *gid)
{
  int ret = 0;
  if (gid->serial < s->gid.serial)
  {
    /* silently discard samples for outdated objects */
  }
  else if (gid->serial > s->gid.serial)
  {
    /* samples for newer objects cause the old one to be deleted and a new one to be created */
    static const nn_mtime_t mtime_zero = { 0 };
    if (s->ntopics == mtr->ntopics && s->state != MTR_SST_DEL)
    {
      s->state = MTR_SST_DEL;
      out[ret++] = s;
    }
    ut_avlDelete (&mtr_samples_td, &mtr->samples, s);
    sched_expiry (mtr, s, mtime_zero);
    return update_new (mtr, &out[ret], idx, vmsg, gid);
  }
  else if (s->state == MTR_SST_DEL)
  {
    /* Silently discard updates to something that's already dead, unless the incoming one has a newer timestamp */
  }
  else if (s->vmsg[idx] != NULL)
  {
    /* QoS change or similar */
    c_free (s->vmsg[idx]);
    s->vmsg[idx] = c_keep (vmsg);
    if (s->ntopics == mtr->ntopics)
    {
      s->state = MTR_SST_UPD;
      out[ret++] = s;
    }
  }
  else
  {
    /* Working our way towards completion,  */
    s->vmsg[idx] = c_keep (vmsg);
    s->ntopics++;
    if (s->ntopics == mtr->ntopics)
    {
      s->state = MTR_SST_NEW;
      out[ret++] = s;
    }
  }
  return ret;
}

static int update_new_dead (struct mtreader *mtr, const v_gid *gid)
{
  struct mtr_sample *s;
  if ((s = new_mtr_sample (mtr, gid)) == NULL)
    return ERR_OUT_OF_MEMORY;
  s->state = MTR_SST_DEL;
  ut_avlInsert (&mtr_samples_td, &mtr->samples, s);
  sched_expiry (mtr, s, add_duration_to_mtime (now_mt (), mtr->dur_expire));
  return 0;
}

static int update_existing_dead (struct mtreader *mtr, const struct mtr_sample *out[2], struct mtr_sample *s, const v_gid *gid)
{
  int ret = 0;
  if (gid->serial < s->gid.serial)
  {
    /* silently discard samples for outdated objects */
  }
  else if (gid->serial > s->gid.serial)
  {
    /* samples for newer objects cause the old one to be deleted and a new one to be created */
    static const nn_mtime_t mtime_zero = { 0 };
    if (s->ntopics == mtr->ntopics && s->state != MTR_SST_DEL)
    {
      s->state = MTR_SST_DEL;
      out[ret++] = s;
    }
    ut_avlDelete (&mtr_samples_td, &mtr->samples, s);
    sched_expiry (mtr, s, mtime_zero);
    return update_new_dead (mtr, gid);
  }
  else if (s->state == MTR_SST_DEL)
  {
    /* Silently discard updates to something that's already dead */
  }
  else
  {
    /* schedule for removal, inform caller if complete */
    s->state = MTR_SST_DEL;
    if (s->ntopics == mtr->ntopics)
      out[ret++] = s;
    sched_expiry (mtr, s, add_duration_to_mtime (now_mt (), mtr->dur_expire));
  }
  return ret;
}

static unsigned lookup_topic_idx (const struct mtreader *mtr, const struct u_topic_s *tp)
{
  unsigned i;
  /* Lookup reader to get index in array of readers - this is for small numbers of readers,
   so a linear scan has good performance - and extract the gid from the topic.  User type
   sits at vmsg+1. */
  for (i = 0; i < mtr->ntopics; i++)
  {
    if (mtr->topics[i] == tp)
      break;
  }
  assert (i < mtr->ntopics);
  return i;
}

int update_mtreader (struct mtreader *mtr, const struct mtr_sample *out[2], const struct u_topic_s *tp, v_state sample_state, struct v_message_s *vmsg)
{
  v_gid gid;
  unsigned idx;
  int ret = 0;

  /* delete expired samples (which may include some that have been returned by the
     previous call to update_mtreader) */
  {
    nn_mtime_t tnow = now_mt ();
    struct mtr_sample *s;
    while ((s = ut_fibheapMin (&mtr_fibheap_def, &mtr->expiry_heap)) != NULL && s->texpire.v <= tnow.v)
      expire_one_mtr_sample (mtr);
  }

  idx = lookup_topic_idx (mtr, tp);
  c_fieldBlobCopy (mtr->gid_field[idx], vmsg + 1, &gid);

  if (config.enabled_logcats & LC_TRACE)
  {
    char *tpname = u_topicName (mtr->topics[idx]);
    TRACE (("update_mtreader: %s gid %x:%x:%x sample state %x vmsg state %x\n", tpname, gid.systemId, gid.localId, gid.serial, (unsigned) sample_state, (unsigned) vmsg->_parent.nodeState));
    os_free (tpname);
  }

  /* process write aspect of message */
  if (vmsg->_parent.nodeState & L_WRITE)
  {
    struct mtr_sample *s;
    if ((s = ut_avlLookup (&mtr_samples_td, &mtr->samples, &gid)) == NULL)
      ret = update_new (mtr, out, idx, vmsg, &gid);
    else
      ret = update_existing (mtr, out, idx, vmsg, s, &gid);
  }
  /* then dispose/unregister aspect */
  if (sample_state & (L_DISPOSED | L_UNREGISTER))
  {
    struct mtr_sample *s;
    if ((s = ut_avlLookup (&mtr_samples_td, &mtr->samples, &gid)) == NULL)
      ret = update_new_dead (mtr, &gid);
    else
      ret = update_existing_dead (mtr, out, s, &gid);
  }
  return ret;
}

int update_mtreader_setflag (struct mtreader *mtr, const v_gid *key, unsigned flag)
{
  struct mtr_sample *s;
  if ((s = ut_avlLookup (&mtr_samples_td, &mtr->samples, key)) == NULL)
    return ERR_INVALID_DATA;
  else
  {
    s->flag = flag;
    return 0;
  }
}

static c_field make_query_field (v_topic tp, const char *field)
{
  c_type utype = v_topicDataType (tp);
  c_field fld;
  if ((fld = c_fieldNew (utype, field)) == NULL)
  {
    char *tpname = v_topicName (tp);
    NN_ERROR2 ("query_mtreader_eq: field %s not found in topic %s\n", field, tpname);
  }
  return fld;
}

struct make_query_field_action_arg {
  const char *field;
  c_field res;
};

static void make_query_field_action (v_public vtopic, void *varg)
{
  struct make_query_field_action_arg *arg = varg;
  arg->res = make_query_field ((v_topic) vtopic, arg->field);
}

int query_mtreader (const struct mtreader *mtr, const struct mtr_sample ***result, const struct u_topic_s *tp, const char *field, int (*pred) (const void *a, const void *b), const void *b)
{
  unsigned idx = lookup_topic_idx (mtr, tp);
  int rc;
  c_field fld;

  {
    struct make_query_field_action_arg arg;
    arg.field = field;
    if (u_observableAction ((u_observable) tp, make_query_field_action, &arg) != U_RESULT_OK || arg.res == NULL)
      return ERR_UNSPECIFIED;
    fld = arg.res;
  }

  {
    ut_avlIter_t it;
    struct mtr_sample *s;
    unsigned n;
    /* Two-pass: count first, then allocate & return */
    n = 0;
    for (s = ut_avlIterFirst (&mtr_samples_td, &mtr->samples, &it); s != NULL; s = ut_avlIterNext (&it))
    {
      if (s->ntopics == mtr->ntopics && s->state != MTR_SST_DEL && !s->flag && pred (c_fieldGetAddress (fld, s->vmsg[idx] + 1), b))
        n++;
    }
    if (n == 0)
    {
      *result = NULL;
      rc = 0;
    }
    else if ((*result = os_malloc ((unsigned) n * sizeof (**result))) == NULL)
    {
      rc = ERR_OUT_OF_MEMORY;
    }
    else
    {
      unsigned i = 0;
      for (s = ut_avlIterFirst (&mtr_samples_td, &mtr->samples, &it); s != NULL; s = ut_avlIterNext (&it))
      {
        if (s->ntopics == mtr->ntopics && s->state != MTR_SST_DEL && !s->flag && pred (c_fieldGetAddress (fld, s->vmsg[idx] + 1), b))
        {
          /* s must have been returned in the past, and we return it as NEW only once */
          s->state = MTR_SST_UPD;
          (*result)[i++] = s;
        }
      }
      assert (i == n);
      rc = (int) n;
    }
  }

  c_free (fld);
  return rc;
}

const struct mtr_sample *mtr_first (const struct mtreader *mtr, struct mtr_iter *it)
{
  return ut_avlIterFirst (&mtr_samples_td, &mtr->samples, &it->it);
}

const struct mtr_sample *mtr_next (struct mtr_iter *it)
{
  return ut_avlIterNext (&it->it);
}


/* SHA1 not available (unoffical build.) */
