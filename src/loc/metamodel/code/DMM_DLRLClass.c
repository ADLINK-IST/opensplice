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
/*  c includes */
#include <string.h>
#include <stdio.h>
#include <assert.h>

/*  os abstraction layer includes */
#include "os_heap.h"
#include "os_stdlib.h"

/*  DLRL includes */
#include "DLRL_Util.h"
#include "DLRL_Report.h"
#include "Coll_Compare.h"
#include "DMM_DCPSTopic.h"
#include "DMM_DLRLAttribute.h"
#include "DMM_DLRLRelation.h"
#include "DMM_DLRLMultiAttribute.h"
#include "DMM_DLRLMultiRelation.h"
#include "DMM_InheritanceTable.h"
#include "DMM_DLRLClass.h"

#define ENTITY_NAME "DLRL Meta Model DLRLClass"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;
static LOC_string addError = "Unable to add";

DMM_DLRLClass*
DMM_DLRLClass_new(
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string parentName,
    DMM_Mapping mapping,
    void* userData)
{
    DMM_DLRLClass* _this;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(name);
    /* parentName may be null */
    /* userData may be null */
    assert(mapping < DMM_Mapping_elements);

    DLRL_ALLOC(_this, DMM_DLRLClass, exception,  "%s '%s'", allocError, DLRL_VALID_NAME(name));

    _this->mapping = mapping;
    _this->userData = userData;
    _this->mainTopic = NULL;
    _this->extensionTopic = NULL;
    _this->name = NULL;
    _this->nrOfMandatoryRelations = 0;
    _this->parentName = NULL;
    _this->mainTopic = NULL;
    _this->extensionTopic = NULL;
    Coll_Set_init(&(_this->topics), pointerIsLessThen, TRUE);
    Coll_Set_init(&(_this->placeTopics), pointerIsLessThen, TRUE);
    Coll_Set_init(&(_this->multiPlaceTopics), pointerIsLessThen, TRUE);
    Coll_Set_init(&(_this->attributes), pointerIsLessThen, TRUE);
    Coll_List_init(&(_this->singleRelations));
    Coll_List_init(&(_this->multiRelations));

    DLRL_STRDUP(_this->name, name, exception, "%s '%s' name",
        allocError, DLRL_VALID_NAME(name));

    if(parentName){
        DLRL_STRDUP(_this->parentName, parentName, exception, "%s '%s' parentName '%s'",
            allocError, DLRL_VALID_NAME(name), DLRL_VALID_NAME(parentName));
    }

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        DMM_DLRLClass_delete(_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DMM_DLRLClass_delete(
    DMM_DLRLClass* _this)
{
    Coll_Iter* iterator = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    /* _this may be null */

    if (_this){
        if(_this->name){
            os_free(_this->name);
            _this->name = NULL;
        }
        if(_this->parentName){
            os_free(_this->parentName);
            _this->parentName = NULL;
        }
        /* empty the place topic and multi place topics sets first, dont delete the topic, then empty the topics set */
        /* this is where we can delete the topics as well. Then we just set the main topic and extension topic pointers  */
        /* to nil. All topics are contained within the topics set at least, this set is therefore the last in deletions! */
        iterator = Coll_Set_getFirstElement(&(_this->placeTopics));
        while(iterator){
            void* aPlaceTopic = Coll_Iter_getObject(iterator);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->placeTopics), aPlaceTopic);
        }
        iterator = Coll_Set_getFirstElement(&(_this->multiPlaceTopics));
        while(iterator){
            void* aMultiPlaceTopic = Coll_Iter_getObject(iterator);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->multiPlaceTopics), aMultiPlaceTopic);
        }
        iterator = Coll_Set_getFirstElement(&(_this->topics));
        while(iterator){
            void* aTopic = Coll_Iter_getObject(iterator);
            DMM_DCPSTopic_delete((DMM_DCPSTopic*)aTopic);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->topics), aTopic);
        }
        _this->mainTopic = NULL;
        _this->extensionTopic = NULL;
        while(Coll_List_getNrOfElements(&(_this->singleRelations)) > 0){
            DMM_DLRLRelation* aRelation = (DMM_DLRLRelation*)Coll_List_popBack(&(_this->singleRelations));
            DMM_DLRLRelation_delete(aRelation);
        }
        while(Coll_List_getNrOfElements(&(_this->multiRelations)) > 0){
            DMM_DLRLMultiRelation* aRelation = (DMM_DLRLMultiRelation*)Coll_List_popBack(&(_this->multiRelations));
            DMM_DLRLMultiRelation_delete(aRelation);
        }
        iterator = Coll_Set_getFirstElement(&(_this->attributes));
        while(iterator){
            DMM_DLRLAttribute* anAttribute = (DMM_DLRLAttribute*)Coll_Iter_getObject(iterator);
            if(DMM_DLRLAttribute_getClassID(anAttribute) == DMM_DLRL_MULTI_ATTRIBUTE_CLASS){
                DMM_DLRLMultiAttribute_delete((DMM_DLRLMultiAttribute*)anAttribute);
            } else {/* must be simple base class. */
                DMM_DLRLAttribute_delete(anAttribute);
            }
            iterator            =    Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->attributes), (void*)anAttribute);
        }
        /*  delete the struct itself */
        os_free(_this);
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
    }
    DLRL_INFO(INF_EXIT);
}

