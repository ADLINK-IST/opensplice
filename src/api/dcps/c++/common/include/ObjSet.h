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
#ifndef DDS_OPENSPLICE_OBJECT_SET_H
#define DDS_OPENSPLICE_OBJECT_SET_H

#include "ccpp.h"
#include "vortex_os.h"

OS_CLASS(ut_set);

namespace DDS
{
    namespace OpenSplice
    {
        class ObjSet
        {
        private:
            ut_set myObjSet;
            DDS::Boolean keepRef;

            static os_equality
            fnCompareElements(const DDS::Object_ptr element1, const DDS::Object_ptr element2, void *arg);

            static void
            fnFreeValue(DDS::Object_ptr element, void *arg);

            static os_int32
            objSetActionWrapper(DDS::Object_ptr o, void *arg);

            static DDS::Boolean
            toObjSeq(DDS::Object_ptr element, void *arg);

        public:
            ObjSet(DDS::Boolean keepRef);
            ~ObjSet();

            DDS::ReturnCode_t
            init();

            DDS::ReturnCode_t
            deinit();

            typedef DDS::Boolean (*ObjSetActionFunc) (DDS::Object_ptr element, void *arg);

            DDS::Boolean
            walk(ObjSetActionFunc, void *arg);

            DDS::Boolean
            insertElement(DDS::Object_ptr element);

            DDS::Boolean
            removeElement(DDS::Object_ptr element);

            DDS::Boolean
            containsElement(DDS::Object_ptr element);

            DDS::Object_ptr
            findElement(DDS::Object_ptr element);

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

#endif /* DDS_OPENSPLICE_OBJECT_SET_H */
