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
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Filter</xsl:text></xsl:with-param>
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
            <xsl:with-param name="text"><value-of select="@NAME"/>Selection</xsl:with-param>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'.'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
<xsl:value-of select="$NL"/>
<xsl:call-template name="copyright-notice" />
<xsl:value-of select="$NL"/>
<xsl:call-template name="package-name"/>
/**
 * <P>This is a callback abstract class that must be implemented by the user.
 * When implemented, it can be attached to the corresponding
 * {@link <xsl:value-of select="$selectionClassNameFullyQualified"/>} upon creation, which will
 * invoke the check_object() callback method to determine wether or not an
 * object passed the criteria as defined by the check_object callback algorithm.
 * This callback operation will also be called for an potential sub-classes if available</P>
 */
public abstract class <xsl:value-of select="$className"/>Filter extends org.opensplice.dds.dlrl.FilterCriterionImpl{

    /**
     * <P>This callback function will be invoked when this filter is attached to
     * a selection and the selection is being refreshed (either DLRL invoked or
     * application invoked). The return value of this operation will determine
     * wether or not the object becomes a part of the selection.</P>
     *
     * @param an_object the object to evaluate if it matches the filter criteria
     * @param membership_state An enum indicating the relation of the to-be-evaluated
     * object to the selection this filter belongs to. Will always be UNDEFINED.
     * @return <code>true</code> if the object should become a part of the
     * selection and <code>false</code> if the object should not become
     * part of the selection.
     */
    public abstract boolean check_object(<xsl:value-of select="$classNameFullyQualified"/> an_object, DDS.MembershipState membership_state);
}
    </VALUEDEF-out>
    </redirect:write>
</xsl:if>
</xsl:template>
</xsl:stylesheet>