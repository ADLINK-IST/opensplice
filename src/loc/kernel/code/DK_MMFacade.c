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
/* c includes */
#include <assert.h>
#include "os_stdlib.h"

/* database includes */
#include "c_iterator.h"
#include "c_stringSupport.h"

/* user includes */
#include "u_entity.h"

/* DLRL util includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL Metamodel includes */
#include "DMM_DCPSField.h"
#include "DMM_DCPSTopic.h"
#include "DMM_DLRLAttribute.h"
#include "DMM_DLRLMultiAttribute.h"
#include "DMM_DLRLMultiRelation.h"
#include "DMM_InheritanceTable.h"
#include "DMM_Types.h"

/* DLRL kernel includes */
#include "DK_DCPSUtility.h"
#include "DK_MMFacade.h"
#include "DK_ObjectHomeAdmin.h"
#include "DLRL_Kernel_private.h"

static void
DK_MMFacade_us_resolveRelation(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    DMM_DLRLRelation* relation);

static void
DK_MMFacade_us_resolveRelationImmutability(
    DK_ObjectHomeAdmin* home,
    DMM_DLRLRelation* relation);

static DMM_DLRLClass*
DK_MMFacade_us_resolveRelationType(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    DMM_DLRLClass* owner,
    DMM_DLRLRelation* relation);

static void
DK_MMFacade_us_mapDLRLMetaModelToDCPSMetaModel(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception);

/* NOT IN DESIGNstatic void DK_MMFacade_us_resolveDatabaseTopicForDCPSTopic(DK_ObjectHomeAdmin* home, DLRL_Exception* exception, */
/*                                                     DK_TopicInfo* topicInfo); */
/* NOT IN DESIGN - param changed */
static void
DK_MMFacade_us_resolveDatabaseTopicFieldsForDMMTopic(
    DLRL_Exception* exception,
    DK_TopicInfo* topicInfo);


