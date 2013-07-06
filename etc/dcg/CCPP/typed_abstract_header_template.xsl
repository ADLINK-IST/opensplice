<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xalan="http://xml.apache.org/xalan"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
    <xsl:param name="output.dir" select="'.'"/>
     <xsl:include href="typed_abstract_common_template.xsl"/>

    <xsl:template match="IDL">
        <xsl:variable name="filename">
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="@baseFile"/>
            <xsl:text>_abstract.h</xsl:text>
        </xsl:variable>
        <redirect:write file="{$output.dir}/{$filename}">
            <FOODLRL-out>
                <xsl:call-template name="copyright-notice"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#ifndef CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_ABSTRACT_H</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#define CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_ABSTRACT_H</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>Dlrl_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:for-each select="VALUEDEF">
                    <xsl:call-template name="VALUEDEF">
                        <xsl:with-param name="white-space"/>
                    </xsl:call-template>
                </xsl:for-each>
                <xsl:for-each select="MODULE">
                    <xsl:call-template name="MODULE">
                        <xsl:with-param name="white-space"/>
                    </xsl:call-template>
                </xsl:for-each>
                <xsl:text>#endif /* CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_ABSTRACT_H */</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
            </FOODLRL-out>
        </redirect:write>
    </xsl:template>


    <xsl:template name="MODULE">
        <xsl:param name="white-space"/>

        <xsl:variable name="prefixedName">
            <xsl:call-template name="ccpp-name">
                <xsl:with-param name="name" select="@NAME"/>
            </xsl:call-template>
        </xsl:variable>
        <!-- must always generate the name spaces, even if the module doesn't contain any dlrl valuetypes
         because inner modules may contain dlrl valuetypes... we can make the xslt more intelligent here, but
         thats a TODO for now. -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>namespace </xsl:text>
        <xsl:value-of select="$prefixedName"/>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:for-each select="VALUEDEF">
            <xsl:call-template name="VALUEDEF">
                <xsl:with-param name="white-space">
                    <xsl:text>    </xsl:text>
                    <xsl:value-of select="$white-space"/>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:for-each>
        <xsl:for-each select="MODULE">
            <xsl:call-template name="MODULE">
                <xsl:with-param name="white-space">
                    <xsl:text>    </xsl:text>
                    <xsl:value-of select="$white-space"/>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:for-each>
        <xsl:value-of select="$white-space"/>
        <xsl:text>};</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>


    <xsl:template name="VALUEDEF">
        <xsl:param name="white-space"/>

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
            <xsl:variable name="prefixedFullTopicTypeName">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="mainTopic/@typename"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="prefixedFullNameExceptLast">
                <xsl:variable name="temp">
                    <xsl:call-template name="string-search-replace-except-last">
                        <xsl:with-param name="text" select="@NAME"/>
                        <xsl:with-param name="from" select="'::'"/>
                        <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                        <xsl:with-param name="prefixKeywords" select="'yes'"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:if test="string-length($temp)!=0">
                  <xsl:value-of select="$temp"/>
                  <xsl:text>::</xsl:text>
                </xsl:if>
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

            <xsl:call-template name="typedObject__class-begin">
                <xsl:with-param name="white-space" select="$white-space"/>
                <xsl:with-param name="prefixedFullName" select="$prefixedFullName"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="prefixedFullTopicTypeName" select="$prefixedFullTopicTypeName"/>
                <xsl:with-param name="fullNameWithUnderscores" select="$fullNameWithUnderscores"/>
                <xsl:with-param name="classNameFullyQualifiedExceptLast" select="$prefixedFullNameExceptLast"/>
            </xsl:call-template>
            <xsl:call-template name="typedObject__contructor">
                <xsl:with-param name="classNameFullyQualifiedExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="white-space">
                    <xsl:value-of select="$white-space"/>
                    <xsl:text>    </xsl:text>
                </xsl:with-param>
                <xsl:with-param name="mode">
                    <xsl:text>header</xsl:text>
                </xsl:with-param>
            </xsl:call-template>
            <xsl:call-template name="typedObject__destructor">
                <xsl:with-param name="classNameFullyQualifiedExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="white-space">
                    <xsl:value-of select="$white-space"/>
                    <xsl:text>    </xsl:text>
                </xsl:with-param>
                <xsl:with-param name="mode">
                    <xsl:text>header</xsl:text>
                </xsl:with-param>
            </xsl:call-template>
            <!-- generate all operations needed for (shared) statemember support i.e., get_x/set_x/is_x_modified -->
            <xsl:apply-templates select="STATEMEMBER" mode="accessors">
                <xsl:with-param name="mainTopicName" select="mainTopic/@name"/>
                <xsl:with-param name="prefixedFullNameExceptLast" select="$prefixedFullNameExceptLast"/>
                <xsl:with-param name="white-space">
                    <xsl:value-of select="$white-space"/>
                    <xsl:text>    </xsl:text>
                </xsl:with-param>
                <xsl:with-param name="mode">
                    <xsl:text>header</xsl:text>
                </xsl:with-param>
            </xsl:apply-templates>
            <xsl:call-template name="typedObject__class-end">
                <xsl:with-param name="white-space" select="$white-space"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <xsl:template name="typedObject__class-end">
        <xsl:param name="white-space"/>

        <!-- Generate Foo_abstract :
            };
        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>};</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="typedObject__class-begin">
        <xsl:param name="white-space"/>
        <xsl:param name="prefixedFullName"/>
        <xsl:param name="nonPrefixedName"/>
        <xsl:param name="prefixedFullTopicTypeName"/>
        <xsl:param name="prefixedNamespace"/>
        <xsl:param name="fullNameWithUnderscores"/>
        <xsl:param name="classNameFullyQualifiedExceptLast"/>

        <xsl:value-of select="$white-space"/>
        <xsl:text>/****** Generated for type: </xsl:text>
        <xsl:value-of select="@NAME"/>
        <xsl:text> ******/</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generates the following code for valuetype 'Foo' defined in module 'test'
        /**
         * This is generated code that represents an abstract application class.
         */
         -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>/**</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text> * This is generated code that represents an abstract application class.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text> * DO NOT edit this code, instead implement missing method implementations</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text> * in the corresponding {@link </xsl:text>
        <xsl:value-of select="$classNameFullyQualifiedExceptLast"/>
        <xsl:text>_impl} class.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text> */</xsl:text>
        <xsl:value-of select="$NL"/>
        <!-- Generate Foo_abstract :

            class Foo_abstract :
                public virtual test::Foo,
                public DDS::ObjectRoot_impl
            {

        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>class </xsl:text>
        <xsl:value-of select="$nonPrefixedName"/>
        <xsl:text>_abstract :</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$white-space"/>
        <xsl:text>    public virtual </xsl:text>
        <xsl:value-of select="$prefixedFullName"/>
        <xsl:text>,</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$white-space"/>
        <xsl:text>    public DDS::ObjectRoot_impl</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$white-space"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate typed friend functions
        friend void
            (ccpp_CountrySimDLRL_Country_us_setPreviousTopic) (
                DLRL_LS_object lsObject,
                DLRL_LS_object lsTopic);
        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>    friend void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>        (ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_setPreviousTopic) (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object lsObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object lsTopic);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate typed friend functions
        friend DLRL_LS_object
            ccpp_CountrySimDLRL_Country_us_getPreviousTopic (
                DLRL_LS_object lsObject);
        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>    friend DLRL_LS_object</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>        (ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_getPreviousTopic) (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate typed friend functions
        friend void
            ccpp_CountrySimDLRL_Country_us_setCurrentTopic (
                DLRL_LS_object lsObject,
                DLRL_LS_object lsTopic);
        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>    friend void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>        (ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_setCurrentTopic) (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object lsObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object lsTopic);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate typed friend functions
        friend DLRL_LS_object
            ccpp_CountrySimDLRL_Country_us_getCurrentTopic (
                DLRL_LS_object lsObject);
        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>    friend DLRL_LS_object</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>        (ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_getCurrentTopic) (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object lsObject);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate typed friend functions
        friend void
            (ccpp_test_Foo_us_changeRelations)(
                DLRL_LS_object ownerObject,
                DLRL_LS_object relationObject,
                LOC_unsigned_long index);
        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>    friend void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>        (ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_changeRelations) (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object ownerObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object relationObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            LOC_unsigned_long index);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate typed friend functions
        friend void
            (ccpp_test_Foo_us_setCollections)(
                DLRL_LS_object ownerObject,
                DLRL_LS_object collectionObject,
                LOC_unsigned_long index);
        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>    friend void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>        (ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_setCollections) (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object ownerObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object collectionObject,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            LOC_unsigned_long index);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate typed friend functions
        friend void
            (ccpp_test_Foo_us_clearLSObjectAdministration)(
                DLRL_LS_object ls_object);
        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>    friend void</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>        (ccpp_</xsl:text>
        <xsl:value-of select="$fullNameWithUnderscores"/>
        <xsl:text>_us_clearLSObjectAdministration) (</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>            DLRL_LS_object ls_object);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <!-- Generate Foo_abstract (continued):
            private:
                test::FooTopic* currentTopic;
                test::FooTopic* previousTopic;

            and for each collection:
                test:FooSet* fooSet;
        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>    private:</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$white-space"/>
        <xsl:text>        </xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>* currentTopic;</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:value-of select="$white-space"/>
        <xsl:text>        </xsl:text>
        <xsl:value-of select="$prefixedFullTopicTypeName"/>
        <xsl:text>* previousTopic;</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:for-each select="STATEMEMBER">
            <xsl:choose>
                <xsl:when test="keyDescription and not(multiPlaceTopic)">
                    <xsl:variable name="prefixedAttributeName">
                        <xsl:call-template name="ccpp-name">
                            <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="returnType">
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

                    <xsl:value-of select="$white-space"/>
                    <xsl:text>        </xsl:text>
                    <xsl:value-of select="$returnType"/>
                    <xsl:text>* </xsl:text>
                    <xsl:value-of select="$prefixedAttributeName"/>
                    <xsl:text>;</xsl:text>
                    <xsl:value-of select="$NL"/>
                </xsl:when>
                <xsl:when test="multiPlaceTopic">
                    <xsl:variable name="prefixedAttributeName">
                        <xsl:call-template name="ccpp-name">
                            <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="type">
                        <xsl:call-template name="resolveStatememberIdlType"/>
                    </xsl:variable>

                    <xsl:for-each select="//VALUEFORWARDDEF[@NAME=$type]">
                        <xsl:variable name="returnType">
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

                        <xsl:value-of select="$white-space"/>
                        <xsl:text>        </xsl:text>
                        <xsl:value-of select="$returnType"/>
                        <xsl:text>* </xsl:text>
                        <xsl:value-of select="$prefixedAttributeName"/>
                        <xsl:text>;</xsl:text>
                        <xsl:value-of select="$NL"/>
                    </xsl:for-each>
                </xsl:when>
            </xsl:choose>
        </xsl:for-each>

        <xsl:value-of select="$NL"/>
    </xsl:template>

</xsl:stylesheet>
