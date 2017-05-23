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
#ifndef CPP_DDS_OPENSPLICE_ENTITYCONTAINER_H
#define CPP_DDS_OPENSPLICE_ENTITYCONTAINER_H


namespace DDS {
    namespace OpenSplice {
        class EntityContainer
        {
        protected:
            /*
             * Cleanup
             */
            template <class TFactory>
            DDS::ReturnCode_t
            wlReq_deleteFactoryList(
                DDS::OpenSplice::ObjSet *entityList)
            {
                DDS::ReturnCode_t result = DDS::RETCODE_OK;
                DDS::ReturnCode_t endResult = DDS::RETCODE_OK;
                DDS::ObjSeq_var objSeq = entityList->getObjSeq();
                DDS::ULong i, nrEntities = objSeq->length();

                /* When an error is detected during the deletion of an entity,
                 * than continue deleting the next instances so that you delete
                 * as much as possible. However, save the error you encountered
                 * in the endResult, so that the caller knows that not everything
                 * deleted successfully.
                 */
                for (i = 0; i < nrEntities; i++) {
                    TFactory entity = dynamic_cast<TFactory>(objSeq[i].in());
                    result = entity->delete_contained_entities();
                    if (result == DDS::RETCODE_OK) {
                        result = entity->deinit();
                        if (result == DDS::RETCODE_OK) {
                            (void) entityList->removeElement(entity);
                        } else {
                            endResult = result;
                        }
                    } else {
                        endResult = result;
                    }
                }
                return endResult;
            }

            template <class TEntity>
            DDS::ReturnCode_t
            wlReq_deleteEntityList(
                DDS::OpenSplice::ObjSet *entityList)
            {
                DDS::ReturnCode_t result = DDS::RETCODE_OK;
                DDS::ReturnCode_t endResult = DDS::RETCODE_OK;
                DDS::ObjSeq_var objSeq = entityList->getObjSeq();
                DDS::ULong i, nrEntities = objSeq->length();

                /* When an error is detected during the deletion of an entity,
                 * than continue deleting the next instances so that you delete
                 * as much as possible. However, save the error you encountered
                 * in the endResult, so that the caller knows that not everything
                 * deleted successfully.
                 */
                for (i = 0; i < nrEntities; i++) {
                    TEntity entity = dynamic_cast<TEntity>(objSeq[i].in());
                    result = entity->deinit();
                    if (result == DDS::RETCODE_OK) {
                        (void) entityList->removeElement(entity);
                    } else {
                        endResult = result;
                    }
                }
                return endResult;
            }

        };
    }
}

#endif /* CPP_DDS_OPENSPLICE_ENTITYCONTAINER_H */