void
DK_MMFacade_us_createDLRLClass(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    const LOC_string parentName,
    DMM_Mapping mapping)
{
    DLRL_INFO(INF_ENTER);

    assert(mapping < DMM_Mapping_elements);
    assert(home);
    assert(exception);
    /* parentName may be null */

    home->meta_representative = DMM_DLRLClass_new(exception, home->name, parentName, mapping, (void*)DK_Entity_ts_duplicate((DK_Entity*)home));
    /* if an exception occurred, release the duplicated pointer before propagating! */
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        DK_Entity_ts_release((DK_Entity*)home);
    }
    DLRL_Exception_PROPAGATE(exception);
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_createMainTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName)
{
    DMM_DCPSTopic* mainTopic = NULL;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(name);
    /* typeName may be null */

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Failed to create main topic % because no class type meta information is known for this home",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name));
    }

    mainTopic = DMM_DCPSTopic_new(exception, name, typeName);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLClass_setMainTopic(home->meta_representative, exception, mainTopic);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && mainTopic)
    {
        DMM_DCPSTopic_delete(mainTopic);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_createExtensionTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName)
{
    DMM_DCPSTopic* extensionTopic = NULL;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(name);
    /* typeName may be null */

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Failed to create extension topic % because no class type meta information is known for this home",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name));
    }
    extensionTopic = DMM_DCPSTopic_new(exception, name, typeName);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLClass_setExtensionTopic(home->meta_representative, exception, extensionTopic);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && extensionTopic)
    {
        DMM_DCPSTopic_delete(extensionTopic);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_createPlaceTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName)
{
    DMM_DCPSTopic* placeTopic;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(name);
    /* typeName may be null */

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Failed to create place topic % because no class type meta information is known for this home",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name));
    }
    placeTopic = DMM_DCPSTopic_new(exception, name, typeName);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLClass_addPlaceTopic(home->meta_representative, exception, placeTopic);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_createMultiPlaceTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName)
{
    DMM_DCPSTopic* multiPlaceTopic;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(name);
    /* typeName may be null */

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Failed to create multi place topic % because no class type meta information is known for this home",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name));
    }
    multiPlaceTopic = DMM_DCPSTopic_new(exception, name, typeName);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLClass_addMultiPlaceTopic(home->meta_representative, exception, multiPlaceTopic);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_createDCPSField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    DMM_KeyType fieldType,
    DMM_AttributeType type,
    LOC_string owningTopicName)
{
    DMM_DCPSTopic* aTopic;
    DMM_DCPSField* aField;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(name);
    assert(fieldType < DMM_KeyType_elements);
    assert(type < DMM_AttributeType_elements);
    assert(owningTopicName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add an field %s to the DCPS topic %s. "
            "No class type meta information is known for this home",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name), DLRL_VALID_NAME(owningTopicName));
    }

    aTopic = DMM_DLRLClass_findTopicByName(home->meta_representative, owningTopicName);
    if(!aTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add an field %s to the DCPS topic %s. "
            "No meta information found for the DCPS topic",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name), DLRL_VALID_NAME(owningTopicName));
    }
    aField = DMM_DCPSField_new(exception, name, fieldType, type, aTopic);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DCPSTopic_addField(aTopic, exception, aField);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_createRelation(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_boolean isComposition,
    LOC_string name,
    LOC_string typeName,
    LOC_string associatedRelationName,
    LOC_boolean isOptional)
{
    DMM_DLRLRelation* relation;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(name);
    assert(typeName);
    /* associatedRelationName may be null */

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to create relation %s with type %s. "
            "No class type meta information is known for this home to attach the relation too.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name), DLRL_VALID_NAME(typeName));
    }
    relation = DMM_DLRLRelation_new(exception, isComposition, name, typeName, associatedRelationName,
                                    home->meta_representative, isOptional);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLClass_addRelation(home->meta_representative, exception, relation);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_createMultiRelation(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_boolean isComposition,
    LOC_string name,
    LOC_string typeName,
    LOC_string associatedRelationName,
    DMM_Basis basis)
{
    DMM_DLRLMultiRelation* multiRelation;
    LOC_unsigned_long index = 0;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(name);
    assert(typeName);
    /* associatedRelationName may be null */
    assert(basis < DMM_Basis_elements);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to create multi relation %s with type %s. "
            "No class type meta information is known for this home to attach the multi relation too.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name), DLRL_VALID_NAME(typeName));
    }
    index = Coll_List_getNrOfElements(DMM_DLRLClass_getMultiRelations(home->meta_representative));
    /* multi relation is always mandatory i.e. we dont support NULL pointers in multi relations */
    multiRelation = DMM_DLRLMultiRelation_new(exception, isComposition, name, typeName, associatedRelationName,
                                                basis, home->meta_representative, index, FALSE);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLClass_addRelation(home->meta_representative, exception, (DMM_DLRLRelation*)multiRelation);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_createAttribute(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_boolean isImmutable,
    DMM_AttributeType type)
{
    DMM_DLRLAttribute* attribute;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(name);
    assert(type < DMM_AttributeType_elements);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to create attribute %s. "
            "No class type meta information is known for this home to attach the attribute too.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name));
    }
    attribute = DMM_DLRLAttribute_new(exception, name, isImmutable, type, home->meta_representative);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLClass_addAttribute(home->meta_representative, exception, attribute);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_createMultiAttribute(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_boolean isImmutable,
    DMM_AttributeType type,
    DMM_Basis basis)
{
    DMM_DLRLMultiAttribute* multiAttribute;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(name);
    assert(type < DMM_AttributeType_elements);
    assert(basis < DMM_Basis_elements);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to create multi attribute %s. "
            "No class type meta information is known for this home to attach the multi attribute too.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(name));
    }
    multiAttribute = DMM_DLRLMultiAttribute_new(exception, name, isImmutable, type, basis, home->meta_representative);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLClass_addAttribute(home->meta_representative, exception, (DMM_DLRLAttribute*)multiAttribute);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_mapDLRLAttributeToDCPSField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string attributeName,
    LOC_string fieldName)
{
    DMM_DLRLAttribute* theAttribute;
    DMM_DCPSTopic* theTopic;
    DMM_DCPSField* theField;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(attributeName);
    assert(fieldName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map attribute %s to DCPS field %s. "
            "No class type meta information is known for this home.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(fieldName));
    }
    theAttribute = DMM_DLRLClass_findAttributeByName(home->meta_representative, attributeName);
    if(!theAttribute)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map attribute %s to DCPS field %s. "
            "Unable to find the specified attribute within the type represented by this home.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(fieldName));
    }
    theTopic = DMM_DLRLAttribute_getTopic(theAttribute);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map attribute %s to DCPS field %s. "
            "No DCPS topic has been mapped on the specified attribute.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(fieldName));
    }
    theField = DMM_DCPSTopic_findFieldByName(theTopic, fieldName);
    if(!theField)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map attribute %s to DCPS field %s. "
            "The DCPS field cannot be found within topic %s", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
            DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(fieldName),
           DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(theTopic)));
    }
    DMM_DLRLAttribute_setField(theAttribute, theField);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_mapDLRLMultiAttributeToIndexDCPSField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string attributeName,
    LOC_string indexFieldName)
{
    DMM_DLRLAttribute* theAttribute;
    DMM_DCPSTopic* theTopic;
    DMM_DCPSField* theField;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(attributeName);
    assert(indexFieldName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map multi attribute %s to index DCPS field %s. "
            "No class type meta information is known for this home.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(indexFieldName));
    }

    theAttribute = DMM_DLRLClass_findAttributeByName(home->meta_representative, attributeName);
    if(!theAttribute)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map multi attribute %s to index DCPS field %s. "
            "Unable to find the multi attribute.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(indexFieldName));
    }
    if(DMM_DLRLAttribute_getClassID(theAttribute) != DMM_DLRL_MULTI_ATTRIBUTE_CLASS)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map multi attribute %s to index DCPS field %s. "
            "The attribute found is not a multi attribute.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(indexFieldName));
    }

    theTopic = DMM_DLRLAttribute_getTopic(theAttribute);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map multi attribute %s to index DCPS field %s. "
            "No topic has been mapped for this multi attribute.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(indexFieldName));
    }

    theField = DMM_DCPSTopic_findFieldByName(theTopic, indexFieldName);
    if(!theField)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map multi attribute %s to index DCPS field %s. "
            "Cannot locate a field with name %s within the topic %s of the multi attribute.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(indexFieldName),
            DLRL_VALID_NAME(indexFieldName), DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(theTopic)));
    }
    if(DMM_DCPSField_getFieldType(theField) != DMM_KEYTYPE_KEY || DMM_DCPSField_getFieldType(theField) != DMM_KEYTYPE_SHARED_KEY)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map multi attribute %s to index DCPS field %s. "
            "The field with name %s within the topic %s of the multi attribute is not a key field.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName),
            DLRL_VALID_NAME(indexFieldName), DLRL_VALID_NAME(indexFieldName),
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(theTopic)));
    }

    DMM_DLRLMultiAttribute_setIndexField((DMM_DLRLMultiAttribute*)theAttribute, theField);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_mapDLRLAttributeToDCPSTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string attributeName,
    LOC_string topicName)
{
    DMM_DLRLAttribute* theAttribute;
    DMM_DCPSTopic* theTopic;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(attributeName);
    assert(topicName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map attribute %s to DCPS topic %s. "
            "No class type meta information is known for this home.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
            DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(topicName));
    }

    theAttribute = DMM_DLRLClass_findAttributeByName(home->meta_representative, attributeName);
    if(!theAttribute)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map attribute %s to DCPS topic %s. "
            "Cannot locate the attribute within the meta model class representation of this ObjectHome.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName), DLRL_VALID_NAME(topicName));
    }
    theTopic = DMM_DLRLClass_findTopicByName(home->meta_representative, topicName);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to map attribute %s to DCPS topic %s. "
            "Cannot locate DCPS topic %s.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(attributeName),
            DLRL_VALID_NAME(topicName), DLRL_VALID_NAME(topicName));
    }
    DMM_DLRLAttribute_setTopic(theAttribute, theTopic);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_setDLRLRelationValidityField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string validityFieldName)
{
    DMM_DLRLRelation* theRelation;
    DMM_DCPSTopic* theTopic;
    DMM_DCPSField* theField;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(relationName);
    assert(validityFieldName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to set a validity field  to relation %s with field %s"
            "No class type meta information is known for this home.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
            DLRL_VALID_NAME(relationName), DLRL_VALID_NAME(validityFieldName));
    }
    theRelation = DMM_DLRLClass_findRelationByName(home->meta_representative, relationName);
    if(!theRelation)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to set a validity field  to relation %s with field %s"
            "Cannot locate the relation.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationName),
            DLRL_VALID_NAME(validityFieldName));
    }

    theTopic = DMM_DLRLRelation_getOwnerTopic(theRelation);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to set a validity field  to relation %s with field %s"
            "No DCPS topic has been mapped to the relation.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
             DLRL_VALID_NAME(relationName), DLRL_VALID_NAME(validityFieldName));
    }

    theField = DMM_DCPSTopic_findFieldByName(theTopic, validityFieldName);
    if(!theField)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to set a validity field  to relation %s with field %s"
            "Cannot locate the dcps field %s in the relations topic %s.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationName), DLRL_VALID_NAME(validityFieldName),
            DLRL_VALID_NAME(validityFieldName), DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(theTopic)));
    }

    if((DMM_DCPSField_getFieldType(theField) == DMM_KEYTYPE_FOREIGN_KEY) ||
            (DMM_DCPSField_getFieldType(theField) == DMM_KEYTYPE_SHARED_KEY))
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to set a validity field  to relation %s with field %s"
            "The located relation field %s in the relations topic %s is indicated as a foreign or shared "
            "key field in the topic.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationName),
            DLRL_VALID_NAME(validityFieldName), DLRL_VALID_NAME(validityFieldName),
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(theTopic)));
    }

    DMM_DLRLRelation_setValidityField(theRelation, exception, theField);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_addDLRLRelationKeyFieldPair(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string ownerKeyName,
    LOC_string targetKeyName)
{
    DMM_DLRLRelation* theRelation;
    DMM_DCPSTopic* theTopic;
    DMM_DCPSField* theField;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(relationName);
    assert(ownerKeyName);
    assert(targetKeyName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to add a key field pair to relation %s with relation key field %s and target topic key field %s. "
            "No class type meta information is known for this home.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
            DLRL_VALID_NAME(relationName), DLRL_VALID_NAME(ownerKeyName), DLRL_VALID_NAME(targetKeyName));
    }

    theRelation = DMM_DLRLClass_findRelationByName(home->meta_representative, relationName);
    if(!theRelation)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to add a key field pair to relation %s with relation key field %s and target topic key field %s. "
            "Cannot locate the relation.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationName),
            DLRL_VALID_NAME(ownerKeyName), DLRL_VALID_NAME(targetKeyName));
    }

    theTopic = DMM_DLRLRelation_getOwnerTopic(theRelation);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to add a key field pair to relation %s with relation key field %s and target topic key field %s. "
            "No DCPS topic has been mapped to the relation.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
             DLRL_VALID_NAME(relationName), DLRL_VALID_NAME(ownerKeyName), DLRL_VALID_NAME(targetKeyName));
    }

    theField = DMM_DCPSTopic_findFieldByName(theTopic, ownerKeyName);
    if(!theField)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to add a key field pair to relation %s with relation key field %s and target topic key field %s. "
            "Cannot locate the relation key field %s in the relations topic %s.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationName), DLRL_VALID_NAME(ownerKeyName),
            DLRL_VALID_NAME(targetKeyName), DLRL_VALID_NAME(ownerKeyName),
           DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(theTopic)));
    }
    if((DMM_DCPSField_getFieldType(theField) != DMM_KEYTYPE_FOREIGN_KEY) &&
            (DMM_DCPSField_getFieldType(theField) != DMM_KEYTYPE_SHARED_KEY))
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to add a key field pair to relation %s with relation key field %s and target topic key field %s. "
            "The located relation key field %s in the relations topic %s is not indicated as a foreign or shared "
            "key field in the topic.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationName),
            DLRL_VALID_NAME(ownerKeyName), DLRL_VALID_NAME(targetKeyName), DLRL_VALID_NAME(ownerKeyName),
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(theTopic)));
    }

    DMM_DLRLRelation_addOwnerKey(theRelation, exception, theField);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLRelation_addTargetKeyName(theRelation, exception, targetKeyName);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_setDLRLRelationTopicPair(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string ownerTopicName,
    LOC_string targetTopicName)
{
    DMM_DLRLRelation* theRelation;
    DMM_DCPSTopic* theTopic;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(home);
    assert(ownerTopicName);
    assert(relationName);
    assert(targetTopicName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to add a topic pair to relation %s with relation topic %s and target topic %s. "
            "No class type meta information is known for this home.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
            DLRL_VALID_NAME(relationName), DLRL_VALID_NAME(ownerTopicName), DLRL_VALID_NAME(targetTopicName));
    }

    theRelation = DMM_DLRLClass_findRelationByName(home->meta_representative, relationName);
    if(!theRelation)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to add a topic pair to relation %s with relation topic %s and target topic %s. "
            "Cannot locate the relation.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationName),
            DLRL_VALID_NAME(ownerTopicName), DLRL_VALID_NAME(targetTopicName));
    }

    theTopic = DMM_DLRLClass_findTopicByName(home->meta_representative, ownerTopicName);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. "
            "Unable to add a topic pair to relation %s with relation topic %s and target topic %s. "
            "Cannot locate the relations topic.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
            DLRL_VALID_NAME(relationName), DLRL_VALID_NAME(ownerTopicName), DLRL_VALID_NAME(targetTopicName));
    }

    DMM_DLRLRelation_setTargetTopicName(theRelation, exception, targetTopicName);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLRelation_setOwnerTopic(theRelation, theTopic);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_addOwnerField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string fieldName)
{
    DMM_DLRLRelation* theRelation;
    DMM_DCPSTopic* theTopic;
    DMM_DCPSField* theField;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(relationName);
    assert(fieldName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add owner field %s to relation %s "
            "Reason: No class type meta information is known for this home.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(fieldName), DLRL_VALID_NAME(relationName));
    }

    theRelation = DMM_DLRLClass_findRelationByName(home->meta_representative, relationName);
    if(!theRelation)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add owner field %s to relation %s "
            "Reason: Cannot locate the relation.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(fieldName), DLRL_VALID_NAME(relationName));
    }
    theTopic = DMM_DLRLRelation_getOwnerTopic(theRelation);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add owner field %s to relation %s "
            "Reason: No DCPS topic has been mapped to the relation.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(fieldName), DLRL_VALID_NAME(relationName));
    }

    theField = DMM_DCPSTopic_findFieldByName(theTopic, fieldName);
    if(!theField)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add owner field %s to relation %s "
            "Reason: Cannot locate the relation owner field in the relations topic.","DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(fieldName), DLRL_VALID_NAME(relationName));
    }

    DMM_DLRLRelation_addOwnerKey(theRelation, exception, theField);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_addTargetField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string fieldName)
{
    DMM_DLRLRelation* theRelation;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(relationName);
    assert(fieldName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add target field name %s to relation %s "
            "Reason: No class type meta information is known for this home.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(fieldName), DLRL_VALID_NAME(relationName));
    }

    theRelation = DMM_DLRLClass_findRelationByName(home->meta_representative, relationName);
    if(!theRelation)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add target field name %s to relation %s "
            "Reason: Cannot locate the relation.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(fieldName), DLRL_VALID_NAME(relationName));
    }

    DMM_DLRLRelation_addTargetKeyName(theRelation, exception, fieldName);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_setMultiRelationRelationTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string relationTopicName)
{
    DMM_DLRLMultiRelation* theRelation;
    DMM_DCPSTopic* theTopic;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(relationName);
    assert(relationTopicName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to set DCPS topic %s as the relation topic of "
           "relation %s. Reason: No class type meta information is known for this home.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicName), DLRL_VALID_NAME(relationName));
    }

    theRelation = DMM_DLRLClass_findMultiRelationByName(home->meta_representative, relationName);
    if(!theRelation)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to set DCPS topic %s as the relation topic of "
           "relation %s. Reason: Cannot locate the multi relation.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicName), DLRL_VALID_NAME(relationName));
    }
    theTopic = DMM_DLRLClass_findMultiPlaceTopicByName(home->meta_representative, relationTopicName);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to set DCPS topic %s as the relation topic of "
           "relation %s. Reason: Cannot locate the relations topic in the list of multi place topics.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicName),
            DLRL_VALID_NAME(relationName));
    }

    DMM_DLRLMultiRelation_setRelationTopic(theRelation, theTopic);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_addRelationTopicOwnerField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string relationTopicFieldName)
{
    DMM_DLRLMultiRelation* theRelation;
    DMM_DCPSTopic* theTopic;
    DMM_DCPSField* theField;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(relationName);
    assert(relationTopicFieldName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic owner field %s to "
            "the multi relation %s. Reason: No class type meta information is known for this home.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName),
            DLRL_VALID_NAME(relationName));
    }

    theRelation = DMM_DLRLClass_findMultiRelationByName(home->meta_representative, relationName);
    if(!theRelation)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic owner field %s to "
            "the multi relation %s. Reason:  Cannot locate the multi relation.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName), DLRL_VALID_NAME(relationName));
    }
    theTopic = DMM_DLRLMultiRelation_getRelationTopic(theRelation);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic owner field %s to "
            "the multi relation %s. Reason:  The multi relation doesnt have a relation topic.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName),
            DLRL_VALID_NAME(relationName));
    }
    theField = DMM_DCPSTopic_findFieldByName(theTopic, relationTopicFieldName);
    if(!theField)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic owner field %s to "
            "the multi relation %s. Reason:  Couldn't find the DCPS field within the relation topic.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName),
             DLRL_VALID_NAME(relationName));
    }

    DMM_DLRLMultiRelation_addRelationTopicOwnerField(theRelation, exception, theField);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_addRelationTopicTargetField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string relationTopicFieldName)
{
    DMM_DLRLMultiRelation* theRelation;
    DMM_DCPSTopic* theTopic;
    DMM_DCPSField* theField;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(relationName);
    assert(relationTopicFieldName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic target field %s to "
            "the multi relation %s. Reason: No class type meta information is known for this home.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName),
            DLRL_VALID_NAME(relationName));
    }

    theRelation = DMM_DLRLClass_findMultiRelationByName(home->meta_representative, relationName);
    if(!theRelation)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic target field %s to "
            "the multi relation %s. Reason:  Cannot locate the multi relation.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName), DLRL_VALID_NAME(relationName));
    }
    theTopic = DMM_DLRLMultiRelation_getRelationTopic(theRelation);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic target field %s to "
            "the multi relation %s. Reason:  The multi relation doesnt have a relation topic.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName),
            DLRL_VALID_NAME(relationName));
    }
    theField = DMM_DCPSTopic_findFieldByName(theTopic, relationTopicFieldName);
    if(!theField)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic target field %s to "
            "the multi relation %s. Reason:  Couldn't find the DCPS field within the relation topic.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName),
             DLRL_VALID_NAME(relationName));
    }

    DMM_DLRLMultiRelation_addRelationTopicTargetField(theRelation, exception, theField);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_setRelationTopicIndexField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string relationTopicFieldName)
{
    DMM_DLRLMultiRelation* theRelation;
    DMM_DCPSTopic* theTopic;
    DMM_DCPSField* theField;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(relationName);
    assert(relationTopicFieldName);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic index field %s to "
            "the multi relation %s. Reason: No class type meta information is known for this home.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName),
            DLRL_VALID_NAME(relationName));
    }

    theRelation = DMM_DLRLClass_findMultiRelationByName(home->meta_representative, relationName);
    if(!theRelation)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic index field %s to "
            "the multi relation %s. Reason:  Cannot locate the multi relation.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName), DLRL_VALID_NAME(relationName));
    }
    theTopic = DMM_DLRLMultiRelation_getRelationTopic(theRelation);
    if(!theTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic index field %s to "
            "the multi relation %s. Reason:  The multi relation doesnt have a relation topic.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName),
            DLRL_VALID_NAME(relationName));
    }
    theField = DMM_DCPSTopic_findFieldByName(theTopic, relationTopicFieldName);
    if(!theField)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to construct the meta model for %s '%s'. Unable to add a relation topic index field %s to "
            "the multi relation %s. Reason:  Couldn't find the DCPS field within the relation topic.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(relationTopicFieldName),
             DLRL_VALID_NAME(relationName));
    }

    DMM_DLRLMultiRelation_setRelationTopicIndexField(theRelation, theField);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* resolves information for single and multi relations */
