<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
<xsl:param name="output.dir" select="'.'"/>
<xsl:include href="typed_java_common.xsl"/>
<xsl:template match="VALUEDEF"><!-- redirected -->
<xsl:variable name="isNonIncludedSharedValueDef">
    <xsl:call-template name="does-valuedef-inherit-from-objectroot-and-is-not-included"/>
</xsl:variable>
<xsl:if test="$isNonIncludedSharedValueDef='true'">
    <xsl:variable name="filename">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>List</xsl:text></xsl:with-param>
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
<xsl:value-of select="$NL"/>
<xsl:call-template name="copyright-notice" />
<xsl:value-of select="$NL"/>
<xsl:call-template name="package-name" />

/**
 * This class is currently not supported.
 *
 * This class is the typed implementation for the abstract DDS.List
 * class. It represents a linked list that stores elements of
 * type <xsl:value-of select="$classNameFullyQualified"/> object, or one of its sub-classes.
 */
public final class <xsl:value-of select="$className"/>List extends DDS.List{

    //disallow instantiation of this class
    private <xsl:value-of select="$className"/>List(){}

    /**
     * Returns an Object array that contains all <xsl:value-of select="$classNameFullyQualified"/> elements that are
     * currently stored in the List.
     *
     * @return the <xsl:value-of select="$classNameFullyQualified"/>[] that contains all available elements.
     * @throws DDS.AlreadyDeleted if the owner of the List has already been deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] values() throws DDS.AlreadyDeleted{
        return (<xsl:value-of select="$classNameFullyQualified"/>[])jniGetValues();
    }

    /**
     * Retrieves a <xsl:value-of select="$classNameFullyQualified"/> item from the List, based on its index.
     *
     * @param key the index that identifies the item that is to be retrieved.
     * @return the item that corresponds to the specified index (may be null)
     * or null if no element can be found for the specified index
     * @throws DDS.AlreadyDeleted if the owner of the List has already been deleted.
     * @throws DDS.NoSuchElement if is no <xsl:value-of select="$classNameFullyQualified"/> element present that matches the specified index
     */
    public <xsl:value-of select="$classNameFullyQualified"/> get(int key) throws DDS.AlreadyDeleted, DDS.NoSuchElement{
        return (<xsl:value-of select="$classNameFullyQualified"/>)jniGet(key);
    }

    /**
     * Stores the specified <xsl:value-of select="$classNameFullyQualified"/> item into the List, using the length() as index.
     * i.e. stores the element at the end of the list.
     *
     * A PreconditionNotMet is raised if any of the following preconditions is violated:
     * &lt;ul>&lt;li>The List is not located in a (writeable) CacheAccess;&lt;/li>
     * &lt;li>The List belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul>
     *
     * @param key the index that will identify the specified item.
     * @param value the item that needs to be stored in the List.
     * @throws DDS.AlreadyDeleted if the owner of the List has already been deleted.
     */
    public void add(<xsl:value-of select="$classNameFullyQualified"/> value) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted{
        jniAdd(value);
    }

    /**
     * Stores the specified <xsl:value-of select="$classNameFullyQualified"/> item into the List, using the specified index
     * as its identifier. If the index already represented another item in the
     * List, then that item will be replaced by the currently specified item. It is only allowed to replace
     * already contained elements in the list or to add an element at the end of the list (the index may not be larger then
     * the length of the list).
     *
     * A PreconditionNotMet is raised if any of the following preconditions is violated:
     * &lt;ul>&lt;li>The List is not located in a (writeable) CacheAccess;&lt;/li>
     * &lt;li>The List belongs to an ObjectRoot which is not yet registered.&lt;/li>
     * &lt;li>The specified key value represents an index larger then the current length of the list.&lt;/li>&lt;/ul>
     *
     * @param key the index that will identify the specified item.
     * @param value the item that needs to be stored in the List.
     * @throws DDS.AlreadyDeleted if the owner of the List has already been deleted.
     */
    public void put(int key, <xsl:value-of select="$classNameFullyQualified"/> value) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted{
        jniPut(key, value);
    }
}

    </VALUEDEF-out>
    </redirect:write>
</xsl:if>
</xsl:template>
</xsl:stylesheet>