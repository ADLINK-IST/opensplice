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
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Set</xsl:text></xsl:with-param>
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
    <xsl:variable name="prefixedName">
        <xsl:call-template name="java-name">
            <xsl:with-param name="name" select="$className"/>
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
<xsl:value-of select="$NL"/>
<xsl:call-template name="copyright-notice" />
<xsl:value-of select="$NL"/>
<xsl:call-template name="package-name" />

/**
 * This class is the typed implementation for the abstract DDS.Set
 * class. It represents a Set that stores instances of a <xsl:value-of select="$prefixedName"/>
 * object, or one of its sub-classes. A set can not store the same instance
 * more than once.
 */
public final class <xsl:value-of select="$className"/>Set extends DDS.Set{

    //disallow instantiation of this class
    private <xsl:value-of select="$className"/>Set(){}

    /**
     * Stores the specified <xsl:value-of select="$classNameFullyQualified"/> item into the Set. If the item was already
     * contained in the Set, then this operation will have no effect.
     *
     * A PreconditionNotMet is raised if any of the following preconditions is violated:
     * &lt;ul>&lt;li>The Set is not located in a (writeable) CacheAccess;&lt;/li>
     * &lt;li>The Set belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul>
     *
     * @param value the item that needs to be stored in the Set.
     * @throws DDS.AlreadyDeleted if the owner of the Set has already been deleted.
     * @throws DDS.PreconditionNotMet if any of the preconditions where not met.
     */
    public void add(<xsl:value-of select="$classNameFullyQualified"/> value) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        jniAdd(value);
    }

    /**
     * Returns an Object array that contains all <xsl:value-of select="$classNameFullyQualified"/> elements that were
     * added during the last update round. It is recommended to use the values() operation instead of this
     * operation when dealing with a collection belonging to an ObjectRoot with read_state
     * OBJECT_NEW. In this case both lists will be equal and the values() operation will give
     * better performance. But only in the described case, in other situations it's
     * recommended to use this operation. When this collection belongs to an ObjectRoot with read_state
     * VOID then this operation will always return a zero length array.
     *
     * @return the <xsl:value-of select="$classNameFullyQualified"/>[] that contains all added <xsl:value-of select="$classNameFullyQualified"/> elements.
     * @throws DDS.AlreadyDeleted if the owner of the Set has already been deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] added_elements() throws DDS.AlreadyDeleted{
        return (<xsl:value-of select="$classNameFullyQualified"/>[])jniAddedElements();
    }

    /**
     * Returns whether the specified <xsl:value-of select="$classNameFullyQualified"/> element is already contained in
     * the Set (<code>true</code>) or not (<code>false</code>).
     *
     * @param value the item that needs to be examined.
     * @return whether the specified element is already contained in the Set.
     * @throws DDS.AlreadyDeleted if the owner of the Set has already been deleted.
     */
    public boolean contains(<xsl:value-of select="$classNameFullyQualified"/> value) throws DDS.AlreadyDeleted{
        return jniContains(value);
    }

    /**
     * Removes the specified <xsl:value-of select="$classNameFullyQualified"/> element from the Set. If the specified
     * element is not contained in the Set, then this operation will have
     * no effect.
     *
     * A PreconditionNotMet is raised if any of the following preconditions is violated:
     * &lt;ul>&lt;li>The Set is not located in a (writeable) CacheAccess;&lt;/li>
     * &lt;li>The Set belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul>
     *
     * @param value the item that needs to be removed.
     * @throws DDS.AlreadyDeleted if the owner of the Set has already been deleted.
     * @throws DDS.PreconditionNotMet if any of the preconditions where not met.
     */
    public void remove(<xsl:value-of select="$classNameFullyQualified"/> value) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        jniRemove(value);
    }

    /**
     * Returns an Object array that contains all <xsl:value-of select="$classNameFullyQualified"/> elements that are
     * currently contained in the Set.
     *
     * @return the <xsl:value-of select="$classNameFullyQualified"/>[] that contains all available <xsl:value-of select="$classNameFullyQualified"/> elements.
     * @throws DDS.AlreadyDeleted if the owner of the Set has already been deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] values() throws DDS.AlreadyDeleted{
        return (<xsl:value-of select="$classNameFullyQualified"/>[])jniGetValues();
    }

    /**
     * Returns an Object array that contains all <xsl:value-of select="$classNameFullyQualified"/> elements that were
     * removed during the last update round. When this collection belongs to an ObjectRoot with read_state
     * VOID or OBJECT_NEW then this operation will always return a zero length array.
     *
     * @return the <xsl:value-of select="$classNameFullyQualified"/>[] that contains all removed <xsl:value-of select="$classNameFullyQualified"/> elements.
     * @throws DDS.AlreadyDeleted if the owner of the Set has already been deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] removed_elements() throws DDS.AlreadyDeleted{
        return (<xsl:value-of select="$classNameFullyQualified"/>[])jniRemovedElements();
    }
}

    </VALUEDEF-out>
    </redirect:write>
</xsl:if>
</xsl:template>
</xsl:stylesheet>