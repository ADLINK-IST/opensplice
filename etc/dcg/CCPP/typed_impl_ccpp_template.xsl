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
            <xsl:text>_impl.cpp</xsl:text>
        </xsl:variable>
        <redirect:write file="{$output.dir}/{$filename}">
            <FOODLRL-out>
                <xsl:call-template name="copyright-notice"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:for-each select="VALUEDEF">
                    <xsl:call-template name="VALUEDEF"/>
                </xsl:for-each>
                <xsl:for-each select="MODULE">
                    <xsl:call-template name="MODULE"/>
                </xsl:for-each>
                <xsl:value-of select="$NL"/>
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


            <xsl:text>/****** Generated for type: </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> ******/</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate Foo_impl (continued):
                    test::Foo_impl::Foo_impl(){}
                    test::Foo_impl::~Foo_impl(){}

            -->
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>_impl::</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>_impl(){}</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>_impl::</xsl:text>
            <xsl:text>~</xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>_impl(){}</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
<!-- TODO CPP003: build in support for local attributes and operations -->

            <xsl:value-of select="$NL"/>
        </xsl:if>
    </xsl:template>

</xsl:stylesheet>