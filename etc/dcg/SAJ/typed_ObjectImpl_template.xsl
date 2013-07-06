<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
<xsl:param name="output.dir" select="'.'"/>
<xsl:include href="typed_java_common.xsl"/>
<xsl:template match="VALUEDEF">
<xsl:variable name="isNonIncludedSharedValueDef">
    <xsl:call-template name="does-valuedef-inherit-from-objectroot-and-is-not-included"/>
</xsl:variable>
<xsl:if test="$isNonIncludedSharedValueDef='true'">
    <!-- redirected -->
    <xsl:variable name="filename">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Impl</xsl:text></xsl:with-param>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'/'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
        <xsl:text>.java</xsl:text>
    </xsl:variable>
    <redirect:write file="{$output.dir}/{$filename}">
    <VALUEDEF-out>
    <xsl:variable name="className">
        <xsl:call-template name="string-return-text-after-last-token">
            <xsl:with-param name="text" select="@NAME"/>
            <xsl:with-param name="token" select="'::'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="classNameFullyQualified">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text" select="@NAME"/>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'.'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="homeClassNameFullyQualified">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Home</xsl:text></xsl:with-param>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'.'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
<xsl:value-of select="$NL"/>
<xsl:call-template name="package-name"/>

/**
 * This is template code that represents the implementation for the
 * corresponding {@link <xsl:value-of select="$classNameFullyQualified"/>} class. All methods
 * that are not implemented by that class, MUST be implemented in
 * this class.
 * Be advised that this class does NOT have a public constructor
 * and may only be instantiated by the middleware, or by using the
 * <code>create_object</code> operation on the corresponding
 * {@link <xsl:value-of select="$homeClassNameFullyQualified"/>} in a writeable CacheAccess.
 */
public final class <xsl:value-of select="$className"/>Impl extends <xsl:value-of select="$classNameFullyQualified"/> {

    //disallow instantiation of this class
    protected <xsl:value-of select="$className"/>Impl(){}
    <xsl:apply-templates select="STATEMEMBER|OPERATION" />
}
    </VALUEDEF-out>
    </redirect:write>
</xsl:if>
</xsl:template>

    <xsl:template match="STATEMEMBER">
        <xsl:choose>
        <xsl:when test="valueField" />
        <xsl:when test="keyDescription" />
        <!-- now it is not a local attribute: NB local is missing in local classes -->
        <xsl:otherwise>
            <xsl:variable name="arrayBrackets">
                <xsl:call-template name="get-array-brackets">
                    <xsl:with-param name="action">bracket-only</xsl:with-param>
                    <xsl:with-param name="userData">0</xsl:with-param>
                </xsl:call-template>
            </xsl:variable>
    /**
     * The 'local' attribute <xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template>
     */
    private <xsl:call-template name="java-type" /><xsl:value-of select="$arrayBrackets"/>
        <xsl:text> </xsl:text>
        <xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template>
        <xsl:text>;</xsl:text>

    /**
     * @see <xsl:value-of select="ancestor::MODULE/@NAME" />
        <xsl:text>#</xsl:text>
        <xsl:call-template name="java-type" />
        <xsl:text> </xsl:text>
        <xsl:call-template name="get-array-brackets">
            <xsl:with-param name="action">bracket-only</xsl:with-param>
            <xsl:with-param name="userData">0</xsl:with-param>
        </xsl:call-template>
        <xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template>()
     */
    <xsl:call-template name="statemember-visibility" />
    <xsl:call-template name="java-type" />
    <xsl:call-template name="get-array-brackets">
        <xsl:with-param name="action">bracket-only</xsl:with-param>
        <xsl:with-param name="userData">0</xsl:with-param>
    </xsl:call-template>
    <xsl:text> </xsl:text>
    <xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template>() {
        return this.<xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template>;
    }

    /**
     * @see <xsl:value-of select="ancestor::MODULE/@NAME" />
        <xsl:text>#void </xsl:text>
        <xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template>
        <xsl:text>(</xsl:text>
        <xsl:call-template name="java-type" />
        <xsl:call-template name="get-array-brackets">
            <xsl:with-param name="action">bracket-only</xsl:with-param>
            <xsl:with-param name="userData">0</xsl:with-param>
        </xsl:call-template><xsl:text>)</xsl:text>
     */
    <xsl:call-template name="statemember-visibility" />
    <xsl:text>void </xsl:text>
    <xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template>
    <xsl:text>(</xsl:text>
    <xsl:call-template name="java-type" />
    <xsl:call-template name="get-array-brackets">
        <xsl:with-param name="action">bracket-only</xsl:with-param>
        <xsl:with-param name="userData">0</xsl:with-param>
    </xsl:call-template>
    <xsl:text> </xsl:text>
    <xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template>) {
        this.<xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template> = <xsl:call-template name="java-name"><xsl:with-param name="name" select="DECLARATOR/@NAME"/></xsl:call-template>;
    }
    </xsl:otherwise>
    </xsl:choose>
    </xsl:template>

    <xsl:template match="OPERATION">
    <xsl:call-template name="operation-signature" /> {
        // TODO - implement operation body
        <xsl:call-template name="default-return" />
    }

    </xsl:template>
</xsl:stylesheet>