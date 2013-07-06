<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xalan="http://xml.apache.org/xalan"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">

    <xsl:param name="output.dir" select="'.'"/>
    <xsl:include href="typed_java_common.xsl"/>

    <xsl:template match="VALUEDEF">
        <xsl:variable name="isNonIncludedSharedValueDef">
            <xsl:call-template name="does-valuedef-inherit-from-objectroot-and-is-not-included"/>
        </xsl:variable>

        <!-- only need to do stuff for valuetypes that are shared through dlrl -->
        <xsl:if test="$isNonIncludedSharedValueDef='true'">

            <!-- define some usefull vars -->
            <xsl:variable name="mainTopicName" select="mainTopic/@name"/>
            <xsl:variable name="filename">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'/'"/>
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
                <xsl:text>.java</xsl:text>
            </xsl:variable>
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
            <xsl:variable name="implClassNameFullyQualified">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text"><xsl:value-of select="@NAME"/><xsl:text>Impl</xsl:text></xsl:with-param>
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
            <xsl:variable name="mainTopicFullName">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="mainTopic/@typename"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'.'"/>
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>

            <!-- redirected -->
            <redirect:write file="{$output.dir}/{$filename}">
                <VALUEDEF-out>
                    <!-- generate copy right header and package declaration -->
                    <xsl:call-template name="generate-file-begin"/>
                    <!-- generate javadoc for the class and the class open stuff -->
                    <xsl:call-template name="generate-class-begin">
                        <xsl:with-param name="implClassNameFullyQualified" select="$implClassNameFullyQualified"/>
                        <xsl:with-param name="prefixedName" select="$prefixedName"/>
                    </xsl:call-template>
                    <!-- generate all instance variables used by this class -->
                    <xsl:call-template name="generate-class-instance-vars">
                        <xsl:with-param name="homeClassNameFullyQualified" select="$homeClassNameFullyQualified"/>
                        <xsl:with-param name="mainTopicFullName" select="$mainTopicFullName"/>
                    </xsl:call-template>
                    <!-- generate all operations needed for (local/shared) statemember support i.e., get_x/set_x/is_x_modified -->
                    <xsl:apply-templates select="STATEMEMBER" mode="accessors">
                        <xsl:with-param name="mainTopicName" select="$mainTopicName"/>
                    </xsl:apply-templates>
                    <!-- generate all operations defined in IDL -->
                    <xsl:apply-templates select="OPERATION"/>
                    <!-- close the class -->
                    <xsl:value-of select="$NL"/>
                    <xsl:text>}</xsl:text>
                    <xsl:value-of select="$NL"/>
                </VALUEDEF-out>
            </redirect:write>
        </xsl:if>
    </xsl:template>

    <xsl:template name="generate-file-begin">
        <xsl:value-of select="$NL"/>
        <xsl:call-template name="copyright-notice" />
        <xsl:value-of select="$NL"/>
        <xsl:call-template name="package-name" />
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="generate-class-begin">
        <xsl:param name="implClassNameFullyQualified"/>
        <xsl:param name="prefixedName"/>

        <!-- Generates the following code for valuetype 'Foo' defined in module 'test'
        /**
         * This is generated code that represents an abstract application class.
         * DO NOT edit this code, instead implement missing method implementations
         * in the corresponding {@link test.FooImpl} class.
         *
         * An important note is that in the current implementation getters for attributes mapped as
         * a struct, union or array in IDL will simply return a direct pointer to the object
         * representing that IDL concept. This means that an application could potentially
         * change fields in these objects, causing a change in the DLRL object as well (I.E.
         * the next time a getter was done the same object pointer is returned reflecting
         * the changes made.). In the future the DLRL implementation will return copies of
         * such objects. However for the present time it is a coding guideline to never alter
         * an retrieved value that represents a struct, union or array. It's also important
         * to realize that because the implementation will return copies in the future that
         * at that time such modifications <i>are</i> allowed as it wouldn't impact the DLRL
         * object.
         */
         -->
        <xsl:text>/**</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * This is generated code that represents an abstract application class.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * DO NOT edit this code, instead implement missing method implementations</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * in the corresponding {@link </xsl:text>
        <xsl:value-of select="$implClassNameFullyQualified"/>
        <xsl:text>} class.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * An important note is that in the current implementation getters for attributes mapped as</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * a struct, union or array in IDL will simply return a direct pointer to the object</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * representing that IDL concept. This means that an application could potentially</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * change fields in these objects, causing a change in the DLRL object as well (I.E.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * the next time a getter was done the same object pointer is returned reflecting</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * the changes made.). In the future the DLRL implementation will return copies of</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * such objects. However for the present time it is a coding guideline to never alter</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * an retrieved value that represents a struct, union or array. It's also important</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * to realize that because the implementation will return copies in the future that</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * at that time such modifications &lt;i&gt;are&lt;/i&gt; allowed as it wouldn't impact the DLRL</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> * object.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text> */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <!-- the following is generated in this template code
            (example class "Foo'):

            public abstract class Foo extends DDS.ObjectRoot

        -->
        <xsl:text>public abstract class </xsl:text>
        <xsl:value-of select="$prefixedName"/>
        <xsl:text> extends DDS.ObjectRoot {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="generate-class-instance-vars">
        <xsl:param name="homeClassNameFullyQualified"/>
        <xsl:param name="mainTopicFullName"/>

        <!-- the following is generated in this template code
            (example class "Foo', defined in module 'test', mainTopic test::FooTopic):

        // NOTE: All (private) attributes declared within this object are updated by the DLRL. The DLRL knows
        // about them through the metaModel, which is defined in test::FooHome,
        // or through pre-defined definitions/naming conventions.
        // The DLRL will also re-use the attribute objects at any time it sees fit.
        private test::FooTopic currentTopic = null;
        private test::FooTopic previousTopic = null; /* WARNING: may contain a 'very old' topic sample which can be reused at any time by the DLRL */

        -->
        <xsl:text>    // NOTE: All (private) attributes declared within this object are updated by the DLRL. The DLRL knows</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    // about them through the metaModel, which is defined in </xsl:text>
        <xsl:value-of select="$homeClassNameFullyQualified"/>
        <xsl:text>,</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    // or through pre-defined definitions/naming conventions.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    // The DLRL will also re-use the attribute objects at any time it sees fit.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    private </xsl:text>
        <xsl:value-of select="$mainTopicFullName" />
        <xsl:text> currentTopic = null;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    private </xsl:text>
        <xsl:value-of select="$mainTopicFullName" />
        <xsl:text> previousTopic = null;/* WARNING: may contain a 'very old' topic sample which can be reused at any time by the DLRL */</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>

        <!-- generates all instance vars needed for the various attributes support -->
        <xsl:apply-templates select="STATEMEMBER" mode="declaration"/>
    </xsl:template>

    <xsl:template match="STATEMEMBER" mode="declaration">
        <xsl:choose>
            <!-- is it a mono relation? -->
            <xsl:when test="keyDescription and not(multiPlaceTopic)">
                <xsl:call-template name="generate-monoRelation-instance-vars"/>
            </xsl:when>
            <!-- or is it a multi relation? -->
            <xsl:when test="multiPlaceTopic">
                <xsl:call-template name="generate-multiRelation-instance-vars"/>
            </xsl:when>
            <!-- else we dont do anything -->
        </xsl:choose>
    </xsl:template>


    <xsl:template name="generate-monoRelation-instance-vars">
        <xsl:variable name="attributeName" select="DECLARATOR/@NAME" />
        <xsl:variable name="prefixedAttributeName">
            <xsl:call-template name="java-name">
                <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="prefixedTypeName">
            <xsl:variable name="idlType">
                <xsl:call-template name="resolveStatememberIdlType"/>
            </xsl:variable>
            <xsl:call-template name="string-search-replace">
                <xsl:with-param name="text" select="$idlType"/>
                <xsl:with-param name="from" select="'::'"/>
                <xsl:with-param name="to" select="'.'"/>
                <xsl:with-param name="prefixKeywords" select="'yes'"/>
            </xsl:call-template>
        </xsl:variable>

        <!-- generates the following code for mono relation 'myBar'
        private test.Bar myBar;
        private boolean myBarIsFound = true;
        -->
        <xsl:text>    private </xsl:text>
        <xsl:value-of select="$prefixedTypeName"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="$prefixedAttributeName" />
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>    private boolean </xsl:text>
        <xsl:value-of select="$attributeName" />
        <xsl:text>IsFound = true;</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:value-of select="$NL" />
    </xsl:template>

    <xsl:template name="generate-multiRelation-instance-vars">
        <xsl:variable name="type">
            <xsl:call-template name="resolveStatememberIdlType"/>
        </xsl:variable>
        <xsl:variable name="attributeName" select="DECLARATOR/@NAME" />
        <!-- must locate the forward valuetype def so we can determine the actual type of
             this multi relation
        -->
        <xsl:for-each select="//VALUEFORWARDDEF[@NAME=$type]">
            <xsl:variable name="prefixedAttributeName">
                <xsl:call-template name="java-name">
                    <xsl:with-param name="name" select="$attributeName"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="prefixedTypeName">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text">
                        <xsl:value-of select="@itemType"/>
                        <xsl:value-of select="@pattern"/>
                    </xsl:with-param>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'.'"/>
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    private </xsl:text>
            <xsl:value-of select="$prefixedTypeName"/>
            <xsl:text> </xsl:text>
            <xsl:value-of select="$prefixedAttributeName" />
            <xsl:text>;</xsl:text>
            <xsl:value-of select="$NL"/>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="OPERATION">
        <xsl:text>abstract </xsl:text>
        <xsl:call-template name="operation-signature" />
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template match="STATEMEMBER" mode="accessors">
        <xsl:param name="mainTopicName"/>

        <xsl:variable name="attributeName" select="DECLARATOR/@NAME" />

        <xsl:choose>
            <!-- is it a mono attribute? -->
            <xsl:when test="valueField">
                <xsl:call-template name="generate-monoAttribute-operations">
                    <xsl:with-param name="mainTopicName" select="$mainTopicName"/>
                </xsl:call-template>
            </xsl:when>
            <!-- or is it a monoRelation? -->
            <xsl:when test="keyDescription and not (multiPlaceTopic)">
                <xsl:call-template name="generate-monoRelation-operations">
                    <xsl:with-param name="mainTopicName" select="$mainTopicName"/>
                </xsl:call-template>
            </xsl:when>
            <!-- or is it a multiRelation? -->
            <xsl:when test="multiPlaceTopic">
                <xsl:call-template name="generate-multiRelation-operations"/>
            </xsl:when>
            <!-- if none of the above it must be a local attribute (multi attribute is not supported) -->
            <xsl:otherwise>
                <xsl:call-template name="generate-localAttribute-operations"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="generate-localAttribute-operations">
        <xsl:variable name="attributeName" select="DECLARATOR/@NAME" />
        <xsl:variable name="attributeType">
            <xsl:variable name="idlType">
                <xsl:call-template name="resolveStatememberIdlType"/>
            </xsl:variable>
            <xsl:call-template name="get-java-type-for-resolved-idl-type">
                <xsl:with-param name="idlType" select="$idlType"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="arrayBrackets">
            <xsl:call-template name="get-array-brackets">
                <xsl:with-param name="action" select="'bracket-only'"/>
                <xsl:with-param name="userData" select="'0'"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="prefixedAttributeName">
            <xsl:call-template name="java-name">
                <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:call-template name="localAttribute_get">
            <xsl:with-param name="attributeName" select="$attributeName"/>
            <xsl:with-param name="prefixedAttributeName" select="$prefixedAttributeName"/>
            <xsl:with-param name="attributeType" select="$attributeType"/>
            <xsl:with-param name="arrayBrackets" select="$arrayBrackets"/>
        </xsl:call-template>
        <xsl:call-template name="localAttribute_set">
            <xsl:with-param name="attributeName" select="$attributeName"/>
            <xsl:with-param name="prefixedAttributeName" select="$prefixedAttributeName"/>
            <xsl:with-param name="attributeType" select="$attributeType"/>
            <xsl:with-param name="arrayBrackets" select="$arrayBrackets"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template name="localAttribute_set">
        <xsl:param name="attributeName"/>
        <xsl:param name="prefixedAttributeName"/>
        <xsl:param name="attributeType"/>
        <xsl:param name="arrayBrackets"/>

        <!-- Generates the following code for local attribute 'public long xx':
            /**
             * Sets the 'local' xx attribute.
             *
             * @param the attribute value
             */
            public abstract void xx(int xx);

        -->
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * Sets the 'local' </xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text> attribute.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @param the attribute value</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>    </xsl:text>
        <xsl:call-template name="statemember-visibility"/>
        <xsl:text>abstract void </xsl:text>
        <xsl:value-of select="$prefixedAttributeName"/>
        <xsl:text>(</xsl:text>
        <xsl:value-of select="$attributeType"/>
        <xsl:value-of select="$arrayBrackets"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="$prefixedAttributeName"/>
        <xsl:text>);</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:value-of select="$NL" />
    </xsl:template>

    <xsl:template name="localAttribute_get">
        <xsl:param name="attributeName"/>
        <xsl:param name="prefixedAttributeName"/>
        <xsl:param name="attributeType"/>
        <xsl:param name="arrayBrackets"/>

        <!-- Generates the following code for local attribute 'public long xx':
            /**
             * Returns the 'local' xx attribute.
             *
             * @return the attribute value
             */
            public abstract int xx();

        -->
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * Returns the 'local' </xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text> attribute.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @return the attribute value</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>    </xsl:text>
        <xsl:call-template name="statemember-visibility"/>
        <xsl:text>abstract </xsl:text>
        <xsl:value-of select="$attributeType"/>
        <xsl:value-of select="$arrayBrackets"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="$prefixedAttributeName"/>
        <xsl:text>();</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:value-of select="$NL" />
    </xsl:template>

    <xsl:template name="generate-monoAttribute-operations">
        <xsl:param name="mainTopicName"/>

        <xsl:variable name="attributeName" select="DECLARATOR/@NAME" />
        <xsl:variable name="attributeType">
            <xsl:variable name="idlType">
                <xsl:call-template name="resolveStatememberIdlType"/>
            </xsl:variable>
            <xsl:call-template name="get-java-type-for-resolved-idl-type">
                <xsl:with-param name="idlType" select="$idlType"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="arrayBrackets">
            <xsl:call-template name="get-array-brackets">
                <xsl:with-param name="action" select="'bracket-only'"/>
                <xsl:with-param name="userData" select="'0'"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="prefixedTopicFieldName">
            <xsl:call-template name="java-name">
                <xsl:with-param name="name" select="valueField"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="must-setter-be-generated">
            <xsl:call-template name="must-monoAttribute-setter-be-generated">
                <xsl:with-param name="valueFieldName" select="valueField"/>
                <xsl:with-param name="topicName" select="$mainTopicName"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:call-template name="monoAttribute__get">
            <xsl:with-param name="attributeName" select="$attributeName"/>
            <xsl:with-param name="attributeType" select="$attributeType"/>
            <xsl:with-param name="arrayBrackets" select="$arrayBrackets"/>
            <xsl:with-param name="prefixedTopicFieldName" select="$prefixedTopicFieldName"/>
        </xsl:call-template>

        <xsl:if test="string-length($must-setter-be-generated)=0">
            <xsl:variable name="isImmutable">
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

            <xsl:call-template name="monoAttribute__set">
                <xsl:with-param name="attributeName" select="$attributeName"/>
                <xsl:with-param name="immutable" select="$isImmutable"/>
                <xsl:with-param name="attributeType" select="$attributeType"/>
                <xsl:with-param name="arrayBrackets" select="$arrayBrackets"/>
                <xsl:with-param name="prefixedTopicFieldName" select="$prefixedTopicFieldName"/>
            </xsl:call-template>
        </xsl:if>
        <xsl:call-template name="monoAttribute__is_xxx_modified">
            <xsl:with-param name="attributeName" select="$attributeName"/>
            <xsl:with-param name="prefixedTopicFieldName" select="$prefixedTopicFieldName"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template name="monoAttribute__get">
        <xsl:param name="attributeName"/>
        <xsl:param name="attributeType"/>
        <xsl:param name="arrayBrackets"/>
        <xsl:param name="prefixedTopicFieldName"/>

        <!-- Generates the following code for shared attribute 'public long myX' mapped onto topic member 'x':
        /**
         * Returns the 'shared' x attribute. If the object state was not yet available,
         * it will be fetched from the DCPS.
         *
         * @return the corresponding attribute.
         */
         -->
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * Returns the 'shared' </xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text> attribute. If the object state was not yet available,</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * it will be fetched from the DCPS.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @return the corresponding attribute.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL" />

        <!-- Generates the following code for shared attribute 'public long myX' mapped onto topic member 'x':
            public final int get_x() {
                return currentTopic.x;
            }
        -->
        <xsl:text>    public final </xsl:text>
        <xsl:value-of select="$attributeType"/>
        <xsl:value-of select="$arrayBrackets"/>
        <xsl:text> get_</xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>() {</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>        return currentTopic.</xsl:text>
        <xsl:value-of select="$prefixedTopicFieldName"/>
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:value-of select="$NL" />
    </xsl:template>

    <xsl:template name="monoAttribute__is_xxx_modified">
        <xsl:param name="attributeName"/>
        <xsl:param name="prefixedTopicFieldName"/>

        <xsl:variable name="idlTypeSpec">
            <xsl:call-template name="get-idl-type-spec-id"/>
        </xsl:variable>

       <!-- Generates the following code for shared attribute 'public long myX' mapped onto topic member 'x':
        /**
         * Returns <code>true</code> if the 'shared' myX attribute has
         * been modified in the previous update round, <code>false</code> if not. For objects with
         * read_state NEW or VOID <code>false</code> is always returned. If the object state was not yet available,
         * it will be fetched from the DCPS.
         * Currently union and struct attributes will always indicate they are modified.
         *
         * @return the corresponding attribute.
         */
         -->
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * Returns &lt;code>true&lt;/code> if the 'shared' </xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text> attribute has</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * been modified in the previous update round, &lt;code>false&lt;/code> if not. For objects with</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * read_state NEW or VOID &lt;code>false&lt;/code> is always returned. If the object state was not yet available,</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * it will be fetched from the DCPS.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * Currently union and struct attributes will always indicate they are modified.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @return the corresponding attribute.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     */</xsl:text>
        <!-- Generates the following code for shared attribute 'public long myX' mapped onto topic member 'x':
        public final boolean is_myX_modified(){
            return prevTopicValid && (currentTopic.x != previousTopic.x);
        }
         -->
        <xsl:value-of select="$NL" />
        <xsl:text>    public final boolean is_</xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>_modified(){</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>        return prevTopicValid &amp;&amp; (</xsl:text>

        <xsl:choose>
            <xsl:when test="$idlTypeSpec='STRING' or $idlTypeSpec='WSTRING' ">
                <xsl:text>!(currentTopic.</xsl:text>
                <xsl:value-of select="$prefixedTopicFieldName"/>
                <xsl:text>.equals(previousTopic.</xsl:text>
                <xsl:value-of select="$prefixedTopicFieldName"/>
                <xsl:text>)));</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:when test="$idlTypeSpec='ENUM'">
                <xsl:text>currentTopic.</xsl:text>
                <xsl:value-of select="$prefixedTopicFieldName"/>
                <xsl:text>.value() != previousTopic.</xsl:text>
                <xsl:value-of select="$prefixedTopicFieldName"/>
                <xsl:text>.value());</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:when test="$idlTypeSpec='STRUCT'">
                <xsl:text>true);//structs always return true for the moment</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:when test="$idlTypeSpec='UNION'">
                <xsl:text>true);//unions always return true for the moment</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:when test="$idlTypeSpec='ARRAY'">
                <xsl:text>true);//unions always return true for the moment</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:when test="$idlTypeSpec='SIMPLE' or $idlTypeSpec='ANY'">
                <xsl:text>currentTopic.</xsl:text>
                <xsl:value-of select="$prefixedTopicFieldName"/>
                <xsl:text> != previousTopic.</xsl:text>
                <xsl:value-of select="$prefixedTopicFieldName"/>
                <xsl:text>);</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:when>
            <xsl:otherwise><!-- must be sequence or unknown, either case return true. -->
                <xsl:text>true);</xsl:text>
                <xsl:value-of select="$NL"/>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="monoAttribute__set">
        <xsl:param name="attributeName"/>
        <xsl:param name="immutable"/>
        <xsl:param name="attributeType"/>
        <xsl:param name="arrayBrackets"/>
        <xsl:param name="prefixedTopicFieldName"/>

        <!--Generates the following code for shared attribute 'public long myX' mapped onto topic member 'x':
            /**
             * Sets the 'shared' myX attribute.
             *
             * A PreconditionNotMet is raised if any of the following preconditions is violated:
             * - ObjectRoot is not in a (writeable) cacheaccess;
             * - ObjectRoot is already registered (i.e. identity may not be changed anymore), as this field represents a 'key' field of this object.
             * @param val the new value for the corresponding attribute.
             * @throws DDS.AlreadyDeleted if the object being manipulated is already deleted
             * @throws DDS.PreconditionNotMet if any of the preconditions are not met.
             */
        -->
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * Sets the 'shared' </xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text> attribute.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * A PreconditionNotMet is raised if </xsl:text>
        <xsl:if test="$immutable='true'">
            <xsl:text>any of the following preconditions</xsl:text>
        </xsl:if>
        <xsl:if test="$immutable!='true'">
            <xsl:text>the following precondition</xsl:text>
        </xsl:if>
        <xsl:text> is violated:</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * - ObjectRoot is not in a (writeable) cacheaccess</xsl:text>
        <xsl:if test="$immutable='true'">
            <xsl:text>;</xsl:text>
        </xsl:if>
        <xsl:if test="$immutable!='true'">
            <xsl:text>.</xsl:text>
        </xsl:if>
        <xsl:value-of select="$NL" />
        <xsl:text>     * </xsl:text>
        <xsl:if test="$immutable='true'">
            <xsl:text>- ObjectRoot is already registered (i.e. identity may not be changed anymore), as this field represents a 'key' field of this object.</xsl:text>
        </xsl:if>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @param val the new value for the corresponding attribute. </xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @throws DDS.AlreadyDeleted if the object being manipulated is already deleted</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @throws DDS.PreconditionNotMet if any of the preconditions are not met.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     */        </xsl:text>
        <xsl:value-of select="$NL" />
        <!--Generates the following code for shared attribute 'public long myX' mapped onto topic member 'x':
            public final void set_myX(int val)  throws DDS.PreconditionNotMet, DDS.AlreadyDeleted{
                //first validate if the change is allowed, if not an exception will be raised.
                validateAndRegisterObjectChange(true);
                currentTopic.x = val;
            }
        -->
        <xsl:text>    public final void set_</xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>(</xsl:text>
        <xsl:value-of select="$attributeType"/>
        <xsl:value-of select="$arrayBrackets"/>
        <xsl:text> val)  throws DDS.PreconditionNotMet, DDS.AlreadyDeleted{</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>        //first validate if the change is allowed, if not an exception will be raised.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>        validateAndRegisterObjectChange(</xsl:text><xsl:value-of select="$immutable"/><xsl:text>);</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>        currentTopic.</xsl:text>
        <xsl:value-of select="$prefixedTopicFieldName"/>
        <xsl:text> = val;</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:value-of select="$NL" />
    </xsl:template>

    <xsl:template name="generate-monoRelation-operations">
        <xsl:param name="mainTopicName"/>

        <xsl:variable name="attributeName" select="DECLARATOR/@NAME" />
        <xsl:variable name="prefixedAttributeName">
            <xsl:call-template name="java-name">
                <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="returnType">
            <xsl:variable name="idlType">
                <xsl:call-template name="resolveStatememberIdlType"/>
            </xsl:variable>
            <xsl:call-template name="string-search-replace">
                <xsl:with-param name="text" select="$idlType"/>
                <xsl:with-param name="from" select="'::'"/>
                <xsl:with-param name="to" select="'.'"/>
                <xsl:with-param name="prefixKeywords" select="'yes'"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="index" select="TYPEREF/@INDEX"/>
        <xsl:variable name="relationHasForeignKeys">
            <xsl:call-template name="hasForeignKeysForMonoRelation">
                <xsl:with-param name="topicName" select="$mainTopicName"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:call-template name="monoRelation__get">
            <xsl:with-param name="attributeName" select="$attributeName"/>
            <xsl:with-param name="returnType" select="$returnType"/>
            <xsl:with-param name="prefixedAttributeName" select="$prefixedAttributeName"/>
        </xsl:call-template>
        <xsl:call-template name="monoRelation__set">
            <xsl:with-param name="attributeName" select="$attributeName"/>
            <xsl:with-param name="returnType" select="$returnType"/>
            <xsl:with-param name="index" select="$index"/>
            <xsl:with-param name="relationHasForeignKeys" select="$relationHasForeignKeys"/>
            <xsl:with-param name="topicName" select="$mainTopicName"/>
            <xsl:with-param name="prefixedAttributeName" select="$prefixedAttributeName"/>
        </xsl:call-template>
    </xsl:template>

    <xsl:template name="monoRelation__get">
        <xsl:param name="attributeName"/>
        <xsl:param name="returnType"/>
        <xsl:param name="prefixedAttributeName"/>

        <!-- generates the following code for mono relation 'myBar'
        /**
         * Returns a reference to the related 'myBar' test.Bar. If the related object's state
         * is not yet available in DCPS, a DDS.NotFound exception will be thrown.
         *
         * @return the related object or null if no relation is managed.
         * @throws DDS.NotFound
         *            if the related object cannot be found.
         * @throws DDS.AlreadyDeleted if the current Object is already deleted.
         */
        -->
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * Returns a reference to the related '</xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>' </xsl:text>
        <xsl:value-of select="$returnType"/>
        <xsl:text>. If the related object's state</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * is not yet available in DCPS, a DDS.NotFound exception will be thrown.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * @return the related object or null if no relation is managed.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * @throws DDS.NotFound</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     *            if the related object cannot be found.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * @throws DDS.AlreadyDeleted if the current Object is already deleted.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>

        <!-- generates the following code for mono relation 'myBar'
        public final test.Bar get_myBar() throws DDS.NotFound {
            if(!myBarIsFound) {
                throw new DDS.NotFound("The related object represented by attribute 'myBar' could not be located by the DLRL");
            }
            return myBar;
        }
        -->
        <xsl:text>    </xsl:text>
        <xsl:text>public final </xsl:text>
        <xsl:value-of select="$returnType"/>
        <xsl:text> get_</xsl:text>
        <xsl:value-of select="$attributeName" />
        <xsl:text>() throws DDS.NotFound {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        if(!</xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>IsFound) {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>            throw new DDS.NotFound("The related object represented by attribute '</xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>' could not be located by the DLRL");</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        return </xsl:text>
        <xsl:value-of select="$prefixedAttributeName"/>
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
    </xsl:template>

    <xsl:template name="monoRelation__set">
        <xsl:param name="attributeName"/>
        <xsl:param name="returnType"/>
        <xsl:param name="index"/>
        <xsl:param name="relationHasForeignKeys"/>
        <xsl:param name="topicName"/>
        <xsl:param name="prefixedAttributeName"/>


        <!-- Generates for following code for mono relation 'myBar'
        /**
         * Sets a reference to the related 'myBar' test.Bar. Relationships may only
         * be set between registered objects. If this or one of the other preconditions is not met, an exception is raised.

         * A PreconditionNotMet is raised if any of the following preconditions is violated:
         * <ul><li>'Owner' ObjectRoot is not in a (writeable) CacheAccess;</li>
         * <li>If the value of the parameter is NIL, but the relation was modeled as a mandatory relation;</li>
         * <li>If the ObjectRoot in the parameter ('target') has different keys then the 'owner' ObjectRoot and the relation is mapped using so called 'shared' keys;</li>
         * <li>'Owner' ObjectRoot is not yet registered;</li>
         * <li>'Target' ObjectRoot is not yet registered;</li>
         * <li>'Target' ObjectRoot has already been deleted (this does not include marked for destruction!);</li>
         * <li>'Target' ObjectRoot does not belong to any CacheAccess or a different CacheAccess.</li></ul>
         *
         * @param val The new object to which to set the relation or null
         * @throws DDS.PreconditionNotMet if one of the preconditions is not met.
         * @throws DDS.AlreadyDeleted if the Object or it's cache access has already been deleted.
         */
         -->
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * Sets a reference to the related '</xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>' </xsl:text>
        <xsl:value-of select="$returnType"/>
        <xsl:text>. Relationships may only</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * be set between registered objects. If this or one of the other preconditions is not met, an exception is raised.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * A PreconditionNotMet is raised if any of the following preconditions is violated:</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * &lt;ul>&lt;li>'Owner' ObjectRoot is not in a (writeable) CacheAccess;&lt;/li></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * &lt;li>If the value of the parameter is NIL, but the relation was modeled as a mandatory relation;&lt;/li></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * &lt;li>If the ObjectRoot in the parameter ('target') has different keys then the 'owner' ObjectRoot and the relation is mapped using so called 'shared' keys;&lt;/li></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * &lt;li>'Owner' ObjectRoot is not yet registered;&lt;/li></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * &lt;li>'Target' ObjectRoot is not yet registered;&lt;/li></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * &lt;li>'Target' ObjectRoot has already been deleted (this does not include marked for destruction!);&lt;/li></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * &lt;li>'Target' ObjectRoot does not belong to any CacheAccess or a different CacheAccess.&lt;/li>&lt;/ul></xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * @param val The new object to which to set the relation or null</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * @throws DDS.PreconditionNotMet if one of the preconditions is not met.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     * @throws DDS.AlreadyDeleted if the Object or it's cache access has already been deleted.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL"/>

        <!-- Generates for following code for mono relation 'myBar'
            public final void set_myBar(test.Bar val) throws DDS.PreconditionNotMet {
                //changes the relationship within the DLRL kernel, also ensure the object is marked
                //as being changed if applicable.
                jniChangeRelationship(0, (DDS.ObjectRoot)val);
                //change the values in the topic
                if(val != null){
                    currentTopic.myBarIDName = val.get_barIDName();
                    currentTopic.myBarIDNumber = val.get_barIDNumber();
                }
                myBar = val;
                myBarIsFound = true;
            }
        -->
        <xsl:text>    </xsl:text>
        <xsl:text>public final void set_</xsl:text>
        <xsl:value-of select="$attributeName" />
        <xsl:text>(</xsl:text>
        <xsl:value-of select="$returnType"/>
        <xsl:text> val) throws DDS.PreconditionNotMet {</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        //changes the relationship within the DLRL kernel, also ensure the object is marked</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        //as being changed if applicable.</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:text>        jniChangeRelationship(</xsl:text>
        <xsl:value-of select="$index"/>
        <xsl:text>, (DDS.ObjectRoot)val);</xsl:text>
        <xsl:value-of select="$NL"/>

        <xsl:choose>
            <xsl:when test="(string-length($relationHasForeignKeys) != 0 and keyMapping) or validityField and not (oid) ">
                <xsl:text>        //change the values in the topic</xsl:text>
                <xsl:value-of select="$NL" />
                <xsl:text>        if(val != null){</xsl:text>
                <xsl:value-of select="$NL" />
                <xsl:for-each select="keyMapping">
                    <xsl:variable name="ownerFieldName" select="@ownerField"/>
                    <xsl:variable name="preFixedOwnerFieldName">
                        <xsl:call-template name="java-name">
                            <xsl:with-param name="name" select="@ownerField"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="isSharedKey">
                        <xsl:for-each select="ancestor::VALUEDEF/DCPSField">
                            <xsl:variable name="name" select="@name"/>
                            <xsl:variable name="topic" select="@topic"/>
                            <xsl:if test="$ownerFieldName=$name">
                                <xsl:if test="$topicName=$topic">
                                    <xsl:variable name="keyType" select="@keyType"/>
                                    <xsl:if test="$keyType='_SHARED_KEY'">
                                        <xsl:text>yes</xsl:text>
                                    </xsl:if>
                                </xsl:if>
                            </xsl:if>
                        </xsl:for-each>
                    </xsl:variable>

                    <xsl:if test="string-length($isSharedKey) = 0 ">
                        <xsl:text>            currentTopic.</xsl:text>
                        <xsl:value-of select="$preFixedOwnerFieldName"/>
                        <xsl:text> = val.get_</xsl:text>
                        <xsl:value-of select="@targetAttributeName"/>
                        <xsl:text>();</xsl:text>
                        <xsl:value-of select="$NL" />
                    </xsl:if>
                </xsl:for-each>
                <xsl:if test="validityField">
                    <xsl:text>            currentTopic.</xsl:text>
                    <xsl:value-of select="validityField/@name"/>
                    <xsl:text> = </xsl:text>
                    <xsl:value-of select="validityField/@validValue"/>
                    <xsl:text>;</xsl:text>
                    <xsl:value-of select="$NL" />
                </xsl:if>
                <xsl:text>        }</xsl:text>
                <xsl:value-of select="$NL" />
                <xsl:if test="validityField">
                    <xsl:text>        else {</xsl:text>
                    <xsl:value-of select="$NL" />
                    <xsl:text>            currentTopic.</xsl:text>
                    <xsl:value-of select="validityField/@name"/>
                    <xsl:text> = </xsl:text>
                    <xsl:value-of select="validityField/@invalidValue"/>
                    <xsl:text>;</xsl:text>
                    <xsl:value-of select="$NL" />
                    <xsl:text>        }</xsl:text>
                    <xsl:value-of select="$NL" />
                </xsl:if>
            </xsl:when>
            <xsl:otherwise>
                <xsl:if test="oid">
                    <xsl:text>        if(val != null){</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>            copyOidFrom(val, currentTopic.</xsl:text>
                    <xsl:value-of select="oid/@oidField"/>
                    <xsl:text>);</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:if test="oid/@nameField">
                        <xsl:text>            </xsl:text>
                        <xsl:text>currentTopic.</xsl:text>
                        <xsl:value-of select="oid/@nameField"/>
                        <xsl:text> = getMainTopicName(val);</xsl:text>
                        <xsl:value-of select="$NL"/>
                    </xsl:if>
                    <xsl:text>        } else {</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>            currentTopic.</xsl:text>
                    <xsl:value-of select="oid/@oidField"/>
                    <xsl:text>.systemId = 0;</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>            currentTopic.</xsl:text>
                    <xsl:value-of select="oid/@oidField"/>
                    <xsl:text>.localId = 0;</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:text>            currentTopic.</xsl:text>
                    <xsl:value-of select="oid/@oidField"/>
                    <xsl:text>.serial = 0;</xsl:text>
                    <xsl:value-of select="$NL"/>
                    <xsl:if test="oid/@nameField">
                        <xsl:text>            </xsl:text>
                        <xsl:text>currentTopic.</xsl:text>
                        <xsl:value-of select="oid/@nameField"/>
                        <xsl:text> = "";</xsl:text>
                        <xsl:value-of select="$NL"/>
                    </xsl:if>
                    <xsl:text>        }</xsl:text>
                    <xsl:value-of select="$NL"/>
                </xsl:if>
                <!-- for a singleton, which has no foreign keys or
                     for a relation mapped by shared keys, also execute
                     in oid mapping but although valid input it would be
                     really (emphasis on really) weird if someone uses a
                     seperate validity field when he uses oids as an oid
                     value of (0,0,0) is already able to indicate a null.
                     so we dont optimize this code generated, if we wont
                     spend time in combining the 'if' statements
                  -->
                <xsl:if test="validityField">
                    <xsl:text>        //change the values in the topic</xsl:text>
                    <xsl:value-of select="$NL" />
                    <xsl:text>        if(val != null){</xsl:text>
                    <xsl:value-of select="$NL" />
                    <xsl:text>            currentTopic.</xsl:text>
                    <xsl:value-of select="validityField/@name"/>
                    <xsl:text> = </xsl:text>
                    <xsl:value-of select="validityField/@validValue"/>
                    <xsl:text>;</xsl:text>
                    <xsl:value-of select="$NL" />
                    <xsl:text>        } else {</xsl:text>
                    <xsl:value-of select="$NL" />
                    <xsl:text>            currentTopic.</xsl:text>
                    <xsl:value-of select="validityField/@name"/>
                    <xsl:text> = </xsl:text>
                    <xsl:value-of select="validityField/@invalidValue"/>
                    <xsl:text>;</xsl:text>
                    <xsl:value-of select="$NL" />
                    <xsl:text>        }</xsl:text>
                    <xsl:value-of select="$NL" />
                </xsl:if>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>        </xsl:text>
        <xsl:value-of select="$prefixedAttributeName"/>
        <xsl:text> = val;</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>        </xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>IsFound = true;</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:value-of select="$NL" />
    </xsl:template>

    <xsl:template name="generate-multiRelation-operations">
        <xsl:variable name="attributeName" select="DECLARATOR/@NAME" />
        <xsl:variable name="index" select="TYPEREF/@INDEX"/>
        <xsl:variable name="type">
            <xsl:call-template name="resolveStatememberIdlType"/>
        </xsl:variable>
        <xsl:variable name="prefixedAttributeName">
            <xsl:call-template name="java-name">
                <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:for-each select="//VALUEFORWARDDEF[@NAME=$type]">
            <xsl:variable name="forwardPattern" select="@pattern"/>
            <xsl:variable name="forwardItemType" select="@itemType"/>
            <xsl:variable name="returnType">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text">
                        <xsl:value-of select="@itemType"/>
                        <xsl:value-of select="@pattern"/>
                    </xsl:with-param>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'.'"/>
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="prefixedAttributeName">
                <xsl:call-template name="java-name">
                    <xsl:with-param name="name" select="$attributeName"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:call-template name="multiRelation__get">
                <xsl:with-param name="attributeName" select="$attributeName"/>
                <xsl:with-param name="returnType" select="$returnType"/>
                <xsl:with-param name="prefixedAttributeName" select="$prefixedAttributeName"/>
            </xsl:call-template>

            <xsl:call-template name="multiRelation__set">
                <xsl:with-param name="attributeName" select="$attributeName"/>
                <xsl:with-param name="returnType" select="$returnType"/>
                <xsl:with-param name="forwardPattern" select="$forwardPattern"/>
                <xsl:with-param name="index" select="$index"/>
            </xsl:call-template>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="multiRelation__get">
        <xsl:param name="attributeName"/>
        <xsl:param name="returnType"/>
        <xsl:param name="prefixedAttributeName"/>

        <!-- Generates the following code for multiRelation 'bars' of type test::BarSet
            /**
             * Returns a reference to the corresponding 'bars' test.BarSet.
             *
             * @return the corresponding test.BarSet.
             */
            public final test.BarSet get_bars() {
                return bars;
            }
        -->
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * Returns a reference to the corresponding '</xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>' </xsl:text>
        <xsl:value-of select="$returnType"/>
        <xsl:text>.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @return the corresponding </xsl:text>
        <xsl:value-of select="$returnType"/>
        <xsl:text>.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>    </xsl:text>
        <xsl:text>public final </xsl:text>
        <xsl:value-of select="$returnType"/>
        <xsl:text> get_</xsl:text>
        <xsl:value-of select="$attributeName" />
        <xsl:text>() {</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>        return </xsl:text>
        <xsl:value-of select="$prefixedAttributeName"/>
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:value-of select="$NL" />
    </xsl:template>

    <xsl:template name="multiRelation__set">
        <xsl:param name="attributeName"/>
        <xsl:param name="returnType"/>
        <xsl:param name="forwardPattern"/>
        <xsl:param name="index"/>

        <!-- Generates the following code for multiRelation 'bars' of type test::BarSet
        /**
         * Sets the content of the object's 'bars' test.BarSet
         * to equal the content of the collection provided as argument. If neccesary this means adding/removing or re-arranging
         * elements in the object's collection in such a manner that the object's collection has the exact same content
         * as the collection provided as argument.
         * A PreconditionNotMet is raised if any of the following preconditions is violated:
         * <ul><li>ObjectRoot is not in a (writeable) cacheaccess;</li>
         * <li>Owner ObjectRoot of the destination collection is not yet registered;</li>
         * <li>Owner ObjectRoot of the source collection is not yet registered;</li>
         * <li>Owner ObjectRoot of the source collection has already been deleted (this does not include marked for destruction!);</li>
         * <li>Owner ObjectRoot of the source collection does not belong to any CacheAccess or a different CacheAccess.</li></ul>
         * A DDS.BadParameter is raised is a NIL value is provided with this function.
         *
         * @param The collection from which to copy the content (!=null)
         * @throws DDS.AlreadyDeleted if any of the involved collections has already been deleted.
         * @throws DDS.PreconditionNotMet if any of the preconditions are not met.
         * @throws DDS.BadParameter if the val parameter was a NIL value.
         */
        public final void set_bars(test.BarSet val) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
            jniChangeCollection(0, (DDS.Set)val);
        }
        -->
        <xsl:text>    /**</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * Sets the content of the object's '</xsl:text>
        <xsl:value-of select="$attributeName"/>
        <xsl:text>' </xsl:text>
        <xsl:value-of select="$returnType"/>
        <xsl:value-of select="$NL" />
        <xsl:text>     * to equal the content of the collection provided as argument. If neccesary this means adding/removing or re-arranging</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * elements in the object's collection in such a manner that the object's collection has the exact same content</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * as the collection provided as argument.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * A PreconditionNotMet is raised if any of the following preconditions is violated:</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * &lt;ul>&lt;li>ObjectRoot is not in a (writeable) cacheaccess;&lt;/li></xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * &lt;li>Owner ObjectRoot of the destination collection is not yet registered;&lt;/li></xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * &lt;li>Owner ObjectRoot of the source collection is not yet registered;&lt;/li></xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * &lt;li>Owner ObjectRoot of the source collection has already been deleted (this does not include marked for destruction!);&lt;/li></xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * &lt;li>Owner ObjectRoot of the source collection does not belong to any CacheAccess or a different CacheAccess.&lt;/li>&lt;/ul></xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * A DDS.BadParameter is raised is a NIL value is provided with this function.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     *</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @param The collection from which to copy the content (!=null)</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @throws DDS.AlreadyDeleted if any of the involved collections has already been deleted.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @throws DDS.PreconditionNotMet if any of the preconditions are not met.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     * @throws DDS.BadParameter if the val parameter was a NIL value.</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>     */</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:text>    </xsl:text>
        <xsl:text>public final void set_</xsl:text>
        <xsl:value-of select="$attributeName" />
        <xsl:text>(</xsl:text>
        <xsl:value-of select="$returnType"/>
        <xsl:text> val) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:choose>
            <xsl:when test="$forwardPattern='StrMap'">
                <xsl:text>        jniChangeCollection(</xsl:text>
                <xsl:value-of select="$index"/>
                <xsl:text>, (DDS.StrMap)val);</xsl:text>
                <xsl:value-of select="$NL" />
            </xsl:when>
            <xsl:when test="$forwardPattern='IntMap'">
                <xsl:text>        jniChangeCollection(</xsl:text>
                <xsl:value-of select="$index"/>
                <xsl:text>, (DDS.IntMap)val);</xsl:text>
                <xsl:value-of select="$NL" />
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>        jniChangeCollection(</xsl:text>
                <xsl:value-of select="$index"/>
                <xsl:text>, (DDS.Set)val);</xsl:text>
                <xsl:value-of select="$NL" />
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$NL" />
        <xsl:value-of select="$NL" />
    </xsl:template>

    <!--A setter for a mono attribute must only be generated if the dcps field doesn't
        represent a foreign key -->
    <xsl:template name="must-monoAttribute-setter-be-generated">
        <xsl:param name="valueFieldName"/>
        <xsl:param name="topicName"/>

        <xsl:if test="string-length($valueFieldName) != 0">
            <xsl:for-each select="ancestor::node()/DCPSField">
                <xsl:variable name="name" select="@name"/>
                <xsl:variable name="topic" select="@topic"/>

                <xsl:if test="$name=$valueFieldName">
                    <xsl:if test="$topicName=$topic">
                        <xsl:variable name="keyType" select="@keyType"/>
                        <xsl:variable name="immutable" select="@immutable"/>

                        <xsl:choose>
                            <xsl:when test="$immutable='true'">
                                <xsl:text>no</xsl:text>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:if test="$keyType='_FOREIGN_KEY'">
                                    <xsl:text>no</xsl:text>
                                </xsl:if>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:if>
                </xsl:if>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <xsl:template name="hasForeignKeysForMonoRelation">
        <xsl:param name="topicName"/>

        <xsl:for-each select="keyDescription/keyField">
            <xsl:variable name="dcpsFieldName" select="."/>

            <xsl:if test="string-length($dcpsFieldName) != 0">
                <xsl:for-each select="ancestor::node()/DCPSField">
                    <xsl:variable name="name" select="@name"/>
                    <xsl:variable name="topic" select="@topic"/>

                    <xsl:if test="$name=$dcpsFieldName">
                        <xsl:if test="$topicName=$topic">
                            <xsl:variable name="keyType" select="@keyType"/>

                            <xsl:if test="$keyType='_FOREIGN_KEY'">
                                <xsl:text>yes</xsl:text>
                            </xsl:if>
                        </xsl:if>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>
