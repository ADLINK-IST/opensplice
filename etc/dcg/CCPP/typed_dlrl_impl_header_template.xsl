<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xalan="http://xml.apache.org/xalan"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
    <xsl:param name="output.dir" select="'.'"/>
    <xsl:param name="import.macro" select="'.'"/>
    <xsl:include href="common_dlrl.xsl"/>

    <xsl:template match="IDL">
        <xsl:variable name="filename">
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="@baseFile"/>
            <xsl:text>Dlrl_impl.h</xsl:text>
        </xsl:variable>
        <redirect:write file="{$output.dir}/{$filename}">
            <FOODLRL-out>
                <xsl:call-template name="copyright-notice"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#ifndef CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_DLRL_IMPL_H</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#define CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_DLRL_IMPL_H</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "DLRL_Kernel.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_ObjectHome_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_ObjectRoot_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_Set_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_List_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_StrMap_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_IntMap_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_CacheAccess_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_Cache_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_Selection_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_FilterCriterion_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:for-each select="VALUEDEF">
                    <xsl:call-template name="VALUEDEF">
                        <xsl:with-param name="whiteSpace"/>
                    </xsl:call-template>
                </xsl:for-each>
                <xsl:for-each select="MODULE">
                    <xsl:call-template name="MODULE">
                        <xsl:with-param name="whiteSpace"/>
                    </xsl:call-template>
                </xsl:for-each>
                <xsl:value-of select="$NL"/>
                <xsl:text>#endif /* CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_DLRL_IMPL_H */</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
            </FOODLRL-out>
        </redirect:write>
    </xsl:template>

    <xsl:template name="MODULE">
        <xsl:param name="whiteSpace"/>

        <xsl:variable name="prefixedName">
            <xsl:call-template name="ccpp-name">
                <xsl:with-param name="name" select="@NAME"/>
            </xsl:call-template>
        </xsl:variable>
        <!-- must always generate the name spaces, even if the module doesn't contain any dlrl valuetypes
         because inner modules may contain dlrl valuetypes... we can make the xslt more intelligent here, but
         thats a TODO for now. I let 'old' code remain because it used to be that no namespace was generate and
         no further module traversion was done if a module was encountered without dlrl valuetypes, which is
         obviously wrong as child modules nodes are not checked! -->
        <!--xsl:variable name="contains-dlrl-valuetypes">
            <xsl:for-each select="VALUEDEF">
                <xsl:variable name="isNonIncludedSharedValueDef">
                    <xsl:call-template name="does-valuedef-inherit-from-objectroot-and-is-not-included"/>
                </xsl:variable>
                <xsl:if test="$isNonIncludedSharedValueDef='true'">
                    <xsl:text>true</xsl:text>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>

        <xsl:if test="string-length($contains-dlrl-valuetypes)!=0"-->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>namespace </xsl:text>
            <xsl:value-of select="$prefixedName"/>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>{</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:for-each select="VALUEDEF">
                <xsl:call-template name="VALUEDEF">
                    <xsl:with-param name="whiteSpace">
                        <xsl:text>    </xsl:text>
                        <xsl:value-of select="$whiteSpace"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:for-each>
            <xsl:for-each select="MODULE">
                <xsl:call-template name="MODULE">
                    <xsl:with-param name="whiteSpace">
                        <xsl:text>    </xsl:text>
                        <xsl:value-of select="$whiteSpace"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:for-each>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
        <!--/xsl:if-->
    </xsl:template>

    <xsl:template name="VALUEDEF">
        <xsl:param name="whiteSpace"/>

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
            <xsl:variable name="nonPrefixedName">
                <xsl:call-template name="string-return-text-after-last-token">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="token" select="'::'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="prefixedFullNameExceptLast">
                <xsl:call-template name="string-search-replace-except-last">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
                <xsl:text>::</xsl:text>
                <xsl:value-of select="$nonPrefixedName"/>
            </xsl:variable>
            <xsl:variable name="fullNameWithUnderscores">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'_'"/>
                    <xsl:with-param name="prefixKeywords" select="'no'"/>
                </xsl:call-template>
            </xsl:variable>


            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>/****** Generated for type: </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> ******/</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
            void
            ccpp_CountrySimDLRL_Country_us_setPreviousTopic (
                DLRL_LS_object lsObject,
                DLRL_LS_object lsTopic);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>void</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_setPreviousTopic (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object lsObject,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object lsTopic);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
            DLRL_LS_object
            ccpp_CountrySimDLRL_Country_us_getPreviousTopic (
                DLRL_LS_object lsObject);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>DLRL_LS_object</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_getPreviousTopic (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object lsObject);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
            void
            ccpp_CountrySimDLRL_Country_us_setCurrentTopic (
                DLRL_LS_object lsObject,
                DLRL_LS_object lsTopic);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>void</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_setCurrentTopic (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object lsObject,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object lsTopic);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
            DLRL_LS_object
            ccpp_CountrySimDLRL_Country_us_getCurrentTopic (
                DLRL_LS_object lsObject);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>DLRL_LS_object</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_getCurrentTopic (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object lsObject);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
                    DDS::ObjectRoot_impl*
                    ccpp_test_Foo_us_createTypedObject (
                        DLRL_Exception* exception);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>DDS::ObjectRoot_impl*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_createTypedObject (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_Exception* exception);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
                    DLRL_LS_object
                    ccpp_test_Foo_us_createTypedSet (
                        DLRL_Exception* exception,
                        DK_Collection* collection);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>DLRL_LS_object</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_createTypedSet (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DK_Collection* collection);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
                    DLRL_LS_object
                    ccpp_test_Foo_us_createTypedStrMap (
                        DLRL_Exception* exception,
                        DK_Collection* collection);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>DLRL_LS_object</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_createTypedStrMap (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DK_Collection* collection);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
                    DLRL_LS_object
                    ccpp_test_Foo_us_createTypedIntMap (
                        DLRL_Exception* exception,
                        DK_Collection* collection);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>DLRL_LS_object</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_createTypedIntMap (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DK_Collection* collection);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
                void
                ccpp_test_Foo_us_changeRelations(
                    DLRL_LS_object ownerObject,
                    DLRL_LS_object relationObject,
                    LOC_unsigned_long index);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>void</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_changeRelations (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object ownerObject,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object relationObject,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    LOC_unsigned_long index);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate typed functions
                void
                ccpp_test_Foo_us_setCollections(
                    DLRL_LS_object ownerObject,
                    DLRL_LS_object collectionObject,
                    LOC_unsigned_long index);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>void</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_setCollections (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object ownerObject,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object collectionObject,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    LOC_unsigned_long index);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <!-- Generate typed functions
                void
                ccpp_test_Foo_us_clearLSObjectAdministration(
                    DLRL_LS_object ownerObject);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>void</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_clearLSObjectAdministration (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object ownerObject);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <!-- Generate typed functions
                LOC_boolean
                ccpp_test_Foo_us_checkObjectForSelection(
                    DLRL_Exception* exception,
                    DLRL_LS_object filter,
                    DLRL_LS_object objectAdmin);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>LOC_boolean</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_checkObjectForSelection (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object filter,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object objectAdmin);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <!-- Generate typed functions
                void
                ccpp_test_Foo_us_triggerListenerInsertedObject(
                    DLRL_Exception* exception,
                    DLRL_LS_object listener,
                    DLRL_LS_object objectAdmin);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>void</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_triggerListenerInsertedObject (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object listener,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object objectAdmin);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <!-- Generate typed functions
                void
                ccpp_test_Foo_us_triggerListenerModifiedObject(
                    DLRL_Exception* exception,
                    DLRL_LS_object listener,
                    DLRL_LS_object objectAdmin);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>void</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_triggerListenerModifiedObject (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object listener,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object objectAdmin);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <!-- Generate typed functions
                void
                ccpp_test_Foo_us_triggerListenerRemovedObject(
                    DLRL_Exception* exception,
                    DLRL_LS_object listener,
                    DLRL_LS_object objectAdmin);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>void</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_triggerListenerRemovedObject (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object listener,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    DLRL_LS_object objectAdmin);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>


            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>typedef </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>HomeInterface* </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_ptr;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>typedef </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>HomeInterface_var </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_var;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome :
                /**
                 * This class is the typed implementation for the abstract ObjectHome
                 * class. It manages all instances of type {@link test::Foo}.
                 */
                class <optional_import_macro> FooHome :
                    public virtual test::FooHomeInterface,
                    public DDS::ObjectHome_impl
                {

            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>/**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * This class is the typed implementation for the abstract ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * class. It manages all instances of type {@link </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>}.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>class </xsl:text>
            <xsl:if test="string-length($import.macro)!=0">
                <xsl:value-of select="$import.macro"/>
                <xsl:text> </xsl:text>
            </xsl:if>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home :</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>HomeInterface,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public DDS::ObjectHome_impl</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>{</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                public:
                    /**
                     * This is the default constructor for the test::FooHome. It initializes
                     * the test::FooHome in the default configuration (i.e.
                     * {@link DDS::ObjectHome#auto_deref()} will return &lt;code&gt;true&lt;/code&gt; and
                     * {@link DDS::ObjectHome#content_filter()} will return &lt;code&gt;null&lt;/code&gt;).
                     */
                    FooHome();
                    virtual ~FooHome();

                    typedef FooHome_ptr _ptr_type;

                    typedef FooHome_var _var_type;

                    static FooHome_ptr _duplicate (FooHome_ptr obj);

                    static FooHome_ptr _narrow (CORBA::Object_ptr obj);

                    static FooHome_ptr _unchecked_narrow (CORBA::Object_ptr obj);

                    static FooHome_ptr _nil ();

                    FooHome_ptr _this ();
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>public:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This is the default constructor for the </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home. It initializes</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * the </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home in the default configuration (i.e.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * {@link DDS::ObjectHome#auto_deref()} will return &lt;code&gt;true&lt;/code&gt; and</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * {@link DDS::ObjectHome#content_filter()} will return &lt;code&gt;null&lt;/code&gt;).</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home();</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual ~</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home();</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    typedef </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_ptr _ptr_type;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    typedef </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_var _var_type;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    static </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_ptr _duplicate (</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_ptr obj);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    static </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_ptr _narrow (</xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Object_ptr obj);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    static </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_ptr _unchecked_narrow (</xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Object_ptr obj);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    static </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_ptr _nil ();</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Home_ptr _this ();</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>


            <!-- Generate FooHome (continued):
                    /**
                     * Returns the list of all attached test::FooListener entities.
                     * This operation may not be called during any ObjectListener callback.
                     * This operation may also not be called during a check_object callback of a
                     * test::FooFilter object that  belongs to a
                     * test::FooSelection attached to the ObjectHome
                     * instance for which this operation is called. Both situations would
                     * result in a deadlock.
                     * This operation is however allowed during a CacheListener callback.
                     *
                     * @return the list of attached test::FooListener entities.
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     */
                    virtual test::FooListenerSeq*
                    listeners(
                        ) THROW_ORB_AND_USER_EXCEPTIONS(
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns the list of all attached </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener entities.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may not be called during any ObjectListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may also not be called during a check_object callback of a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Filter object that  belongs to a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection attached to the ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * instance for which this operation is called. Both situations would</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * result in a deadlock.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation is however allowed during a CacheListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the list of attached </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener entities.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>ListenerSeq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    listeners(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Attaches a test::FooListener to this test::FooHome.
                     * It is possible to specify whether the Listener should also listen for
                     * incoming events on the contained objects. Each listener instance can
                     * only be attached once. This operation may not be called during ObjectListener
                     * callback. This operation may also not be called during a check_object callback of a
                     * test::FooFilter object that  belongs to a
                     * test::FooSelection attached to the ObjectHome
                     * instance for which this operation is called. Both situations would
                     * result in a deadlock. This operation is however allowed during
                     * a CacheListener callback.
                     *
                     * @param listener the test::FooListener to be attached.
                     * @param concerns_contained_objects when set to &lt;code&gt;true&lt;/code&gt;, the
                     * listener will also listen for incoming events on the contained objects.
                     * @return a boolean that specifies whether the listener was successfully
                     * attached (&lt;code&gt;true&lt;/code&gt;) or not (&lt;code&gt;false&lt;/code&gt;) because
                     * it was already attached before.
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     */
                    virtual CORBA::Boolean
                    attach_listener (
                        test::FooListener_ptr listener,
                        CORBA::Boolean concerns_contained_objects) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Attaches a </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener to this </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * It is possible to specify whether the Listener should also listen for</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * incoming events on the contained objects. Each listener instance can</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * only be attached once. This operation may not be called during ObjectListener</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * callback. This operation may also not be called during a check_object callback of a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Filter object that  belongs to a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection attached to the ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * instance for which this operation is called. Both situations would</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * result in a deadlock. This operation is however allowed during</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * a CacheListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param listener the </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener to be attached.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param concerns_contained_objects when set to &lt;code&gt;true&lt;/code&gt;, the</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * listener will also listen for incoming events on the contained objects.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return a boolean that specifies whether the listener was successfully</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * attached (&lt;code&gt;true&lt;/code&gt;) or not (&lt;code&gt;false&lt;/code&gt;) because</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * it was already attached before.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Boolean</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    attach_listener (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener_ptr listener,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Boolean concerns_contained_objects) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Detaches a test::FooListener from this test::FooHome.
                     * This operation may not be called during any ObjectListener
                     * callback. This operation may also not be called during a check_object callback of a
                     * test::FooFilter object that  belongs to a
                     * test::FooSelection attached to the ObjectHome
                     * instance for which this operation is called. Both situations would
                     * result in a deadlock. This operation is however allowed during
                     * a CacheListener callback.
                     *
                     * @param listener the test::FooListener to be detached.
                     * @return a boolean that specifies whether the listener was successfully
                     * detached (&lt;code&gt;true&lt;/code&gt;), or was not attached in the first place
                     * (&lt;code&gt;false&lt;/code&gt;).
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     */
                    virtual CORBA::Boolean
                    detach_listener(
                        test::FooListener_ptr listener) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Detaches a </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener from this </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may not be called during any ObjectListener</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * callback. This operation may also not be called during a check_object callback of a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Filter object that  belongs to a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection attached to the ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * instance for which this operation is called. Both situations would</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * result in a deadlock. This operation is however allowed during</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * a CacheListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param listener the </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener to be detached.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return a boolean that specifies whether the listener was successfully</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * detached (&lt;code&gt;true&lt;/code&gt;), or was not attached in the first place</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * (&lt;code&gt;false&lt;/code&gt;).</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Boolean</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    detach_listener (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener_ptr listener) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Creates a test::FooSelection within this test::FooHome that will work
                     * based upon the provided criterion.
                     * Upon creation time it must be specified wether the selection
                     * will be refreshed using the refresh operation of the selection
                     * (auto_refresh is &lt;code&gt;false&lt;/code&gt;)or refreshed each time object
                     * updates arrive in the cache (due to an application triggered or DCPS
                     * triggered refresh)(auto_refresh is &lt;code&gt;true&lt;/code&gt;). This value can
                     * not be changed after the selection has been created.
                     * The current implementation will always create selections with auto_refresh
                     * set to &lt;code&gt;false&lt;/code&gt;.
                     *
                     * @param criterion the SelectionCriterion determining how the selection
                     * determines which DLRL objects become a part of the Selection.
                     * @param auto_refresh specifies whether the selection
                     * will be refreshed using the refresh operation of the selection
                     * (auto_refresh is &lt;code&gt;false&lt;/code&gt;)or refreshed each time object
                     * updates arrive in the cache (due to an application triggered or DCPS
                     * triggered refresh)(auto_refresh is &lt;code&gt;true&lt;/code&gt;)
                     * @param concerns_contained_objects Not supported currently
                     * @return The created selection object.
                     * @throws DDS::PreconditionNotMet if the Cache to which the ObjectHome
                     * belongs is still in initial pubsub mode or the ObjectHome doesnt yet belong to any Cache.
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     */
                    virtual test::FooSelection_ptr
                    create_selection(
                        DDS::SelectionCriterion_ptr criterion,
                        CORBA::Boolean auto_refresh,
                        CORBA::Boolean concerns_contained_objects) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted,
                            DDS::PreconditionNotMet);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Creates a </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection within this </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home that will work</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * based upon the provided criterion.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Upon creation time it must be specified wether the selection</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * will be refreshed using the refresh operation of the selection</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * (auto_refresh is &lt;code&gt;false&lt;/code&gt;)or refreshed each time object</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * updates arrive in the cache (due to an application triggered or DCPS</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * triggered refresh)(auto_refresh is &lt;code&gt;true&lt;/code&gt;). This value can</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * not be changed after the selection has been created.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * The current implementation will always create selections with auto_refresh</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * set to &lt;code&gt;false&lt;/code&gt;.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param criterion the SelectionCriterion determining how the selection</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * determines which DLRL objects become a part of the Selection.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param auto_refresh specifies whether the selection</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * will be refreshed using the refresh operation of the selection</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * (auto_refresh is &lt;code&gt;false&lt;/code&gt;)or refreshed each time object</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * updates arrive in the cache (due to an application triggered or DCPS</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * triggered refresh)(auto_refresh is &lt;code&gt;true&lt;/code&gt;)</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param concerns_contained_objects Not supported currently</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return The created selection object.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::PreconditionNotMet if the Cache to which the ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * belongs is still in initial pubsub mode or the ObjectHome doesnt yet belong to any Cache.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection_ptr</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    create_selection(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::SelectionCriterion_ptr criterion,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Boolean auto_refresh,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Boolean concerns_contained_objects) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Deletes a test::FooSelection of this test::FooHome.
                     * This operation may not be called during any ObjectListener callback. This operation
                     * may also not be called during a check_object callback of a
                     * test::FooFilter object that  belongs to a
                     * test:FooSelection attached to the ObjectHome
                     * instance for which this operation is called. Both situations would
                     * result in a deadlock.This operation is however allowed during
                     * a CacheListener callback.

                     * @param a_selection the test:FooSelection to be deleted.
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     * @throws DDS::PreconditionNotMet if the selection provided was not
                     * created by this ObjectHome
                     */
                    virtual void
                    delete_selection(
                        test::FooSelection_ptr a_selection) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted,
                            DDS::PreconditionNotMet);
            -->

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Deletes a </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection of this </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may not be called during any ObjectListener callback. This operation</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * may also not be called during a check_object callback of a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Filter object that  belongs to a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection attached to the ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * instance for which this operation is called. Both situations would</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * result in a deadlock.This operation is however allowed during</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * a CacheListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param a_selection the </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection to be deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::PreconditionNotMet if the selection provided was not</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * created by this ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual void</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    delete_selection(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection_ptr a_selection) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Returns the list of all attached test::FooSelection entities.
                     *
                     * @return the list of attached test::FooSelection entities.
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     */
                    virtual test::FooSelectionSeq*
                    selections(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>      * Returns the list of all attached </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection entities.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>      *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>      * @return the list of attached </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection entities.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>      * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>      */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>SelectionSeq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    selections(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Returns the list of all test::Foo objects that have
                     * been modified in a specified Cache during the last update round.
                     * This operation may not be called during any ObjectListener callback.
                     * This operation may also not be called during a check_object callback of a
                     * test::FooFilter object that  belongs to a
                     * test::FooSelection attached to the ObjectHome
                     * instance for which this operation is called. Both situations would
                     * result in a deadlock. This operation is however allowed during
                     * a CacheListener callback.
                     *
                     * @param source the cache from which the list of modified objects needs to be obtained.
                     * @return the list of modified test::Foo objects.
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     */
                    virtual test::FooSeq*
                    get_modified_objects(
                        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns the list of all </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> objects that have</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * been modified in a specified Cache during the last update round.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may not be called during any ObjectListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may also not be called during a check_object callback of a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Filter object that  belongs to a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection attached to the ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * instance for which this operation is called. Both situations would</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * result in a deadlock. This operation is however allowed during</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * a CacheListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param source the cache from which the list of modified objects needs to be obtained.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the list of modified </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> objects.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    get_modified_objects(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Returns the list of all test::Foo objects that have
                     * been deleted in a specified Cache during the last update round.
                     * This operation may not be called during any ObjectListener callback.
                     * This operation may also not be called during a check_object callback of a
                     * test::FooFilter object that  belongs to a
                     * test::FooSelection attached to the ObjectHome
                     * instance for which this operation is called. Both situations would
                     * result in a deadlock. This operation is however allowed during
                     * a CacheListener callback.
                     *
                     * @param source the cache from which the list of deleted objects needs to be obtained.
                     * @return the list of deleted test::Foo objects.
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     */
                    virtual test::FooSeq*
                    get_deleted_objects(
                        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns the list of all </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> objects that have</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * been deleted in a specified Cache during the last update round.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may not be called during any ObjectListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may also not be called during a check_object callback of a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Filter object that  belongs to a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection attached to the ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * instance for which this operation is called. Both situations would</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * result in a deadlock. This operation is however allowed during</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * a CacheListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param source the cache from which the list of deleted objects needs to be obtained.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the list of deleted </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> objects.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    get_deleted_objects(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Returns the list of all test::Foo objects that have
                     * been created in a specified Cache during the last update round.
                     * This operation may not be called during any ObjectListener callback.
                     * This operation may also not be called during a check_object callback of a
                     * test::FooFilter object that  belongs to a
                     * test::FooSelection attached to the ObjectHome
                     * instance for which this operation is called. Both situations would
                     * result in a deadlock. This operation is however allowed during
                     * a CacheListener callback.
                     *
                     * @param source the cache from which the list of created objects needs to be obtained.
                     * @return the list of created test::Foo objects.
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     */
                    virtual test::FooSeq*
                    get_created_objects(
                        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns the list of all </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> objects that have</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * been created in a specified Cache during the last update round.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may not be called during any ObjectListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may also not be called during a check_object callback of a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Filter object that  belongs to a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection attached to the ObjectHome</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * instance for which this operation is called. Both situations would</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * result in a deadlock. This operation is however allowed during</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * a CacheListener callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param source the cache from which the list of created objects needs to be obtained.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the list of created </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> objects.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    get_created_objects(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Returns the list of all {@link test::Foo} objects that are
                     * available in a specified {@link DDS::CacheBase}. Note that this list never
                     * contains objects that are considered deleted. This includes objects that are still
                     * contained in the list resulting from a call to the get_deleted_objects() operation
                     * of this object home.
                     * This operation may not be called during any {@link DDS::ObjectListener} callback.
                     * This operation may also not be called during a {@link test::FooFilter#check_object} callback
                     * that belongs to a {@link test::FooSelection} attached to the {@link DDS::ObjectHome}
                     * instance for which this operation is called. Both situations would
                     * result in a deadlock.This operation is however allowed during
                     * a {@link DDS::CacheListener} callback.
                     *
                     * @param source the {@link DDS::Cache} from which the list of available objects should be obtained.
                     * @return the list of available test::Foo objects.
                     * @throws DDS::AlreadyDeleted if the current Home is already deleted.
                     */
                    virtual test::FooSeq*
                    get_objects(
                        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                        DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns the list of all {@link </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>} objects that are</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * available in a specified {@link DDS::CacheBase}. Note that this list never</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * contains objects that are considered deleted. This includes objects that are still</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * contained in the list resulting from a call to the get_deleted_objects() operation</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * of this object home.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may not be called during any {@link DDS::ObjectListener} callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation may also not be called during a {@link </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Filter#check_object} callback</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * that belongs to a {@link </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection} attached to the {@link DDS::ObjectHome}</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * instance for which this operation is called. Both situations would</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * result in a deadlock.This operation is however allowed during</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * a {@link DDS::CacheListener} callback.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param source the {@link DDS::Cache} from which the list of available objects should be obtained.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the list of available </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> objects.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the current Home is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    get_objects(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Pre-create a new DLRL object in order to fill its content before the allocation
                     * of the OID. This method takes as parameter the {@link DDS::CacheAccess} concerned
                     * with this operation. The CacheAccess must belong to a {@link DDS::Cache} which
                     * has a DCPS state of ENABLED. The CacheAccess must also have a usage of WRITE_ONLY
                     * or READ_WRITE. Failure to satisfy either precondition will result in a
                     * PreconditionNotMet exception being raised.
                     *
                     * @param access The cache access which is concerned by the pre-creation.
                     * @return the pre-created object.
                     * @throws DDS::AlreadyDeleted if the ObjectHome or CacheAccess is already deleted.
                     * @throws DDS::PreconditionNotMet if the owning Cache does not have an ENABLED dcps state or
                     * if the cache access in question is not writeable.
                     */
                    virtual test::Foo*
                    create_unregistered_object(
                        DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::PreconditionNotMet,
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Pre-create a new DLRL object in order to fill its content before the allocation</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * of the OID. This method takes as parameter the {@link DDS::CacheAccess} concerned</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * with this operation. The CacheAccess must belong to a {@link DDS::Cache} which</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * has a DCPS state of ENABLED. The CacheAccess must also have a usage of WRITE_ONLY</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * or READ_WRITE. Failure to satisfy either precondition will result in a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * PreconditionNotMet exception being raised.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param access The cache access which is concerned by the pre-creation.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the pre-created object.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the ObjectHome or CacheAccess is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::PreconditionNotMet if the owning Cache does not have an ENABLED dcps state or</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * if the cache access in question is not writeable.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    create_unregistered_object(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::PreconditionNotMet,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /*
                     * Look up an ObjectRoot with the specified oid in the specified CacheBase.
                     *
                     * This operation is currently not supported.
                     *
                     * @param oid The ObjectIDentity (OID) of the ObjectRoot wanted.
                     * @param source the target CacheBase to search in.
                     * @throws DDS::NotFound if the object with the specified OID could not be located.
                     * @throws DDS::AlreadyDeleted If the ObjectHome or CacheBase have already been deleted.
                     */
                    virtual test::Foo*
                    find_object(
                        const DDS::DLRLOid& oid,
                        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::NotFound,
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /*</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Look up an ObjectRoot with the specified oid in the specified CacheBase.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation is currently not supported.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param oid The ObjectIDentity (OID) of the ObjectRoot wanted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param source the target CacheBase to search in.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::NotFound if the object with the specified OID could not be located.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted If the ObjectHome or CacheBase have already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    find_object(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        const DDS::DLRLOid&amp; oid,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::CacheBase_ptr source) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::NotFound,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Register an object resulting from {@link test::FooHome#create_unregistered_object()}.
                     * This operation embeds a logic to derive a suitable OID from the object content. Only
                     * objects created by {@link test::FooHome#create_unregistered_object()} can be passed
                     * as parameter, a PreconditionNotMet is raised otherwise. If the result of the computation
                     * leads to an existing OID, an AlreadyExisting exception is raised. Once an object has been
                     * registered, the fields that make up its identity (i.e. the fields that are mapped onto
                     * the keyfields of the corresponding topics) may not be changed anymore.
                     *
                     * @param unregistered_object An object created by a call to {@link test::FooHome#create_unregistered_object()}
                     * @throws DDS::PreconditionNotMet if the provided object was not created by calling the
                     * {@link test::FooHome#create_unregistered_object()} operation.
                     * @throws DDS::AlreadyExisting If the result of the OID computation leads to an existing OID.
                     * @throws DDS::AlreadyDeleted If the ObjectHome or CacheAccess is already deleted.
                     */
                    virtual void
                    register_object(
                        test::Foo* unregistered_object) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::PreconditionNotMet,
                            DDS::AlreadyExisting,
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Register an object resulting from {@link </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home#create_unregistered_object()}.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This operation embeds a logic to derive a suitable OID from the object content. Only</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * objects created by {@link </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home#create_unregistered_object()} can be passed</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * as parameter, a PreconditionNotMet is raised otherwise. If the result of the computation</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * leads to an existing OID, an AlreadyExisting exception is raised. Once an object has been</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * registered, the fields that make up its identity (i.e. the fields that are mapped onto</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * the keyfields of the corresponding topics) may not be changed anymore.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param unregistered_object An object created by a call to {@link </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home#create_unregistered_object()}</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::PreconditionNotMet if the provided object was not created by calling the</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * {@link </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home#create_unregistered_object()} operation.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyExisting If the result of the OID computation leads to an existing OID.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted If the ObjectHome or CacheAccess is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual void</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    register_object(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>* unregistered_object) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::PreconditionNotMet,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyExisting,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    /**
                     * Creates and returns a new DLRL object. This operation takes as parameter the
                     * {@link DDS::CacheAccess} concerned by the creation. The CacheAccess must belong to a
                     * {@link DDS::Cache} which has a DCPS state of ENABLED. The CacheAccess must also have
                     * a usage of WRITE_ONLY or READ_WRITE. Failure to satisfy either precondition will
                     * result in a PreconditionNotMet exception being raised.
                     *
                     * @param access The cache access which is concerned by the creation.
                     * @return the newly created object.
                     * @throws DDS::AlreadyDeleted if the ObjectHome or CacheAccess is already deleted.
                     * @throws DDS::PreconditionNotMet if the owning Cache does not have an ENABLED dcps state or
                     * if the cache access in question is not writeable.
                     */
                    virtual test::Foo*
                    create_object(
                        DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::PreconditionNotMet,
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Creates and returns a new DLRL object. This operation takes as parameter the</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * {@link DDS::CacheAccess} concerned by the creation. The CacheAccess must belong to a</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * {@link DDS::Cache} which has a DCPS state of ENABLED. The CacheAccess must also have</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * a usage of WRITE_ONLY or READ_WRITE. Failure to satisfy either precondition will</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * result in a PreconditionNotMet exception being raised.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param access The cache access which is concerned by the creation.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the newly created object.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the ObjectHome or CacheAccess is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::PreconditionNotMet if the owning Cache does not have an ENABLED dcps state or</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * if the cache access in question is not writeable.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    create_object(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::PreconditionNotMet,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                protected:
                    virtual DDS::ReturnCode_t
                    registerType(
                        DDS::DomainParticipant_ptr participant,
                        LOC_char* typeName,
                        LOC_char* topicName);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>protected:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual DDS::ReturnCode_t</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    registerType(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::DomainParticipant_ptr participant,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        LOC_char* typeName,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        LOC_char* topicName);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                    virtual void
                    loadMetaModel(
                        );
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual void</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    loadMetaModel(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        );</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome (continued):
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSelection:
                /**
                 * This class is the typed implementation of the Selection interface.
                 * It contains instances of type {@link test::Foo}
                 * that match the {@link DDS::SelectionCriterion} provided at creation time
                 * of the Selection.
                 */
                class FooSelection_impl :
                    public virtual test::FooSelection,
                    public DDS::Selection_impl
                {

                    friend class test::FooHome;
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>/**</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * This class is the typed implementation of the Selection interface.</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * It contains instances of type {@link </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>}</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * that match the {@link DDS::SelectionCriterion} provided at creation time</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * of the Selection.</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> */</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>class </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Selection_impl:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public DDS::Selection_impl</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>{</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    friend class </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Home;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSelection (continued):
            private:
                FooSelection_impl();
                virtual ~FooSelection_impl();
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>private:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Selection_impl();</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual ~</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Selection_impl();</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSelection (continued):
            protected:
                virtual CORBA::ULong
                check_objects(
                    DLRL_Exception* exception,
                    DLRL_LS_object filterCriterion,
                    const DDS::ObjectRootSeq& lsObjects,
                    DK_ObjectAdmin** inputAdmins,
                    DK_ObjectAdmin** passedAdmins);
            -->

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>protected:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::ULong </xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    check_objects(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DLRL_LS_object filterCriterion,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        const DDS::ObjectRootSeq&amp; lsObjects,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DK_ObjectAdmin** inputAdmins,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DK_ObjectAdmin** passedAdmins);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <!-- Generate FooSelection (continued):
                public:
                    /**
                     * Returns the {@link test::Foo}
                     * objects that are a part of the selection since the last time it was
                     * refreshed. I.E. the last time the refresh operation of the selection
                     * was called (@see DDS::Selection#auto_refresh() returns <code>false</code>)
                     * or the last time the related {@link DDS::Cache} was refreshed
                     * (@see DDS::Selectionauto_refresh() returns <code>true</code>).
                     *
                     * @throws DDS::AlreadyDeleted if the Selection is already deleted.
                     * @return the DLRL objects that are members of this selection.
                     */
                    virtual test::FooSeq*
                    members(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>public:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns the {@link </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>}</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * objects that are a part of the selection since the last time it was</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * refreshed. I.E. the last time the refresh operation of the selection</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * was called (@see DDS::Selection#auto_refresh() returns &lt;code>false&lt;/code>)</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * or the last time the related {@link DDS::Cache} was refreshed</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * (@see DDS::Selectionauto_refresh() returns &lt;code>true&lt;/code>).</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the Selection is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the DLRL objects that are members of this selection.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    members(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSelection (continued):
                    virtual test::FooSeq*
                    get_inserted_objects(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);

                    virtual test::FooSeq*
                    get_modified_objects(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);

                    virtual test::FooSeq*
                    get_removed_objects(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    get_inserted_members(</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    get_modified_members(</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    get_removed_members(</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSelection (continued):
                    /*
                     * Not supported
                     *
                     * @param listener
                     * @throws DDS::AlreadyDeleted if the Selection is already deleted.
                     * @return
                     */
                    virtual test::FooSelectionListener_ptr
                    set_listener(
                        test::FooSelectionListener_ptr listener) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /*</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Not supported</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param listener</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the Selection is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the previously attached listener or null if none</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>SelectionListener_ptr</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    set_listener(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>SelectionListener_ptr listener) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSelection (continued):
                    /*
                     * Not supported
                     *
                     * @throws DDS::AlreadyDeleted if the Selection is already deleted.
                     * @return the currently attached listener
                     */
                    virtual test::FooSelectionListener_ptr
                    listener(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /*</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Not supported</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the Selection is already deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the currently attached listener</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>SelectionListener_ptr</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    listener(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSelection (continued):
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooFilter:
                /**
                 * <P>This is a callback abstract class that must be implemented by the
                 * user. When implemented, it can be attached to the corresponding
                 * {@link test::FooSelection} upon
                 * creation, which will invoke the check_object() callback method to
                 * determine wether or not an object passed the criteria as defined by the
                 * check_object callback algorithm. This callback operation will also be
                 * called for potential sub-classes of the DLRL class if available</P>
                 */
                class  <optional_import_macro> FooFilter :
                    public virtual test::FooFilterInterface,
                    public DDS::FilterCriterion_impl
                {
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>/**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * &lt;P>This is a callback abstract class that must be implemented by the</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * user. When implemented, it can be attached to the corresponding</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * {@link </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection} upon</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * creation, which will invoke the check_object() callback method to</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * determine wether or not an object passed the criteria as defined by the</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * check_object callback algorithm. This callback operation will also be</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * called for potential sub-classes of the DLRL class if available&lt;/P></xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>class </xsl:text>
            <xsl:if test="string-length($import.macro)!=0">
                <xsl:value-of select="$import.macro"/>
                <xsl:text> </xsl:text>
            </xsl:if>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Filter:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>FilterInterface,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public DDS::FilterCriterion_impl</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>{</xsl:text>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooFilter (continued):
                protected:
                    FooFilter();
                    virtual ~FooFilter();
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    protected:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Filter();</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        virtual ~</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Filter();</xsl:text>
            <xsl:value-of select="$NL"/>
            <!-- Generate FooFilter (continued):
                /**
                 * This callback function will be invoked when this filter is attached
                 * to a selection and the selection is being refreshed (either DLRL invoked
                 * or application invoked).
                 *
                 * &lt;P>The return value of this operation will
                 * determine wether or not the object becomes a part of the
                 * selection.&lt;/P>
                 *
                 * @param an_object the object to evaluate if it matches the filter criteria
                 * @param membership_state An enum indicating the relation of the
                 * to-be-evaluated object to the selection this filter belongs to. Will
                 * always be UNDEFINED.
                 * @return &lt;code>true&lt;/code> if the object should become a part of the
                 * selection and &lt;code>false&lt;/code> if the object should not become
                 * part of the selection.
                 */
                 public:
                    virtual DDS::Boolean
                    check_object (
                        Track* an_object,
                        DDS::MembershipState membership_state) = 0;
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * This callback function will be invoked when this filter is attached </xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * to a selection and the selection is being refreshed (either DLRL invoked </xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * or application invoked).</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>* &lt;P>The return value of this operation will </xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * determine wether or not the object becomes a part of the </xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * selection.&lt;/P></xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param an_object the object to evaluate if it matches the filter criteria</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param membership_state An enum indicating the relation of the </xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * to-be-evaluated object to the selection this filter belongs to. Will</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * always be UNDEFINED.</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return &lt;code>true&lt;/code> if the object should become a part of the</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * selection and &lt;code>false&lt;/code> if the object should not become</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * part of the selection.</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        virtual </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Boolean</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        check_object (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>* an_object,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::MembershipState membership_state) = 0;</xsl:text>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooFilter (continued):
                };

                typedef FooFilterInterface_ptr FooFilter_ptr;
                typedef FooFilterInterface_var FooFilter_var;
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>typedef </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>FilterInterface_ptr </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Filter_ptr;</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>typedef </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>FilterInterface_var </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Filter_var;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet_impl:
                /**
                 * This class is the typed implementation for the abstract DDS::Set
                 * class. It represents a Set that stores instances of a test::Foo
                 * object, or one of its sub-classes. A set can not store the same instance
                 * more than once.
                 */
                class FooSet_impl :
                    public virtual test::FooSet,
                    public DDS::Set_impl
                {
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>/**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * This class is the typed implementation for the abstract DDS::Set</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * class. It represents a Set that stores instances of a </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * object, or one of its sub-classes. A set can not store the same instance</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * more than once.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>class </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Set_impl:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Set,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public DDS::Set_impl</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>{</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet_impl:
                friend DLRL_LS_object
                    ccpp_test_Foo_us_createTypedSet (
                        DLRL_Exception* exception,
                        DK_Collection* collection);

                private:
                    FooSet_impl(DK_SetAdmin* collection);

                    virtual ~FooSet_impl();
            -->

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    friend DLRL_LS_object</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_createTypedSet (</xsl:text>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:value-of select="$NL"/>
            <xsl:text>        DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DK_Collection* collection);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    private:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Set_impl(DK_SetAdmin* collection);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        virtual ~</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Set_impl();</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet_impl (continued):
                public:
                    /**
                     * Stores the specified test::Foo item into the Set. If the item was already
                     * contained in the Set, then this operation will have no effect.
                     *
                     * A PreconditionNotMet is raised if any of the following preconditions is violated:
                     * &lt;ul>&lt;li>The Set is not located in a (writeable) CacheAccess;&lt;/li>
                     * &lt;li>The Set belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul>
                     *
                     * @param value the item that needs to be stored in the Set.
                     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.
                     * @throws DDS::PreconditionNotMet if any of the preconditions where not met.
                     */
                    virtual void
                    add(
                        test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted,
                            DDS::PreconditionNotMet);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>public:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Stores the specified </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> item into the Set. If the item was already</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * contained in the Set, then this operation will have no effect.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * A PreconditionNotMet is raised if any of the following preconditions is violated:</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * &lt;ul>&lt;li>The Set is not located in a (writeable) CacheAccess;&lt;/li></xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * &lt;li>The Set belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul></xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param value the item that needs to be stored in the Set.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::PreconditionNotMet if any of the preconditions where not met.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual void</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    add(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet_impl (continued):
                    /**
                     * Returns an Object array that contains all test::Foo elements that were
                     * added during the last update round. It is recommended to use the values() operation instead of this
                     * operation when dealing with a collection belonging to an ObjectRoot with read_state
                     * OBJECT_NEW. In this case both lists will be equal and the values() operation will give
                     * better performance. But only in the described case, in other situations it's
                     * recommended to use this operation. When this collection belongs to an ObjectRoot with read_state
                     * VOID then this operation will always return a zero length array.
                     *
                     * @return the test::Foo[] that contains all added test::Foo elements.
                     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.
                     */
                    virtual test::FooSeq*
                    added_elements(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns an Object array that contains all </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> elements that were</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * added during the last update round. It is recommended to use the values() operation instead of this</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * operation when dealing with a collection belonging to an ObjectRoot with read_state</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * OBJECT_NEW. In this case both lists will be equal and the values() operation will give</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * better performance. But only in the described case, in other situations it's</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * recommended to use this operation. When this collection belongs to an ObjectRoot with read_state</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * VOID then this operation will always return a zero length array.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>[] that contains all added </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> elements.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    added_elements(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet_impl (continued):
                    /**
                     * Returns whether the specified test::Foo element is already contained in
                     * the Set (<code>true</code>) or not (<code>false</code>).
                     *
                     * @param value the item that needs to be examined.
                     * @return whether the specified element is already contained in the Set.
                     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.
                     */
                    virtual CORBA::Boolean
                    contains(
                        test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                        DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns whether the specified </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> element is already contained in</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * the Set (&lt;code>true&lt;/code>) or not (&lt;code>false&lt;/code>).</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param value the item that needs to be examined.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return whether the specified element is already contained in the Set.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Boolean</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    contains(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet_impl (continued):
                    /**
                     * Removes the specified test::Foo element from the Set. If the specified
                     * element is not contained in the Set, then this operation will have
                     * no effect.
                     *
                     * A PreconditionNotMet is raised if any of the following preconditions is violated:
                     * &lt;ul>&lt;li>The Set is not located in a (writeable) CacheAccess;&lt;/li>
                     * &lt;li>The Set belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul>
                     *
                     * @param value the item that needs to be removed.
                     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.
                     * @throws DDS::PreconditionNotMet if any of the preconditions where not met.
                     */
                    virtual void
                    remove(
                        test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted,
                            DDS::PreconditionNotMet);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Removes the specified </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> element from the Set. If the specified</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * element is not contained in the Set, then this operation will have</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * no effect.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * A PreconditionNotMet is raised if any of the following preconditions is violated:</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * &lt;ul>&lt;li>The Set is not located in a (writeable) CacheAccess;&lt;/li></xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * &lt;li>The Set belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul></xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param value the item that needs to be removed.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::PreconditionNotMet if any of the preconditions where not met.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual void</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    remove(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet_impl (continued):
                    /**
                     * Returns an Object array that contains all test::Foo elements that are
                     * currently contained in the Set.
                     *
                     * @return the test::Foo[] that contains all available test::Foo elements.
                     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.
                     */
                    virtual test::FooSeq*
                    values(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns an Object array that contains all </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> elements that are</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * currently contained in the Set.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>[] that contains all available </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> elements.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    values(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet_impl (continued):
                    /**
                     * Returns an Object array that contains all test::Foo elements that were
                     * removed during the last update round. When this collection belongs to an ObjectRoot with read_state
                     * VOID or OBJECT_NEW then this operation will always return a zero length array.
                     *
                     * @return the test::Foo[] that contains all removed test::Foo elements.
                     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.
                     */
                    virtual test::FooSeq*
                    removed_elements(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns an Object array that contains all </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> elements that were</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * removed during the last update round. When this collection belongs to an ObjectRoot with read_state</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * VOID or OBJECT_NEW then this operation will always return a zero length array.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>[] that contains all removed </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> elements.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Set has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    removed_elements(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet_impl (continued):
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooStrMap_impl:
                /**
                 * This class is the typed implementation for the abstract DDS::StrMap
                 * class. It represents a Map that stores key-value pairs in which the key
                 * represents a String and the value represent a <xsl:value-of select="$prefixedName"/>
                 * object, or one of its sub-classes.
                 */
                class FooStrMap_impl :
                    public virtual test::FooStrMap,
                    public DDS::StrMap_impl
                {
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>/**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * This class is the typed implementation for the abstract DDS::StrMap</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * class. It represents a Map that stores key-value pairs in which the key</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * represents a String and the value represent a </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * object, or one of its sub-classes.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> */</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>class </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>StrMap_impl:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>StrMap,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public DDS::StrMap_impl</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>{</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooStrMap_impl:
                friend DLRL_LS_object
                    ccpp_test_Foo_us_createTypedStrMap (
                        DLRL_Exception* exception,
                        DK_Collection* collection);

                private:
                    FooStrMap_impl(DK_MapAdmin* collection);

                    virtual ~FooStrMap_impl();
            -->

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    friend DLRL_LS_object</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_createTypedStrMap (</xsl:text>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:value-of select="$NL"/>
            <xsl:text>        DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DK_Collection* collection);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    private:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>StrMap_impl(DK_MapAdmin* collection);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        virtual ~</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>StrMap_impl();</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooStrMap_impl (continued):
                public:
                    /**
                     * Returns an Object array that contains all test::Foo elements that are
                     * currently stored in the Map.
                     *
                     * @return the test::Foo[] that contains all available test::Foo elements.
                     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.
                     */
                    virtual test::FooSeq*
                    values(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                        DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>public:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns an Object array that contains all </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> elements that are</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * currently stored in the Map.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>[] that contains all available </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>elements.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    values(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooStrMap_impl (continued):
                    /**
                     * Retrieves a test::Foo item from the Map, based on its key.
                     *
                     * @param key the key that identifies the item that is to be retrieved.
                     * @return the item that corresponds to the specified key (may be null)
                     * or null if no element can be found for the specified key
                     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.
                     * @throws DDS::NoSuchElement if is no test::Foo element present that matches the specified key
                     */
                    virtual test::Foo*
                    get(
                        const char* key) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted,
                            DDS::NoSuchElement);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Retrieves a </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> item from the Map, based on its key.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param key the key that identifies the item that is to be retrieved.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the item that corresponds to the specified key (may be null)</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * or null if no element can be found for the specified key</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::NoSuchElement if is no </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> element present that matches the specified key</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    get(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        const char* key) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::NoSuchElement);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooStrMap_impl (continued):
                    /**
                     * Stores the specified test::Foo item into the Map, using the specified key
                     * as its identifier. If the key already represented another item in the
                     * Map, then that item will be replaced by the currently specified item.
                     *
                     * A PreconditionNotMet is raised if any of the following preconditions is violated:
                     * &lt;ul>&lt;li>The Map is not located in a (writeable) CacheAccess;&lt;/li>
                     * &lt;li>The Map belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul>
                     *
                     * @param key the key that will identify the specified item.
                     * @param value the item that needs to be stored in the Map.
                     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.
                     */
                    virtual void
                    put(
                        const char* key,
                        test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted,
                            DDS::PreconditionNotMet);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Stores the specified </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> item into the Map, using the specified key</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * as its identifier. If the key already represented another item in the</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Map, then that item will be replaced by the currently specified item.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * A PreconditionNotMet is raised if any of the following preconditions is violated:</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * &lt;ul>&lt;li>The Map is not located in a (writeable) CacheAccess;&lt;/li></xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * &lt;li>The Map belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul></xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param key the key that will identify the specified item.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param value the item that needs to be stored in the Map.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual void</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    put(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        const char* key,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooStrMap_impl (continued):
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- next two classes are in seperate templates to avoid XSLT compile errors -->
            <xsl:call-template name="FooIntMapTemplate">
                <xsl:with-param name="whiteSpace" select="$whiteSpace"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
            </xsl:call-template>

            <xsl:call-template name="FooListTemplate">
                <xsl:with-param name="whiteSpace" select="$whiteSpace"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
            </xsl:call-template>

        </xsl:if>
    </xsl:template>

    <xsl:template name="FooIntMapTemplate">
        <xsl:param name ="whiteSpace"/>
        <xsl:param name ="prefixedFullName"/>
        <xsl:param name ="nonPrefixedName"/>
        <xsl:param name ="prefixedFullNameExceptLast"/>
        <xsl:param name ="fullNameWithUnderscores"/>
            <!-- Generate FooIntMap_impl:
                /**
                 * This class is the typed implementation for the abstract DDS::IntMap
                 * class. It represents a Map that stores key-value pairs in which the key
                 * represents an integer and the value represent a test::Foo
                 * object, or one of its sub-classes.
                 */
                class FooIntMap_impl :
                    public virtual test::FooIntMap,
                    public DDS::IntMap_impl
                {
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>/**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * This class is the typed implementation for the abstract DDS::IntMap</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * class. It represents a Map that stores key-value pairs in which the key</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * represents an integer and the value represent a </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> * object, or one of its sub-classes.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text> */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>class </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>IntMap_impl :</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>IntMap,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public DDS::IntMap_impl</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>{</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooIntMap_impl:
                friend DLRL_LS_object
                    ccpp_test_Foo_us_createTypedIntMap (
                        DLRL_Exception* exception,
                        DK_Collection* collection);

                private:
                    FooIntMap_impl(DK_MapAdmin* collection);

                    virtual ~FooIntMap_impl();
            -->

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    friend DLRL_LS_object</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    ccpp_</xsl:text>
            <xsl:value-of select="$fullNameWithUnderscores"/>
            <xsl:text>_us_createTypedIntMap (</xsl:text>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:value-of select="$NL"/>
            <xsl:text>        DLRL_Exception* exception,</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        DK_Collection* collection);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    private:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>IntMap_impl(DK_MapAdmin* collection);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        virtual ~</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>IntMap_impl();</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>


            <!-- Generate FooIntMap_impl (continued):
                public:
                    /**
                     * Returns an Object array that contains all test::Foo elements that are
                     * currently stored in the Map.
                     *
                     * @return the test::Foo[] that contains all available elements.
                     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.
                     */
                    virtual test::FooSeq*
                    values(
                        ) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>public:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Returns an Object array that contains all </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> elements that are</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * currently stored in the Map.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>[] that contains all available elements.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq*</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    values(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooIntMap_impl (continued):
                    /**
                     * Retrieves a test::Foo item from the Map, based on its key.
                     *
                     * @param key the key that identifies the item that is to be retrieved.
                     * @return the item that corresponds to the specified key (may be null)
                     * or null if no element can be found for the specified key
                     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.
                     * @throws DDS::NoSuchElement if is no test::Foo element present that matches the specified key
                     */
                    virtual test::Foo*
                    get(
                        CORBA::Long key) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted,
                            DDS::NoSuchElement);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Retrieves a </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> item from the Map, based on its key.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param key the key that identifies the item that is to be retrieved.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @return the item that corresponds to the specified key (may be null)</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * or null if no element can be found for the specified key</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::NoSuchElement if is no </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> element present that matches the specified key</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>*</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    get(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Long key) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::NoSuchElement);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooIntMap_impl (continued):
                    /**
                     * Stores the specified test::Foo item into the Map, using the specified key
                     * as its identifier. If the key already represented another item in the
                     * Map, then that item will be replaced by the currently specified item.
                     *
                     * A PreconditionNotMet is raised if any of the following preconditions is violated:
                     * &lt;ul>&lt;li>The Map is not located in a (writeable) CacheAccess;&lt;/li>
                     * &lt;li>The Map belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul>
                     *
                     * @param key the key that will identify the specified item.
                     * @param value the item that needs to be stored in the Map.
                     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.
                     * @throws DDS::PreconditionNotMet if any of the preconditions where not met.
                     */
                    virtual void
                    put(
                        CORBA::Long key,
                        test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                            DDS::AlreadyDeleted,
                            DDS::PreconditionNotMet);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    /**</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Stores the specified </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> item into the Map, using the specified key</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * as its identifier. If the key already represented another item in the</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * Map, then that item will be replaced by the currently specified item.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * A PreconditionNotMet is raised if any of the following preconditions is violated:</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * &lt;ul>&lt;li>The Map is not located in a (writeable) CacheAccess;&lt;/li></xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * &lt;li>The Map belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul></xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     *</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param key the key that will identify the specified item.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @param value the item that needs to be stored in the Map.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the Map has already been deleted.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     * @throws DDS::PreconditionNotMet if any of the preconditions where not met.</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>     */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    virtual void</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    put(</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:call-template name="get_corba_module_name"/>
            <xsl:text>::Long key,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::AlreadyDeleted,</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>            DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooIntMap_impl (continued):
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

    </xsl:template>


    <xsl:template name="FooListTemplate">
        <xsl:param name ="whiteSpace"/>
        <xsl:param name ="prefixedFullName"/>
        <xsl:param name ="nonPrefixedName"/>
        <xsl:param name ="prefixedFullNameExceptLast"/>

        <!-- Generate FooList_impl:
            /**
             * This class is currently not supported.
             *
             * This class is the typed implementation for the abstract DDS::List
             * class. It represents a linked list that stores elements of
             * type test::Foo object, or one of its sub-classes.
             */
            class FooList_impl :
                public virtual test::FooList,
                public DDS::List_impl
            {
        -->
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>/**</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text> * This class is currently not supported.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text> *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text> * This class is the typed implementation for the abstract DDS::List</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text> * class. It represents a linked list that stores elements of </xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text> * type </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text> object, or one of its sub-classes.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text> */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>class </xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>List_impl :</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    public virtual </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>List,</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    public DDS::List_impl</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate FooList_impl (continued):
            public:
                /**
                 * Returns an Object array that contains all test::Foo elements that are
                 * currently stored in the List.
                 *
                 * @return the test::Foo[] that contains all available elements.
                 * @throws DDS::AlreadyDeleted if the owner of the List has already been deleted.
                 */
                virtual test::FooSeq*
                values(
                    ) THROW_ORB_AND_USER_EXCEPTIONS (
                        DDS::AlreadyDeleted);
        -->
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>public:</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * Returns an Object array that contains all </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text> elements that are</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * currently stored in the List.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @return the </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>[] that contains all available elements.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the List has already been deleted.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    virtual </xsl:text>
        <xsl:value-of select="$prefixedFullNameExceptLast"/>
        <xsl:text>Seq*</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    values(</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>        ) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate FooList_impl (continued):
                /**
                 * Retrieves a test::Foo item from the List, based on its index.
                 *
                 * @param key the index that identifies the item that is to be retrieved.
                 * @return the item that corresponds to the specified index (may be null)
                 * or null if no element can be found for the specified index
                 * @throws DDS::AlreadyDeleted if the owner of the List has already been deleted.
                 * @throws DDS::NoSuchElement if is no test::Foo element present that matches the specified index
                 */
                virtual test::Foo*
                get(
                    CORBA::Long key) THROW_ORB_AND_USER_EXCEPTIONS (
                        DDS::AlreadyDeleted,
                        DDS::NoSuchElement);
        -->
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * Retrieves a </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text> item from the List, based on its index.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @param key the index that identifies the item that is to be retrieved.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @return the item that corresponds to the specified index (may be null)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * or null if no element can be found for the specified index</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the List has already been deleted.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @throws DDS::NoSuchElement if is no </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text> element present that matches the specified index</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    virtual </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>*</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    get(</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>        </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Long key) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>            DDS::AlreadyDeleted,</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>            DDS::NoSuchElement);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate FooList_impl (continued):
                /**
                 * Stores the specified test::Foo item into the List, using the length() as index.
                 * i.e. stores the element at the end of the list.
                 *
                 * A PreconditionNotMet is raised if any of the following preconditions is violated:
                 * &lt;ul>&lt;li>The List is not located in a (writeable) CacheAccess;&lt;/li>
                 * &lt;li>The List belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul>
                 *
                 * @param value the item that needs to be stored in the List.
                 * @throws DDS::AlreadyDeleted if the owner of the List has already been deleted.
                 */
                virtual void
                add(
                    test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                        DDS::PreconditionNotMet,
                        DDS::AlreadyDeleted);
        -->
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>/**</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * Stores the specified </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text> item into the List, using the length() as index.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * i.e. stores the element at the end of the list.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * A PreconditionNotMet is raised if any of the following preconditions is violated:</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * &lt;ul>&lt;li>The List is not located in a (writeable) CacheAccess;&lt;/li></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * &lt;li>The List belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @param value the item that needs to be stored in the List.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the List has already been deleted.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    virtual void</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    add(</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>        </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>            DDS::PreconditionNotMet,</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate FooList_impl (continued):
                /**
                 * Stores the specified test::Foo item into the List, using the specified index
                 * as its identifier. If the index already represented another item in the
                 * List, then that item will be replaced by the currently specified item. It is only allowed to replace
                 * already contained elements in the list or to add an element at the end of the list (the index may not be larger then
                 * the length of the list).
                 *
                 * A PreconditionNotMet is raised if any of the following preconditions is violated:
                 * &lt;ul>&lt;li>The List is not located in a (writeable) CacheAccess;&lt;/li>
                 * &lt;li>The List belongs to an ObjectRoot which is not yet registered.&lt;/li>
                 * &lt;li>The specified key value represents an index larger then the current length of the list.&lt;/li>&lt;/ul>
                 *
                 * @param key the index that will identify the specified item.
                 * @param value the item that needs to be stored in the List.
                 * @throws DDS::AlreadyDeleted if the owner of the List has already been deleted.
                 */
                virtual void
                put(
                    CORBA::Long key,
                    test::Foo* value) THROW_ORB_AND_USER_EXCEPTIONS (
                        DDS::PreconditionNotMet,
                        DDS::AlreadyDeleted);
        -->
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * Stores the specified </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text> item into the List, using the specified index</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * as its identifier. If the index already represented another item in the</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * List, then that item will be replaced by the currently specified item. It is only allowed to replace</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * already contained elements in the list or to add an element at the end of the list (the index may not be larger then</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * the length of the list).</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * A PreconditionNotMet is raised if any of the following preconditions is violated:</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * &lt;ul>&lt;li>The List is not located in a (writeable) CacheAccess;&lt;/li></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * &lt;li>The List belongs to an ObjectRoot which is not yet registered.&lt;/li></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * &lt;li>The specified key value represents an index larger then the current length of the list.&lt;/li>&lt;/ul></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @param key the index that will identify the specified item.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @param value the item that needs to be stored in the List.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     * @throws DDS::AlreadyDeleted if the owner of the List has already been deleted.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    virtual void</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>    put(</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>        </xsl:text>
        <xsl:call-template name="get_corba_module_name"/>
        <xsl:text>::Long key,</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>        </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>* value) THROW_ORB_AND_USER_EXCEPTIONS (</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>            DDS::PreconditionNotMet,</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>            DDS::AlreadyDeleted);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate FooList_impl (continued):
            };
        -->
        <xsl:value-of select="$whiteSpace"/>
        <xsl:text>};</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>
</xsl:stylesheet>