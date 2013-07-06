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
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>SelectionListener</xsl:text></xsl:with-param>
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
    <xsl:variable name="selectionClassNameFullyQualified">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Selection</xsl:text></xsl:with-param>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'.'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
<xsl:value-of select="$NL"/>
<xsl:call-template name="copyright-notice" />
<xsl:value-of select="$NL"/>
<xsl:call-template name="package-name" />

/**
 * <P>This is a callback interface that must be implemented by the user.
 * When implemented, it can be attached to a corresponding
 * {@link <xsl:value-of select="$selectionClassNameFullyQualified"/>}, which will invoke the
 * appropriate callback methods when an object of type <xsl:value-of select="$classNameFullyQualified"/>,
 * or possibly one of its sub-classes enters or leaves the selection or when an
 * object contained in the selection is modified.</P>
 */
public interface <xsl:value-of select="$className"/>SelectionListener extends DDS.SelectionListener{

    /**
     * Will be invoked by the associated {@link <xsl:value-of select="$selectionClassNameFullyQualified"/>} when
     * an object of type <xsl:value-of select="$classNameFullyQualified"/>, or one of its sub-classes becomes a
     * member of this selection (i.e. an object is detected to meet the criteria of the selection for the first time).
     *
     * @param the_object the reference to the new Object member.
     */
    public void on_object_modified (<xsl:value-of select="$classNameFullyQualified"/> the_object);

    /**
     * Will be invoked by the associated {@link <xsl:value-of select="$selectionClassNameFullyQualified"/>} when
     * an object of type <xsl:value-of select="$classNameFullyQualified"/>, or one of its sub-classes which already
     * was a member of the associated Selection has been modified in the update round (i.e. an object has been modified but
     * still meets the criteria of the selection).
     *
     * @param the_object the reference to the modified Object.
     */
    public void on_object_in (<xsl:value-of select="$classNameFullyQualified"/> the_object);

    /**
     * Will be invoked by the associated {@link <xsl:value-of select="$selectionClassNameFullyQualified"/>} when
     * an object of type <xsl:value-of select="$classNameFullyQualified"/>, or one of its sub-classes leaves this
     * selection (i.e. no longer meets the criteria).
     *
     * @param the_object the reference to the deleted Object.
     * @return whether the event should be propagated to the parent class.
     */
    public void on_object_out (<xsl:value-of select="$classNameFullyQualified"/> the_object);

}

    </VALUEDEF-out>
    </redirect:write>
</xsl:if>
</xsl:template>
</xsl:stylesheet>