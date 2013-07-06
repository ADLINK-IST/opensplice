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
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Selection</xsl:text></xsl:with-param>
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
    <xsl:variable name="selectionListenerClassNameFullyQualified">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>SelectionListener</xsl:text></xsl:with-param>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'.'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="filterClassNameFullyQualified">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Filter</xsl:text></xsl:with-param>
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
 * This class is the typed implementation of the Selection interface.
 * It contains instances of type {@link <xsl:value-of select="$classNameFullyQualified"/>}
 * that match the {@link DDS.SelectionCriterion} provided at creation time
 * of the Selection.
 */
public final class <xsl:value-of select="$className"/>Selection extends org.opensplice.dds.dlrl.SelectionImpl{

    <xsl:value-of select="$className"/>Selection() {
        // prevent instantiation
    }

    /**
     * Returns the {@link <xsl:value-of select="$classNameFullyQualified"/>}
     * objects that are a part of the selection since the last time it was
     * refreshed. I.E. the last time the refresh operation of the selection
     * was called (@see DDS.Selection#auto_refresh() returns <code>false</code>)
     * or the last time the related {@link DDS.Cache} was refreshed
     * (@see DDS.Selectionauto_refresh() returns <code>true</code>).
     *
     * @throws DDS.AlreadyDeleted if the Selection is already deleted.
     * @return the DLRL objects that are members of this selection.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] members() throws DDS.AlreadyDeleted {
        return (<xsl:value-of select="$classNameFullyQualified"/>[]) jniMembers();
    }

    /*
     * Not supported
     *
     * @param listener
     * @throws DDS.AlreadyDeleted if the Selection is already deleted.
     * @return the previously attached listener or null if none
     */
    public <xsl:value-of select="$selectionListenerClassNameFullyQualified"/> set_listener(<xsl:value-of select="$selectionListenerClassNameFullyQualified"/> listener) throws DDS.AlreadyDeleted{
        return (<xsl:value-of select="$selectionListenerClassNameFullyQualified"/>)jniSetListener((DDS.SelectionListener)listener);
    }

    public <xsl:value-of select="$classNameFullyQualified"/>[] get_inserted_members()
    {
        return (<xsl:value-of select="$classNameFullyQualified"/>[])jniInsertedMembers();
    }

    public <xsl:value-of select="$classNameFullyQualified"/>[] get_modified_members()
    {
        return (<xsl:value-of select="$classNameFullyQualified"/>[])jniModifiedMembers();
    }

    public <xsl:value-of select="$classNameFullyQualified"/>[] get_removed_members()
    {
        return (<xsl:value-of select="$classNameFullyQualified"/>[])jniRemovedMembers();
    }

    /*
     * Not supported
     *
     * @throws DDS.AlreadyDeleted if the Selection is already deleted.
     * @return the currently attached listener
     */
    public <xsl:value-of select="$selectionListenerClassNameFullyQualified"/> listener() throws DDS.AlreadyDeleted{
        return (<xsl:value-of select="$selectionListenerClassNameFullyQualified"/>)jniListener();
    }

    // part of the algorithm used to determine which objects belong to a selection when doing a refresh
    // this code is located in the generated class for performance reasons.
    protected int[] checkObjects(DDS.FilterCriterion filter, DDS.ObjectRoot[] objects){
        int[] indexes = new int[objects.length+1]; //+1 to ensure the -1 can be added to indicate the end
        int position = 0;
        <xsl:value-of select="$filterClassNameFullyQualified"/> typedFilter = (<xsl:value-of select="$filterClassNameFullyQualified"/>) filter;
        for (int count = 0; count &lt; objects.length; count++) {
            <xsl:value-of select="$classNameFullyQualified"/> anObject = (<xsl:value-of select="$classNameFullyQualified"/>) objects[count];
            if (typedFilter.check_object(anObject, DDS.MembershipState.UNDEFINED_MEMBERSHIP)) {
                indexes[position] = count;
                position++;
            }
        }
        // -1 indicates the end of the indexes of the array
        indexes[position] = -1;
        return indexes;
    }
}
    </VALUEDEF-out>
    </redirect:write>
</xsl:if>
</xsl:template>
</xsl:stylesheet>