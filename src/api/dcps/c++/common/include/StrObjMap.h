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
#ifndef DDS_OPENSPLICE_STRING_MAP_H
#define DDS_OPENSPLICE_STRING_MAP_H

#include "ccpp.h"
#include "vortex_os.h"

OS_CLASS(ut_table);

namespace DDS
{
    namespace OpenSplice
    {
        class StrObjMap
        {
        private:
            ut_table myStrMap;
            DDS::Boolean keepRef;

            static os_equality
            fnCompareElements(const char *element1, const char *element2, void *arg);

            static void
            fnFreeKey(char *element, void *arg);

            static void
            fnFreeValue(DDS::Object_ptr element, void *arg);

            static os_int32
            strObjMapActionWrapper(const char *k, DDS::Object_ptr o, void *arg);

            static DDS::Boolean
            toObjSeq(const char *key, DDS::Object_ptr element, void *arg);

        public:
            StrObjMap(DDS::Boolean keepRef);
            ~StrObjMap();

            DDS::ReturnCode_t
            init();

            DDS::ReturnCode_t
            deinit();

            typedef DDS::Boolean (*StrObjMapActionFunc) (const char *key, DDS::Object_ptr element, void *arg);

            DDS::Boolean
            walk(StrObjMapActionFunc, void *arg);

            const DDS::Object *
            insertElement(const char *key, DDS::Object_ptr element);

            DDS::Object_ptr
            removeElement(const char *key);

            DDS::Object_ptr
            findElement(const char *key);

            DDS::Long
            getNrElements();

            DDS::ObjSeq *
            getObjSeq();

            void
            getObjSeq(DDS::ObjSeq &destination);

            void
            clear();
        };
    }
}

#endif /* DDS_OPENSPLICE_STRING_MAP_H */