void
DK_MMFacade_us_resolveRelation(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    DMM_DLRLRelation* relation)
{
    DMM_DLRLClass* type;
    LOC_string targetTopicName;
    DMM_DCPSTopic* targetTopic;
    Coll_List* targetKeyNames;
    LOC_string associatedRelationName;
    Coll_Iter* iterator = NULL;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(relation);

    DK_MMFacade_us_resolveRelationImmutability(home, relation);
    type = DK_MMFacade_us_resolveRelationType(home, exception, home->meta_representative, relation);
    DLRL_Exception_PROPAGATE(exception);
    DMM_DLRLRelation_setType(relation, type);
    targetTopicName = DMM_DLRLRelation_getTargetTopicName(relation);
    if(!targetTopicName)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to resolve meta information for %s '%s'. "
            "No target topic name defined for relation %s of type %s within DLRL class %s.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)),
            DLRL_VALID_NAME(DMM_DLRLClass_getName(type)),
            DLRL_VALID_NAME(DMM_DLRLClass_getName(home->meta_representative)));
    }
    targetTopic = DMM_DLRLClass_findTopicByName(type, targetTopicName);
    if(!targetTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to resolve meta information for %s '%s'. "
            "No target topic %s found for relation %s of type %s within DLRL class %s.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(targetTopicName),
            DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)), DLRL_VALID_NAME(DMM_DLRLClass_getName(type)),
            DLRL_VALID_NAME(DMM_DLRLClass_getName(home->meta_representative)));
    }
    DMM_DLRLRelation_setTargetTopic(relation, targetTopic);
    targetKeyNames = DMM_DLRLRelation_getTargetKeyNames(relation);
    if(Coll_List_getNrOfElements(targetKeyNames) != Coll_List_getNrOfElements(DMM_DCPSTopic_getKeyFields(targetTopic)))
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to resolve meta information for %s '%s'. "
            "There is a mismatch between the number of target key fields identifying relation %s of type %s within DLRL"
            " class %s compared to the number of key fields actually found in the target topic with name %s.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)),
            DLRL_VALID_NAME(DMM_DLRLClass_getName(type)),
            DLRL_VALID_NAME(DMM_DLRLClass_getName(home->meta_representative)), DLRL_VALID_NAME(targetTopicName));
    }
    iterator = Coll_List_getFirstElement(targetKeyNames);
    while(iterator)
    {
        LOC_string aKeyName = Coll_Iter_getObject(iterator);
        DMM_DCPSField* targetField = DMM_DCPSTopic_findFieldByName(targetTopic, aKeyName);
        if(!targetField)
        {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to resolve meta information for %s '%s'. "
                "Field '%s' not found within target topic %s found for relation %s of type %s within DLRL class %s.",
                "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(aKeyName), DLRL_VALID_NAME(targetTopicName),
                DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)), DLRL_VALID_NAME(DMM_DLRLClass_getName(type)),
                DLRL_VALID_NAME(DMM_DLRLClass_getName(home->meta_representative)));
        }
        if((DMM_DCPSField_getFieldType(targetField) != DMM_KEYTYPE_KEY) &&
                    (DMM_DCPSField_getFieldType(targetField) != DMM_KEYTYPE_SHARED_KEY))
        {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to resolve meta information for %s '%s'. "
                "In relation %s of type %s within DLRL class %s the topic field '%s' is not a key field within target "
                "topic %s.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
                DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)), DLRL_VALID_NAME(DMM_DLRLClass_getName(type)),
                DLRL_VALID_NAME(DMM_DLRLClass_getName(home->meta_representative)), DLRL_VALID_NAME(aKeyName),
                DLRL_VALID_NAME(targetTopicName));
        }
        DMM_DLRLRelation_addTargetKey(relation, exception, targetField);
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
    }
    /* resolve the associated relation */
    associatedRelationName = DMM_DLRLRelation_getAssociatedRelationName(relation);
    /* associated relation may be null */
    if(associatedRelationName)
    {
        DMM_DLRLRelation* associatedRelation = DMM_DLRLClass_findRelationByName(type, associatedRelationName);
        if(!associatedRelation)
        {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to resolve meta information for %s '%s'. "
                "Unable to resolve association of relation %s of type %s within DLRL class %s to the associated "
                "relation '%s' within class %s. Cannot find associated relation in target type.",
                "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)),
                DLRL_VALID_NAME(DMM_DLRLClass_getName(type)),
                DLRL_VALID_NAME(DMM_DLRLClass_getName(home->meta_representative)),
                DLRL_VALID_NAME(associatedRelationName), DLRL_VALID_NAME(DMM_DLRLClass_getName(type)));
        }
        DMM_DLRLRelation_setAssociatedWith(relation, associatedRelation);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_resolveRelationImmutability(
    DK_ObjectHomeAdmin* home,
    DMM_DLRLRelation* relation)
{
    LOC_boolean immutable = FALSE;
    Coll_List* ownerKeys = NULL;
    Coll_Iter* iterator = NULL;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(relation);

    /* only single relations can be immutable */
    if(DMM_DLRLRelation_getClassID(relation) == DMM_DLRL_RELATION_CLASS)
    {
        ownerKeys = DMM_DLRLRelation_getOwnerKeys(relation);
        iterator = Coll_List_getFirstElement(ownerKeys);
        while(iterator)
        {
            DMM_DCPSField* aField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
            if(DMM_DCPSField_getFieldType(aField) == DMM_KEYTYPE_SHARED_KEY)
            {
                immutable = TRUE;
            }
            iterator = Coll_Iter_getNext(iterator);
        }
    }
    DMM_DLRLRelation_setImmutability(relation, immutable);
    DLRL_INFO(INF_EXIT);
}

