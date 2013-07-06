<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
<xsl:param name="output.dir" select="'.'"/>
<xsl:include href="typed_java_common.xsl"/>
<xsl:template match="ENUM">
    <xsl:if test="@fromIncludedIdl='false'">
    <!-- redirected -->
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
    <ENUM-out>
    <xsl:variable name="enum_name">
        <xsl:call-template name="string-return-text-after-last-token">
            <xsl:with-param name="text" select="@NAME"/>
            <xsl:with-param name="token" select="'::'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="enum_nameFQ">
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

public class <xsl:value-of select="$enum_name"/> {
	private int __value;
    <xsl:for-each select="ENUMMEMBER"><xsl:if test="position()=last()">
	private static int __size = <xsl:value-of select="position()"/>;
        </xsl:if></xsl:for-each>
	private static <xsl:value-of select="$enum_nameFQ"/>[] __array = new <xsl:value-of select="$enum_nameFQ"/> [__size];
    <xsl:for-each select="ENUMMEMBER">
    public static final int _<xsl:value-of select="@NAME"/> = <xsl:value-of select="position()-1"/>;

    public static final <xsl:value-of select="$enum_nameFQ"/><xsl:text> </xsl:text><xsl:value-of select="@NAME"/> = new <xsl:value-of select="$enum_nameFQ"/>(_<xsl:value-of select="@NAME"/>);
    </xsl:for-each>
    public int value() {
		return __value;
    }

    public static <xsl:value-of select="$enum_nameFQ"/> from_int(int value){
		if (value &gt;= 0 &amp;&amp; value &lt; __size){
			return __array[value];
		} else {
			throw new java.lang.RuntimeException();
		}
    }

    // constructor
    protected <xsl:value-of select="$enum_name"/>(int value){
		__value = value;
		__array[__value] = this;
    }
}
    </ENUM-out>
    </redirect:write>
    </xsl:if>
</xsl:template>
</xsl:stylesheet>