DMM_DCPSTopic*
DMM_DLRLClass_getMainTopic(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->mainTopic;
}

LOC_string
DMM_DLRLClass_getName(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->name;
}

LOC_string
DMM_DLRLClass_getParentName(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->parentName;
}

DMM_Mapping
DMM_DLRLClass_getMapping(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->mapping;
}

void
DMM_DLRLClass_setMainTopic(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DCPSTopic* mainTopic)
{
    LOC_long returnCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(mainTopic);
    assert(exception);
    assert(DMM_DCPSTopic_getTopicName(mainTopic));

    if(DMM_DLRLClass_findTopicByName(_this, DMM_DCPSTopic_getTopicName(mainTopic))){
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION, "Topic %s already exists within %s '%s'",
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(mainTopic)), ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    returnCode =  Coll_Set_add(&(_this->topics), (void *)mainTopic);
    if(returnCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s main topic  '%s' to topic set of %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(mainTopic)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    _this->mainTopic = mainTopic;
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLClass_setExtensionTopic(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DCPSTopic* extensionTopic)
{
    LOC_long returnCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(extensionTopic);
    assert(exception);

    if(DMM_DLRLClass_findTopicByName(_this, DMM_DCPSTopic_getTopicName(extensionTopic))){
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION, "Topic %s already exists within %s '%s'",
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(extensionTopic)), ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    returnCode =  Coll_Set_add(&(_this->topics), (void *)extensionTopic);
    if(returnCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s extension topic '%s' to topic set of %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(extensionTopic)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    _this->extensionTopic = extensionTopic;
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DMM_DCPSTopic*
DMM_DLRLClass_getExtensionTopic(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->extensionTopic;
}

Coll_Set*
DMM_DLRLClass_getPlaceTopics(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->placeTopics);
}

Coll_Set*
DMM_DLRLClass_getMultiPlaceTopics(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->multiPlaceTopics);
}

Coll_List*
DMM_DLRLClass_getRelations(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->singleRelations);
}

Coll_List*
DMM_DLRLClass_getMultiRelations(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->multiRelations);
}

Coll_Set*
DMM_DLRLClass_getAttributes(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->attributes);
}