/* cache must be locked when calling this operation (administrative) */
DMM_DLRLClass*
DK_MMFacade_us_resolveRelationType(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    DMM_DLRLClass* owner,
    DMM_DLRLRelation* relation)
{
    DMM_DLRLClass* type = NULL;
    LOC_string typeName;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(owner);
    assert(relation);

    typeName = DMM_DLRLRelation_getTypeName(relation);
    if(!typeName)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to resolve meta information for %s '%s'. "
            "Unable to resolve the type of relation %s within DLRL class %s", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
            DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)), DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)));
    }
    if(0 == strcmp(typeName, owner->name))
    {
        /* the relation points to an object of the same type. */
        type = owner;
    } else
    {
        DK_ObjectHomeAdmin* typeHome = DK_CacheAdmin_us_findHomeByName(home->cache, typeName);
        if(!typeHome)
        {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to resolve meta information for %s '%s'. "
                "The ObjectHome %s, that manages the target object type of relation %s within DLRL class %s, has not "
                "been defined within the DLRL Kernel Cache.", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name),
                DLRL_VALID_NAME(typeName), DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)),
                DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)));
        }
        if(!typeHome->meta_representative)
        {
            DK_Entity_ts_release((DK_Entity*)typeHome);
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to resolve meta information for %s '%s'. "
                "The target ObjectHome %s has no meta information defined regarding the type of relation %s "
                "within DLRL class %s", "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name), DLRL_VALID_NAME(typeName),
                DLRL_VALID_NAME(DMM_DLRLRelation_getName(relation)), DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)));
        }
        type = typeHome->meta_representative;
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return type;
}

