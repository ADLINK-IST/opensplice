<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xalan="http://xml.apache.org/xalan"
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
    <xsl:variable name="pathFullResolvedAndPrefixed">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text" select="@NAME"/>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'/'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="className">
        <xsl:call-template name="string-return-text-after-last-token">
            <xsl:with-param name="text" select="@NAME"/>
            <xsl:with-param name="token" select="'::'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="pathFullResolvedAndPrefixAllExceptLast">
        <xsl:variable name="tmpName">
            <xsl:call-template name="string-search-replace-except-last">
                <xsl:with-param name="text" select="@NAME"/>
                <xsl:with-param name="from" select="'::'"/>
                <xsl:with-param name="to" select="'/'"/>
                <xsl:with-param name="prefixKeywords" select="'yes'"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="string-length($tmpName)!=0">
                <xsl:value-of select="$tmpName"/>
                <xsl:text>/</xsl:text>
                <xsl:value-of select="$className"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$className"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:variable>
    <xsl:variable name="typeNamePath">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text" select="mainTopic/@typename"/>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'/'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="filename">
        <xsl:value-of select="$pathFullResolvedAndPrefixAllExceptLast"/><xsl:text>Home.java</xsl:text>
    </xsl:variable>
    <redirect:write file="{$output.dir}/{$filename}">
    <VALUEDEF-out>

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
    <xsl:variable name="listenerClassNameFullyQualified">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Listener</xsl:text></xsl:with-param>
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
    <xsl:variable name="selectionClassNameFullyQualified">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Selection</xsl:text></xsl:with-param>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'.'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="mainTopicName"              select="mainTopic/@name"/>
    <xsl:variable name="currentValDefName"          select="@NAME"/>
    <xsl:variable name="mapping">
        <xsl:choose>
            <xsl:when test="mainTopic/keyDescription/@content='NoOid'">
                <xsl:text>_PREDEFINED</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>_DEFAULT</xsl:text><!-- SimpleOid & FullOid -->
            </xsl:otherwise>
        </xsl:choose>
    </xsl:variable>
    <xsl:variable name="targetImplClass">
        <xsl:choose>
            <xsl:when test="TARGET_IMPL_CLASS">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="TARGET_IMPL_CLASS/@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'/'"/>
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$pathFullResolvedAndPrefixAllExceptLast"/>
                <xsl:text>Impl</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:variable>

<xsl:value-of select="$NL"/>
<xsl:call-template name="copyright-notice" />
<xsl:value-of select="$NL"/>
<xsl:call-template name="package-name" />

import org.opensplice.dds.dlrl.metamodel.*;

/**
 * This class is the typed implementation for the abstract ObjectHome
 * class. It manages all instances of type {@link <xsl:value-of select="$classNameFullyQualified"/>}.
 */
public final class <xsl:value-of select="$className"/>Home extends DDS.ObjectHome {

    /**
     * This is the default constructor for the <xsl:value-of select="$homeClassNameFullyQualified"/>. It initializes
     * the <xsl:value-of select="$homeClassNameFullyQualified"/> in the default configuration (i.e.
     * {@link DDS.ObjectHome#auto_deref()} will return &lt;code&gt;true&lt;/code&gt; and
     * {@link DDS.ObjectHome#content_filter()} will return &lt;code&gt;null&lt;/code&gt;).
     */
    public <xsl:value-of select="$className"/>Home(){
        super();
        //first param = the IDL name which can be used to find the home by name
        //second param = the fully qualified name of the class JNI style with any java keywords prefixed
        //third param = the fully qualified name of the class JNI style with all, except the last java keyword prefixed
        //fourth param = the fully qualified name of the topic class JNI style with any java keywords prefixed.
        //fifth param = the fully qualified name of the implementation class JNI style. Any java keywords will be prefixed.
        jniConstructObjectHome( "<xsl:value-of select="@NAME"/>",
                                "<xsl:value-of select="$pathFullResolvedAndPrefixed"/>",
                                "<xsl:value-of select="$pathFullResolvedAndPrefixAllExceptLast"/>",
                                "<xsl:value-of select="$typeNamePath"/>",
                                "<xsl:value-of select="$targetImplClass"/>");
    }