void
DMM_DLRLClass_addPlaceTopic(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DCPSTopic* placeTopic)
{
    LOC_long returnCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(placeTopic);

    if(DMM_DLRLClass_findTopicByName(_this, DMM_DCPSTopic_getTopicName(placeTopic))){
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION, "Topic %s already exists within %s '%s'",
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(placeTopic)), ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    returnCode =  Coll_Set_add(&(_this->topics), (void *)placeTopic);
    if(returnCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s place topic  '%s' to topics set of %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(placeTopic)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    returnCode =  Coll_Set_add(&(_this->placeTopics), (void *)placeTopic);
    if(returnCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s place topic '%s' to place topic set %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(placeTopic)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLClass_addMultiPlaceTopic(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DCPSTopic* multiPlaceTopic)
{
    LOC_long returnCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(multiPlaceTopic);

    if(DMM_DLRLClass_findTopicByName(_this, DMM_DCPSTopic_getTopicName(multiPlaceTopic))){
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION, "Topic %s already exists within %s '%s'",
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(multiPlaceTopic)), ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    returnCode =  Coll_Set_add(&(_this->topics), (void *)multiPlaceTopic);
    if(returnCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s multi place topic  '%s' to topics set of %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(multiPlaceTopic)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    returnCode =  Coll_Set_add(&(_this->multiPlaceTopics), (void *)multiPlaceTopic);
    if(returnCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s multi place topic '%s' to multi place topic set of %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(multiPlaceTopic)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLClass_addRelation(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DLRLRelation* relation)
{
    LOC_long returnCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(relation);

    if(DMM_DLRLRelation_getClassID(relation) == DMM_DLRL_RELATION_CLASS){
        returnCode =  Coll_List_pushBack(&(_this->singleRelations), (void *)relation);
    } else {
        assert(DMM_DLRLRelation_getClassID(relation) == DMM_DLRL_MULTI_RELATION_CLASS);
        returnCode =  Coll_List_pushBack(&(_this->multiRelations), (void *)relation);
    }
    if(returnCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s relation '%s' to %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLClass_addAttribute(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DLRLAttribute* attribute)
{
    LOC_long returnCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(attribute);

    returnCode =  Coll_Set_add(&(_this->attributes), (void *)attribute);
    if(returnCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s attribute '%s' to %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DLRLAttribute_getName(attribute)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DMM_DCPSTopic*
DMM_DLRLClass_findTopicByName(
    DMM_DLRLClass* _this,
    LOC_string topicName)
{
    DMM_DCPSTopic* returnValue = NULL;
    Coll_Iter* iterator;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(topicName);

    iterator = Coll_Set_getFirstElement(&(_this->topics));
    while(iterator && !returnValue){
        DMM_DCPSTopic* aTopic = (DMM_DCPSTopic*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DCPSTopic_getTopicName(aTopic), topicName)){
            returnValue = aTopic;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_INFO(INF_EXIT);
    return returnValue;
}


DMM_DCPSTopic*
DMM_DLRLClass_findMultiPlaceTopicByName(
    DMM_DLRLClass* _this,
    LOC_string topicName)
{
    DMM_DCPSTopic* returnValue = NULL;
    Coll_Iter* iterator;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(topicName);

    iterator = Coll_Set_getFirstElement(&(_this->multiPlaceTopics));
    while(iterator && !returnValue){
        DMM_DCPSTopic* aTopic = (DMM_DCPSTopic*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DCPSTopic_getTopicName(aTopic), topicName)){
            returnValue = aTopic;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_INFO(INF_EXIT);
    return returnValue;
}

DMM_DLRLAttribute*
DMM_DLRLClass_findAttributeByName(
    DMM_DLRLClass* _this,
    LOC_string attributeName)
{
    DMM_DLRLAttribute* returnValue = NULL;
    Coll_Iter* iterator;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(attributeName);

    iterator = Coll_Set_getFirstElement(&(_this->attributes));
    while(iterator && !returnValue){
        DMM_DLRLAttribute* anAttribute = (DMM_DLRLAttribute*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DLRLAttribute_getName(anAttribute), attributeName)){
            returnValue = anAttribute;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

DMM_DLRLRelation*
DMM_DLRLClass_findRelationByName(
    DMM_DLRLClass* _this,
    const char* relationName)
{
    DMM_DLRLRelation* returnValue = NULL;
    Coll_Iter* iterator = NULL;
    DMM_DLRLRelation* aRelation = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(relationName);

    iterator = Coll_List_getFirstElement(&(_this->singleRelations));
    while(iterator && !returnValue){
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DLRLRelation_getName(aRelation), relationName)){
            returnValue = aRelation;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    iterator = Coll_List_getFirstElement(&(_this->multiRelations));
    while(iterator && !returnValue){
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DLRLRelation_getName(aRelation), relationName)){
            returnValue = aRelation;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

DMM_DLRLRelation*
DMM_DLRLClass_getSingleRelation(
    DMM_DLRLClass* _this,
    LOC_unsigned_long relationIndex)
{
    DMM_DLRLRelation* returnValue = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);

    if(relationIndex < Coll_List_getNrOfElements(&(_this->singleRelations))){
        returnValue = (DMM_DLRLRelation*)Coll_List_getObject(&(_this->singleRelations), relationIndex);
    }
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

DMM_DLRLMultiRelation*
DMM_DLRLClass_findMultiRelationByName(
    DMM_DLRLClass* _this,
    LOC_string relationName)
{
    DMM_DLRLMultiRelation* returnValue = NULL;
    Coll_Iter* iterator = NULL;
    DMM_DLRLRelation* aRelation = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(relationName);

    iterator = Coll_List_getFirstElement(&(_this->multiRelations));
    while(iterator && !returnValue){
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DLRLRelation_getName(aRelation), relationName)){
            returnValue = (DMM_DLRLMultiRelation*)aRelation;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

Coll_Set*
DMM_DLRLClass_getTopics(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->topics);
}

void*
DMM_DLRLClass_getUserData(
    DMM_DLRLClass* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->userData;
}

LOC_unsigned_long
DMM_DLRLClass_getNrOfMandatorySingleRelations(
    DMM_DLRLClass* _this)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->nrOfMandatoryRelations;
}

void
DMM_DLRLClass_calculateNrOfMandatorySingleRelations(
    DMM_DLRLClass* _this)
{
    DMM_DLRLRelation* aRelation = NULL;
    Coll_Iter* iterator = NULL;

    DLRL_INFO(INF_ENTER);

    assert(_this);

    iterator = Coll_List_getFirstElement(&(_this->singleRelations));
    while(iterator){
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        if(!DMM_DLRLRelation_isOptional(aRelation)){
            _this->nrOfMandatoryRelations++;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_INFO(INF_EXIT);

}