void
DK_MMFacade_us_mapDLRLMetaModelToDCPSMetaModel(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception)
{
    Coll_List* topicInfos;
    Coll_Iter* iterator;
    DK_TopicInfo* aTopicInfo;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);

    topicInfos = DK_ObjectHomeAdmin_us_getTopicInfos(home);
    iterator = Coll_List_getFirstElement(topicInfos);
    while(iterator)
    {
        aTopicInfo = (DK_TopicInfo*)Coll_Iter_getObject(iterator);
        DK_MMFacade_us_resolveDatabaseTopicFieldsForDMMTopic(exception, aTopicInfo);
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MMFacade_us_resolveDatabaseTopicFieldsForDMMTopic(
    DLRL_Exception* exception,
    DK_TopicInfo* topicInfo)
{
    DK_TopicInfoHolder holder;
    u_topic topic = NULL;
    u_result result = U_RESULT_OK;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(topicInfo);

    holder.exception = exception;
    holder.topicInfo = topicInfo;
    topic = DK_TopicInfo_us_getTopic(topicInfo);
    result = u_entityWriteAction((u_entity)topic, DK_DCPSUtility_us_resolveDatabaseFields, &holder);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "An error occured while looking up an instance handle.");
    DLRL_Exception_PROPAGATE(exception);/* exception in the holder! */

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}


