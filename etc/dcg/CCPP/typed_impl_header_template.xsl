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
            <xsl:text>_impl.h</xsl:text>
        </xsl:variable>
        <redirect:write file="{$output.dir}/{$filename}">
            <FOODLRL-out>
                <xsl:call-template name="copyright-notice"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#ifndef CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_IMPL_H</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#define CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_IMPL_H</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_abstract.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "DLRL_Report.h"</xsl:text>
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
                <xsl:text>#endif /* CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_IMPL_H */</xsl:text>
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
         thats a TODO for now. -->
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
            <xsl:variable name="prefixedFullTopicTypeName">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="mainTopic/@typename"/>
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
            <xsl:variable name="prefixedNamespace">
                <xsl:call-template name="string-search-replace-except-last">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="prefixedFullNameUnderscores">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'_'"/>
                    <xsl:with-param name="prefixKeywords" select="'no'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="prefixedFullNameExceptLast">
                <xsl:value-of select="$prefixedNamespace"/>
                <xsl:text>::</xsl:text>
                <xsl:value-of select="$nonPrefixedName"/>
            </xsl:variable>


            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>/****** Generated for type: </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> ******/</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate Foo_impl :
                class Foo_impl :
                    public test::Foo_abstract
                {

            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>class </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>_impl :</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    public </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>_abstract</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>{</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate Foo_impl (continued):
                friend DDS::ObjectRoot_impl*
                    test::ccpp_test_Foo_us_createTypedObject (
                        DLRL_Exception* exception);
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    friend DDS::ObjectRoot_impl*</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$prefixedNamespace"/>
            <xsl:text>::ccpp_</xsl:text>
            <xsl:value-of select="$prefixedFullNameUnderscores"/>
            <xsl:text>_us_createTypedObject (</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:text>            DLRL_Exception* exception);</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <!-- Generate Foo_impl (continued):
                private:
                    Foo_impl();
                    virtual ~Foo_impl();

            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    private:</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>_impl();</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>        virtual ~</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>_impl();</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
<!-- TODO CPP003 build in support for local attributes and operations -->
            <!--xsl:for-each select="STATEMEMBER">
                <xsl:call-template name="STATEMEMBER">
                    <xsl:with-param name="white-space">
                        <xsl:text>    </xsl:text>
                        <xsl:value-of select="$whiteSpace"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:template>

            <xsl:for-each select="OPERATION">
                <xsl:call-template name="OPERATION">
                    <xsl:with-param name="white-space">
                        <xsl:text>    </xsl:text>
                        <xsl:value-of select="$whiteSpace"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:for-each-->
            <!-- Generate Foo_impl :
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
        </xsl:if>
    </xsl:template>

    <xsl:template name="OPERATION">
        <xsl:param name="white-space"/>

        <xsl:value-of select="$white-space"/>
        <xsl:text>public:</xsl:text>
        <xsl:value-of select="$NL"/>

        <!-- return type -->
        <xsl:value-of select="$white-space"/>
        <xsl:call-template name="ccpp-type"/>
        <xsl:text> </xsl:text>

        <!-- operation name type -->
        <xsl:call-template name="ccpp-name">
            <xsl:with-param name="name" select="@NAME"/>
        </xsl:call-template>

        <!-- parameters -->
        <xsl:text>(</xsl:text>
        <xsl:apply-templates select="PARAMETER" />
        <xsl:text>)</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>    /* TODO -- add implementation */</xsl:text><!-- this todo is meant for the user of the DLRL -->
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template match="PARAMETER">
     <!--   <xsl:call-template name="ccpp-param-type"/>
        <xsl:text> </xsl:text>
        <xsl:call-template name="ccpp-name">
            <xsl:with-param name="name" select="@NAME"/>
        </xsl:call-template>
        <xsl:if test="position() &lt; last()">
            <xsl:text>, </xsl:text>
        </xsl:if>
        -->
    </xsl:template>

    <xsl:template name="STATEMEMBER">
        <xsl:param name="white-space"/>

        <!-- is it a local attribute? -->
        <xsl:if test="not (valueField) and not(multiPlaceTopic) and not(keyDescription)">
            <xsl:variable name="visibility">
                <xsl:call-template name="statemember-visibility"/>
            </xsl:variable>
            <xsl:variable name="statememberName" select="DECLARATOR/@NAME"/>
            <xsl:variable name="statememberNamePrefixed">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="DECLARATOR/@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="statememberType">
                <xsl:call-template name="ccpp-type"/>
            </xsl:variable>

            <!-- Generate Foo_impl (continued). For example statemember 'public long myX' :
                public:
            -->
            <xsl:value-of select="$white-space"/>
            <xsl:text>    /* generated for attribute </xsl:text>
            <xsl:value-of select="$statememberName"/>
            <xsl:text> */</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$white-space"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$visibility"/>
            <xsl:text>:</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:call-template name="generate-local-attribute">
                <xsl:with-param name="statememberNamePrefixed" select="$statememberNamePrefixed"/>
                <xsl:with-param name="statememberType" select="$statememberType"/>
                <xsl:with-param name="white-space">
                    <xsl:text>        </xsl:text>
                    <xsl:value-of select="$white-space"/>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>








    <xsl:template name="generate-local-attribute_ooold">
        <xsl:param name="statememberNamePrefixed"/>
        <xsl:param name="statememberType"/>
        <xsl:param name="white-space"/>

        <!-- Generate Foo_impl (continued). For example statemember 'public long x' :
                virtual CORBA::Long x(
                    );

        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>virtual </xsl:text>
        <xsl:value-of select="$statememberType"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="$statememberNamePrefixed"/>
        <xsl:text>(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>    );</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate Foo_impl (continued). For example statemember 'public long x' :
                virtual void x(
                    CORBA::Long x);

        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>virtual void </xsl:text>
        <xsl:value-of select="$statememberNamePrefixed"/>
        <xsl:text>(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$statememberType"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="$statememberNamePrefixed"/>
        <xsl:text>);</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="generate-local-attribute">
        <xsl:param name="statememberNamePrefixed"/>
        <xsl:param name="statememberType"/>
        <xsl:param name="white-space"/>

        <!-- Generate Foo_abstract (continued). For example statemember 'public long x' :
                virtual CORBA::Long x(
                    ) = 0;

        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>virtual </xsl:text>
        <xsl:value-of select="$statememberType"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="$statememberNamePrefixed"/>
        <xsl:text>(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>    ) = 0;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- Generate Foo_abstract (continued). For example statemember 'public long x' :
                virtual void x(
                    CORBA::Long x) = 0;

        -->
        <xsl:value-of select="$white-space"/>
        <xsl:text>virtual void </xsl:text>
        <xsl:value-of select="$statememberNamePrefixed"/>
        <xsl:text>(</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$white-space"/>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$statememberType"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="$statememberNamePrefixed"/>
        <xsl:text>) = 0;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>



</xsl:stylesheet>