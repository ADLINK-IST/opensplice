<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
<xsl:param name="output.dir" select="'.'"/>
<xsl:include href="typed_java_common.xsl"/>
<xsl:template match="STRUCT">
    <xsl:if test="@fromIncludedIdl='false' and @FORWARD='false'"><!-- redirected -->
    <xsl:variable name="filename">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/></xsl:with-param>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'/'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
        <xsl:text>.java</xsl:text>
    </xsl:variable>
    <redirect:write file="{$output.dir}/{$filename}">
    <STRUCT-out>
    <xsl:variable name="struct_name">
        <xsl:call-template name="string-return-text-after-last-token">
            <xsl:with-param name="text" select="@NAME"/>
            <xsl:with-param name="token" select="'::'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="prefixedName">
        <xsl:call-template name="java-name">
            <xsl:with-param name="name" select="$struct_name"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="struct_nameFQ">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text" select="@NAME"/>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'.'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
<xsl:value-of select="$NL"/>
<xsl:call-template name="copyright-notice" />
<xsl:value-of select="$NL"/>
<xsl:call-template name="package-name"/>

public final class <xsl:value-of select="$prefixedName"/> implements Cloneable{

<xsl:for-each select="MEMBER">
    <xsl:call-template name="generate_struct_or_union_attribute_initialized">
        <xsl:with-param name="name">
            <xsl:call-template name="java-name">
                <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
            </xsl:call-template>
        </xsl:with-param>
        <xsl:with-param name="accessorType">
            <xsl:text>    public </xsl:text>
        </xsl:with-param>
    </xsl:call-template>
</xsl:for-each>
<xsl:value-of select="$NL"/>
<xsl:text>    public </xsl:text>
<xsl:value-of select="$prefixedName"/>
<xsl:text>(){</xsl:text>
<xsl:value-of select="$NL"/>
<xsl:for-each select="MEMBER">
    <xsl:variable name="name">
        <xsl:call-template name="java-name">
            <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:call-template name="generate_union_or_struct_constructor_content">
        <xsl:with-param name="name">
            <xsl:value-of select="$name"/>
        </xsl:with-param>
    </xsl:call-template>
</xsl:for-each>
<xsl:text>    }</xsl:text>
<xsl:value-of select="$NL"/>
<xsl:value-of select="$NL"/>
<xsl:value-of select="$NL"/>
<xsl:call-template name="struct-value-constructor">
    <xsl:with-param name="struct_name" select="$prefixedName"/>
</xsl:call-template>
<xsl:value-of select="$NL"/>

<xsl:call-template name="clone-operation">
    <xsl:with-param name="struct_nameFQ" select="$struct_nameFQ"/>
</xsl:call-template>
<xsl:text>}</xsl:text><xsl:value-of select="$NL"/>
    </STRUCT-out>
    </redirect:write>
    </xsl:if>
</xsl:template>

<xsl:template name="struct-value-constructor">
    <xsl:param name="struct_name"/>

    <xsl:text>    /* Constructor for all values defined within this struct */</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>    public </xsl:text><xsl:value-of select="$struct_name"/><xsl:text>(</xsl:text><xsl:call-template name="struct-value-constructor-parameters"/><xsl:text>){</xsl:text><xsl:value-of select="$NL"/>
        <xsl:for-each select="MEMBER">
            <xsl:variable name="memberName">
                <xsl:call-template name="java-name">
                    <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:text>        this.</xsl:text><xsl:value-of select="$memberName"/><xsl:text> = </xsl:text><xsl:value-of select="$memberName"/><xsl:text>;</xsl:text><xsl:value-of select="$NL"/>
        </xsl:for-each>
    <xsl:text>    }</xsl:text><xsl:value-of select="$NL"/>
</xsl:template>

<xsl:template name="struct-value-constructor-parameters">
    <xsl:for-each select="MEMBER">
        <xsl:variable name="arrayBrackets">
            <xsl:call-template name="get-array-brackets">
                <xsl:with-param name="action">bracket-only</xsl:with-param>
                <xsl:with-param name="userData">0</xsl:with-param>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="javaType">
            <xsl:call-template name="java-type"/>
        </xsl:variable>
        <xsl:variable name="memberName">
            <xsl:call-template name="java-name">
                <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:value-of select="$javaType"/><xsl:value-of select="$arrayBrackets"/><xsl:text> </xsl:text><xsl:value-of select="$memberName"/>
        <xsl:if test="position() != last()">
            <xsl:text>, </xsl:text>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

<xsl:template name="clone-operation">

    <xsl:param name="struct_nameFQ"/>
    <xsl:text>    /* Creates a deep clone of this struct </xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>     *</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>     * If one of the contained classes does not support cloning then null is returned.</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>     * Does not perform cycle detection.</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>     */</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>    public Object clone(){</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>        try{</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>            </xsl:text><xsl:value-of select="$struct_nameFQ"/><xsl:text> object = (</xsl:text><xsl:value-of select="$struct_nameFQ"/><xsl:text>)super.clone();</xsl:text><xsl:value-of select="$NL"/>
        <xsl:for-each select="MEMBER">
            <xsl:variable name="memberName">
                <xsl:call-template name="java-name">
                    <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:call-template name="struct-or-union-clone_generateMemberClone">
                <xsl:with-param name="memberName" select="$memberName"/>
                <xsl:with-param name="whiteSpaceStart"><xsl:text>            </xsl:text></xsl:with-param>
            </xsl:call-template>
        </xsl:for-each>
        <xsl:text>            return object;
        } catch (java.lang.CloneNotSupportedException e){
            return null;
        }
    }</xsl:text><xsl:value-of select="$NL"/>
</xsl:template>

</xsl:stylesheet>