void
DK_MMFacade_us_resolveMetaModel(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception)
{
    Coll_List* relations;
    DMM_DLRLRelation* aRelation;
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long count = 0;
    DK_ObjectHomeAdmin* relatedHome = NULL;
    DMM_DLRLClass* relatedClass = NULL;
    LOC_unsigned_long nrOfRelations = 0;
    DMM_DCPSTopic* mainTopic;
    Coll_List* keyFields;
    DMM_DCPSField* aKeyfield;
    c_field dataBaseField;
    c_valueKind kind;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);

    if(!home->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to resolve meta information for %s '%s'. No class type meta information known for this home.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name));
    }
    relations = DMM_DLRLClass_getRelations(home->meta_representative);
    nrOfRelations = Coll_List_getNrOfElements(relations);
    assert(!(home->relatedHomes));
    if(nrOfRelations > 0)
    {
        DLRL_ALLOC_WITH_SIZE(home->relatedHomes, (sizeof(DK_ObjectHomeAdmin*)*nrOfRelations), exception,
                                                    "Unable to allocate array container for related object homes!");
        memset(home->relatedHomes, 0, (sizeof(DK_ObjectHomeAdmin*)*nrOfRelations));
    }/* else do nothing */
    iterator = Coll_List_getFirstElement(relations);
    assert((!iterator && (nrOfRelations == 0)) || (iterator && (nrOfRelations > 0)));
    while(iterator)
    {
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        DK_MMFacade_us_resolveRelation(home, exception, aRelation);
        DLRL_Exception_PROPAGATE(exception);
        relatedClass = DMM_DLRLRelation_getType(aRelation);
        assert(relatedClass);
        relatedHome = DMM_DLRLClass_getUserData(relatedClass);
        home->relatedHomes[count] = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)relatedHome);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }
    relations = DMM_DLRLClass_getMultiRelations(home->meta_representative);
    iterator = Coll_List_getFirstElement(relations);
    while(iterator)
    {
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        DK_MMFacade_us_resolveRelation(home, exception, aRelation);
        DLRL_Exception_PROPAGATE(exception);
        if(!DMM_DLRLMultiRelation_getRelationTopic((DMM_DLRLMultiRelation*)aRelation))
        {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
              "Unable to resolve meta information for %s '%s'. A multi relation was not mapped to a seperate topic.",
              "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name));
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DK_MMFacade_us_mapDLRLMetaModelToDCPSMetaModel(home, exception);
    DLRL_Exception_PROPAGATE(exception);

    /*for all keyfields:*/
    if(DMM_DLRLClass_getMapping(home->meta_representative) == DMM_MAPPING_DEFAULT)
    {
        count = 0;/*set it to zero!!*/
        /* assert that the keyfields for an object with default mapping looks as expected.
         * ie simpleoid = 3 long keyfields, fulloid = 1 string keyfield followed by 3 long keyfields
         */
        mainTopic = DMM_DLRLClass_getMainTopic(home->meta_representative);
        keyFields = DMM_DCPSTopic_getKeyFields(mainTopic);
        if(Coll_List_getNrOfElements(keyFields) != 3 && Coll_List_getNrOfElements(keyFields) != 4)
        {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION, "Unable to resolve meta information for %s '%s'. "
            "This class type was mapped following default rules, but the number of keyfields are not as expected!",
              "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name));
        }
        iterator = Coll_List_getLastElement(keyFields);/* start at the end! */
        while(iterator)
        {
            assert(count <= 3);/* max 4 fields allowed for default mapping */
            aKeyfield = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
            dataBaseField = DMM_DCPSField_getDatabaseField(aKeyfield);
            kind = c_fieldValueKind(dataBaseField);
            if((count < 3 && kind != V_LONG) || (count == 3 && kind != V_STRING))
            {
                DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                    "Unable to resolve meta information for %s '%s'. The keyfields for this class type do not match "
                    "requirements for classes mapped following default mapping rules.", "DLRL Kernel ObjectHomeAdmin",
                    DLRL_VALID_NAME(home->name));
            }
            iterator = Coll_Iter_getPrev(iterator);
            count++;
        }
    }

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        if(home->relatedHomes)
        {
            assert(nrOfRelations > 0);
            for(count = 0; count < nrOfRelations; count++)
            {
                if(home->relatedHomes[count])
                {
                    DK_Entity_ts_release((DK_Entity*)home->relatedHomes[count]);
                }
            }
            os_free(home->relatedHomes);
            home->relatedHomes = NULL;
        }
    }
    DLRL_INFO(INF_EXIT);
}

