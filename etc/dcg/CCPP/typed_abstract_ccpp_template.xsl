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
            <xsl:text>_abstract.cpp</xsl:text>
        </xsl:variable>
        <redirect:write file="{$output.dir}/{$filename}">
            <FOODLRL-out>
                <xsl:call-template name="copyright-notice"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_abstract.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "DLRL_Report.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "DLRL_Kernel.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:for-each select="VALUEDEF">
                    <xsl:call-template name="process-valuetype"/>
                </xsl:for-each>
                <xsl:for-each select="MODULE">
                    <xsl:call-template name="MODULE"/>
                </xsl:for-each>
            </FOODLRL-out>
        </redirect:write>
    </xsl:template>

    <xsl:template name="MODULE">
        <xsl:for-each select="VALUEDEF">
            <xsl:call-template name="process-valuetype"/>
        </xsl:for-each>
        <xsl:for-each select="MODULE">
            <xsl:call-template name="MODULE"/>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="process-valuetype">
        <xsl:variable name="isNonIncludedSharedValueDef">
            <xsl:call-template name="does-valuedef-inherit-from-objectroot-and-is-not-included"/>
        </xsl:variable>

        <!-- only need to do stuff for valuetypes that are shared through dlrl -->
        <xsl:if test="$isNonIncludedSharedValueDef='true'">
            <xsl:variable name="nonPrefixedName">
                <xsl:call-template name="string-return-text-after-last-token">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="token" select="'::'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="classNameFullyQualifiedExceptLast">
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


            <xsl:text>/****** Generated for type: </xsl:text>
            <xsl:value-of select="@NAME"/>
            <xsl:text> ******/</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:call-template name="typedObject__contructor">
                <xsl:with-param name="classNameFullyQualifiedExceptLast" select="$classNameFullyQualifiedExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="white-space"/>
                <xsl:with-param name="mode">
                    <xsl:text>ccpp</xsl:text>
                </xsl:with-param>
            </xsl:call-template>
            <xsl:call-template name="typedObject__destructor">
                <xsl:with-param name="classNameFullyQualifiedExceptLast" select="$classNameFullyQualifiedExceptLast"/>
                <xsl:with-param name="nonPrefixedName" select="$nonPrefixedName"/>
                <xsl:with-param name="white-space"/>
                <xsl:with-param name="mode">
                    <xsl:text>ccpp</xsl:text>
                </xsl:with-param>
            </xsl:call-template>
            <!-- generate all operations needed for (shared) statemember support i.e., get_x/set_x/is_x_modified -->
            <xsl:apply-templates select="STATEMEMBER" mode="accessors">
                <xsl:with-param name="mainTopicName" select="mainTopic/@name"/>
                <xsl:with-param name="classNameFullyQualifiedExceptLast" select="$classNameFullyQualifiedExceptLast"/>
                <xsl:with-param name="white-space"/>
                <xsl:with-param name="mode">
                    <xsl:text>ccpp</xsl:text>
                </xsl:with-param>
            </xsl:apply-templates>
        </xsl:if>
    </xsl:template>

</xsl:stylesheet>
