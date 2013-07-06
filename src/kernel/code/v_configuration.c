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
#include "v_configuration.h"
#include "v_public.h"

static v_cfNode
v_configurationNodeResolveNode (
    v_cfNode node,
    c_ulong id);

v_configuration
v_configurationNew(
    v_kernel kernel)
{
    v_configuration config;
    
    config = v_configuration(v_objectNew(kernel,K_CONFIGURATION));
    v_publicInit(v_public(config));
    config->root = NULL;
    config->uri = NULL;
    config->idCounter = 0;
    
    return config;
}

void
v_configurationFree(
    v_configuration config)
{
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));
    v_publicFree(v_public(config));
    c_free(config);
}

void
v_configurationSetRoot(
    v_configuration config,
    v_cfElement root)
{
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));
    assert(root != NULL);
    assert(C_TYPECHECK(root, v_cfElement));
    
    config->root = root;
}

v_cfElement
v_configurationGetRoot(
    v_configuration config)
{
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));

    return c_keep(config->root);
}

void
v_configurationSetUri(
    v_configuration config,
    const c_char *uri)
{
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));
    if (config->uri != NULL) {
        c_free(config->uri);

    }
    if (uri != NULL) {
        config->uri = c_stringNew(c_getBase(config), uri);
    } else {
        config->uri = NULL;
    }
}

const c_char *
v_configurationGetUri(
    v_configuration config)
{
    const c_char *result = NULL;
    if(config != NULL) {
        assert(C_TYPECHECK(config, v_configuration));
        result = config->uri;
    }
    return result;
}

c_ulong
v_configurationIdNew(
    v_configuration config)
{
    c_ulong result;
    
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));
    
    result = config->idCounter;
    config->idCounter++;
    
    return result;
}

v_cfNode
v_configurationGetNode(
    v_configuration config,
    c_ulong id)
{
    v_cfNode node;
    
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));
    
    node = v_cfNode(config->root);
    
    if(node->id != id){
        node = v_configurationNodeResolveNode(node, id);
    }
    return node;
}

static v_cfNode
v_configurationNodeResolveNode(
    v_cfNode node,
    c_ulong id)
{
    v_cfNode result, child;
    c_iter iter;
    
    result = NULL;
    
    switch(node->kind){
        case V_CFELEMENT:
        iter = v_cfElementGetChildren(v_cfElement(node));
        child = v_cfNode(c_iterTakeFirst(iter));
       
        while((child != NULL) && (result == NULL)){
            if(child->id == id){
                result = child;
            } else {
                result = v_configurationNodeResolveNode(child, id);
            }
            child = v_cfNode(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);
       
        if(result == NULL){
            iter = v_cfElementGetAttributes(v_cfElement(node));
            child = v_cfNode(c_iterTakeFirst(iter));
            
            while((child != NULL) && (result == NULL)){
                if(child->id == id){
                    result = child;
                } else {
                    result = v_configurationNodeResolveNode(child, id);
                }
                child = v_cfNode(c_iterTakeFirst(iter));
            }
            c_iterFree(iter);
        }
        break;
        case V_CFDATA:
        case V_CFATTRIBUTE:
        default:
        break;
    }
    return result;
}