/* caller must delete the list, not the content */
Coll_List*
DK_MMFacade_us_getMultiRelationNames(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception)
{
    Coll_List* relations = NULL;
    Coll_List* relationNames = NULL;
    DMM_DLRLRelation* aRelation = NULL;
    Coll_Iter* iterator = NULL;
    long errorCode;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(home->meta_representative);

    relationNames = Coll_List_new();
    if(!relationNames)
    {
         DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to retrieve the relation names of %s '%s'. "
            "Couldn't allocate a list object to hold the relation names.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name));
    }
    relations = DMM_DLRLClass_getMultiRelations(home->meta_representative);
    iterator = Coll_List_getFirstElement(relations);
    while(iterator)
    {
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        errorCode = Coll_List_pushBack(relationNames, DMM_DLRLRelation_getName(aRelation));
        if (errorCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add the relation name to list of relation names");
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && relationNames)
    {
        while(Coll_List_getNrOfElements(relationNames) > 0)
        {
            Coll_List_popBack(relationNames);
        }
        Coll_List_delete(relationNames);
    }
    DLRL_INFO(INF_EXIT);
    return relationNames;
}


Coll_List*
DK_MMFacade_us_getSingleRelationNames(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception)
{
    Coll_List* relations = NULL;
    Coll_List* relationNames = NULL;
    Coll_Iter* iterator = NULL;
    DMM_DLRLRelation* aRelation = NULL;
    long errorCode;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(home->meta_representative);

    relationNames = Coll_List_new();
    if(!relationNames)
    {
         DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to retrieve the relation names of %s '%s'. "
            "Couldn't allocate a list object to hold the relation names.",
            "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name));
    }
    relations = DMM_DLRLClass_getRelations(home->meta_representative);
    iterator = Coll_List_getFirstElement(relations);
    while(iterator)
    {
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        errorCode = Coll_List_pushBack(relationNames, DMM_DLRLRelation_getName(aRelation));
        if (errorCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add the relation name to list of relation names");
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && relationNames)
    {
        while(Coll_List_getNrOfElements(relationNames) > 0)
        {
            Coll_List_popBack(relationNames);
        }
        Coll_List_delete(relationNames);
        relationNames = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return relationNames;
}

DK_RelationType
DK_MMFacade_us_getRelationType(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName)
{
    Coll_List* relations = NULL;
    Coll_Iter* iterator = NULL;
    LOC_boolean found = FALSE;
    DK_RelationType type = DK_RELATION_TYPE_REF;/* default */
    DMM_DLRLRelation* aRelation = NULL;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(home->meta_representative);
    assert(relationName);

    relations = DMM_DLRLClass_getRelations(home->meta_representative);
    iterator = Coll_List_getFirstElement(relations);
    while(iterator && !found)
    {
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DLRLRelation_getName(aRelation), relationName))
        {
            type = DK_RELATION_TYPE_REF;
            found = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    relations = DMM_DLRLClass_getMultiRelations(home->meta_representative);
    iterator = Coll_List_getFirstElement(relations);
    while(iterator && !found)
    {
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DLRLRelation_getName(aRelation), relationName))
        {
            DMM_Basis basis = DMM_DLRLMultiRelation_getBasis((DMM_DLRLMultiRelation*)aRelation);
            if(basis == DMM_BASIS_STR_MAP)
            {
                type = DK_RELATION_TYPE_STR_MAP;
            } else if(basis == DMM_BASIS_INT_MAP)
            {
                type = DK_RELATION_TYPE_INT_MAP;
            } else
            {
                assert(basis == DMM_BASIS_SET);
                type = DK_RELATION_TYPE_SET;
            }
            found = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    if(!found)
    {
         DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to retrieve the relation type for relation %s of %s '%s'. "
            "The relation for the specific name cannot be found for the type represented by this object home.",
            DLRL_VALID_NAME(relationName), "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(home->name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return type;
}

/* returns null if type cant be found, return value doesnt have to be freed */
LOC_string
DK_MMFacade_us_getTargetTypeNameForRelation(
    DK_ObjectHomeAdmin* home,
    LOC_string relationName)
{
    LOC_string typeName = NULL;
    LOC_boolean found = FALSE;
    Coll_List* relations = NULL;
    Coll_Iter* iterator = NULL;
    DMM_DLRLRelation* aRelation = NULL;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(home->meta_representative);
    assert(relationName);

    relations = DMM_DLRLClass_getRelations(home->meta_representative);
    iterator = Coll_List_getFirstElement(relations);
    while(iterator && !found)
    {
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DLRLRelation_getName(aRelation), relationName))
        {
            typeName = DMM_DLRLRelation_getTypeName(aRelation);
            found = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    relations = DMM_DLRLClass_getMultiRelations(home->meta_representative);
    iterator = Coll_List_getFirstElement(relations);
    while(iterator && !found)
    {
        aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DLRLRelation_getName(aRelation), relationName))
        {
            typeName = DMM_DLRLRelation_getTypeName(aRelation);
            found = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_INFO(INF_EXIT);
    return typeName;
}

void
DK_MMFacade_us_getKeyFieldNamesForDefaultMappedObject(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    LOC_string* oidfieldName,
    LOC_string* nameField)
{
    LOC_string tmpOidFieldName;
    DMM_DLRLClass* metaRep;
    DMM_DCPSTopic* mainTopic;
    Coll_List* keyFields;
    DMM_DCPSField* nameKeyfield;
    DMM_DCPSField* oidKeyfield;
    c_iter nameList = NULL;/* must init to null */
    c_long length;
    void* tmp;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(home);
    assert(oidfieldName);
    assert(nameField);

    metaRep = DK_ObjectHomeAdmin_us_getMetaRepresentative(home);
    if(DMM_DLRLClass_getMapping(metaRep) == DMM_MAPPING_DEFAULT)
    {
        mainTopic = DMM_DLRLClass_getMainTopic(metaRep);
        keyFields = DMM_DCPSTopic_getKeyFields(mainTopic);
        if(Coll_List_getNrOfElements(keyFields) == 4)
        {
            nameKeyfield = Coll_List_getObject(keyFields, 0);
            DLRL_STRDUP((*nameField), DMM_DCPSField_getName(nameKeyfield), exception,
                                                                    "Unable to complete operation. Out of resources");
        }
        oidKeyfield = Coll_List_getObject(keyFields, 1);
        tmpOidFieldName = DMM_DCPSField_getName(oidKeyfield);
        nameList = c_splitString((char*)tmpOidFieldName, ".");
        length = c_iterLength(nameList);
        /* Oid field will contain exactly one '.' We need to only keep the text before the '.', as the stuff behind it
         * are fields in the DLRLOid object, which are irrelevant. Multiple '.' arent allowed cos we dont allow nesting.
         */
        if(length != 2)
        {
            /* throw an exception, if DCG and this remain in synch this exception wont ever occur */
            DLRL_Exception_THROW(exception, DLRL_ERROR,
                "Meta information is not as expected. DCG must have failed "
                "it's checking. Expected keyfield '%s' to have exactly one '.' seperator.",
                tmpOidFieldName);
        }
        *oidfieldName = (LOC_string)c_iterTakeFirst(nameList);
    }

    DLRL_Exception_EXIT(exception);
    if(nameList)
    {
        tmp = c_iterTakeFirst(nameList);
        while(tmp)
        {
            os_free(tmp);
            tmp = c_iterTakeFirst(nameList);
        }
        c_iterFree(nameList);
    }
    DLRL_INFO(INF_EXIT);
}
