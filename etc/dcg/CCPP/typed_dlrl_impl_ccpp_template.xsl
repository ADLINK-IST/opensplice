<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xalan="http://xml.apache.org/xalan"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
    <xsl:param name="output.dir" select="'.'"/>
    <xsl:include href="common_dlrl.xsl"/>

    <xsl:template match="IDL">
        <xsl:variable name="filename">
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="@baseFile"/>
            <xsl:text>Dlrl_impl.cpp</xsl:text>
        </xsl:variable>
        <redirect:write file="{$output.dir}/{$filename}">
            <FOODLRL-out>
                <xsl:call-template name="copyright-notice"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>Dlrl_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "DLRL_Report.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "DLRL_Kernel.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "DLRL_Util.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:for-each select="//VALUEDEF/TARGET_IMPL_INCLUDE_PATH/@NAME[not(.=preceding::VALUEDEF/TARGET_IMPL_INCLUDE_PATH/@NAME)]">
                    <xsl:text>#include "</xsl:text>
                    <xsl:value-of select="."/>
                    <xsl:text>"</xsl:text>
                    <xsl:value-of select="$NL"/>
                </xsl:for-each>
                <xsl:value-of select="$NL"/>
                <xsl:for-each select="VALUEDEF">
                    <xsl:call-template name="VALUEDEF"/>
                </xsl:for-each>
                <xsl:for-each select="MODULE">
                    <xsl:call-template name="MODULE"/>
                </xsl:for-each>
            </FOODLRL-out>
        </redirect:write>
    </xsl:template>

    <xsl:template name="MODULE">
        <xsl:for-each select="VALUEDEF">
            <xsl:call-template name="VALUEDEF"/>
        </xsl:for-each>
        <xsl:for-each select="MODULE">
            <xsl:call-template name="MODULE"/>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="VALUEDEF">

        <xsl:variable name="isNonIncludedSharedValueDef">
            <xsl:call-template name="does-valuedef-inherit-from-objectroot-and-is-not-included"/>
        </xsl:variable>

        <xsl:if test="$isNonIncludedSharedValueDef='true'">
            <xsl:variable name="prefixedFullName">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="fullNameWithUnderscores">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'_'"/>
                    <xsl:with-param name="prefixKeywords" select="'no'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="nonPrefixedName">
                <xsl:call-template name="string-return-text-after-last-token">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="token" select="'::'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="prefixedNamespace">
                <xsl:call-template name="string-search-replace-except-last">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="prefixedFullNameExceptLast">
                <xsl:if test="string-length($prefixedNamespace)!=0">
                    <xsl:value-of select="$prefixedNamespace"/>
                    <xsl:text>::</xsl:text>
                </xsl:if>
                <xsl:value-of select="$nonPrefixedName"/>
            </xsl:variable>
            <xsl:variable name="prefixedFullTopicTypeName">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="mainTopic/@typename"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="fullTopicTypeNameWithUnderscores">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="mainTopic/@typename"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'_'"/>
                    <xsl:with-param name="prefixKeywords" select="'no'"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>/****** Generated for type: </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> ******/</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <!-- generate macro for downcast (example valuetype 'Foo' defined in module 'test'
            #define DOWNCAST_test_Foo(lsObject) \
                dynamic_cast<test::Foo*>(reinterpret_cast<CORBA::ValueBase*>(lsObject))

            #define DOWNCAST_test_Foo_abstract(lsObject) \
                dynamic_cast<test::Foo_abstract*>(reinterpret_cast<CORBA::ValueBase*>(lsObject))

            #define DOWNCAST_test_Foo_SELECTION(lsObject) \
                dynamic_cast<test::FooSelection_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

            #define DOWNCAST_test_Foo_SELECTION_LISTENER(lsObject) \
                dynamic_cast<test::FooSelectionListener_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

            #define DOWNCAST_test_Foo_Listener(lsObject) \
                dynamic_cast<test::FooListener_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))
            -->
            <xsl:text>#define DOWNCAST_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>(lsObject) \</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>    dynamic_cast&lt;</xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>*>(reinterpret_cast&lt;</xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::ValueBase*>(lsObject))</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <xsl:text>#define DOWNCAST_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_abstract(lsObject) \</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>    dynamic_cast&lt;</xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>_abstract*>(reinterpret_cast&lt;</xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::ValueBase*>(lsObject))</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <xsl:text>#define DOWNCAST_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_SELECTION(lsObject) \</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>    dynamic_cast&lt;</xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection_ptr>(reinterpret_cast&lt;</xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Object_ptr>(lsObject))</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>#define DOWNCAST_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_SELECTION_LISTENER(lsObject) \</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>    dynamic_cast&lt;</xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>SelectionListener_ptr>(reinterpret_cast&lt;</xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Object_ptr>(lsObject))</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>#define DOWNCAST_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_LISTENER(lsObject) \</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>    dynamic_cast&lt;</xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener_ptr>(reinterpret_cast&lt;</xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Object_ptr>(lsObject))</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- call all template to generate each typed operation -->

            <!-- The 'typedClass_cacheInitializer-predeclaration', 'typedClass_createTypedObjectSeq' and
            'typedClass_addToTypedObjectSeq' must be generated before the 'typedHomeConstructor' as
            they are static method declarations used by the operation generated by the 'typedHomeConstructor' template
            -->
            <xsl:call-template name="typedClass_cacheInitializer-predeclaration">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__typedTopicCopyOutExternDecl">
                <xsl:with-param name="fullTopicTypeNameWithUnderscores" select="$fullTopicTypeNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__typedTopicCopyInExternDecl">
                <xsl:with-param name="fullTopicTypeName" select="$prefixedFullTopicTypeName"/>
                <xsl:with-param name="fullTopicTypeNameWithUnderscores" select="$fullTopicTypeNameWithUnderscores"/>
            </xsl:call-template>


            <xsl:call-template name="typedClass_createTypedObjectSeq">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedClass_addToTypedObjectSeq">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>

            <xsl:call-template name="typedHomeConstructor">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHomeDestructor">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome___duplicate">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome___narrow">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome___unchecked_narrow">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome___nil">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome___this">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_listeners">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_attach_listener">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_detach_listener">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_create_selection">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_delete_selection">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_selections">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_get_modified_objects">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_get_deleted_objects">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_get_created_objects">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_get_objects">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_create_unregistered_object">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_find_object">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_register_object">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_create_object">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_registerType">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSelection_members">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSelection_get_inserted_members">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSelection_get_modified_members">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSelection_get_removed_members">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSelection_set_listener">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSelection_listener">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSelection_check_objects">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
            </xsl:call-template>
            <xsl:call-template name="typedSelection_constructor">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
            </xsl:call-template>
            <xsl:call-template name="typedSelection_destructor">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
            </xsl:call-template>

            <xsl:call-template name="typedFilter_constructor">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
            </xsl:call-template>
            <xsl:call-template name="typedFilter_destructor">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
            </xsl:call-template>

            <xsl:call-template name="typedSet_constructor-destructor">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
            </xsl:call-template>
            <xsl:call-template name="typedSet_add">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSet_added_elements">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSet_contains">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSet_remove">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSet_values">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedSet_removed_elements">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>

            <xsl:call-template name="typedStrMap_constructor-destructor">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
            </xsl:call-template>
            <xsl:call-template name="typedStrMap_put">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedStrMap_get">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedStrMap_values">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedIntMap_constructor-destructor">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
            </xsl:call-template>
            <xsl:call-template name="typedIntMap_values">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedIntMap_get">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedIntMap_put">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedList_put">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedList_add">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedList_get">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedList_values">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name="typedHome_loadMetaModel">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:if test="mainTopic/keyDescription/@content='SimpleOid' or mainTopic/keyDescription/@content='FullOid'">
                <xsl:call-template name="callbackFunc__setTopicOidField">
                    <xsl:with-param name="fullTopicTypeNameWithUnderscores" select="$fullTopicTypeNameWithUnderscores"/>
                    <xsl:with-param name="prefixedFullTopicTypeName" select="$prefixedFullTopicTypeName"/>
                </xsl:call-template>
            </xsl:if>
            <xsl:if test="mainTopic/keyDescription/@content='FullOid'">
                <xsl:call-template name="callbackFunc__setTopicClassName">
                    <xsl:with-param name="fullTopicTypeNameWithUnderscores" select="$fullTopicTypeNameWithUnderscores"/>
                    <xsl:with-param name="prefixedFullTopicTypeName" select="$prefixedFullTopicTypeName"/>
                </xsl:call-template>
            </xsl:if>

            <xsl:call-template name ="callbackFunc__createTypedTopic">
                <xsl:with-param name="fullTopicTypeNameWithUnderscores" select="$fullTopicTypeNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullTopicTypeName" select="$prefixedFullTopicTypeName"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__createTypedSet">
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__createTypedIntMap">
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__createTypedStrMap">
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__invokeDeletedObjectCallback">
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__invokeModifiedObjectCallback">
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__invokeNewObjectCallback">
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__createTypedObject">
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__setPreviousTopic">
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="prefixedFullTopicTypeName" select="$prefixedFullTopicTypeName"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__getPreviousTopic">
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__setCurrentTopic">
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullTopicTypeName" select="$prefixedFullTopicTypeName"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__getCurrentTopic">
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__typedTopicInitializeTopicCache">
                <xsl:with-param name="fullTopicTypeNameWithUnderscores" select="$fullTopicTypeNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__createTypedObjectSeq">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__addElementToTypedObjectSeq">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__createTypedSelectionSeq">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__addElementToTypedSelectionSeq">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__createTypedListenerSeq">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__addElementToTypedListenerSeq">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__changeRelations">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
            </xsl:call-template>
            <xsl:call-template name ="callbackFunc__setCollections">
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
            </xsl:call-template>

            <xsl:call-template name="callbackFunc__clearLSObjectAdministration">
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
            </xsl:call-template>
            <xsl:call-template name="callbackFunc__checkObjectForSelection">
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
            </xsl:call-template>
            <xsl:call-template name="callbackFunc__selectionListenerCallOperations">
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
            </xsl:call-template>

            <!-- must be last as it refers to other generated operation signatures -->
            <xsl:call-template name ="typedClass_cacheInitializer">
                <xsl:with-param name="prefixedNamespace" select="$prefixedNamespace"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="fullTopicTypeNameWithUnderscores" select="$fullTopicTypeNameWithUnderscores"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <xsl:template name ="typedClass_cacheInitializer-predeclaration">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        static void
        ccpp_test_Foo_us_initializeObjectCache(
            ccpp_TypedObjectCache* objectCache
            );
        -->
        <xsl:text>static void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_initializeObjectCache(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_TypedObjectCache* objectCache);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedClass_createTypedObjectSeq">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        static void ccpp_test_Foo_createTypedObjectSeq(
            DLRL_Exception* exception,
            void* userData,
            void** arg,
            LOC_unsigned_long size)
        {
			test::FooSeq* cppTempSeq;

            DLRL_INFO(INF_ENTER);

            assert(exception);
            assert(!userData);
            assert(arg);
            assert(!*arg);

            cppTempSeq = new test::FooSeq(size);
            DLRL_VERIFY_ALLOC(cppTempSeq, exception, "Unable to allocate memory");
			cppTempSeq->length(size);
			*arg = reinterpret_cast<void*>(cppTempSeq);

            DLRL_Exception_EXIT(exception);
            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>static void ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_createTypedObjectSeq(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void* userData,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void** arg,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long size)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
		<xsl:text>    </xsl:text>
		<xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq* cppTempSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(!userData);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(arg);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(!*arg);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
		<xsl:text>    cppTempSeq = new </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq(size);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_ALLOC(cppTempSeq, exception, "Unable to allocate memory");</xsl:text>
        <xsl:value-of select="$NL"/>
		<xsl:text>    cppTempSeq->length(size);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    *arg = reinterpret_cast&lt;void*>(cppTempSeq);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedClass_addToTypedObjectSeq">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        static void ccpp_test_Foo_us_addToTypedObjectSeq(
            DLRL_Exception* exception,
            void* userData,
            void** arg,
            LOC_unsigned_long count,
            DLRL_LS_object notDuppedObject)/* must perform the duplicate ourselves! */
        {
            test::FooSeq* objectSeq;
            test::Foo* object;

            DLRL_INFO(INF_ENTER);

            assert(exception);
            assert(!userData);
            assert(arg);
            assert(*arg);
            assert(notDuppedObject);

            objectSeq = reinterpret_cast&lt;test::FooSeq*>(*arg);
            object = DOWNCAST_test_Foo(notDuppedObject);
            CORBA::add_ref(object);
            (*objectSeq)[count] = object;

            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>static void ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_addToTypedObjectSeq(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void* userData,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void** arg,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long count,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object notDuppedObject)/* must perform the duplicate ourselves! */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq* objectSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* object;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(!userData);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(arg);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(*arg);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(notDuppedObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectSeq = reinterpret_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*>(*arg);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    object = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>(notDuppedObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::add_ref(object);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    (*objectSeq)[count] = object;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="typedHomeConstructor">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>

        <!-- The following code is generated by this template for example type 'Foo' defined in module 'test'

        test::FooHome::FooHome()
        {
            DLRL_Exception exception;
            DK_ObjectHomeAdmin* newHome = NULL;
            ccpp_TypedObjectCache* objectCache = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            /* Allocate and initialize the function cache for the type represented
             * by this object home
             */
            objectCache = reinterpret_cast<ccpp_TypedObjectCache*>(os_malloc(sizeof(ccpp_TypedObjectCache)));
            DLRL_VERIFY_ALLOC(objectCache, &exception, "Out of resources");
            ccpp_test_Foo_us_initializeObjectCache(objectCache);
            /* create the kernel representative of the ObjectHome. Set the created
             * function cache as user data for the newly created object home so we may
             * access it later.
             */
            newHome = DK_ObjectHomeAdmin_new(&amp;exception, "<xsl:value-of select="@NAME"/>");
            DLRL_Exception_PROPAGATE(&amp;exception);
            DK_ObjectHomeAdmin_ts_setUserData(newHome, &amp;exception, reinterpret_cast<void*>(objectCache));
            DLRL_Exception_PROPAGATE(&amp;exception);
            /* set objectCache to NULL to ensure it is not freed twice if this operation
             * unwinds. Because at this point the home we created will be managing the
             * userData
             */
            objectCache = NULL;
            /* The private 'home' pointer will take over the ref count of the kernel
             * objecthome obtained from the new objectHomeAdmin operation. We are
             * basically linking the newly created kernel ObjectHome with the c++
             * ObjectHome we just created.
             */
            home = newHome;
            /* newHome value is now managed by this c++ object and will be cleaned
             * in it's destructor, set it to NULL for safety (i.e. prevent issues in
             * later extensions to this code).
             */
            newHome = NULL;

            DLRL_Exception_EXIT(&amp;exception);
            /* If an exception occured we need to perform rollback actions */
            if(exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                if(newHome)
                {
                    /* We need to first properly delete the object home so it has a
                     * chance to clean up it's resources. Afterwards we can release
                     * the reference count of the kernel objecthome which will result
                     * in the allocated memory to be freed.
                     */
                    DK_ObjectHomeAdmin_ts_delete(newHome, NULL);
                    DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(newHome));
                    newHome = NULL;
                }
                if(objectCache){
                    os_free(objectCache);
                }
            }
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>Home()</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_ObjectHomeAdmin* newHome = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_TypedObjectCache* objectCache = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_init(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Allocate and initialize the function cache for the type represented</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * by this object home</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache = reinterpret_cast&lt;ccpp_TypedObjectCache*>(os_malloc(sizeof(ccpp_TypedObjectCache)));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_ALLOC(objectCache, &amp;exception, "Out of resources");</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_initializeObjectCache(objectCache);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* create the kernel representative of the ObjectHome. Set the created</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * function cache as user data for the newly created object home so we may</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * access it later.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    newHome = DK_ObjectHomeAdmin_new(&amp;exception, "</xsl:text>
        <xsl:value-of select="@NAME"/>
        <xsl:text>");</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_ObjectHomeAdmin_ts_setUserData(newHome, &amp;exception, reinterpret_cast&lt;void*>(objectCache));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* set objectCache to NULL to ensure it is not freed twice if this operation</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * unwinds. Because at this point the home we created will be managing the</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * userData</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* The private 'home' pointer will take over the ref count of the kernel</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * objecthome obtained from the new objectHomeAdmin operation. We are</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * basically linking the newly created kernel ObjectHome with the c++</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * ObjectHome we just created.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    home = newHome;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* newHome value is now managed by this c++ object and will be cleaned</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * in it's destructor, set it to NULL for safety (i.e. prevent issues in</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * later extensions to this code).</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    newHome = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* If an exception occured we need to perform rollback actions */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    if(exception.exceptionID != DLRL_NO_EXCEPTION)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        if(newHome)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>            /* We need to first properly delete the object home so it has a</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>             * chance to clean up it's resources. Afterwards we can release</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>             * the reference count of the kernel objecthome which will result</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>             * in the allocated memory to be freed.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>             */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>            DK_ObjectHomeAdmin_ts_delete(newHome, NULL);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>            DK_Entity_ts_release(reinterpret_cast&lt;DK_Entity*>(newHome));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>            newHome = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        if(objectCache){</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>            os_free(objectCache);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_DlrlUtils_us_handleException(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="typedHomeDestructor">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>

        <!-- The following code is generated by this template for example type 'Foo' defined in module 'test'

        test::FooHome::~FooHome(){}

        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::~</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>Home(){}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="typedHome___duplicate">
        <xsl:param name="prefixedFullNameExceptLast"/>

        <!-- The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooHome_ptr
            test::FooHome::_duplicate (
                test::FooHome_ptr obj)
            {
                CORBA::Object::_duplicate(obj);
                return obj;
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home_ptr</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::_duplicate (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home_ptr obj) </xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Object::_duplicate(obj);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return obj;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="typedHome___narrow">
        <xsl:param name="prefixedFullNameExceptLast"/>

        <!-- The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooHome_ptr
            test::FooHome::_narrow (
                CORBA::Object_ptr obj)
            {
                return dynamic_cast<test::FooHome_ptr>(obj);
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home_ptr</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::_narrow (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Object_ptr obj) </xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home_ptr>(obj);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="typedHome___unchecked_narrow">
        <xsl:param name="prefixedFullNameExceptLast"/>

        <!-- The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooHome_ptr
            test::FooHome::_unchecked_narrow (CORBA::Object_ptr obj)
            {
                return dynamic_cast<test::FooHome_ptr>(obj);
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home_ptr</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::_unchecked_narrow (</xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Object_ptr obj) </xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home_ptr>(obj);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="typedHome___nil">
        <xsl:param name="prefixedFullNameExceptLast"/>

        <!-- The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooHome_ptr
            test::FooHome::_nil ()
            {
                return 0;
            }
        -->

        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home_ptr</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::_nil ()</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return 0;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="typedHome___this">
        <xsl:param name="prefixedFullNameExceptLast"/>

        <!-- The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooHome_ptr
            test::FooHome::_this ()
            {
                return this;
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home_ptr</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::_this ()</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return this;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedHome_listeners">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'

        test::FooListenerSeq*
        test::FooHome::listeners(
            ) THROW_ORB_AND_USER_EXCEPTIONS(
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooListenerSeq* listenersSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            DK_ObjectHomeAdmin_ts_getLSListeners(
                home,
                &amp;exception,
                NULL,
                reinterpret_cast<void**>(&listenersSeq));
            DLRL_Exception_PROPAGATE(&amp;exception);

            DLRL_Exception_EXIT(&amp;exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return listenersSeq;
        }-->

        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>ListenerSeq*</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::listeners(</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS(</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>ListenerSeq* listenersSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_ObjectHomeAdmin_ts_getLSListeners(
        home,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;listenersSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return listenersSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_attach_listener">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        CORBA::Boolean
        test::FooHome::attach_listener (
            test::FooListener_ptr listener,
            CORBA::Boolean concerns_contained_objects) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            CORBA::Boolean succeeded = FALSE;
            DLRL_LS_object cppObject = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            cppObject = UPCAST_DLRL_LS_OBJECT(test::FooListener::_duplicate(listener));

            succeeded = static_cast<CORBA::Boolean>(DK_ObjectHomeAdmin_ts_attachListener(
                        home,
                        &amp;exception,
                        NULL,
                        cppObject,
                        static_cast<LOC_boolean>(concerns_contained_objects)));
            DLRL_Exception_PROPAGATE(&amp;exception);

            DLRL_Exception_EXIT(&amp;exception);
            if(!succeeded || exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                CORBA::release(reinterpret_cast<CORBA::Object_ptr>(cppObject));
            }
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return succeeded;
        }
        -->
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::attach_listener (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Listener_ptr listener,</xsl:text>
        <xsl:value-of select="$NL"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean concerns_contained_objects) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::AlreadyDeleted)
{
    DLRL_Exception exception;
    </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Boolean succeeded = FALSE;
    DLRL_LS_object cppObject = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    cppObject = UPCAST_DLRL_LS_OBJECT(</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Listener::_duplicate(listener));

    succeeded = static_cast&lt;</xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Boolean>(DK_ObjectHomeAdmin_ts_attachListener(
                home,
                &amp;exception,
                NULL,
                cppObject,
                static_cast&lt;LOC_boolean>(concerns_contained_objects)));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    if(!succeeded || exception.exceptionID != DLRL_NO_EXCEPTION)
    {
        </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::release(reinterpret_cast&lt;</xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Object_ptr>(cppObject));
    }
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return succeeded;
}

</xsl:text>

    </xsl:template>

    <xsl:template name ="typedHome_detach_listener">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        CORBA::Boolean
        test::FooHome::detach_listener(
            test::FooListener_ptr listener) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            CORBA::Boolean succeeded = FALSE;
            DLRL_LS_object cppObject = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            cppObject = DK_ObjectHomeAdmin_ts_detachListener(
                        home,
                        &amp;exception,
                        NULL,
                        UPCAST_DLRL_LS_OBJECT(listener));
            DLRL_Exception_PROPAGATE(&amp;exception);

            if(cppObject)
            {
                succeeded = TRUE;
                CORBA::release(reinterpret_cast<CORBA::Object_ptr>(cppObject));
            }

            DLRL_Exception_EXIT(&amp;exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return succeeded;
        }
        -->
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::detach_listener(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Listener_ptr listener) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::AlreadyDeleted)
{
    DLRL_Exception exception;
    </xsl:text>
    <xsl:call-template name="get_corba_module_name"/>
    <xsl:text>::Boolean succeeded = FALSE;
    DLRL_LS_object cppObject = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    cppObject = DK_ObjectHomeAdmin_ts_detachListener(
                home,
                &amp;exception,
                NULL,
                UPCAST_DLRL_LS_OBJECT(listener));
    DLRL_Exception_PROPAGATE(&amp;exception);

    if(cppObject)
    {
        succeeded = TRUE;
        </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::release(reinterpret_cast&lt;</xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Object_ptr>(cppObject));
    }

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return succeeded;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_create_selection">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSelection_ptr
        test::FooHome::create_selection(
            DDS::SelectionCriterion_ptr criterion,
            CORBA::Boolean auto_refresh,
            CORBA::Boolean concerns_contained_objects) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted,
                DDS::PreconditionNotMet)
        {
            test::FooSelection_impl* cpp_selection = NULL;
            DK_SelectionAdmin* selection = NULL;
            DLRL_Exception exception;
            DK_SelectionCriterion kernelCriterion;
            DK_CriterionKind kind;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            DLRL_VERIFY_NOT_NULL(&amp;exception, criterion, "criterion");

            kind = static_cast<DK_CriterionKind>(criterion->kind());

            if(kind == DK_CRITERION_KIND_FILTER)
            {
                kernelCriterion.filterCriterion = UPCAST_DLRL_LS_OBJECT(CORBA::Object::_duplicate(criterion));
            } else
            {
                assert(kind == DK_CRITERION_KIND_QUERY);
                kernelCriterion.queryCriterion = NULL;/* not yet supported */
            }

            cpp_selection = new test::FooSelection_impl();
            DLRL_VERIFY_ALLOC(cpp_selection, &amp;exception, "Out of resources.");

            selection = DK_ObjectHomeAdmin_ts_createSelection(
                        home,
                        &amp;exception,
                        NULL,
                        UPCAST_DLRL_LS_OBJECT(cpp_selection),
                        &kernelCriterion,
                        kind,
                        static_cast<LOC_boolean>(auto_refresh),
                        static_cast<LOC_boolean>(concerns_contained_objects));
            DLRL_Exception_PROPAGATE(&exception);
            DDS::Selection::_duplicate(cpp_selection);

            setSelection(cpp_selection, selection);

            DLRL_Exception_EXIT(&amp;exception);
            if(exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                /* must perform clean up */
                if(kind == DK_CRITERION_KIND_FILTER)
                {
                    CORBA::release(criterion);
                }
                if(cpp_selection)
                {
                    CORBA::release(cpp_selection);
                }
                /* dont have to clean up 'selection', its the last call that can raise
                 * an exception and if it does it will return a null pointer anyway.
                 */
            }
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return cpp_selection;
        }
        -->

        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_ptr</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::create_selection(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DDS::SelectionCriterion_ptr criterion,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean auto_refresh,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean concerns_contained_objects) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::PreconditionNotMet)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl* cpp_selection = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_SelectionAdmin* selection = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_SelectionCriterion kernelCriterion;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_CriterionKind kind;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_init(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_NOT_NULL(&amp;exception, criterion, "criterion");</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$NL"/>
        <xsl:text>    cpp_selection = new </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Selection_impl();
    DLRL_VERIFY_ALLOC(cpp_selection, &amp;exception, "Out of resources.");

    kind = static_cast&lt;DK_CriterionKind>(criterion->kind());

    if(kind == DK_CRITERION_KIND_FILTER)
    {
        kernelCriterion.filterCriterion = UPCAST_DLRL_LS_OBJECT(</xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Object::_duplicate(criterion));
    } else
    {
        assert(kind == DK_CRITERION_KIND_QUERY);
        kernelCriterion.queryCriterion = NULL;/* not yet supported */
    }

    selection = DK_ObjectHomeAdmin_ts_createSelection(
                home,
                &amp;exception,
                NULL,
                UPCAST_DLRL_LS_OBJECT(cpp_selection),
                &amp;kernelCriterion,
                kind,
                static_cast&lt;LOC_boolean>(auto_refresh),
                static_cast&lt;LOC_boolean>(concerns_contained_objects));
    DLRL_Exception_PROPAGATE(&amp;exception);
    DDS::Selection::_duplicate(cpp_selection);

    setSelection(cpp_selection, selection);

    DLRL_Exception_EXIT(&amp;exception);
    if(exception.exceptionID != DLRL_NO_EXCEPTION)
    {
        /* must perform clean up */
        if(kind == DK_CRITERION_KIND_FILTER)
        {
            </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::release(criterion);
        }
        if(cpp_selection)
        {
            </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::release(cpp_selection);
            cpp_selection = NULL;
        }
        /* dont have to clean up 'selection', its the last call that can raise
         * an exception and if it does it will return a null pointer anyway.
         */
    }
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return cpp_selection;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_delete_selection">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::FooHome::delete_selection(
            test::FooSelection_ptr a_selection) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted,
                DDS::PreconditionNotMet)
        {
            DLRL_Exception exception;
            DK_SelectionAdmin* selection;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            selection = getSelectionAdmin(&exception, a_selection, "a_selection");
            DLRL_Exception_PROPAGATE(&exception);

            DK_ObjectHomeAdmin_ts_deleteSelection(home, &exception, NULL, selection);
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&amp;exception);
        }
        -->



        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::delete_selection(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Selection_ptr a_selection) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::AlreadyDeleted,
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;
    DK_SelectionAdmin* selection;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    selection = getSelectionAdmin(&amp;exception, a_selection, "a_selection");
    DLRL_Exception_PROPAGATE(&amp;exception);

    DK_ObjectHomeAdmin_ts_deleteSelection(home, &amp;exception, NULL, selection);
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_selections">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSelectionSeq*
        test::FooHome::selections(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSelectionSeq* selectionsSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            DK_ObjectHomeAdmin_ts_getLSSelections(
                home,
                &amp;exception,
                NULL,
                reinterpret_cast&lt;void**>(&amp;selectionsSeq));
            DLRL_Exception_PROPAGATE(&amp;exception);

            DLRL_Exception_EXIT(&amp;exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return selectionsSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionSeq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::selections(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>SelectionSeq* selectionsSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_ObjectHomeAdmin_ts_getLSSelections(
        home,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;selectionsSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return selectionsSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_get_modified_objects">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooHome::get_modified_objects(
            DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            DK_ObjectHomeAdmin_ts_getModifiedObjects(
                home,
                &amp;exception,
                NULL,
                reinterpret_cast&lt;void**>(&amp;valuesSeq));
            DLRL_Exception_PROPAGATE(&amp;exception);

            DLRL_Exception_EXIT(&amp;exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::get_modified_objects(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_ObjectHomeAdmin_ts_getModifiedObjects(
        home,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_get_deleted_objects">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooHome::get_deleted_objects(
            DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            DK_ObjectHomeAdmin_ts_getDeletedObjects(
                home,
                &amp;exception,
                NULL,
                reinterpret_cast&lt;void**>(&amp;valuesSeq));
            DLRL_Exception_PROPAGATE(&amp;exception);

            DLRL_Exception_EXIT(&amp;exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&amp;exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::get_deleted_objects(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_ObjectHomeAdmin_ts_getDeletedObjects(
        home,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_get_created_objects">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooHome::get_created_objects(
            DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            DK_ObjectHomeAdmin_ts_getCreatedObjects(
                home,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&amp;exception);

            DLRL_Exception_EXIT(&amp;exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&amp;exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::get_created_objects(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_ObjectHomeAdmin_ts_getCreatedObjects(
        home,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_get_objects">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooHome::get_objects(
            DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;
            DK_CacheAccessAdmin* access = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            switch (source->kind()) {
                case DDS::CACHE_KIND:
                    DK_ObjectHomeAdmin_ts_getAllObjectsForCache(
                        home,
                        &exception,
                        NULL,
                        reinterpret_cast<void**>(&valuesSeq));
                    DLRL_Exception_PROPAGATE(&exception);
                    break;
                case DDS::CACHEACCESS_KIND:
                    access = getCacheAccessAdmin(
                        &exception,
                        DDS::CacheAccess::_narrow(source),
                        "source");
                    DLRL_Exception_PROPAGATE(&exception);
                    DK_ObjectHomeAdmin_ts_getAllObjectsForCacheAccess(
                        home,
                        &exception,
                        access,
                        NULL,
                        reinterpret_cast<void**>(&valuesSeq));
                    DLRL_Exception_PROPAGATE(&exception);
                    break;
                default:
                    assert(FALSE);
            }

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::get_objects(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    switch (source->kind()) {
        case DDS::CACHE_KIND:
            DK_ObjectHomeAdmin_ts_getAllObjectsForCache(
                home,
                &amp;exception,
                NULL,
                reinterpret_cast&lt;void**>(&amp;valuesSeq));
            DLRL_Exception_PROPAGATE(&amp;exception);
            break;
        case DDS::CACHEACCESS_KIND:
            access = getCacheAccessAdmin(
                &amp;exception,
                DDS::CacheAccess::_narrow(source),
                "source");
            DLRL_Exception_PROPAGATE(&amp;exception);
            DK_ObjectHomeAdmin_ts_getAllObjectsForCacheAccess(
                home,
                &amp;exception,
                access,
                NULL,
                reinterpret_cast&lt;void**>(&amp;valuesSeq));
            DLRL_Exception_PROPAGATE(&amp;exception);
            break;
        default:
            assert(FALSE);
    }

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_create_unregistered_object">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::Foo*
        test::FooHome::create_unregistered_object(
            DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::PreconditionNotMet,
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            DK_CacheAccessAdmin* accessAdmin;
            test::Foo* objectAdmin = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DLRL_VERIFY_NOT_NULL(&exception, access, "access");

            accessAdmin = getCacheAccessAdmin(&exception, access, "access");
            DLRL_Exception_PROPAGATE(&exception);

            objectAdmin = DOWNCAST_test_Foo(DK_ObjectHomeAdmin_ts_createUnregisteredObject(
                home,
                &exception,
                NULL,
                accessAdmin));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return objectAdmin;
        }
        -->
        <xsl:value-of select="$prefixedFullName"/><xsl:text>*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/><xsl:text>Home::create_unregistered_object(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::PreconditionNotMet,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_CacheAccessAdmin* accessAdmin;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <!-- The code in this section does not require
             any substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>* objectAdmin = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DLRL_VERIFY_NOT_NULL(&amp;exception, access, "access");

    accessAdmin = getCacheAccessAdmin(&amp;exception, access, "access");
    DLRL_Exception_PROPAGATE(&amp;exception);

    objectAdmin = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>(DK_ObjectHomeAdmin_ts_createUnregisteredObject(
        home,
        &amp;exception,
        NULL,
        accessAdmin));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return objectAdmin;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_find_object">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::Foo*
        test::FooHome::find_object(
            const DDS::DLRLOid& oid,
            DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::NotFound,
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            /* not supported */

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return NULL;
        }
        -->
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::find_object(</xsl:text>
        <xsl:value-of select="$NL"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>    const DDS::DLRLOid&amp; oid,
    DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::NotFound,
        DDS::AlreadyDeleted)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    /* not supported */

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return NULL;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_register_object">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::FooHome::register_object(
            test::Foo* unregistered_object) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::PreconditionNotMet,
                DDS::AlreadyExisting,
                DDS::AlreadyDeleted)
        {
            DK_ObjectAdmin* objectAdmin;
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DLRL_VERIFY_NOT_NULL(&exception, unregistered_object, "unregistered_object");

            objectAdmin = getObjectRootAdmin(&exception, unregistered_object, "unregistered_object");
            DLRL_Exception_PROPAGATE(&exception);

            DK_ObjectHomeAdmin_ts_registerObject(home, &exception, NULL, objectAdmin);
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::register_object(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>* unregistered_object) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::PreconditionNotMet,
        DDS::AlreadyExisting,
        DDS::AlreadyDeleted)
{
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DLRL_VERIFY_NOT_NULL(&amp;exception, unregistered_object, "unregistered_object");

    objectAdmin = getObjectRootAdmin(&amp;exception, unregistered_object, "unregistered_object");
    DLRL_Exception_PROPAGATE(&amp;exception);

    DK_ObjectHomeAdmin_ts_registerObject(home, &amp;exception, NULL, objectAdmin);
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_create_object">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::Foo*
        test::FooHome::create_object(
            DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::PreconditionNotMet,
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            DK_CacheAccessAdmin* accessAdmin;
            test::Foo* objectAdmin = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DLRL_VERIFY_NOT_NULL(&exception, access, "access");

            accessAdmin = getCacheAccessAdmin(&exception, access, "access");
            DLRL_Exception_PROPAGATE(&exception);

            objectAdmin = DOWNCAST_test_Foo(DK_ObjectHomeAdmin_ts_createLSObject(
                home,
                &exception,
                NULL,
                accessAdmin));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return objectAdmin;
        }
        -->
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::create_object(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::PreconditionNotMet,
        DDS::AlreadyDeleted)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* accessAdmin;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <!-- The following code does not require
             any substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:value-of select="$prefixedFullName"/><xsl:text>* objectAdmin = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DLRL_VERIFY_NOT_NULL(&amp;exception, access, "access");

    accessAdmin = getCacheAccessAdmin(&amp;exception, access, "access");
    DLRL_Exception_PROPAGATE(&amp;exception);

    objectAdmin = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>(DK_ObjectHomeAdmin_ts_createLSObject(
        home,
        &amp;exception,
        NULL,
        accessAdmin));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return objectAdmin;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_registerType">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        DDS::ReturnCode_t
        test::FooHome::registerType(
            DDS::DomainParticipant_ptr participant,
            LOC_char* typeName,
            LOC_char* topicName)
        {
            DDS::ReturnCode_t retCode = DDS::RETCODE_OK;

            DLRL_INFO(INF_ENTER);

            if (0 == strcmp(typeName, "test::FooTopic"))
            {
                test::FooTopicTypeSupport typeSupport;
                retCode = typeSupport.register_type(participant, typeName);
            }

            DLRL_INFO(INF_EXIT);
            return retCode;
        }
        -->
        <xsl:text>DDS::ReturnCode_t</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::registerType(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DDS::DomainParticipant_ptr participant,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_char* typeName,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_char* topicName)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DDS::ReturnCode_t retCode = DDS::RETCODE_OK;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <xsl:for-each select="mainTopic|STATEMEMBER/multiPlaceTopic">
            <xsl:variable name="fullNameTypeSupport">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text">
                        <xsl:value-of select="@typename"/>
                        <xsl:text>TypeSupport</xsl:text>
                    </xsl:with-param>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- dont replace just prefix key words -->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    </xsl:text>
            <xsl:if test="position()!='1'">
                <xsl:text>else </xsl:text>
            </xsl:if>
            <xsl:text>if(0 == strcmp(typeName, "</xsl:text>
            <xsl:value-of select="@typename"/>
            <xsl:text>"))</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>    {</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$fullNameTypeSupport"/>
            <xsl:text> typeSupport;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        retCode = typeSupport.register_type(participant, typeName);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>    }</xsl:text>
            <xsl:value-of select="$NL"/>
        </xsl:for-each>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return retCode;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedSelection_members">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooSelection_impl::members(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DK_SelectionAdmin_ts_getLSMembers(
                selection,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl::members(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_SelectionAdmin_ts_getLSMembers(
        selection,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedSelection_get_inserted_members">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooSelection_impl::get_inserted_members(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DK_SelectionAdmin_ts_getLSInsertedMembers(
                selection,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl::get_inserted_members(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_SelectionAdmin_ts_getLSInsertedMembers(
        selection,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedSelection_get_modified_members">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooSelection_impl::get_modified_members(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DK_SelectionAdmin_ts_getLSModifiedMembers(
                selection,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl::get_modified_members(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_SelectionAdmin_ts_getLSModifiedMembers(
        selection,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedSelection_get_removed_members">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooSelection_impl::get_removed_members(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DK_SelectionAdmin_ts_getLSRemovedMembers(
                selection,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl::get_removed_members(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_SelectionAdmin_ts_getLSRemovedMembers(
        selection,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedSelection_set_listener">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSelectionListener_ptr
        test::FooSelection_impl::set_listener(
            test::FooSelectionListener_ptr listener) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            test::FooSelectionListener_ptr retVal = NULL;
            DLRL_LS_object tmp;
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            tmp = DK_SelectionAdmin_ts_setListener(
                selection,
                &amp;exception,
                NULL,
                UPCAST_DLRL_LS_OBJECT(listener));
            DLRL_Exception_PROPAGATE(&amp;exception);
            if(tmp != NULL)
            {
                retVal = DOWNCAST_test_Foo_SELECTION_LISTENER(tmp);
            }

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return retVal;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl::set_listener(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr listener) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr retVal = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object tmp;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_init(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    tmp = DK_SelectionAdmin_ts_setListener(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        selection,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        &amp;exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        NULL,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        UPCAST_DLRL_LS_OBJECT(listener));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    if(tmp != NULL)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        retVal = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_SELECTION_LISTENER(tmp);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_DlrlUtils_us_handleException(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return retVal;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedSelection_check_objects">
        <xsl:param name="prefixedFullNameExceptLast"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            CORBA::ULong
            test::FooSelection_impl::check_objects(
                DLRL_Exception* exception,
                DLRL_LS_object filterCriterion,
                const DDS::ObjectRootSeq& lsObjects,
                DK_ObjectAdmin** inputAdmins,
                DK_ObjectAdmin** passedAdmins)
            {
                test::FooFilter_ptr cppFilter;
                test::Foo* cppObject;
                CORBA::ULong nrObjects, i, j = 0;

                cppFilter = dynamic_cast<test::FooSelection_ptr>(reinterpret_cast<CORBA::Object_ptr>(filterCriterion));
                nrObjects = lsObjects->length();
                for (i = 0; i < nrObjects; i++) {
                    cppObject = dynamic_cast<test::Foo *>(lsObjects[i].in());
                    if (cppFilter->check_object(cppObject, DDS::UNDEFINED_MEMBERSHIP)) {
                        passedAdmin[j++] = inputAdmins[i];
                    }
                }
                return j;
            }
        -->
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::ULong</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl::check_objects(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object filterCriterion,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    const DDS::ObjectRootSeq&amp; lsObjects,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_ObjectAdmin** inputAdmins,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_ObjectAdmin** passedAdmins)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Filter_ptr cppFilter;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>* cppObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::ULong nrObjects, i, j = 0;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    cppFilter = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Filter_ptr>(reinterpret_cast&lt;</xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Object_ptr>(filterCriterion));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    nrObjects = lsObjects.length();</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    for (i = 0; i &lt; nrObjects; i++) {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        cppObject = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>*>(lsObjects[i].in());</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        if (cppFilter->check_object(cppObject, DDS::UNDEFINED_MEMBERSHIP)) {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>            passedAdmins[j++] = inputAdmins[i];</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return j;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedSelection_constructor">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooSelection_impl::FooSelection_impl(
                )
            {
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl::</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>Selection_impl(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    )</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedSelection_destructor">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooSelection_impl::~FooSelection_impl(
                )
            {
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl::~</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>Selection_impl(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    )</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedSelection_listener">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSelectionListener_ptr
        test::FooSelection_impl::listener(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_LS_object tmp;
            test::FooSelectionListener_ptr retVal = NULL;
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&amp;exception);

            tmp = DK_SelectionAdmin_ts_getListener(
                selection,
                &amp;exception,
                NULL);
            DLRL_Exception_PROPAGATE(&exception);
            if(tmp != NULL)
            {
                retVal = DOWNCAST_test_Foo_SELECTION_LISTENER(tmp);
            }

            DLRL_Exception_EXIT(&amp;exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&amp;exception);
            return retVal;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection_impl::listener(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr retVal = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object tmp;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_init(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    tmp = DK_SelectionAdmin_ts_getListener(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        selection,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        &amp;exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        NULL);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    if(tmp != NULL)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        retVal = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_SELECTION_LISTENER(tmp);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_DlrlUtils_us_handleException(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return retVal;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedFilter_constructor">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooFilter::FooFilter(
                )
            {
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Filter::</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>Filter(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    )</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedFilter_destructor">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooFilter::~FooFilter(
                )
            {
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Filter::~</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>Filter(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    )</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="typedSet_constructor-destructor">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooSet_impl::FooSet_impl(DK_SetAdmin* collection) : DDS::Set_impl(collection)
            {
            }

            test::FooSet::~FooSet_impl()
            {
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl::</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>Set_impl(DK_SetAdmin* collection) : DDS::Set_impl(collection)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl::~</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>Set_impl()</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>



    <xsl:template name ="typedSet_add">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::FooSet_impl::add(
            test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted,
                DDS::PreconditionNotMet)
        {
            DLRL_Exception exception;
            DK_ObjectAdmin* objectAdmin;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DLRL_VERIFY_NOT_NULL(&exception, value, "value");

            objectAdmin = getObjectRootAdmin(&exception, value, "value");
            DLRL_Exception_PROPAGATE(&exception);

            DK_SetAdmin_ts_add(set, &exception, NULL, objectAdmin);
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&amp;exception);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl::add(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::AlreadyDeleted,
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;
    DK_ObjectAdmin* objectAdmin;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DLRL_VERIFY_NOT_NULL(&amp;exception, value, "value");

    objectAdmin = getObjectRootAdmin(&amp;exception, value, "value");
    DLRL_Exception_PROPAGATE(&amp;exception);

    DK_SetAdmin_ts_add(set, &amp;exception, NULL, objectAdmin);
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedSet_added_elements">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooSet_impl::added_elements(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DK_SetAdmin_ts_getLSAddedElements(
                set,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&amp;exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl::added_elements(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_SetAdmin_ts_getLSAddedElements(
        set,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedSet_contains">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        CORBA::Boolean
        test::FooSet_impl::contains(
            test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
            DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            DK_ObjectAdmin* objectAdmin;
            CORBA::Boolean retVal = FALSE;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DLRL_VERIFY_NOT_NULL(&exception, value, "value");

            objectAdmin = getObjectRootAdmin(&exception, value, "value");
            DLRL_Exception_PROPAGATE(&exception);

            retVal = DK_SetAdmin_ts_contains(set, &exception, objectAdmin);
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&amp;exception);
        }
        -->
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl::contains(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (
    DDS::AlreadyDeleted)
{
    DLRL_Exception exception;
    DK_ObjectAdmin* objectAdmin;
    </xsl:text>
    <xsl:call-template name="get_corba_module_name"/>
    <xsl:text>::Boolean retVal = FALSE;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DLRL_VERIFY_NOT_NULL(&amp;exception, value, "value");

    objectAdmin = getObjectRootAdmin(&amp;exception, value, "value");
    DLRL_Exception_PROPAGATE(&amp;exception);

    retVal = static_cast&lt;</xsl:text>
    <xsl:call-template name="get_corba_module_name"/>
    <xsl:text>::Boolean>(DK_SetAdmin_ts_contains(set, &amp;exception, objectAdmin));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return retVal;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedSet_remove">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::FooSet_impl::remove(
            test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted,
                DDS::PreconditionNotMet)
        {
            DLRL_Exception exception;
            DK_ObjectAdmin* objectAdmin;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DLRL_VERIFY_NOT_NULL(&exception, value, "value");

            objectAdmin = getObjectRootAdmin(&exception, value, "value");
            DLRL_Exception_PROPAGATE(&exception);

            DK_SetAdmin_ts_remove(set, &exception, NULL, objectAdmin);
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl::remove(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::AlreadyDeleted,
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;
    DK_ObjectAdmin* objectAdmin;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DLRL_VERIFY_NOT_NULL(&amp;exception, value, "value");

    objectAdmin = getObjectRootAdmin(&amp;exception, value, "value");
    DLRL_Exception_PROPAGATE(&amp;exception);

    DK_SetAdmin_ts_remove(set, &amp;exception, NULL, objectAdmin);
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedSet_values">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooSet_impl::values(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DK_SetAdmin_ts_getLSValues(
                set,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl::values(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_SetAdmin_ts_getLSValues(
        set,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedSet_removed_elements">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooSet_impl::removed_elements(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DK_SetAdmin_ts_getLSRemovedElements(
                set,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl::removed_elements(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_SetAdmin_ts_getLSRemovedElements(
        set,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name="typedStrMap_constructor-destructor">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooStrMap_impl::FooStrMap_impl(DK_MapAdmin* collection) : DDS::StrMap_impl(collection)
            {
            }

            test::FooStrMap::~FooStrMap_impl()
            {
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>StrMap_impl::</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>StrMap_impl(DK_MapAdmin* collection) : DDS::StrMap_impl(collection)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>StrMap_impl::~</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>StrMap_impl()</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedStrMap_values">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooStrMap_impl::values(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
            DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DK_MapAdmin_ts_getLSValues(
                map,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>StrMap_impl::values(
    ) THROW_ORB_AND_USER_EXCEPTIONS (
    DDS::AlreadyDeleted)
{
    DLRL_Exception exception;
    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_MapAdmin_ts_getLSValues(
        map,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedStrMap_get">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::Foo*
        test::FooStrMap_impl::get(
            const char* key) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted,
                DDS::NoSuchElement)
        {
            test::Foo* retVal = NULL;
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DLRL_VERIFY_NOT_NULL(&exception, key, "key");

            retVal = DOWNCAST_test_Foo(DK_MapAdmin_ts_get(
                map,
                &exception,
                NULL,
                reinterpret_cast<const void*>(key)));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return retVal;
        }
        -->
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>StrMap_impl::get(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    const char* key) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::NoSuchElement)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>* retVal = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DLRL_VERIFY_NOT_NULL(&amp;exception, key, "key");

    retVal = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>(DK_MapAdmin_ts_get(
        map,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;const void*>(key)));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return retVal;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedStrMap_put">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::FooStrMap_impl::put(
            const char* key,
            test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted,
                DDS::PreconditionNotMet)
        {
            DLRL_Exception exception;
            DK_ObjectAdmin* objectAdmin;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DLRL_VERIFY_NOT_NULL(&exception, key, "key");
            DLRL_VERIFY_NOT_NULL(&exception, value, "value");

            objectAdmin = getObjectRootAdmin(&exception, value, "value");
            DLRL_Exception_PROPAGATE(&exception);

            DK_MapAdmin_ts_put(
                map,
                &exception,
                NULL,
                reinterpret_cast<const void*>(key),
                objectAdmin);
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>StrMap_impl::put(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    const char* key,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <!-- The remaining code from this point does not require
             anymore substitution. Therefore it will be contained within
             one text element. Because layout inside a text element takes
             every space into account you'll see the code jump a little
             to the left to ensure it's generated in a proper layout.
             Alternatively we could place every line between single
             text elements, but that would make it less easy to read.
        -->
        <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::AlreadyDeleted,
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;
    DK_ObjectAdmin* objectAdmin;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DLRL_VERIFY_NOT_NULL(&amp;exception, key, "key");
    DLRL_VERIFY_NOT_NULL(&amp;exception, value, "value");

    objectAdmin = getObjectRootAdmin(&amp;exception, value, "value");
    DLRL_Exception_PROPAGATE(&amp;exception);

    DK_MapAdmin_ts_put(
        map,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;const void*>(key),
        objectAdmin);
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
}

</xsl:text>
    </xsl:template>

    <xsl:template name="typedIntMap_constructor-destructor">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="nonPrefixedName"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            test::FooIntMap_impl::FooIntMap_impl(DK_MapAdmin* collection) : DDS::IntMap_impl(collection)
            {
            }

            test::FooIntMap::~FooIntMap_impl()
            {
            }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>IntMap_impl::</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>IntMap_impl(DK_MapAdmin* collection) : DDS::IntMap_impl(collection)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>IntMap_impl::~</xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>IntMap_impl()</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedIntMap_values">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooIntMap_impl::values(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;
            test::FooSeq* valuesSeq = NULL;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DK_MapAdmin_ts_getLSValues(
                map,
                &exception,
                NULL,
                reinterpret_cast<void**>(&valuesSeq));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return valuesSeq;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>IntMap_impl::values(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq* valuesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DK_MapAdmin_ts_getLSValues(
        map,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;void**>(&amp;valuesSeq));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return valuesSeq;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedIntMap_get">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::Foo*
        test::FooIntMap_impl::get(
            CORBA::Long key) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted,
                DDS::NoSuchElement)
        {
            test::Foo* retVal = NULL;
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            retVal = DOWNCAST_test_Foo(DK_MapAdmin_ts_get(
                map,
                &exception,
                NULL,
                reinterpret_cast<const void*>(&key)));
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return retVal;
        }
        -->
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>IntMap_impl::get(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Long key) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::AlreadyDeleted,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        DDS::NoSuchElement)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* retVal = NULL;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_init(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    retVal = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>(DK_MapAdmin_ts_get(
        map,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;const void*>(&amp;key)));
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return retVal;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedIntMap_put">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::FooIntMap_impl::put(
            CORBA::Long key,
            test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted,
                DDS::PreconditionNotMet)
        {
            DLRL_Exception exception;
            DK_ObjectAdmin* objectAdmin;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            DLRL_VERIFY_NOT_NULL(&exception, value, "value");

            objectAdmin = getObjectRootAdmin(&exception, value, "value");
            DLRL_Exception_PROPAGATE(&exception);

            DK_MapAdmin_ts_put(
                map,
                &exception,
                NULL,
                reinterpret_cast<const void*>(&key),
                objectAdmin);
            DLRL_Exception_PROPAGATE(&exception);

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>IntMap_impl::put(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Long key,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::AlreadyDeleted,
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;
    DK_ObjectAdmin* objectAdmin;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    DLRL_VERIFY_NOT_NULL(&amp;exception, value, "value");

    objectAdmin = getObjectRootAdmin(&amp;exception, value, "value");
    DLRL_Exception_PROPAGATE(&amp;exception);

    DK_MapAdmin_ts_put(
        map,
        &amp;exception,
        NULL,
        reinterpret_cast&lt;const void*>(&amp;key),
        objectAdmin);
    DLRL_Exception_PROPAGATE(&amp;exception);

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedList_values">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::FooSeq*
        test::FooList_impl::values(
            ) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            /* not yet supported */

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return NULL;
        }
        -->
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>List_impl::values(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::AlreadyDeleted)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    /* not yet supported */

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return NULL;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedList_get">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        test::Foo*
        test::FooList_impl::get(
            CORBA::Long key) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::AlreadyDeleted,
                DDS::NoSuchElement)
        {
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            /* not yet supported */

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
            return NULL;
        }
        -->
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>List_impl::get(
    </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Long key) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::AlreadyDeleted,
        DDS::NoSuchElement)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    /* not yet supported */

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
    return NULL;
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedList_add">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::FooList_impl::add(
            test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::PreconditionNotMet,
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            /* not yet supported */

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>List_impl::add(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::PreconditionNotMet,
        DDS::AlreadyDeleted)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    /* not yet supported */

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedList_put">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::FooList_impl::put(
            CORBA::Long key,
            test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                DDS::PreconditionNotMet,
                DDS::AlreadyDeleted)
        {
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            /* not yet supported */

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>List_impl::put(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Long key,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (
        DDS::PreconditionNotMet,
        DDS::AlreadyDeleted)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&amp;exception);

    /* not yet supported */

    DLRL_Exception_EXIT(&amp;exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&amp;exception);
}

</xsl:text>
    </xsl:template>

    <xsl:template name ="typedHome_loadMetaModel">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::FooHome::loadMetaModel(
            )
        {
            DLRL_Exception exception;

            DLRL_INFO(INF_ENTER);

            DLRL_Exception_init(&exception);

            /* some type specific implementation code, see template code for details */

            DLRL_Exception_EXIT(&exception);
            DLRL_INFO(INF_EXIT);
            ccpp_DlrlUtils_us_handleException(&exception);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Home::loadMetaModel(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    )</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception exception;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_init(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <xsl:variable name="mapping">
            <xsl:choose>
                <xsl:when test="mainTopic/keyDescription/@content='NoOid'">
                    <xsl:text>DMM_MAPPING_PREDEFINED</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>DMM_MAPPING_DEFAULT</xsl:text><!-- SimpleOid & FullOid -->
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="currentValueDef" select="."/>
        <xsl:variable name="mainTopicName"              select="mainTopic/@name"/>
        <!-- Next code generates the following c++ code for an object with a NoOid mapping:
        /***********************************************/
        /* Create the DLRL meta model class first!     */
        /***********************************************/
        DK_MMFacade_us_createDLRLClass(
            home,
            &amp;exception,
            NULL,
            DMM_MAPPING_PREDEFINED);
        DLRL_Exception_PROPAGATE(&exception);

        -->
        <xsl:text>    /***********************************************/</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Create the DLRL meta model class first!     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /***********************************************/</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_MMFacade_us_createDLRLClass(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        home,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        &amp;exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        NULL,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        </xsl:text>
        <xsl:value-of select="$mapping"/>
        <xsl:text>);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Next code generates the following c++ code for an object with a
        One mainTopic (struct 'FooTopic' in module 'test' with topic name 'Foo') a
        nd one multiplace topic(struct 'FooSetTopic' in module 'test' with topic name 'FooSet')
        (extension/place topics unsupported):
        /***********************************************/
        /* insert the topics this class (main,         */
        /* extension, place and multiplace)            */
        /***********************************************/
        DK_MMFacade_us_createMainTopic(
            home,
            &exception,
            "Foo",
            "test::FooTopic");
        DLRL_Exception_PROPAGATE(&exception);

        DK_MMFacade_us_createMultiPlaceTopic(
            home,
            &exception,
            "FooSet",
            "test::FooSetTopic");
        DLRL_Exception_PROPAGATE(&exception);

        -->
        <xsl:text>    /***********************************************/</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* insert the topics this class (main,         */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* extension, place and multiplace)            */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /***********************************************/</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_MMFacade_us_createMainTopic(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        home,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        &amp;exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        "</xsl:text>
        <xsl:value-of select="mainTopic/@name"/>
        <xsl:text>",</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        "</xsl:text>
        <xsl:value-of select="mainTopic/@typename"/>
        <xsl:text>");</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:for-each select="STATEMEMBER/multiPlaceTopic">
            <xsl:text>    DK_MMFacade_us_createMultiPlaceTopic(</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        home,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        &amp;exception,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        "</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>",</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        "</xsl:text>
            <xsl:value-of select="@typename"/>
            <xsl:text>");</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
        </xsl:for-each>

        <!-- Following template code generates the following c++ code
        for an struct member myX in topic struct Foo.
        /***********************************************/
        /* insert all DCPS fields and map them to their*/
        /*  respective topics (types default to long)  */
        /***********************************************/
        DK_MMFacade_us_createDCPSField(
            home,
            &exception,
            "myX",
            DMM_KEYTYPE_NORMAL,
            DMM_ATTRIBUTETYPE_LONG,
            "Foo");
        DLRL_Exception_PROPAGATE(&exception);
        -->
        <xsl:text>    /***********************************************/</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* insert all DCPS fields and map them to their*/</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* respective topics (types default to long)   */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /***********************************************/</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:for-each select="DCPSField">
            <xsl:text>    DK_MMFacade_us_createDCPSField(</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        home,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        &amp;exception,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        "</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>",</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        DMM_KEYTYPE</xsl:text>
            <xsl:value-of select="@keyType"/>
            <xsl:text>,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        DMM_ATTRIBUTETYPE_LONG,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        "</xsl:text>
            <xsl:value-of select="@topic"/>
            <xsl:text>");</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
        </xsl:for-each>

        <!-- only generate for non local attributes
            (valueField | keyDescription | multiPlaceTopic)
        -->
        <xsl:for-each select="STATEMEMBER">
            <!-- some handy variables we use everywhere -->
            <xsl:variable name="dlrlAttributeName" select="DECLARATOR/@NAME"/>
            <xsl:variable name="type" select="TYPEREF/@TYPE"/>
            <xsl:variable name="currentStateMember" select="."/>

            <!-- a valueField indicates a mono attribute -->
            <xsl:if test="valueField">
                <!-- determine if the mono attribute is immutable
                    by calling a special template, store the result
                    in the variable.
                -->
                <xsl:variable name="immutable">
                    <xsl:call-template name="determineMonoAttributeImmutabilityFromWithinStateMemberElement">
                        <xsl:with-param name="dcpsFieldName" select="valueField"/>
                        <xsl:with-param name="topicName" select="$mainTopicName"/>
                        <xsl:with-param name="trueVal">
                            <xsl:text>TRUE</xsl:text>
                        </xsl:with-param>
                        <xsl:with-param name="falseVal">
                            <xsl:text>FALSE</xsl:text>
                        </xsl:with-param>
                    </xsl:call-template>
                </xsl:variable>
                <!-- now we are completely ready to generate the calls
                    for a mono attribute insertion into the DLRL meta model
                -->
                <!-- for each mono attribute ('x') which is not immutable ('false')
                    and mapped to struct member 'myX' in topic 'test::FooTopic' the following
                    is generated:
                    /***********************************************/
                    /* Meta model insertion of mono attribute 'x'  */
                    /* (types default to long)                     */
                    /***********************************************/
                    DK_MMFacade_us_createAttribute(
                        home,
                        &exception,
                        "x",
                        FALSE,
                        DMM_ATTRIBUTETYPE_LONG);
                    DLRL_Exception_PROPAGATE(&exception);

                -->
                <xsl:text>    /***********************************************/</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    /* Meta model insertion of mono attribute '</xsl:text>
                <xsl:value-of select="$dlrlAttributeName"/>
                <xsl:text>'  */</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    /* (types default to long)                     */</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    /***********************************************/</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    DK_MMFacade_us_createAttribute(</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        home,</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        &amp;exception,</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        "</xsl:text>
                <xsl:value-of select="$dlrlAttributeName"/>
                <xsl:text>",</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        </xsl:text>
                <xsl:value-of select="$immutable"/>
                <xsl:text>,</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        DMM_ATTRIBUTETYPE_LONG);</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <!-- Following part generates:
                    DK_MMFacade_us_mapDLRLAttributeToDCPSTopic(
                        home,
                        &exception,
                        "x",
                        "test::FooTopic");
                    DLRL_Exception_PROPAGATE(&exception);

                -->
                <xsl:text>    DK_MMFacade_us_mapDLRLAttributeToDCPSTopic(</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        home,</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        &amp;exception,</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        "</xsl:text>
                <xsl:value-of select="$dlrlAttributeName"/>
                <xsl:text>",</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        "</xsl:text>
                <xsl:value-of select="$mainTopicName"/>
                <xsl:text>");</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <!-- Following part generates:
                    DK_MMFacade_us_mapDLRLAttributeToDCPSField(
                        home,
                        &exception,
                        "x",
                        "myX");
                    DLRL_Exception_PROPAGATE(&exception);
                -->
                <xsl:text>    DK_MMFacade_us_mapDLRLAttributeToDCPSField(</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        home,</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        &amp;exception,</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        "</xsl:text>
                <xsl:value-of select="$dlrlAttributeName"/>
                <xsl:text>",</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        "</xsl:text>
                <xsl:value-of select="valueField"/>
                <xsl:text>");</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
            </xsl:if>

            <!-- a keyDescription indicates a mono relation -->
             <xsl:if test="keyDescription">
                <!-- The following code is generated within this 'if' element.
                    For a relation with the following characteristics:
                    - target class: 'Bar'defined in module 'test'
                    - relation name: 'myBar'
                    - optional: yes, validityField name is 'isBarValid'
                    - owning class main topic: 'FooTopic' defined in module 'test'
                    - target class main topic: 'BarTopic defined in module 'test'
                    - One foreign key used to describe the relation: called 'barID'
                    in FooTopic and 'ID' in BarTopic.
                    - Associations are not supported so default to NULL.
                    - Compositions are not supported so default to FALSE.

                    The following code is generated for the above described relation:
                    /***********************************************/
                    /* Meta model insertion of mono relation 'myBar' */
                    /***********************************************/
                    DK_MMFacade_us_createRelation(
                        home,
                        &exception,
                        FALSE,
                        "myBar",
                        "test::Bar",
                        NULL,
                        TRUE);
                    DLRL_Exception_PROPAGATE(&exception);

                    DK_MMFacade_us_setDLRLRelationTopicPair(
                        home,
                        &exception,
                        "myBar",
                        "test::FooTopic",
                        "test::BarTopic");
                    DLRL_Exception_PROPAGATE(&exception);

                    DK_MMFacade_us_addDLRLRelationKeyFieldPair(
                        home,
                        &exception,
                        "myBar",
                        "barID",
                        "ID");
                    DLRL_Exception_PROPAGATE(&exception);

                    DK_MMFacade_us_setDLRLRelationValidityField(
                        home,
                        &exception,
                        "myBar",
                        "isBarValid");
                    DLRL_Exception_PROPAGATE(&exception);
                -->


                <!-- is this mono relation optional? A validityField indicates
                    it is optional, and any mono relation mapped onto a
                    SimpleOid or FullOid type is optional by default
                -->
                <xsl:variable name="isOptional">
                    <xsl:choose>
                        <xsl:when test="validityField">
                            <xsl:text>TRUE</xsl:text>
                        </xsl:when>
                        <xsl:when test="keyDescription/@content!='NoOid'">
                            <xsl:text>TRUE</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:text>FALSE</xsl:text>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <!-- store the keydescription element, so we can traverse from
                    this node again later in the algorithm
                 -->
                <xsl:variable name="keyDescriptionElement" select="keyDescription"/>

                <!-- we first need to locate the forward valuetype definition so we
                    know what this relation points to. We are looking for a valuetype
                    def with the correct mame, as we are processed a mono relation
                -->
                <xsl:for-each select="//VALUEDEF[@NAME=$type]">
                    <!-- we have now found the valuetype def that describes the
                        target type of the relation
                    -->
                    <xsl:text>    /***********************************************/</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    /* Meta model insertion of mono relation '</xsl:text>
                    <xsl:value-of select="$dlrlAttributeName"/>
                    <xsl:text>' */</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    /***********************************************/</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <!-- first generate the main create relation call -->
                    <xsl:text>    DK_MMFacade_us_createRelation(</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        home,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        &amp;exception,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        FALSE,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$dlrlAttributeName"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="@NAME"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        NULL,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        </xsl:text>
                    <xsl:value-of select="$isOptional"/>
                    <xsl:text>);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:value-of select="$NL"/>

                    <!-- now we need to determine the maintopic name
                        of the targeted object
                    -->
                    <xsl:text>    DK_MMFacade_us_setDLRLRelationTopicPair(</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        home,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        &amp;exception,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$dlrlAttributeName"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$mainTopicName"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="mainTopic/@name"/>
                    <xsl:text>");</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:value-of select="$NL"/>
                    <!-- Now we need to generate operations which insert the
                        key fields correctly into the method model.
                        Note that the sequence of key fields is important
                        (IE keyfield 1 of the relation topic description matches
                        keyfield 1 of the targeted valuetypes maintopic
                    -->
                    <xsl:for-each select="mainTopic/keyDescription/keyField">
                        <xsl:variable name="targetKeyFieldName" select="."/>
                        <xsl:variable name="targetKeyFieldPosition" select="position()"/>
                        <xsl:for-each select="$keyDescriptionElement/keyField[position()=$targetKeyFieldPosition]">
                            <xsl:text>    DK_MMFacade_us_addDLRLRelationKeyFieldPair(</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>        home,</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>        &amp;exception,</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>        "</xsl:text>
                            <xsl:value-of select="$dlrlAttributeName"/>
                            <xsl:text>",</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>        "</xsl:text>
                            <xsl:value-of select="."/>
                            <xsl:text>",</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>        "</xsl:text>
                            <xsl:value-of select="$targetKeyFieldName"/>
                            <xsl:text>");</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:value-of select="$NL"/>
                        </xsl:for-each>
                    </xsl:for-each>
                </xsl:for-each>
                <xsl:if test="validityField">
                    <xsl:text>    DK_MMFacade_us_setDLRLRelationValidityField(</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        home,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        &amp;exception,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$dlrlAttributeName"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="validityField/@name"/>
                    <xsl:text>");</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                    <xsl:value-of select="$NL"/>
                </xsl:if>
            </xsl:if>

            <!-- a multiPlaceTopic indicates a multi relation -->
            <xsl:if test="multiPlaceTopic">
                <xsl:variable name="indexField" select="multiPlaceTopic/@indexField"/>
                <xsl:variable name="multiPlaceTopicName" select="multiPlaceTopic/@name"/>
                <!-- Following part generates the following code for
                    multiRelation with name 'myBars':
                    /***********************************************/
                    /* Meta model insertion of multi relation 'myBars' */
                    /***********************************************/
                -->
                <xsl:text>    /***********************************************/</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    /* Meta model insertion of multi relation '</xsl:text>
                <xsl:value-of select="$dlrlAttributeName"/>
                <xsl:text>' */</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    /***********************************************/</xsl:text>
                <xsl:value-of select="$NL"/>

                <!-- we first need to locate the forward valuetype
                    definition so we know what this relation points to.
                We are looking for a forward valuetype def with the
                'Set|IntMap|StrMap' pattern, as we are processed a multi relation-->
                <xsl:for-each select="//VALUEFORWARDDEF[@NAME=$type]">
                    <xsl:variable name="basisType">
                        <xsl:call-template name="determineMultiRelationBaseType">
                            <xsl:with-param name="pattern" select="@pattern"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="forwardItemType" select="@itemType"/>

                    <!-- Following part generates the following code for
                        intmap multi relation 'myBars of type 'Bar' (defined in
                        module 'test'). Compositions default to FALSE,
                        associations default to NULL as neither is supported:
                        DK_MMFacade_us_createMultiRelation(
                            home,
                            &exception,
                            FALSE,
                            "myBars",
                            "test::Bar",
                            NULL,
                            DMM_BASIS_INT_MAP);
                        DLRL_Exception_PROPAGATE(&exception);

                    -->
                    <xsl:text>    DK_MMFacade_us_createMultiRelation(</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        home,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        &amp;exception,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        FALSE,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$dlrlAttributeName"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$forwardItemType"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        NULL,</xsl:text>
                    <xsl:text>        DMM_BASIS</xsl:text>
                    <xsl:value-of select="$basisType"/>
                    <xsl:text>);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:value-of select="$NL"/>

                    <!-- Following part generates the following code for
                        multi relation 'myBars and mapped onto multiplace topic
                        'Foo_myBars_topic' defined in module 'test':
                        /* set the topic onto which the multi relation is mapped */
                        DK_MMFacade_us_setMultiRelationRelationTopic(
                            home,
                            &exception,
                            "myBars",
                            "test::Foo_myBars_topic");
                        DLRL_Exception_PROPAGATE(&exception);

                    -->
                    <xsl:text>    /* set the topic onto which the multi relation is mapped */</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    DK_MMFacade_us_setMultiRelationRelationTopic(</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        home,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        &amp;exception,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$dlrlAttributeName"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$multiPlaceTopicName"/>
                    <xsl:text>");</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:value-of select="$NL"/>

                    <xsl:for-each select="//VALUEDEF[@NAME=$forwardItemType]">
                        <xsl:variable name="tempMainTopicName" select="mainTopic/@name"/>
                        <!-- Following part generates the following code for
                            multi relation 'myBars and mapped onto multiplace topic
                            'Foo_myBars_topic' defined in module 'test':
                            DK_MMFacade_us_setDLRLRelationTopicPair(
                                home,
                                &exception,
                                "myBars",
                                "test::FooTopic",
                                "test::BarTopic");
                            DLRL_Exception_PROPAGATE(&exception);
                        -->
                        <xsl:text>    DK_MMFacade_us_setDLRLRelationTopicPair(</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>        home,</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>        &amp;exception,</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>        "</xsl:text>
                        <xsl:value-of select="$dlrlAttributeName"/>
                        <xsl:text>",</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>        "</xsl:text>
                        <xsl:value-of select="$mainTopicName"/>
                        <xsl:text>",</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>        "</xsl:text>
                        <xsl:value-of select="$tempMainTopicName"/>
                        <xsl:text>");</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:value-of select="$NL"/>

                        <xsl:for-each select="mainTopic/keyDescription/keyField">
                            <!-- Following part generates the following code for
                            multi relation 'myBars' with target DCPS field 'barId'.
                            DK_MMFacade_us_addTargetField(
                                home,
                                &exception,
                                "myBars",
                                "barId");
                            DLRL_Exception_PROPAGATE(&exception);

                            -->
                            <xsl:text>    DK_MMFacade_us_addTargetField(</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>        home,</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>        &amp;exception,</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>        "</xsl:text>
                            <xsl:value-of select="$dlrlAttributeName"/>
                            <xsl:text>",</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>        "</xsl:text>
                            <xsl:value-of select="."/>
                            <xsl:text>");</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                            <xsl:value-of select="$NL"/>
                            <xsl:value-of select="$NL"/>
                        </xsl:for-each>
                    </xsl:for-each>
                    <xsl:if test="not (@pattern='Set')">
                        <!-- Following part generates the following code for
                        multi relation 'myBars' which has an indexField ('index').
                        DK_MMFacade_us_setRelationTopicIndexField(
                            home,
                            &exception,
                            "myBars",
                            "index");
                        DLRL_Exception_PROPAGATE(&exception);

                        -->
                        <xsl:text>    /* set the index field within the relation topic */</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>    DK_MMFacade_us_setRelationTopicIndexField(</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>        home,</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>        &amp;exception,</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>        "</xsl:text>
                        <xsl:value-of select="$dlrlAttributeName"/>
                        <xsl:text>",</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>        "</xsl:text>
                        <xsl:value-of select="$indexField"/>
                        <xsl:text>");</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                        <xsl:value-of select="$NL"/>
                        <xsl:value-of select="$NL"/>
                    </xsl:if>
                </xsl:for-each>
                <xsl:for-each select="$currentValueDef/mainTopic/keyDescription/keyField">
                    <!-- Following part generates the following code for
                    multi relation 'myBars' which has an ownerField ('id').
                    DK_MMFacade_us_addOwnerField(
                        home,
                        &exception,
                        "myBars",
                        "id");
                    DLRL_Exception_PROPAGATE(&exception);

                    -->
                    <xsl:text>    DK_MMFacade_us_addOwnerField(</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        home,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        &amp;exception,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$dlrlAttributeName"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="."/>
                    <xsl:text>");</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:value-of select="$NL"/>
                </xsl:for-each>
                <xsl:for-each select="keyDescription/keyField">
                    <!-- Following part generates the following code for
                    multi relation 'myBars' which has an targetField ('barId').
                    DK_MMFacade_us_addRelationTopicTargetField(
                        home,
                        &exception,
                        "myBars",
                        "barId");
                    DLRL_Exception_PROPAGATE(&exception);

                    -->
                    <xsl:text>    DK_MMFacade_us_addRelationTopicTargetField(</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        home,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        &amp;exception,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$dlrlAttributeName"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="."/>
                    <xsl:text>");</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:value-of select="$NL"/>
                </xsl:for-each>
                <xsl:for-each select="multiPlaceTopic/keyDescription/keyField">
                    <!-- Following part generates the following code for
                    multi relation 'myBars' which has an ownerField ('fooId').
                    DK_MMFacade_us_addRelationTopicOwnerField(
                        home,
                        &exception,
                        "myBars",
                        "fooId");
                    DLRL_Exception_PROPAGATE(&exception);

                    -->
                    <xsl:text>    DK_MMFacade_us_addRelationTopicOwnerField(</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        home,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        &amp;exception,</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="$dlrlAttributeName"/>
                    <xsl:text>",</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        "</xsl:text>
                    <xsl:value-of select="."/>
                    <xsl:text>");</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>    DLRL_Exception_PROPAGATE(&amp;exception);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:value-of select="$NL"/>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>

        <xsl:text>    DLRL_Exception_EXIT(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_DlrlUtils_us_handleException(&amp;exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__getCurrentTopic">
        <xsl:param name="prefixedNamespace"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        DLRL_LS_object
        test::ccpp_test_Foo_us_getCurrentTopic (
            DLRL_LS_object lsObject)
        {
            test::Foo_abstract* typedObject;

            /* Cast the untyped object to its typed representative. */
            typedObject = DOWNCAST_test_Foo_abstract(lsObject);

            /* Return the currentTopic field. */
            return REINTERPRET_TO_DLRL_LS_OBJECT(typedObject->currentTopic);
        }
        -->
        <xsl:text>DLRL_LS_object </xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_getCurrentTopic (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsObject)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract* typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Cast the untyped object to its typed representative. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_abstract(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Return the currentTopic field. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return REINTERPRET_TO_DLRL_LS_OBJECT(typedObject->currentTopic);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__setCurrentTopic">
        <xsl:param name="prefixedNamespace"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullTopicTypeName"/>
        <xsl:param name="prefixedFullNameExceptLast"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::ccpp_test_Foo_us_setCurrentTopic (
            DLRL_LS_object lsObject,
            DLRL_LS_object lsTopic)
        {
            test::Foo_abstract* typedObject;

            /* Cast the untyped object to its typed representative. */
            typedObject = DOWNCAST_test_Foo_abstract(lsObject);

            /* Store the specified topic sample in the currentTopic field. */
            typedObject->currentTopic = reinterpret_cast<test::FooTopic*>(lsTopic);
        }
        -->
        <xsl:text>void </xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_setCurrentTopic (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsTopic)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract* typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Cast the untyped object to its typed representative. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_abstract(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Store the specified topic sample in the currentTopic field. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject->currentTopic = reinterpret_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>*>(lsTopic);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

   <xsl:template name ="callbackFunc__getPreviousTopic">
        <xsl:param name="prefixedNamespace"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        DLRL_LS_object
        test::ccpp_test_Foo_us_getPreviousTopic (
            DLRL_LS_object lsObject)
        {
            test::Foo_abstract* typedObject;

            /* Cast the untyped object to its typed representative. */
            typedObject = DOWNCAST_test_Foo_abstract(lsObject);

            /* Return the currentTopic field. */
            return REINTERPRET_TO_DLRL_LS_OBJECT(typedObject->previousTopic);
        }
        -->
        <xsl:text>DLRL_LS_object </xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_getPreviousTopic (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsObject)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract* typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Cast the untyped object to its typed representative. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_abstract(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Return the currentTopic field. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return REINTERPRET_TO_DLRL_LS_OBJECT(typedObject->previousTopic);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__setPreviousTopic">
        <xsl:param name="prefixedNamespace"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="prefixedFullTopicTypeName"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        test::ccpp_test_Foo_us_setPreviousTopic (
            DLRL_LS_object lsObject,
            DLRL_LS_object lsTopic)
        {
            test::Foo_abstract* typedObject;

            /* Cast the untyped object to its typed representative. */
            typedObject = DOWNCAST_test_Foo_abstract(lsObject);

            /* Store the specified topic sample in the currentTopic field. */
            typedObject->previousTopic = reinterpret_cast<test::FooTopic*>(lsTopic);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_setPreviousTopic (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsTopic)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract* typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Cast the untyped object to its typed representative. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_abstract(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Store the specified topic sample in the currentTopic field. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject->previousTopic = reinterpret_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>*>(lsTopic);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__createTypedObject">
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedNamespace"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        DDS::ObjectRoot_impl*
        test::ccpp_test_Foo_us_createTypedObject (
            DLRL_Exception* exception)
        {
            test::Foo_impl *typedObject;

            DLRL_INFO(INF_ENTER);

            /* Instantiate a new typed object. */
            typedObject = new test::Foo_impl();
            DLRL_VERIFY_ALLOC(typedObject, exception, "Out of resources.");

            DLRL_Exception_EXIT(exception);
            DLRL_INFO(INF_EXIT);
            return typedObject;
        }
        -->
        <xsl:text>DDS::ObjectRoot_impl*</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedObject (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:choose>
            <xsl:when test="TARGET_IMPL_CLASS">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="TARGET_IMPL_CLASS/@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/>
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
                <xsl:text>* typedObject;</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$prefixedFullName"/>
                <xsl:text>_impl* typedObject;</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Instantiate a new typed object. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject = new </xsl:text>
        <xsl:choose>
            <xsl:when test="TARGET_IMPL_CLASS">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="TARGET_IMPL_CLASS/@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/>
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
                <xsl:text>();</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$prefixedFullNameExceptLast"/>
                <xsl:text>_impl();</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>    DLRL_VERIFY_ALLOC(typedObject, exception, "Out of resources.");</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__invokeNewObjectCallback">
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'

        CORBA::Boolean
        ccpp_test_Foo_us_invokeNewObjectCallback (
            DLRL_LS_object listener,
            DLRL_LS_object newObject)
        {
            test::FooListener_ptr typedListener;
            test::Foo* typedObject;
            CORBA::Boolean result;

            DLRL_INFO(INF_ENTER);

            /* Cast the untyped object and listener to their typed representatives. */
            typedListener = DOWNCAST_test_Foo_LISTENER(listener);
            typedObject = DOWNCAST_test_Foo(newObject);

            /* Invoke the on_object_created operation. */
            result = typedListener->on_object_created(typedObject);

            DLRL_INFO(INF_EXIT);
            return result;
        }
        -->
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_invokeNewObjectCallback (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object listener,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object newObject)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Listener_ptr typedListener;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean result;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Cast the untyped object and listener to their typed representatives. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedListener = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_LISTENER(listener);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>(newObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Invoke the on_object_created operation. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    result = typedListener->on_object_created(typedObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return result;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__invokeModifiedObjectCallback">
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'

        CORBA::Boolean
        ccpp_test_Foo_us_invokeModifiedObjectCallback (
            DLRL_LS_object listener,
            DLRL_LS_object modifiedObject)
        {
            test::FooListener_ptr typedListener;
            test::Foo* typedObject;
            CORBA::Boolean result;

            DLRL_INFO(INF_ENTER);

            /* Cast the untyped object and listener to their typed representatives. */
            typedListener = DOWNCAST_test_Foo_LISTENER(listener);
            typedObject = DOWNCAST_test_Foo(modifiedObject);

            /* Invoke the on_object_modified operation. */
            result = typedListener->on_object_modified(typedObject);

            DLRL_INFO(INF_EXIT);
            return result;
        }
        -->
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_invokeModifiedObjectCallback (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object listener,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object modifiedObject)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Listener_ptr typedListener;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean result;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Cast the untyped object and listener to their typed representatives. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedListener = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_LISTENER(listener);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>(modifiedObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Invoke the on_object_modified operation. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    result = typedListener->on_object_modified(typedObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return result;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__invokeDeletedObjectCallback">
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'

        CORBA::Boolean
        ccpp_test_Foo_us_invokeDeletedObjectCallback (
            DLRL_LS_object listener,
            DLRL_LS_object deletedObject)
        {
            test::FooListener_ptr typedListener;
            test::Foo* typedObject;
            CORBA::Boolean result;

            DLRL_INFO(INF_ENTER);

            /* Cast the untyped object and listener to their typed representatives. */
            typedListener = DOWNCAST_test_Foo_LISTENER(listener);
            typedObject = DOWNCAST_test_Foo(deletedObject);

            /* Invoke the on_object_deleted operation. */
            result = typedListener->on_object_deleted(typedObject);

            DLRL_INFO(INF_EXIT);
            return result;
        }
        -->

        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_invokeDeletedObjectCallback (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object listener,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object deletedObject)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Listener_ptr typedListener;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Boolean result;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Cast the untyped object and listener to their typed representatives. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedListener = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_LISTENER(listener);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>(deletedObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Invoke the on_object_deleted operation. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    result = typedListener->on_object_deleted(typedObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return result;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__createTypedStrMap">
        <xsl:param name="prefixedNamespace"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        DLRL_LS_object
        test::ccpp_test_Foo_us_createTypedStrMap (
            DLRL_Exception* exception,
            DK_Collection* collection)
        {
            test::FooStrMap_impl *typedStrMap;

            DLRL_INFO(INF_ENTER);

            /* Instantiate a new object. */
            typedStrMap = new test::FooStrMap_impl(reinterpret_cast<DK_MapAdmin*>(collection));
            DLRL_VERIFY_ALLOC(typedStrMap, exception, "Out of resources.");

            DLRL_Exception_EXIT(exception);
            DLRL_INFO(INF_EXIT);

            return VB_UPCAST_DLRL_LS_OBJECT(typedStrMap);
        }
        -->
        <xsl:text>DLRL_LS_object</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedStrMap (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_Collection* collection)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>StrMap_impl *typedStrMap;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Instantiate a new object. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedStrMap = new </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>StrMap_impl(reinterpret_cast&lt;DK_MapAdmin*>(collection));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_ALLOC(typedStrMap, exception, "Out of resources.");</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return VB_UPCAST_DLRL_LS_OBJECT(typedStrMap);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__createTypedIntMap">
        <xsl:param name="prefixedNamespace"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        DLRL_LS_object
        test::ccpp_test_Foo_us_createTypedIntMap (
            DLRL_Exception* exception,
            DK_Collection* collection)
        {
            test::FooIntMap_impl *typedIntMap;

            DLRL_INFO(INF_ENTER);

            /* Instantiate a new object. */
            typedIntMap = new test::FooIntMap_impl(reinterpret_cast<DK_MapAdmin*>(collection));
            DLRL_VERIFY_ALLOC(typedIntMap, exception, "Out of resources.");

            DLRL_Exception_EXIT(exception);
            DLRL_INFO(INF_EXIT);

            return VB_UPCAST_DLRL_LS_OBJECT(typedIntMap);
        }
        -->
        <xsl:text>DLRL_LS_object</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedIntMap (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_Collection* collection)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>IntMap_impl *typedIntMap;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Instantiate a new object. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedIntMap = new </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>IntMap_impl(reinterpret_cast&lt;DK_MapAdmin*>(collection));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_ALLOC(typedIntMap, exception, "Out of resources.");</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return VB_UPCAST_DLRL_LS_OBJECT(typedIntMap);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__createTypedObjectSeq">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        ccpp_test_Foo_us_createTypedObjectSeq(
            DLRL_Exception* exception,
            void** arg,
            LOC_unsigned_long size)
        {
			test::FooSeq* cppTempSeq;

            DLRL_INFO(INF_ENTER);

            assert(exception);
            assert(!(*arg));

            cppTempSeq = new test::FooSeq(size);
            DLRL_VERIFY_ALLOC(cppTempSeq, exception, "Out of resources");
			cppTempSeq->length(size);
			*arg = reinterpret_cast<void*>(cppTempSeq);

            DLRL_Exception_EXIT(exception);
            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedObjectSeq(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void** arg,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long size)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
		<xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
		<xsl:text>Seq* cppTempSeq;</xsl:text>
		<xsl:value-of select="$NL"/>
		<xsl:value-of select="$NL"/>
		<xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(!(*arg));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    cppTempSeq = new </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq(size);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_ALLOC(cppTempSeq, exception, "Out of resources");</xsl:text>
        <xsl:value-of select="$NL"/>
		<xsl:text>	cppTempSeq->length(size);</xsl:text>
        <xsl:value-of select="$NL"/>
		<xsl:text>    *arg = reinterpret_cast&lt;void*>(cppTempSeq);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__addElementToTypedObjectSeq">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullName"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        ccpp_test_Foo_us_addElementToTypedObjectSeq(
            DLRL_Exception* exception,
            void* arg,
            DLRL_LS_object lsObject,
            LOC_unsigned_long count)
        {
            test:Foo* typedObject;

            DLRL_INFO(INF_ENTER);

            assert(exception);
            assert(arg);
            assert(lsObject);

            typedObject = DOWNCAST_test_Foo(lsObject);
            typedObject->_add_ref();
            (*(reinterpret_cast<test::FooSeq*>(arg)))[count] = typedObject;

            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_addElementToTypedObjectSeq(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void* arg,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long count)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(arg);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedObject->_add_ref();</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    (*(reinterpret_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*>(arg)))[count] = typedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__createTypedSelectionSeq">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        ccpp_test_Foo_us_createTypedSelectionSeq(
            DLRL_Exception* exception,
            void** arg,
            LOC_unsigned_long size)
        {
			test::FooSelectionSeq* cppTempSeq;

            DLRL_INFO(INF_ENTER);

            assert(exception);
            assert(!(*arg));

            cppTempSeq = new test::FooSelectionSeq(size);
            DLRL_VERIFY_ALLOC(cppTempSeq, exception, "Out of resources");
			cppTempSeq->length(size);
			*arg = reinterpret_cast<void*>(cppTempSeq);

            DLRL_Exception_EXIT(exception);
            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedSelectionSeq(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void** arg,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long size)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
		<xsl:text>    </xsl:text>
		<xsl:value-of select="$prefixedFullNameExceptLast"/>
		<xsl:text>SelectionSeq* cppTempSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(!(*arg));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    cppTempSeq = new </xsl:text>
		<xsl:value-of select="$prefixedFullNameExceptLast"/>
		<xsl:text>SelectionSeq(size);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_ALLOC(cppTempSeq, exception, "Out of resources");</xsl:text>
        <xsl:value-of select="$NL"/>
		<xsl:text>    cppTempSeq->length(size);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    *arg = reinterpret_cast&lt;void*>(cppTempSeq);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__addElementToTypedSelectionSeq">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        ccpp_test_Foo_us_addElementToTypedSelectionSeq(
            DLRL_Exception* exception,
            void* arg,
            DLRL_LS_object lsObject,
            LOC_unsigned_long count)
        {
            test::FooSelection* typedSelection;

            DLRL_INFO(INF_ENTER);

            assert(exception);
            assert(arg);
            assert(lsObject);

            typedSelection = DOWNCAST_test_Foo_SELECTION(lsObject);
            CORBA::Object::_duplicate(typedSelection);
            (*(reinterpret_cast<test::FooSelectionSeq*>(arg)))[count] = typedSelection;

            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_addElementToTypedSelectionSeq(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void* arg,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long count)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Selection* typedSelection;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(arg);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedSelection = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_SELECTION(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Object::_duplicate(typedSelection);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    (*(reinterpret_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionSeq*>(arg)))[count] = typedSelection;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__createTypedListenerSeq">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        ccpp_test_Foo_us_createTypedListenerSeq(
            DLRL_Exception* exception,
            void** arg,
            LOC_unsigned_long size)
        {
			test::FooListenerSeq* cppTempSeq;

            DLRL_INFO(INF_ENTER);

            assert(exception);
            assert(!(*arg));

            cppTempSeq = new test::FooListenerSeq(size);
            DLRL_VERIFY_ALLOC(cppTempSeq, exception, "Out of resources");
			cppTempSeq->length(size);
			*arg = reinterpret_cast<void*>(cppTempSeq);

            DLRL_Exception_EXIT(exception);
            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedListenerSeq(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void** arg,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long size)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
		<xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
		<xsl:text>ListenerSeq* cppTempSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(!(*arg));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    cppTempSeq = new </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
		<xsl:text>ListenerSeq(size);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_ALLOC(cppTempSeq, exception, "Out of resources");</xsl:text>
        <xsl:value-of select="$NL"/>
		<xsl:text>    cppTempSeq->length(size);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    *arg = reinterpret_cast&lt;void*>(cppTempSeq);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="callbackFunc__changeRelations">
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedNamespace"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test' which has one relation
            of type 'test::Government' and name 'myGov':
            void
            test::ccpp_test_Foo_us_changeRelations(
                DLRL_LS_object ownerObject,
                DLRL_LS_object relationObject,
                LOC_unsigned_long index)
            {
                test::Foo_abstract* typedOwnerObject;

                DLRL_INFO(INF_ENTER);

                assert(ownerObject);
                /* relationObject may be null */

                typedOwnerObject = dynamic_cast<test::Foo_abstract*>(reinterpret_cast<CORBA::ValueBase*>(ownerObject));

                switch (index)
                {
                case 0:
                    {
                        if(typedOwnerObject->myGov)
                        {
                           typedOwnerObject->myGov->_remove_ref();
                           typedOwnerObject->myGov = NULL;
                        }
                        if(relationObject)
                        {
                            test::Government* typedRelationObject;

                            typedRelationObject = dynamic_cast<test::Government*>(reinterpret_cast<CORBA::ValueBase*>(relationObject));
                            typedRelationObject->_add_ref();
                            typedOwnerObject->myGov = typedRelationObject;
                        }
                    }
                    break;
                default:
                    assert(FALSE);
                }

                DLRL_INFO(INF_EXIT);
            }

        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_changeRelations(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object ownerObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object relationObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long index)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract* typedOwnerObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(ownerObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* relationObject may be null */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedOwnerObject = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract*>(reinterpret_cast&lt;</xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::ValueBase*>(ownerObject));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    switch (index)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:for-each select="STATEMEMBER">
            <xsl:if test="keyDescription and not(multiPlaceTopic)">
                <xsl:variable name="index" select="TYPEREF/@INDEX"/>
                <xsl:variable name="prefixedAttributeName">
                    <xsl:call-template name="ccpp-name">
                        <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="prefixedAttributeType">
                    <xsl:variable name="idlType">
                        <xsl:call-template name="resolveStatememberIdlType"/>
                    </xsl:variable>
                    <xsl:call-template name="string-search-replace">
                        <xsl:with-param name="text" select="$idlType"/>
                        <xsl:with-param name="from" select="'::'"/>
                        <xsl:with-param name="to" select="'::'"/>
                        <xsl:with-param name="prefixKeywords" select="'yes'"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:text>    case </xsl:text>
                <xsl:value-of select="$index"/>
                <xsl:text>:</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        {</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>            if(typedOwnerObject-></xsl:text>
                <xsl:value-of select="$prefixedAttributeName"/>
                <xsl:text>)</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>            {</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>               typedOwnerObject-></xsl:text>
                <xsl:value-of select="$prefixedAttributeName"/>
                <xsl:text>->_remove_ref();</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>               typedOwnerObject-></xsl:text>
                <xsl:value-of select="$prefixedAttributeName"/>
                <xsl:text> = NULL;</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>            }</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>            if(relationObject)</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>            {</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>                </xsl:text>
                <xsl:value-of select="$prefixedAttributeType"/>
                <xsl:text>* typedRelationObject;</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>                typedRelationObject = dynamic_cast&lt;</xsl:text>
                <xsl:value-of select="$prefixedAttributeType"/>
                <xsl:text>*>(reinterpret_cast&lt;</xsl:text>
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::ValueBase*>(relationObject));</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>                typedRelationObject->_add_ref();</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>                typedOwnerObject-></xsl:text>
                <xsl:value-of select="$prefixedAttributeName"/>
                <xsl:text> = typedRelationObject;</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>            }</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>        }</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        break;</xsl:text>
            </xsl:if>
        </xsl:for-each>
        <xsl:text>    default: </xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        assert(FALSE);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="callbackFunc__clearLSObjectAdministration">
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedNamespace"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test' which has one collection
            of type 'test::Government' and name 'myGovs' and also one relation with name myGov:
            void
            test::ccpp_test_Foo_us_clearLSObjectAdministration(
                DLRL_LS_object ls_object)
            {
                test::Foo_abstract* cppObject;

                DLRL_INFO(INF_ENTER);

                cppObject = dynamic_cast<test::Foo_abstract*>(reinterpret_cast<DDS::ValueBase*>(ls_object));

                if(cppObject->myGov)
                {
                    cppObject->myGov->_remove_ref();
                    cppObject->myGov = NULL;
                }
                if(cppObject->myGovs)
                {
                    cppObject->myGovs->_remove_ref();
                    cppObject->myGovs = NULL;
                }

                DLRL_INFO(INF_EXIT);
            }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_clearLSObjectAdministration(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object ls_object)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract* cppObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    cppObject = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract*>(reinterpret_cast&lt;DDS::ValueBase*>(ls_object));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:for-each select="STATEMEMBER">
            <xsl:if test="multiPlaceTopic or keyDescription">
                <xsl:variable name="prefixedAttributeName">
                    <xsl:call-template name="ccpp-name">
                        <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:text>    if(cppObject-></xsl:text>
                <xsl:value-of select="$prefixedAttributeName"/>
                <xsl:text>)</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    {</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        cppObject-></xsl:text>
                <xsl:value-of select="$prefixedAttributeName"/>
                <xsl:text>->_remove_ref();</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>        cppObject-></xsl:text>
                <xsl:value-of select="$prefixedAttributeName"/>
                <xsl:text> = NULL;</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>    }</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:if>
        </xsl:for-each>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="callbackFunc__checkObjectForSelection">
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="prefixedNamespace"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            LOC_boolean
            test::ccpp_test_Foo_us_checkObjectForSelection(
                DLRL_Exception* exception,
                DLRL_LS_object filter,
                DLRL_LS_object objectAdmin)
            {
                LOC_boolean retVal = FALSE;
                test::FooFilter_ptr ccpp_filter;
                test::Foo* ccpp_objectAdmin;

                DLRL_INFO(INF_ENTER);

                assert(exception);
                assert(filter);
                assert(objectAdmin);

                ccpp_filter = dynamic_cast&lt;test::FooFilter_ptr>(reinterpret_cast&lt;DDS::Object_ptr>(filter));
                ccpp_objectAdmin = dynamic_cast&lt;test::Foo*>(reinterpret_cast&lt;DDS::ValueBase*>(objectAdmin));

                retVal = static_cast&lt;LOC_boolean>(ccpp_filter->check_object(ccpp_objectAdmin, DDS::UNDEFINED_MEMBERSHIP));

                DLRL_Exception_EXIT(exception);
                DLRL_INFO(INF_EXIT);
                return retVal;
            }
        -->
        <xsl:text>LOC_boolean</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_checkObjectForSelection(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object filter,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object objectAdmin)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_boolean retVal = FALSE;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Filter_ptr ccpp_filter;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* ccpp_objectAdmin;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(filter);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(objectAdmin);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_filter = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Filter_ptr>(reinterpret_cast&lt;DDS::Object_ptr>(filter));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_objectAdmin = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*>(reinterpret_cast&lt;DDS::ValueBase*>(objectAdmin));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    retVal = static_cast&lt;LOC_boolean>(ccpp_filter->check_object(ccpp_objectAdmin, DDS::UNDEFINED_MEMBERSHIP));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return retVal;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="callbackFunc__selectionListenerCallOperations">
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="prefixedNamespace"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
            void
            test::ccpp_test_Foo_us_triggerListenerInsertedObject(
                DLRL_Exception* exception,
                DLRL_LS_object listener,
                DLRL_LS_object objectAdmin)
            {
                test::FooSelectionListener* ccpp_listener;
                test::Foo* ccpp_objectAdmin;

                DLRL_INFO(INF_ENTER);

                assert(exception);
                assert(listener);
                assert(objectAdmin);

                ccpp_listener = dynamic_cast&lt;test::FooSelectionListener*>(reinterpret_cast&lt;DDS::Object_ptr>(listener));
                ccpp_objectAdmin = dynamic_cast&lt;test::Foo*>(reinterpret_cast&lt;DDS::ValueBase*>(objectAdmin));

                ccpp_listener->on_object_in(ccpp_objectAdmin);

                DLRL_Exception_EXIT(exception);
                DLRL_INFO(INF_EXIT);
            }

            void
            test::ccpp_test_Foo_us_triggerListenerModifiedObject(
                DLRL_Exception* exception,
                DLRL_LS_object listener,
                DLRL_LS_object objectAdmin)
            {
                test::FooSelectionListener* ccpp_listener;
                test::Foo* ccpp_objectAdmin;

                DLRL_INFO(INF_ENTER);

                assert(exception);
                assert(listener);
                assert(objectAdmin);

                ccpp_listener = dynamic_cast&lt;test::FooSelectionListener*>(reinterpret_cast&lt;DDS::Object_ptr>(listener));
                ccpp_objectAdmin = dynamic_cast&lt;test::Foo*>(reinterpret_cast&lt;DDS::ValueBase*>(objectAdmin));

                ccpp_listener->on_object_modified(ccpp_objectAdmin);

                DLRL_Exception_EXIT(exception);
                DLRL_INFO(INF_EXIT);
            }

            void
            test::ccpp_test_Foo_us_triggerListenerRemovedObject(
                DLRL_Exception* exception,
                DLRL_LS_object listener,
                DLRL_LS_object objectAdmin)
            {
                test::FooSelectionListener* ccpp_listener;
                test::Foo* ccpp_objectAdmin;

                DLRL_INFO(INF_ENTER);

                assert(exception);
                assert(listener);
                assert(objectAdmin);

                ccpp_listener = dynamic_cast&lt;test::FooSelectionListener*>(reinterpret_cast&lt;DDS::Object_ptr>(listener));
                ccpp_objectAdmin = dynamic_cast&lt;test::Foo*>(reinterpret_cast&lt;DDS::ValueBase*>(objectAdmin));

                ccpp_listener->on_object_out(ccpp_objectAdmin);

                DLRL_Exception_EXIT(exception);
                DLRL_INFO(INF_EXIT);
            }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_triggerListenerInsertedObject(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object listener,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object objectAdmin)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr ccpp_listener;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* ccpp_objectAdmin;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(listener);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(objectAdmin);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_listener = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr>(reinterpret_cast&lt;DDS::Object_ptr>(listener));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_objectAdmin = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*>(reinterpret_cast&lt;DDS::ValueBase*>(objectAdmin));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_listener->on_object_in(ccpp_objectAdmin);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_triggerListenerModifiedObject(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object listener,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object objectAdmin)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr ccpp_listener;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* ccpp_objectAdmin;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(listener);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(objectAdmin);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_listener = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr>(reinterpret_cast&lt;DDS::Object_ptr>(listener));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_objectAdmin = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*>(reinterpret_cast&lt;DDS::ValueBase*>(objectAdmin));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_listener->on_object_modified(ccpp_objectAdmin);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_triggerListenerRemovedObject(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object listener,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object objectAdmin)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr ccpp_listener;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* ccpp_objectAdmin;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(listener);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(objectAdmin);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_listener = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>SelectionListener_ptr>(reinterpret_cast&lt;DDS::Object_ptr>(listener));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_objectAdmin = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*>(reinterpret_cast&lt;DDS::ValueBase*>(objectAdmin));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_listener->on_object_out(ccpp_objectAdmin);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="callbackFunc__setCollections">
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="prefixedNamespace"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test' which has one collection
            of type 'test::Government' and name 'myGovs':
            void
            test::ccpp_test_Foo_us_setCollections(
                DLRL_LS_object ownerObject,
                DLRL_LS_object collectionObject,
                LOC_unsigned_long index)
            {
                test::Foo_abstract* typedOwnerObject;

                DLRL_INFO(INF_ENTER);

                assert(ownerObject);
                assert(collectionObject);

                typedOwnerObject = dynamic_cast<test::Foo_abstract*>(reinterpret_cast<CORBA::ValueBase*>(ownerObject));

                switch (index)
                {
                case 0:
                    {
                            test::GovernmentStrMap* typedCollectionObject;

                            assert(!(typedOwnerObject->myGovs));/* only set once, thus first time it should be a null pointer */
                            typedCollectionObject = dynamic_cast<test::GovernmentStrMap*>(reinterpret_cast<CORBA::ValueBase*>(collectionObject));
                            typedCollectionObject->_add_ref();
                            typedOwnerObject->myGovs = typedCollectionObject;
                        }
                    }
                    break;
                default:
                    assert(FALSE);
                }

                DLRL_INFO(INF_EXIT);
            }

        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_setCollections(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object ownerObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object collectionObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long index)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract* typedOwnerObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(ownerObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(collectionObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedOwnerObject = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>_abstract*>(reinterpret_cast&lt;</xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::ValueBase*>(ownerObject));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    switch (index)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:for-each select="STATEMEMBER">
            <xsl:if test="multiPlaceTopic">
                <xsl:variable name="index" select="TYPEREF/@INDEX"/>
                <xsl:variable name="prefixedAttributeName">
                    <xsl:call-template name="ccpp-name">
                        <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="type">
                    <xsl:call-template name="resolveStatememberIdlType"/>
                </xsl:variable>

                <xsl:for-each select="//VALUEFORWARDDEF[@NAME=$type]">
                    <xsl:variable name="prefixedAttributeType">
                        <xsl:call-template name="string-search-replace">
                            <xsl:with-param name="text">
                                <xsl:value-of select="@itemType"/>
                                <xsl:value-of select="@pattern"/>
                            </xsl:with-param>
                            <xsl:with-param name="from" select="'::'"/>
                            <xsl:with-param name="to" select="'::'"/>
                            <xsl:with-param name="prefixKeywords" select="'yes'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>    case </xsl:text>
                    <xsl:value-of select="$index"/>
                    <xsl:text>:</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        {</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>            </xsl:text>
                    <xsl:value-of select="$prefixedAttributeType"/>
                    <xsl:text>* typedCollectionObject;</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>            assert(!(typedOwnerObject-></xsl:text>
                    <xsl:value-of select="$prefixedAttributeName"/>
                    <xsl:text>));/* only set once, thus first time it should be a null pointer */</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>            typedCollectionObject = dynamic_cast&lt;</xsl:text>
                    <xsl:value-of select="$prefixedAttributeType"/>
                    <xsl:text>*>(reinterpret_cast&lt;</xsl:text>
                    <xsl:call-template name="get_corba_module_name"/>
                    <xsl:text>::ValueBase*>(collectionObject));</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>            typedCollectionObject->_add_ref();</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>            typedOwnerObject-></xsl:text>
                    <xsl:value-of select="$prefixedAttributeName"/>
                    <xsl:text> = typedCollectionObject;</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        }</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>        break;</xsl:text>
                    <xsl:value-of select="$NL"/>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
        <xsl:text>    default: </xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        assert(FALSE);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__addElementToTypedListenerSeq">
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="fullNameWithUnderscores"/>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        ccpp_test_Foo_us_addElementToTypedListenerSeq(
            DLRL_Exception* exception,
            void* arg,
            DLRL_LS_object lsObject,
            LOC_unsigned_long count)
        {
            test::FooListener* typedListener;

            DLRL_INFO(INF_ENTER);

            assert(exception);
            assert(arg);
            assert(lsObject);

            typedListener = DOWNCAST_test_Foo_LISTENER(lsObject);
            CORBA::Object::_duplicate(typedListener);
            (*(reinterpret_cast<test::FooListenerSeq*>(arg)))[count] = typedListener;

            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_addElementToTypedListenerSeq(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void* arg,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    LOC_unsigned_long count)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Listener* typedListener;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(arg);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedListener = DOWNCAST_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_LISTENER(lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Object::_duplicate(typedListener);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    (*(reinterpret_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>ListenerSeq*>(arg)))[count] = typedListener;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__createTypedSet">
        <xsl:param name="prefixedNamespace"/>
        <xsl:param name="prefixedFullNameExceptLast"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        DLRL_LS_object
        test::ccpp_test_Foo_us_createTypedSet (
            DLRL_Exception* exception,
            DK_Collection* collection)
        {
            test::FooSet_impl *typedSet;

            DLRL_INFO(INF_ENTER);

            /* Instantiate a new object. */
            typedSet = new test::FooSet_impl(reinterpret_cast<DK_SetAdmin*>(collection));
            DLRL_VERIFY_ALLOC(typedSet, exception, "Out of resources.");

            DLRL_Exception_EXIT(exception);
            DLRL_INFO(INF_EXIT);

            return VB_UPCAST_DLRL_LS_OBJECT(typedSet);
        }
        -->

        <xsl:text>DLRL_LS_object</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedSet (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DK_Collection* collection)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl *typedSet;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Instantiate a new object. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedSet = new </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Set_impl(reinterpret_cast&lt;DK_SetAdmin*>(collection));</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_ALLOC(typedSet, exception, "Out of resources.");</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return VB_UPCAST_DLRL_LS_OBJECT(typedSet);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__createTypedTopic">
        <xsl:param name="fullTopicTypeNameWithUnderscores"/>
        <xsl:param name="prefixedFullTopicTypeName"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        DLRL_LS_object
        ccpp_test_FooTopic_us_createTypedTopic (
            DLRL_Exception* exception)
        {
            test::FooTopic* typedTopic;

            DLRL_INFO(INF_ENTER);

            /* Instantiate a new object. */
            typedTopic = new test::FooTopic();
            DLRL_VERIFY_ALLOC(typedTopic, exception, "Out of resources.");

            /* init all field used for validity field types to 0. */
            typedTopic->x = 0;

            DLRL_Exception_EXIT(exception);
            DLRL_INFO(INF_EXIT);

            return REINTERPRET_TO_DLRL_LS_OBJECT(typedTopic);
        }
        -->
        <xsl:text>DLRL_LS_object</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>_us_createTypedTopic (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception* exception)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>* typedTopic;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Instantiate a new object. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedTopic = new </xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>();</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_VERIFY_ALLOC(typedTopic, exception, "Out of resources.");</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* init all field used for validity field types to 0. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:for-each select="STATEMEMBER/validityField">
            <xsl:text>    typedTopic-></xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text> = </xsl:text>
            <xsl:value-of select="@invalidValue"/>
            <xsl:text>;</xsl:text>
            <xsl:value-of select="$NL"/>
        </xsl:for-each>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_Exception_EXIT(exception);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    return REINTERPRET_TO_DLRL_LS_OBJECT(typedTopic);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__setTopicClassName">
        <xsl:param name="fullTopicTypeNameWithUnderscores"/>
        <xsl:param name="prefixedFullTopicTypeName"/>
        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        ccpp_test_FooTopic_us_setTopicClassName (
            DLRL_LS_object lsTopic,
            const char *typeName)
        {
            test::FooTopic* typedTopic;

            DLRL_INFO(INF_ENTER);

            /* Cast the topic to its appropriate type. */
            typedTopic = reinterpret_cast<test::FooTopic*>(lsTopic);

            /* set the attribute that holds the type  name.         */
            /* The previous value will be released automatically ,  */
            /* since the IDL mapping of a string in a struct maps   */
            /* onto a 'smart-pointer', that manages itself.         */
            typedTopic->typeName = CORBA::string_dup(typeName);

            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>_us_setTopicClassName (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsTopic,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    const char *typeName)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>* typedTopic;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Cast the topic to its appropriate type. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedTopic = reinterpret_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>*>(lsTopic);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* set the attribute that holds the type name.          */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* The previous value will be released automatically ,  */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* since the IDL mapping of a string in a struct maps   */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* onto a 'smart-pointer', that manages itself.         */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedTopic-></xsl:text>
        <xsl:value-of select="mainTopic/keyDescription/keyField[position()=1]"/>
        <xsl:text> = </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::string_dup(typeName);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__setTopicOidField">
        <xsl:param name="fullTopicTypeNameWithUnderscores"/>
        <xsl:param name="prefixedFullTopicTypeName"/>

        <xsl:variable name="oidFieldName">
            <xsl:call-template name="string-search-replace-except-last">
                <xsl:with-param name="text" select="mainTopic/keyDescription/keyField[position()=2]"/>
                <xsl:with-param name="from" select="'.'"/>
                <xsl:with-param name="to" select="'.'"/><!-- i.e. dont replace, we just want to strip the last keyword -->
                <xsl:with-param name="prefixKeywords" select="'no'"/>
            </xsl:call-template>
        </xsl:variable>

        <!--The following code is generated by this template for example type 'Foo' defined in module 'test'
        void
        ccpp_test_FooTopic_us_setTopicOidField (
            DLRL_LS_object lsTopic,
            const DDS::DLRLOid& oid)
        {
            test::FooTopic* typedTopic;

            DLRL_INFO(INF_ENTER);

            /* Cast the topic to its appropriate type. */
            typedTopic = reinterpret_cast<test::FooTopic*>(lsTopic);

            /* overwrite the attribute that holds the oid. */
            typedTopic->oid = oid;

            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>_us_setTopicOidField (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_LS_object lsTopic,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    const DDS::DLRLOid&amp; oid)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>* typedTopic</xsl:text>
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* Cast the topic to its appropriate type. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedTopic = reinterpret_cast&lt;</xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>*>(lsTopic);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    /* overwrite the attribute that holds the oid. */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    typedTopic-></xsl:text>
        <xsl:value-of select="$oidFieldName"/>
        <xsl:text> = oid;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__typedTopicCopyInExternDecl">
        <xsl:param name="fullTopicTypeName"/>
        <xsl:param name="fullTopicTypeNameWithUnderscores"/>

        <!--The following code is generated by this template for example type 'FooTopic' defined in module 'test'
        extern c_bool
        __test_FooTopic__copyIn(
            c_base base,
            struct test::FooTopic *from,
            struct _test_FooTopic *to);
        -->
        <xsl:text>extern c_bool</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>__</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>__copyIn(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    c_base base,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    struct </xsl:text>
        <xsl:value-of select="$fullTopicTypeName"/>
        <xsl:text> *from,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    struct _</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text> *to);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__typedTopicCopyOutExternDecl">
        <xsl:param name="fullTopicTypeNameWithUnderscores"/>

        <!--The following code is generated by this template for example type 'FooTopic' defined in module 'test'
        extern void
        __test_FooTopic__copyOut(
            void *_from,
            void *_to);
        -->
        <xsl:text>extern void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>__</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>__copyOut(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void *_from,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    void *_to);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="callbackFunc__typedTopicInitializeTopicCache">
        <xsl:param name="fullTopicTypeNameWithUnderscores"/>

        <!--The following code is generated by this template for example type 'FooTopic' defined in module 'test'
            void
            ccpp_test_FooTopic_us_initializeTopicCache(
                ccpp_TypedTopicCache* topicCache)
            {
                DLRL_INFO(INF_ENTER);

                assert(topicCache);

                topicCache->createTypedTopic = ccpp_test_FooTopic_us_createTypedTopic;
                topicCache->setTopicClassName = ccpp_test_FooTopic_us_setTopicClassName;
                topicCache->setTopicOidField = ccpp_test_FooTopic_us_setTopicOidField;
                topicCache->copyIn = (gapi_copyIn)__test_FooTopic__copyIn;
                topicCache->copyOut = (gapi_copyOut)__test_FooTopic__copyOut;

                DLRL_INFO(INF_EXIT);
            }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>_us_initializeTopicCache(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_TypedTopicCache* topicCache)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(topicCache);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    topicCache->createTypedTopic = ccpp_</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>_us_createTypedTopic;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:choose>
            <xsl:when test="mainTopic/keyDescription/@content='SimpleOid' or mainTopic/keyDescription/@content='FullOid'">
                <xsl:text>    topicCache->setTopicOidField = ccpp_</xsl:text>
                <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
                <xsl:text>_us_setTopicOidField;</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>    topicCache->setTopicOidField = NULL;</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:choose>
            <xsl:when test="mainTopic/keyDescription/@content='FullOid'">
                <xsl:text>    topicCache->setTopicClassName = ccpp_</xsl:text>
                <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
                <xsl:text>_us_setTopicClassName;</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>    topicCache->setTopicClassName = NULL;</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>    topicCache->copyIn = (gapi_copyIn)__</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>__copyIn;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    topicCache->copyOut = (gapi_copyOut)__</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>__copyOut;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name ="typedClass_cacheInitializer">
        <xsl:param name="prefixedNamespace"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="fullTopicTypeNameWithUnderscores"/>

        <!--The following code is generated by this template for example type 'FooTopic' defined in module 'test'
        void
        ccpp_test_Foo_us_initializeObjectCache(
            ccpp_TypedObjectCache* objectCache)
        {
            DLRL_INFO(INF_ENTER);

            assert(objectCache);

            objectCache->initializeTopicCache = ccpp_test_FooTopic_us_initializeTopicCache;
            objectCache->getCurrentTopic = test::ccpp_test_Foo_us_getCurrentTopic;
            objectCache->setCurrentTopic = test::ccpp_test_Foo_us_setCurrentTopic;
            objectCache->getPreviousTopic = test::ccpp_test_Foo_us_getPreviousTopic;
            objectCache->setPreviousTopic = test::ccpp_test_Foo_us_setPreviousTopic;
            objectCache->createTypedObject = ccpp_test_Foo_us_createTypedObject;
            objectCache->invokeNewObjectCallback = ccpp_test_Foo_us_invokeNewObjectCallback;
            objectCache->invokeModifiedObjectCallback = ccpp_test_Foo_us_invokeModifiedObjectCallback;
            objectCache->invokeDeletedObjectCallback = ccpp_test_Foo_us_invokeDeletedObjectCallback;
            objectCache->createTypedStrMap = test::ccpp_test_Foo_us_createTypedStrMap;
            objectCache->createTypedIntMap = test::ccpp_test_Foo_us_createTypedIntMap;
            objectCache->createTypedSet = test::ccpp_test_Foo_us_createTypedSet;
            objectCache->createTypedObjectSeq = ccpp_test_Foo_us_createTypedObjectSeq;
            objectCache->addElementToTypedObjectSeq = ccpp_test_Foo_us_addElementToTypedObjectSeq;
            objectCache->createTypedSelectionSeq = ccpp_test_Foo_us_createTypedSelectionSeq;
            objectCache->addElementToTypedSelectionSeq = ccpp_test_Foo_us_addElementToTypedSelectionSeq;
            objectCache->createTypedListenerSeq = ccpp_test_Foo_us_createTypedListenerSeq;
            objectCache->addElementToTypedListenerSeq = ccpp_test_Foo_us_addElementToTypedListenerSeq;
            objectCache->changeRelations = test::ccpp_test_Foo_us_changeRelations;
            objectCache->setCollections = test::ccpp_test_Foo_us_setCollections;
            objectCache->clearLSObjectAdministration = test::ccpp_test_Foo_us_clearLSObjectAdministration;
            objectCache->checkObjectForSelection = test::ccpp_test_Foo_us_checkObjectForSelection;
            objectCache->triggerListenerInsertedObject = test::ccpp_test_Foo_us_triggerListenerInsertedObject;
            objectCache->triggerListenerModifiedObject = test::ccpp_test_Foo_us_triggerListenerModifiedObject;
            objectCache->triggerListenerRemovedObject = test::ccpp_test_Foo_us_triggerListenerRemovedObject;

            DLRL_INFO(INF_EXIT);
        }
        -->
        <xsl:text>void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_initializeObjectCache(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    ccpp_TypedObjectCache* objectCache)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_ENTER);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    assert(objectCache);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->initializeTopicCache = ccpp_</xsl:text>
        <xsl:value-of select="$fullTopicTypeNameWithUnderscores"/>
        <xsl:text>_us_initializeTopicCache;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->getCurrentTopic = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_getCurrentTopic;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->setCurrentTopic = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_setCurrentTopic;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->getPreviousTopic = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_getPreviousTopic;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->setPreviousTopic = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_setPreviousTopic;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->createTypedObject = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->invokeNewObjectCallback = ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_invokeNewObjectCallback;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->invokeModifiedObjectCallback = ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_invokeModifiedObjectCallback;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->invokeDeletedObjectCallback = ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_invokeDeletedObjectCallback;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->createTypedStrMap = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedStrMap;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->createTypedIntMap = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedIntMap;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->createTypedSet = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedSet;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->createTypedObjectSeq = ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedObjectSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->addElementToTypedObjectSeq = ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_addElementToTypedObjectSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->createTypedSelectionSeq = ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedSelectionSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->addElementToTypedSelectionSeq = ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_addElementToTypedSelectionSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->createTypedListenerSeq = ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_createTypedListenerSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->addElementToTypedListenerSeq = ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_addElementToTypedListenerSeq;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->changeRelations = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_changeRelations;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->setCollections = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_setCollections;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->clearLSObjectAdministration = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_clearLSObjectAdministration;</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:text>    objectCache->checkObjectForSelection = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_checkObjectForSelection;</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:text>    objectCache->triggerListenerInsertedObject = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_triggerListenerInsertedObject;</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:text>    objectCache->triggerListenerModifiedObject = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_triggerListenerModifiedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    objectCache->triggerListenerRemovedObject = </xsl:text>
        <xsl:if test="string-length($prefixedNamespace)!=0">
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::</xsl:text>
        </xsl:if>
        <xsl:text>ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_triggerListenerRemovedObject;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>    DLRL_INFO(INF_EXIT);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

</xsl:stylesheet>
