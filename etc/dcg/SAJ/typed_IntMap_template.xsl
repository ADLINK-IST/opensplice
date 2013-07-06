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
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>IntMap</xsl:text></xsl:with-param>
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
 * This class is the typed implementation for the abstract DDS.IntMap
 * class. It represents a Map that stores key-value pairs in which the key
 * represents an integer and the value represent a <xsl:value-of select="$classNameFullyQualified"/>
 * object, or one of its sub-classes.
 */
public final class <xsl:value-of select="$className"/>IntMap extends DDS.IntMap{

    //disallow instantiation of this class
    private <xsl:value-of select="$className"/>IntMap(){}

    /**
     * Returns an Object array that contains all <xsl:value-of select="$classNameFullyQualified"/> elements that are
     * currently stored in the Map.
     *
     * @return the <xsl:value-of select="$classNameFullyQualified"/>[] that contains all available elements.
     * @throws DDS.AlreadyDeleted if the owner of the Map has already been deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] values() throws DDS.AlreadyDeleted{
        return (<xsl:value-of select="$classNameFullyQualified"/>[])jniGetValues();
    }

    /**
     * Retrieves a <xsl:value-of select="$classNameFullyQualified"/> item from the Map, based on its key.
     *
     * @param key the key that identifies the item that is to be retrieved.
     * @return the item that corresponds to the specified key (may be null)
     * or null if no element can be found for the specified key
     * @throws DDS.AlreadyDeleted if the owner of the Map has already been deleted.
     * @throws DDS.NoSuchElement if is no <xsl:value-of select="$classNameFullyQualified"/> element present that matches the specified key
     */
    public <xsl:value-of select="$classNameFullyQualified"/> get(int key) throws DDS.AlreadyDeleted, DDS.NoSuchElement{
        return (<xsl:value-of select="$classNameFullyQualified"/>)jniGet(key);
    }

    /**
     * Stores the specified <xsl:value-of select="$classNameFullyQualified"/> item into the Map, using the specified key
     * as its identifier. If the key already represented another item in the
     * Map, then that item will be replaced by the currently specified item.
     *
     * A PreconditionNotMet is raised if any of the following preconditions is violated:
     * &lt;ul>&lt;li>The Map is not located in a (writeable) CacheAccess;&lt;/li>
     * &lt;li>The Map belongs to an ObjectRoot which is not yet registered.&lt;/li>&lt;/ul>
     *
     * @param key the key that will identify the specified item.
     * @param value the item that needs to be stored in the Map.
     * @throws DDS.AlreadyDeleted if the owner of the Map has already been deleted.
     * @throws DDS.PreconditionNotMet if any of the preconditions where not met.
     */
    public void put(int key, <xsl:value-of select="$classNameFullyQualified"/> value) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        jniPut(key, value);
    }
}

    </VALUEDEF-out>
    </redirect:write>
</xsl:if>
</xsl:template>
</xsl:stylesheet>