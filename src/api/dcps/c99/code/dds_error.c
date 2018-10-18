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

/* OS */
#include <os_defs.h>
#include <os_atomics.h>

/* SAC */
#include <dds_dcps.h>
#include <dds_dcps_private.h>

/* C99 */
#include <dds.h>
#include <dds/error.h>


#define DDS_ERR_CODE_NUM 12
#define DDS_ERR_MOD_NUM 14
#define DDS_ERR_MSG_MAX 128


#define DDS_ERR_NO_INDEX(e) (((-e) & DDS_ERR_NO_MASK) -1)
#define DDS_ERR_MOD_INDEX(e) ((((-e) & DDS_ERR_MOD_MASK) >> 8) -1)


static const char * dds_err_code_array[DDS_ERR_CODE_NUM] =
{
  "Error",
  "Unsupported",
  "Bad Parameter",
  "Precondition Not Met",
  "Out Of Resources",
  "Not Enabled",
  "Immutable Policy",
  "Inconsistent Policy",
  "Already Deleted",
  "Timeout",
  "No Data",
  "Illegal Operation"
};

/* See DDS_MOD_XXX in dds_types.h */

static const char * dds_err_module_array[DDS_ERR_MOD_NUM] =
{
  "QoS",
  "Kernel",
  "DDSI",
  "Stream",
  "Alloc",
  "WaitSeT",
  "Reader",
  "Writer",
  "Condition",
  "ReadCache",
  "Status",
  "Thread",
  "Instance",
  "Participant"
};


static pa_voidp_t dds_fail_func = PA_VOIDP_INIT(NULL);

const char * dds_err_str (int err)
{
    unsigned index = DDS_ERR_NO_INDEX (err);
    if (err >= 0)
    {
        return "Success";
    }
    if (index >= DDS_ERR_CODE_NUM)
    {
        return "Unknown";
    }
    return dds_err_code_array[index];
}

const char * dds_err_mod_str (int err)
{
    unsigned index = DDS_ERR_MOD_INDEX(err);
    if (index >= DDS_ERR_MOD_NUM)
    {
        return "Unknown";
    }
    return dds_err_module_array[index];
}

bool dds_err_check (int err, unsigned flags, const char * where)
{
    if (err < 0) {
        if ((flags & DDS_CHECK_REPORT) != 0) {
            /* Error has already been reported. */
        }

        if ((flags & DDS_CHECK_FAIL) != 0) {
            char msg[DDS_ERR_MSG_MAX];
            snprintf (msg, DDS_ERR_MSG_MAX, "Error %s:%s:M%u", dds_err_mod_str(err), dds_err_str(err), dds_err_minor(err));

            dds_fail(msg, where);
        }

        if ((flags & DDS_CHECK_EXIT) != 0) {
            exit(-1);
        }
    }

    return (err >= 0);
}


void dds_fail_set (dds_fail_fn fn)
{
    pa_stvoidp(&dds_fail_func, (void*)fn);
}

dds_fail_fn dds_fail_get (void)
{
    return (dds_fail_fn)pa_ldvoidp(&dds_fail_func);
}

void dds_fail (const char * msg, const char * where)
{
    dds_fail_fn func = dds_fail_get();
    if (func != NULL) {
        func(msg, where);
    }
}
