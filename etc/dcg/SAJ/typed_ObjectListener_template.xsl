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
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Listener</xsl:text></xsl:with-param>
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
    <xsl:variable name="homeclassNameFullyQualified">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Home</xsl:text></xsl:with-param>
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
 * When implemented, it can be attached to the corresponding
 * {@link <xsl:value-of select="$homeclassNameFullyQualified"/>}, which will invoke the
 * appropriate callback methods on creation, modification and deletion
 * of DLRL Objects of type <xsl:value-of select="$classNameFullyQualified"/>, or possibly one
 * of its sub-classes, by the DCPS. Changes introduced by directly
 * modifying objects locally will not trigger any Listeners. </P>
 * <P>In case of an event for an instance whose class inherits from other
 * classes, first the appropriate Listener method on its most specialized
 * class will be invoked. Furthermore, each of the available callback methods
 * must return a boolean. When it returns <code>true</code>, it means that
 * the event has been fully taken into account and therefore does not need
 * to be propagated to other ObjectListener objects of parent classes. When
 * it returns <code>false</code>, the event will be propagated to the
 * ObjectListener objects attached to the ObjectHome of its parent class.</P>
 */
public interface <xsl:value-of select="$className"/>Listener extends DDS.ObjectListener{

    /**
     * Will be invoked by the {@link <xsl:value-of select="$homeclassNameFullyQualified"/>} on the
     * creation of a new object of type <xsl:value-of select="$classNameFullyQualified"/>, or one of its sub-classes.
     *
     * @param the_object the reference to the newly created Object.
     * @return whether the event should be propagated to the parent class.
     */
    public boolean on_object_modified (<xsl:value-of select="$classNameFullyQualified"/> the_object);

    /**
     * Will be invoked by the {@link <xsl:value-of select="$homeclassNameFullyQualified"/>} on each
     * modification on an object of type <xsl:value-of select="$classNameFullyQualified"/>, or one of its sub-classes.
     *
     * @param the_object the reference to the modified Object.
     * @return whether the event should be propagated to the parent class.
     */
    public boolean on_object_created (<xsl:value-of select="$classNameFullyQualified"/> the_object);

    /**
     * Will be invoked by the {@link <xsl:value-of select="$homeclassNameFullyQualified"/>} on the
     * deletion of an object of type <xsl:value-of select="$classNameFullyQualified"/>, or one of its sub-classes.
     *
     * @param the_object the reference to the deleted Object.
     * @return whether the event should be propagated to the parent class.
     */
    public boolean on_object_deleted (<xsl:value-of select="$classNameFullyQualified"/> the_object);

}

    </VALUEDEF-out>
    </redirect:write>
</xsl:if>
</xsl:template>
</xsl:stylesheet>