    protected void buildMetaModel() throws DDS.BadHomeDefinition {

        /**********************************************************************************************/
        /* Create the DLRL meta model class first!                                                    */
        /**********************************************************************************************/
        jniCreateDLRLClass(null, Mapping.<xsl:value-of select="$mapping"/>);

        /**********************************************************************************************/
        /* insert the topics this class (main, extension, place and multiplace)                       */
        /**********************************************************************************************/
        jniCreateMainTopic("<xsl:value-of select="mainTopic/@name"/>", "<xsl:value-of select="mainTopic/@typename"/>");<xsl:text/>
<xsl:for-each select="STATEMEMBER/multiPlaceTopic">
        jniCreateMultiPlaceTopic("<xsl:value-of select="@name"/>", "<xsl:value-of select="@typename"/>");<xsl:text/>
</xsl:for-each>

        /**********************************************************************************************/
        /* insert all DCPS fields and map them to their respective topics                             */
        /**********************************************************************************************/<xsl:text/>
<xsl:for-each select="DCPSField">
        jniCreateDCPSField("<xsl:value-of select="@name"/>", KeyType.<xsl:value-of select="@keyType"/>, AttributeType._DMM_LONG, "<xsl:value-of select="@topic"/>");<xsl:text/>
</xsl:for-each>
<xsl:variable name="currentValueDef" select="."/>
<!-- only generate for non local attributes (valueField | keyDescription | multiPlaceTopic) -->
<xsl:for-each select="STATEMEMBER">
    <!-- some handy variables we use everywhere -->
    <xsl:variable name="dlrlAttributeName" select="DECLARATOR/@NAME"/>
    <xsl:variable name="type">
        <xsl:call-template name="resolveStatememberIdlType"/>
    </xsl:variable>
    <xsl:variable name="currentStateMember" select="."/>
    <!-- a valueField indicates a mono attribute -->
    <xsl:if test="valueField">
        <!-- determine if the mono attribute is immutable by calling a special template, store the result in the variable.-->
        <xsl:variable name="immutable">
            <xsl:call-template name="determineMonoAttributeImmutabilityFromWithinStateMemberElement">
                <xsl:with-param name="dcpsFieldName" select="valueField"/>
                <xsl:with-param name="topicName" select="$mainTopicName"/>
                <xsl:with-param name="trueVal">
                    <xsl:text>true</xsl:text>
                </xsl:with-param>
                <xsl:with-param name="falseVal">
                    <xsl:text>false</xsl:text>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:variable>
        <!-- now we are completely ready to generate the calls for a mono attribute insertion into the DLRL meta model --><xsl:text/>

        /**********************************************************************************************/
        /* Meta model insertion of mono attribute <xsl:value-of select="$dlrlAttributeName"/> */
        /**********************************************************************************************/
        jniCreateAttribute("<xsl:value-of select="$dlrlAttributeName"/>", <xsl:value-of select="$immutable"/>, AttributeType._DMM_LONG);
        jniMapAttributeToDCPSTopic("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="$mainTopicName"/>");
        jniMapAttributeToDCPSField("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="valueField"/>");
    </xsl:if>
    <!-- a keyDescription indicates a mono relation -->
    <xsl:if test="keyDescription">
        <xsl:variable name="isOptional">
            <xsl:choose>
                <xsl:when test="validityField">
                    <xsl:text>true</xsl:text>
                </xsl:when>
                <xsl:when test="keyDescription/@content!='NoOid'">
                    <xsl:text>true</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>false</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <!-- store the keydescription element, so we can traverse from this node again later in the algorithm -->
        <xsl:variable name="keyDescriptionElement" select="keyDescription"/>
        <!-- we first need to locate the forward valuetype definition so we know what this relation points to.
             We are looking for a valuetype def with the correct mame, as we are processed a mono relation -->
        <xsl:for-each select="//VALUEDEF[@NAME=$type]">
            <!-- we have now found the valuetype def that describes the target type of the relation -->
        /**********************************************************************************************/
        /* Meta model insertion of mono relation <xsl:value-of select="$dlrlAttributeName"/> */
        /**********************************************************************************************/<xsl:text/>
        <!-- first generate the main create relation call --><xsl:text/>
        jniCreateRelation(false, "<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="@NAME"/>", null, <xsl:value-of select="$isOptional"/>);<xsl:text/>
            <!-- now we need to determine the maintopic name of the targeted object--><xsl:text/>
        jniSetRelationTopicPair("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="$mainTopicName"/>", "<xsl:value-of select="mainTopic/@name"/>");<xsl:text/>
                <!-- now we need to generate operations which insert the key fields correct into the method model.
                     Note that the sequence of key fields is important (IE keyfield 1 of the relation topic description matches
                     keyfield 1 of the targeted valuetypes maintopic--><xsl:text/>
                <xsl:for-each select="mainTopic/keyDescription/keyField">
                    <xsl:variable name="targetKeyFieldName" select="."/>
                    <xsl:variable name="targetKeyFieldPosition" select="position()"/>
                    <xsl:for-each select="$keyDescriptionElement/keyField[position()=$targetKeyFieldPosition]">
        jniAddRelationKeyFieldPair("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="."/>", "<xsl:value-of select="$targetKeyFieldName"/>");<xsl:text/>
                    </xsl:for-each>
                </xsl:for-each>
            </xsl:for-each>
        <xsl:if test="validityField">
        jniSetRelationValidityField("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="validityField/@name"/>");
        </xsl:if>
    </xsl:if>
    <!-- a multiPlaceTopic indicates a multi relation -->
    <xsl:if test="multiPlaceTopic">
        <xsl:variable name="indexField" select="multiPlaceTopic/@indexField"/>
        <xsl:variable name="multiPlaceTopicName" select="multiPlaceTopic/@name"/>

        /**********************************************************************************************/
        /* Meta model insertion of multi relation <xsl:value-of select="$dlrlAttributeName"/> */
        /**********************************************************************************************/<xsl:text/>
        <!-- we first need to locate the forward valuetype definition so we know what this relation points to.
             We are looking for a forward valuetype def with the 'Set|IntMap|StrMap' pattern, as we are processed a multi relation --><xsl:text/>
        <xsl:for-each select="//VALUEFORWARDDEF[@NAME=$type]">
            <xsl:variable name="basisType">
                <xsl:call-template name="determineMultiRelationBaseType">
                    <xsl:with-param name="pattern" select="@pattern"/>
                </xsl:call-template>
                <xsl:text>_BASE</xsl:text>
            </xsl:variable>
            <xsl:variable name="forwardItemType" select="@itemType"/>
        jniCreateMultiRelation(false, "<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="$forwardItemType"/>", null, Basis.<xsl:value-of select="$basisType"/>);
        /* set the topic onto which the multi relation is mapped */
        jniSetMultiRelationRelationTopic("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="$multiPlaceTopicName"/>");<xsl:text/>
            <xsl:for-each select="//VALUEDEF[@NAME=$forwardItemType]">
                <xsl:variable name="tempMainTopicName" select="mainTopic/@name"/>
        jniSetRelationTopicPair("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="$mainTopicName"/>", "<xsl:value-of select="$tempMainTopicName"/>");<xsl:text/>
                <xsl:for-each select="mainTopic/keyDescription/keyField">
        jniAddTargetField("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="."/>");<xsl:text/>
                </xsl:for-each>
            </xsl:for-each>
            <xsl:if test="not (@pattern='Set')">
        /* set the index field within the relation topic */
        jniSetRelationTopicIndexField("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="$indexField"/>");<xsl:text/>
            </xsl:if>
        </xsl:for-each>
        <xsl:for-each select="$currentValueDef/mainTopic/keyDescription/keyField">
        jniAddOwnerField("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="."/>");<xsl:text/>
        </xsl:for-each>
        <xsl:for-each select="keyDescription/keyField">
        jniAddRelationTopicTargetField("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="."/>");<xsl:text/>
        </xsl:for-each>
        <xsl:for-each select="multiPlaceTopic/keyDescription/keyField">
        jniAddRelationTopicOwnerField("<xsl:value-of select="$dlrlAttributeName"/>", "<xsl:value-of select="."/>");<xsl:text/>
        </xsl:for-each>
    </xsl:if>
</xsl:for-each>
    }

    protected int registerType(DDS.DomainParticipant participant, java.lang.String typeName, java.lang.String topicName) {
        <xsl:apply-templates select="mainTopic|STATEMEMBER/multiPlaceTopic" />
        return 0;
    }

    /**
     * Returns the list of all attached <xsl:value-of select="$listenerClassNameFullyQualified"/> entities.
     * This operation may not be called during any ObjectListener callback.
     * This operation may also not be called during a check_object callback of a
     * <xsl:value-of select="$filterClassNameFullyQualified"/> object that  belongs to a
     * <xsl:value-of select="$selectionClassNameFullyQualified"/> attached to the ObjectHome
     * instance for which this operation is called. Both situations would
     * result in a deadlock.
     * This operation is however allowed during a CacheListener callback.
     *
     * @return the list of attached <xsl:value-of select="$listenerClassNameFullyQualified"/> entities.
     * @throws DDS.AlreadyDeleted if the current Home is already deleted.
     */
    public <xsl:value-of select="$listenerClassNameFullyQualified"/>[] listeners() throws DDS.AlreadyDeleted {
        return (<xsl:value-of select="$listenerClassNameFullyQualified"/>[]) jniListeners();
    }

    /**
     * Attaches a <xsl:value-of select="$listenerClassNameFullyQualified"/> to this <xsl:value-of select="$homeClassNameFullyQualified"/>.
     * It is possible to specify whether the Listener should also listen for
     * incoming events on the contained objects. Each listener instance can
     * only be attached once. This operation may not be called during ObjectListener
     * callback. This operation may also not be called during a check_object callback of a
     * <xsl:value-of select="$filterClassNameFullyQualified"/> object that  belongs to a
     * <xsl:value-of select="$selectionClassNameFullyQualified"/> attached to the ObjectHome
     * instance for which this operation is called. Both situations would
     * result in a deadlock. This operation is however allowed during
     * a CacheListener callback.
     *
     * @param listener the <xsl:value-of select="$listenerClassNameFullyQualified"/> to be attached.
     * @param concerns_contained_objects when set to &lt;code&gt;true&lt;/code&gt;, the
     * listener will also listen for incoming events on the contained objects.
     * @return a boolean that specifies whether the listener was successfully
     * attached (&lt;code&gt;true&lt;/code&gt;) or not (&lt;code&gt;false&lt;/code&gt;) because
     * it was already attached before.
     * @throws DDS.AlreadyDeleted if the current Home is already deleted.
     */
    public boolean attach_listener (<xsl:value-of select="$listenerClassNameFullyQualified"/> listener, boolean concerns_contained_objects) throws DDS.AlreadyDeleted {
        return jniAttachListener(listener, concerns_contained_objects);
    }

    /**
     * Detaches a <xsl:value-of select="$listenerClassNameFullyQualified"/> from this <xsl:value-of select="$homeClassNameFullyQualified"/>.
     * This operation may not be called during any ObjectListener
     * callback. This operation may also not be called during a check_object callback of a
     * <xsl:value-of select="$filterClassNameFullyQualified"/> object that  belongs to a
     * <xsl:value-of select="$selectionClassNameFullyQualified"/> attached to the ObjectHome
     * instance for which this operation is called. Both situations would
     * result in a deadlock. This operation is however allowed during
     * a CacheListener callback.
     *
     * @param listener the <xsl:value-of select="$listenerClassNameFullyQualified"/> to be detached.
     * @return a boolean that specifies whether the listener was successfully
     * detached (&lt;code&gt;true&lt;/code&gt;), or was not attached in the first place
     * (&lt;code&gt;false&lt;/code&gt;).
     * @throws DDS.AlreadyDeleted if the current Home is already deleted.
     */
    public boolean detach_listener(<xsl:value-of select="$listenerClassNameFullyQualified"/> listener) throws DDS.AlreadyDeleted {
        return jniDetachListener(listener);
    }

    /**
     * Creates a <xsl:value-of select="$selectionClassNameFullyQualified"/> within this <xsl:value-of select="$homeClassNameFullyQualified"/> that will work
     * based upon the provided criterion.
     * Upon creation time it must be specified wether the selection
     * will be refreshed using the refresh operation of the selection
     * (auto_refresh is &lt;code&gt;false&lt;/code&gt;)or refreshed each time object
     * updates arrive in the cache (due to an application triggered or DCPS
     * triggered refresh)(auto_refresh is &lt;code&gt;true&lt;/code&gt;). This value can
     * not be changed after the selection has been created. Take note that a selection
     * created wit auto_refresh set to true can still be refreshed using the 'refresh()'
     * operation. Not only that, if one wants to take into account all existing objects
     * in the Cache at the moment of the creation of the seleciton, one must call the
     * refresh operation explicitly the first time.
     *
     * @param criterion the SelectionCriterion determining how the selection
     * determines which DLRL objects become a part of the Selection.
     * @param auto_refresh specifies whether the selection
     * will be refreshed using the refresh operation of the selection
     * (auto_refresh is &lt;code&gt;false&lt;/code&gt;)or refreshed each time object
     * updates arrive in the cache (due to an application triggered or DCPS
     * triggered refresh)(auto_refresh is &lt;code&gt;true&lt;/code&gt;)
     * @param concerns_contained_objects Not supported currently
     * @return The created selection object.
     * @throws DDS.PreconditionNotMet if the Cache to which the ObjectHome
     * belongs is still in initial pubsub mode or the ObjectHome doesnt yet belong to any Cache.
     * @throws DDS.AlreadyDeleted if the current Home is already deleted.
     */
    public <xsl:value-of select="$selectionClassNameFullyQualified"/> create_selection(DDS.SelectionCriterion criterion, boolean auto_refresh, boolean concerns_contained_objects) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet {
        DDS.Selection aSelection = new <xsl:value-of select="$selectionClassNameFullyQualified"/>();
        jniCreateSelection(aSelection, criterion, criterion.kind().value(), auto_refresh, concerns_contained_objects);
        return (<xsl:value-of select="$selectionClassNameFullyQualified"/>) aSelection;
    }

    /**
     * Deletes a <xsl:value-of select="$selectionClassNameFullyQualified"/> of this <xsl:value-of select="$homeClassNameFullyQualified"/>.
     * This operation may not be called during any ObjectListener callback. This operation
     * may also not be called during a check_object callback of a
     * <xsl:value-of select="$filterClassNameFullyQualified"/> object that  belongs to a
     * <xsl:value-of select="$selectionClassNameFullyQualified"/> attached to the ObjectHome
     * instance for which this operation is called. Both situations would
     * result in a deadlock.This operation is however allowed during
     * a CacheListener callback.

     * @param a_selection the <xsl:value-of select="$selectionClassNameFullyQualified"/> to be deleted.
     * @throws DDS.AlreadyDeleted if the current Home is already deleted.
     * @throws DDS.PreconditionNotMet if the selection provided was not
     * created by this ObjectHome
     */
    public void delete_selection(<xsl:value-of select="$selectionClassNameFullyQualified"/> a_selection) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet {
        jniDeleteSelection(a_selection);
    }

    /**
     * Returns the list of all attached <xsl:value-of select="$selectionClassNameFullyQualified"/> entities.
     *
     * @return the list of attached <xsl:value-of select="$selectionClassNameFullyQualified"/> entities.
     * @throws DDS.AlreadyDeleted if the current Home is already deleted.
     */
    public <xsl:value-of select="$selectionClassNameFullyQualified"/>[] selections() throws DDS.AlreadyDeleted {
        return (<xsl:value-of select="$selectionClassNameFullyQualified"/>[]) jniSelections();
    }

    /**
     * Returns the list of all <xsl:value-of select="$classNameFullyQualified"/> objects that have
     * been modified in a specified Cache during the last update round.
     * This operation may not be called during any ObjectListener callback.
     * This operation may also not be called during a check_object callback of a
     * <xsl:value-of select="$filterClassNameFullyQualified"/> object that  belongs to a
     * <xsl:value-of select="$selectionClassNameFullyQualified"/> attached to the ObjectHome
     * instance for which this operation is called. Both situations would
     * result in a deadlock.This operation is however allowed during
     * a CacheListener callback.
     *
     * @param source the cache from which the list of modified objects needs to be obtained.
     * @return the list of modified <xsl:value-of select="$classNameFullyQualified"/> objects.
     * @throws DDS.AlreadyDeleted if the current Home is already deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] get_modified_objects(DDS.CacheBase source) throws DDS.AlreadyDeleted {
        return (<xsl:value-of select="$classNameFullyQualified"/>[]) jniModifiedObjects(source, source.kind().value());
    }

    /**
     * Returns the list of all <xsl:value-of select="$classNameFullyQualified"/> objects that have
     * been deleted in a specified Cache during the last update round.
     * This operation may not be called during any ObjectListener callback.
     * This operation may also not be called during a check_object callback of a
     * <xsl:value-of select="$filterClassNameFullyQualified"/> object that  belongs to a
     * <xsl:value-of select="$selectionClassNameFullyQualified"/> attached to the ObjectHome
     * instance for which this operation is called. Both situations would
     * result in a deadlock.This operation is however allowed during
     * a CacheListener callback.
     *
     * @param source the cache from which the list of deleted objects needs to be obtained.
     * @return the list of deleted <xsl:value-of select="$classNameFullyQualified"/> objects.
     * @throws DDS.AlreadyDeleted if the current Home is already deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] get_deleted_objects(DDS.CacheBase source) throws DDS.AlreadyDeleted {
        return (<xsl:value-of select="$classNameFullyQualified"/>[]) jniDeletedObjects(source, source.kind().value());
    }

    /**
     * Returns the list of all {@link <xsl:value-of select="$classNameFullyQualified"/>} objects that have
     * been created in a specified {@link DDS.Cache} during the last update round.
     * This operation may not be called during any {@link DDS.ObjectListener} callback.
     * This operation may also not be called during a check_object callback of a
     * <xsl:value-of select="$filterClassNameFullyQualified"/> object that  belongs to a
     * <xsl:value-of select="$selectionClassNameFullyQualified"/> attached to the ObjectHome
     * instance for which this operation is called. Both situations would
     * result in a deadlock.This operation is however allowed during
     * a CacheListener callback.
     *
     * @param source the cache from which the list of created objects needs to be obtained.
     * @return the list of deleted <xsl:value-of select="$classNameFullyQualified"/> objects.
     * @throws DDS.AlreadyDeleted if the current Home is already deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] get_created_objects(DDS.CacheBase source) throws DDS.AlreadyDeleted {
        return (<xsl:value-of select="$classNameFullyQualified"/>[]) jniNewObjects(source, source.kind().value());
    }

    /**
     * Returns the list of all {@link <xsl:value-of select="$classNameFullyQualified"/>} objects that are
     * available in a specified {@link DDS.CacheBase}. Note that this list never
     * contains objects that are considered deleted. This includes objects that are still
     * contained in the list resulting from a call to the get_deleted_objects() operation
     * of this object home.
     * This operation may not be called during any {@link DDS.ObjectListener} callback.
     * This operation may also not be called during a {@link <xsl:value-of select="$filterClassNameFullyQualified"/>#check_object} callback
     * that belongs to a {@link <xsl:value-of select="$selectionClassNameFullyQualified"/>} attached to the {@link DDS.ObjectHome}
     * instance for which this operation is called. Both situations would
     * result in a deadlock.This operation is however allowed during
     * a {@link DDS.CacheListener} callback.
     *
     * @param source the {@link DDS.Cache} from which the list of available objects should be obtained.
     * @return the list of available <xsl:value-of select="$classNameFullyQualified"/> objects.
     * @throws DDS.AlreadyDeleted
     *             if the current Home is already deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/>[] get_objects(DDS.CacheBase source) throws DDS.AlreadyDeleted {
        return (<xsl:value-of select="$classNameFullyQualified"/>[]) jniObjects(source, source.kind().value());
    }

    /**
     * Pre-create a new DLRL object in order to fill its content before the allocation
     * of the OID. This method takes as parameter the {@link DDS.CacheAccess} concerned
     * with this operation. The CacheAccess must belong to a {@link DDS.Cache} which
     * has a DCPS state of ENABLED. The CacheAccess must also have a usage of WRITE_ONLY
     * or READ_WRITE. Failure to satisfy either precondition will result in a
     * PreconditionNotMet exception being raised.
     *
     * @param access The cache access which is concerned by the pre-creation.
     * @return the pre-created object.
     * @throws DDS.AlreadyDeleted if the ObjectHome or CacheAccess is already deleted.
     * @throws DDS.PreconditionNotMet if the owning Cache does not have an ENABLED dcps state or
     * if the cache access in question is not writeable.
     */
    public <xsl:value-of select="$classNameFullyQualified"/> create_unregistered_object(DDS.CacheAccess access) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted {
        return (<xsl:value-of select="$classNameFullyQualified"/>)jniCreateUnregisteredObject(access);
    }

    /*
     * Look up an ObjectRoot with the specified oid in the specified CacheBase.
     *
     * This operation is currently not supported.
     *
     * @param oid The ObjectIDentity (OID) of the ObjectRoot wanted.
     * @param source the target CacheBase to search in.
     * @throws DDS.NotFound if the object with the specified OID could not be located.
     * @throws DDS.AlreadyDeleted If the ObjectHome or CacheBase have already been deleted.
     */
    public <xsl:value-of select="$classNameFullyQualified"/> find_object(DDS.DLRLOid oid, DDS.CacheBase source) throws DDS.NotFound, DDS.AlreadyDeleted{
        return (<xsl:value-of select="$classNameFullyQualified"/>)jniFindObject(oid, source);
    }

    /**
     * Register an object resulting from {@link <xsl:value-of select="$homeClassNameFullyQualified"/>#create_unregistered_object()}.
     * This operation embeds a logic to derive a suitable OID from the object content. Only
     * objects created by {@link <xsl:value-of select="$homeClassNameFullyQualified"/>#create_unregistered_object()} can be passed
     * as parameter, a PreconditionNotMet is raised otherwise. If the result of the computation
     * leads to an existing OID, an AlreadyExisting exception is raised. Once an object has been
     * registered, the fields that make up its identity (i.e. the fields that are mapped onto
     * the keyfields of the corresponding topics) may not be changed anymore.
     *
     * @param unregistered_object An object created by a call to {@link <xsl:value-of select="$homeClassNameFullyQualified"/>#create_unregistered_object()}
     * @throws DDS.PreconditionNotMet if the provided object was not created by calling the
     * {@link <xsl:value-of select="$homeClassNameFullyQualified"/>#create_unregistered_object()} operation.
     * @throws DDS.AlreadyExisting If the result of the OID computation leads to an existing OID.
     * @throws DDS.AlreadyDeleted If the ObjectHome or CacheAccess is already deleted.
     */
    public void register_object(<xsl:value-of select="$classNameFullyQualified"/> unregistered_object) throws DDS.PreconditionNotMet, DDS.AlreadyExisting,
                                                                                            DDS.AlreadyDeleted {
        jniRegisterObject(unregistered_object);
    }

    /**
     * Creates and returns a new DLRL object. This operation takes as parameter the
     * {@link DDS.CacheAccess} concerned by the creation. The CacheAccess must belong to a
     * {@link DDS.Cache} which has a DCPS state of ENABLED. The CacheAccess must also have
     * a usage of WRITE_ONLY or READ_WRITE. Failure to satisfy either precondition will
     * result in a PreconditionNotMet exception being raised.
     *
     * @param access The cache access which is concerned by the creation.
     * @return the newly created object.
     * @throws DDS.AlreadyDeleted if the ObjectHome or CacheAccess is already deleted.
     * @throws DDS.PreconditionNotMet if the owning Cache does not have an ENABLED dcps state or
     * if the cache access in question is not writeable.
     */
    public <xsl:value-of select="$classNameFullyQualified"/> create_object(DDS.CacheAccess access) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted {
        return (<xsl:value-of select="$classNameFullyQualified"/>)jniCreateObject(access);
    }

    private void triggerListeners(DDS.ObjectListener[][] listeners, <xsl:value-of select="$classNameFullyQualified"/>[] newObjects,
                                  <xsl:value-of select="$classNameFullyQualified"/>[] modifiedObjects, <xsl:value-of select="$classNameFullyQualified"/>[] deletedObjects) {
        int listenersLength = listeners.length;
        // process new samples
        int newObjectsLength = newObjects.length;
        for (int count = 0; count &lt; newObjectsLength; count++) {
            boolean fullyProcessed = false;
            <xsl:value-of select="$classNameFullyQualified"/> anObject = newObjects[count];
            for (int listenerCounter = 0; (listenerCounter &lt;  listenersLength) &amp;&amp; !fullyProcessed; listenerCounter++) {
                DDS.ObjectListener[] homeListeners = listeners[listenerCounter];
                if(homeListeners != null){/* in case of inheritance, this list may be NULL*/
                    int homeListenersLength = homeListeners.length;
                    for (int homeListenerCounter = 0; homeListenerCounter &lt; homeListenersLength; homeListenerCounter++) {
                        <xsl:value-of select="$listenerClassNameFullyQualified"/> aListener = (<xsl:value-of select="$listenerClassNameFullyQualified"/>) homeListeners[homeListenerCounter];
                        fullyProcessed = aListener.on_object_created(anObject) | fullyProcessed;
                    }
                }
            }
        }
        // process modified samples
        int modifiedObjectsLength = modifiedObjects.length;
        for (int count = 0; count &lt; modifiedObjectsLength; count++) {
            boolean fullyProcessed = false;
            <xsl:value-of select="$classNameFullyQualified"/> anObject = modifiedObjects[count];
            for (int listenerCounter = 0; (listenerCounter &lt;  listenersLength) &amp;&amp; !fullyProcessed; listenerCounter++) {
                DDS.ObjectListener[] homeListeners = listeners[listenerCounter];
                if(homeListeners != null){/* in case of inheritance, this list may be NULL*/
                    int homeListenersLength = homeListeners.length;
                    for (int homeListenerCounter = 0; homeListenerCounter &lt; homeListenersLength; homeListenerCounter++) {
                        <xsl:value-of select="$listenerClassNameFullyQualified"/> aListener = (<xsl:value-of select="$listenerClassNameFullyQualified"/>) homeListeners[homeListenerCounter];
                        fullyProcessed = aListener.on_object_modified(anObject) | fullyProcessed;
                    }
                }
            }
        }
        // process deleted samples
        int deletedObjectsLength = deletedObjects.length;
        for (int count = 0; count &lt; deletedObjectsLength; count++) {
            boolean fullyProcessed = false;
            <xsl:value-of select="$classNameFullyQualified"/> anObject = deletedObjects[count];
            for (int listenerCounter = 0; (listenerCounter &lt;  listenersLength) &amp;&amp; !fullyProcessed; listenerCounter++) {
                DDS.ObjectListener[] homeListeners = listeners[listenerCounter];
                if(homeListeners != null){/* in case of inheritance, this list may be NULL*/
                    int homeListenersLength = homeListeners.length;
                    for (int homeListenerCounter = 0; homeListenerCounter &lt; homeListenersLength; homeListenerCounter++) {
                        <xsl:value-of select="$listenerClassNameFullyQualified"/> aListener = (<xsl:value-of select="$listenerClassNameFullyQualified"/>)homeListeners[homeListenerCounter];
                        fullyProcessed = aListener.on_object_deleted(anObject) | fullyProcessed;
                    }
                }
            }
        }
    }
}
    </VALUEDEF-out>
    </redirect:write>
</xsl:if>
</xsl:template>

<!--  Generation of register-type() contents -->

<xsl:template match="mainTopic|STATEMEMBER/multiPlaceTopic">
    <xsl:variable name="fullNameTypeSupport">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@typename"/><xsl:text>TypeSupport</xsl:text></xsl:with-param>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'.'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="typeFullName" select="@typename"/>
        if (typeName.equals("<xsl:value-of select="$typeFullName" />")) {
            return new <xsl:value-of select="$fullNameTypeSupport"/>().register_type(participant, typeName);
        }</xsl:template>
</xsl:stylesheet>