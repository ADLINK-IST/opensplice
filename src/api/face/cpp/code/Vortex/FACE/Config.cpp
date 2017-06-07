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
#include "Vortex_FACE.hpp"
#include "Vortex/FACE/Config.hpp"
#include "Vortex/FACE/ReportSupport.hpp"

#include "dds/dds.hpp"

#include "cf_config.h"
#include "cfg_parser.h"

namespace Vortex {
namespace FACE {
static const char* TAG_LIST = "connections_list";
static const char* TAG_CONNECTION = "connection";
}; /* FACE */
}; /* Vortex */


Vortex::FACE::Config::Config() :
        connection(NULL),
        scope(CONFIG_SCOPE_NONE)
{
}

::FACE::RETURN_CODE_TYPE
Vortex::FACE::Config::parse(const std::string &url)
{
    ::FACE::RETURN_CODE_TYPE status;
    cf_element rootElement;
    cfgprs_status s;
    s = cfg_parse_ospl(Vortex::FACE::Config::addUrlPrefix(url).c_str(), &rootElement);
    if (s == CFGPRS_OK) {
        status = this->parseElement(rootElement);
        cf_elementFree(rootElement);
        if (status == ::FACE::NO_ERROR) {
            if (this->configs.empty()) {
                status = ::FACE::INVALID_CONFIG;
                FACE_REPORT_ERROR(status, "No connections within the configuration.");
            }
        }
    } else if (s == CFGPRS_NO_INPUT) {
        status = ::FACE::INVALID_PARAM;
        FACE_REPORT_ERROR(status, "Can not open %s", url.c_str());
    } else if (s == CFGPRS_ERROR) {
        status = ::FACE::INVALID_CONFIG;
        FACE_REPORT_ERROR(status, "The configuration is not valid");
    } else {
        status = ::FACE::NO_ACTION;
    }
    return status;
}

Vortex::FACE::ConnectionConfig::shared_ptr
Vortex::FACE::Config::find(const std::string &name) {
    CFG_MAP_TYPE::iterator it = this->configs.find(name);
    if(it == this->configs.end()) {
        /* Not available: return empty pointer. */
        return ConnectionConfig::shared_ptr();
    }
    return it->second;
}

std::string
Vortex::FACE::Config::addUrlPrefix(const std::string &url) {
    std::string ret;
    if (strncmp(url.c_str(), "file://", strlen("file://")) == 0) {
        ret = url;
    } else {
        ret = std::string("file://") + url;
    }
    return ret;
}


::FACE::RETURN_CODE_TYPE
Vortex::FACE::Config::parseElement(cf_element element) {
    ::FACE::RETURN_CODE_TYPE status = ::FACE::NO_ERROR;

    const c_char *name = cf_nodeGetName((cf_node)element);
    assert(name);

    /* Add this tag to the scope. */
    std::string base = this->scopedTag;
    this->scopedTag.append("::");
    this->scopedTag.append(name);

    switch(this->scope)
    {
    case CONFIG_SCOPE_NONE:
        /* Enforce the opening <connections_list> tag. */
        if (strcmp(name, TAG_LIST) == 0) {
            this->scope = CONFIG_SCOPE_LIST;
            status = this->parseContainedElements(cf_elementGetChilds(element));
            this->scope = CONFIG_SCOPE_NONE;
        } else {
            FACE_REPORT_ERROR(::FACE::INVALID_CONFIG,
                "Unrecognized top-level element (\"%s\"), expected top-level element \"%s\"...", name, TAG_LIST);
            return ::FACE::INVALID_CONFIG;
        }
        break;
    case CONFIG_SCOPE_LIST:
        /* Start new <connection> configuration. */
        if (strcmp(name, TAG_CONNECTION) == 0) {
            this->scope = CONFIG_SCOPE_CONNECTION;
            this->connection = new ConnectionConfig();
            status = this->parseContainedElements(cf_elementGetChilds(element));
            if (status == ::FACE::NO_ERROR) {
                status = this->connection->validate();
            }
            if (status == ::FACE::NO_ERROR) {
                CFG_MAP_TYPE::iterator it = this->configs.find(this->connection->getConnectionName());
                if(it == this->configs.end()) {
                    /* Not available: insert as shared pointer. */
                    ConnectionConfig::shared_ptr ptr(this->connection);
                    this->configs.insert(std::make_pair(ptr->getConnectionName(), ptr));
                } else {
                    /* Already available: fail the parsing. */
                    FACE_REPORT_ERROR(::FACE::INVALID_CONFIG,
                        "Connection name \"%s\" available multiple times", this->connection->getConnectionName().c_str());
                    status = ::FACE::INVALID_CONFIG;
                }
            }
            if (status != ::FACE::NO_ERROR) {
                delete this->connection;
            }
            this->connection = NULL;
            this->scope = CONFIG_SCOPE_LIST;
        }
        break;
    case CONFIG_SCOPE_CONNECTION:
        /* Parse all tags within the <connection> scope. */
        status = this->parseContainedElements(cf_elementGetChilds(element));
        break;
    }

    /* Remove the current tag from the scope. */
    this->scopedTag = base;

    return status;
}

::FACE::RETURN_CODE_TYPE
Vortex::FACE::Config::parseContainedElements(c_iter elements) {
    ::FACE::RETURN_CODE_TYPE status = ::FACE::NO_ERROR;
    cf_node node;

    while (((node = (cf_node) c_iterTakeFirst(elements)) != NULL) &&
           (status == ::FACE::NO_ERROR))
    {
        cf_kind kind = cf_nodeKind(node);
        switch(kind) {
        case CF_ELEMENT:
            status = this->parseElement(cf_element(node));
            break;
        case CF_DATA:
            status = this->parseElementData(cf_data(node));
            break;
        default:
            assert(0);
            break;
        }
    }
    /* Elements of iter elements don't have to be freed, so no worries that iter
     * is potentially not empty here. */
    c_iterFree(elements);
    return status;
}

::FACE::RETURN_CODE_TYPE
Vortex::FACE::Config::parseElementData(cf_data data)
{
    ::FACE::RETURN_CODE_TYPE status = ::FACE::NO_ERROR;
    char first;

    assert(data);

    first = cf_dataValue(data).is.String[0];
    if(strcmp(cf_node(data)->name, "#text") == 0 &&
       (first == '\0'  || first == '\n' || first == '<' || first == ' '))
    {
        /* Skip comments */
    } else {
        c_value xmlValue = cf_dataValue(data);
        if (connection != NULL) {
            status = connection->set(this->scopedTag, xmlValue.is.String);
        }
    }

    return status;
}



