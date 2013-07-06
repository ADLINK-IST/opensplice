<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
<xsl:param name="output.dir" select="'.'"/>
<xsl:include href="typed_java_common.xsl"/>
<xsl:template match="UNION">
    <xsl:if test="@fromIncludedIdl='false' and @FORWARD='false'"><!-- redirected -->
    <xsl:variable name="filename">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text"><xsl:value-of select="@NAME"/></xsl:with-param>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'/'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
        <xsl:text>.java</xsl:text>
    </xsl:variable>
    <redirect:write file="{$output.dir}/{$filename}">
    <UNION-out>
    <xsl:variable name="prefixedName">
        <xsl:call-template name="java-name">
            <xsl:with-param name="name">
                <xsl:call-template name="string-return-text-after-last-token">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="token" select="'::'"/>
                </xsl:call-template>
            </xsl:with-param>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="union_nameFQ">
        <xsl:call-template name="string-search-replace">
            <xsl:with-param name="text" select="@NAME"/>
            <xsl:with-param name="from" select="'::'"/>
            <xsl:with-param name="to" select="'.'"/>
            <xsl:with-param name="prefixKeywords" select="'yes'"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="discriminatorName">
        <xsl:variable name="shouldPrefixDiscriminatorName">
            <xsl:call-template name="should-prefix-discriminator-name">
                <xsl:with-param name="prefixedName" select="$prefixedName"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="string-length($shouldPrefixDiscriminatorName) = 0">
                <xsl:text>discriminator</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>_discriminator</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:variable>
    <xsl:variable name="discrimintatorJavaType">
        <xsl:call-template name="java-type"/>
        <xsl:call-template name="get-array-brackets">
            <xsl:with-param name="action">bracket-only</xsl:with-param>
            <xsl:with-param name="userData">0</xsl:with-param>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="baseIDLType">
        <xsl:call-template name="get-base-IDL-type-non-primitive-for-union-enum-struct"/>
    </xsl:variable>
    <xsl:variable name="defaultCaseValueVar">
        <xsl:call-template name="get-case-discriminator-value">
            <xsl:with-param name="discrimintatorJavaType" select="$discrimintatorJavaType"/>
            <xsl:with-param name="baseIDLType" select="$baseIDLType"/>
            <xsl:with-param name="value" select="@defaultCaseValue"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="defaultCaseValueVarNumberOnly" select="@defaultCaseValue"/>
<xsl:value-of select="$NL"/>
<xsl:call-template name="copyright-notice" />
<xsl:value-of select="$NL"/>
<xsl:call-template name="package-name"/>


public final class <xsl:value-of select="$prefixedName"/> implements Cloneable{

    private <xsl:value-of select="$discrimintatorJavaType"/> _d;

    public <xsl:value-of select="$prefixedName"/>(){
        //must init first branch
<xsl:for-each select="BRANCH">
    <xsl:variable name="name">
        <xsl:call-template name="java-name">
            <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:if test="position()=1">
        <xsl:variable name="firstCaseDiscriminatorValue">
            <xsl:variable name="disValue">
                <xsl:choose>
                    <xsl:when test="string-length(CASE/@isDefault)=0">
                        <xsl:value-of select="CASE/@VALUE"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="$defaultCaseValueVarNumberOnly"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:variable>
            <xsl:call-template name="get-case-discriminator-value">
                <xsl:with-param name="discrimintatorJavaType" select="$discrimintatorJavaType"/>
                <xsl:with-param name="baseIDLType" select="$baseIDLType"/>
                <xsl:with-param name="value" select="$disValue"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:text>        this._d = </xsl:text>
        <xsl:value-of select="$firstCaseDiscriminatorValue"/>
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$NL"/>
        <xsl:call-template name="generate_union_or_struct_constructor_content">
            <xsl:with-param name="name">
                <xsl:text>__</xsl:text>
                <xsl:value-of select="$name"/>
            </xsl:with-param>
        </xsl:call-template>
    </xsl:if>
</xsl:for-each>

    }

    public <xsl:value-of select="$discrimintatorJavaType"/><xsl:text> </xsl:text><xsl:value-of select="$discriminatorName"/>(){
        return this._d;
    }

<xsl:for-each select="BRANCH">
    <xsl:variable name="branchName">
        <xsl:call-template name="java-name">
            <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="javaTypeWithBrackets">
        <xsl:call-template name="java-type"/>
        <xsl:call-template name="get-array-brackets">
            <xsl:with-param name="action">bracket-only</xsl:with-param>
            <xsl:with-param name="userData">0</xsl:with-param>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="firstCaseDiscriminatorValue">
        <xsl:variable name="disValue">
            <xsl:choose>
                <xsl:when test="string-length(CASE/@isDefault)=0">
                    <xsl:value-of select="CASE/@VALUE"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$defaultCaseValueVarNumberOnly"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:call-template name="get-case-discriminator-value">
            <xsl:with-param name="discrimintatorJavaType" select="$discrimintatorJavaType"/>
            <xsl:with-param name="baseIDLType" select="$baseIDLType"/>
            <xsl:with-param name="value" select="$disValue"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="discriminatCheck">
        <xsl:for-each select="CASE">
            <xsl:variable name="caseValue">
                <xsl:variable name="disValue">
                    <xsl:choose>
                        <xsl:when test="string-length(@isDefault)=0">
                            <xsl:value-of select="@VALUE"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="$defaultCaseValueVarNumberOnly"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:call-template name="get-case-discriminator-value">
                    <xsl:with-param name="discrimintatorJavaType" select="$discrimintatorJavaType"/>
                    <xsl:with-param name="baseIDLType" select="$baseIDLType"/>
                    <xsl:with-param name="value" select="$disValue"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:text>(_d != (</xsl:text><xsl:value-of select="$caseValue"/><xsl:text>))</xsl:text><xsl:if test="position()!=last()"> &amp;&amp; </xsl:if>
        </xsl:for-each>
    </xsl:variable>
    <xsl:choose>
        <xsl:when test="position()=1">
            <xsl:call-template name="generate_struct_or_union_attribute_initialized">
                <xsl:with-param name="name">
                    <xsl:text>__</xsl:text>
                    <xsl:value-of select="$branchName"/>
                </xsl:with-param>
                <xsl:with-param name="accessorType">
                    <xsl:text>    private </xsl:text>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
            <xsl:text>    private </xsl:text>
            <xsl:value-of select="$javaTypeWithBrackets"/>
            <xsl:text> __</xsl:text>
            <xsl:value-of select="$branchName"/>
            <xsl:text>;</xsl:text>
            <xsl:value-of select="$NL"/>
        </xsl:otherwise>
    </xsl:choose>
    public <xsl:value-of select="$javaTypeWithBrackets"/><xsl:text> </xsl:text><xsl:value-of select="$branchName"/>(){
        if (<xsl:value-of select="$discriminatCheck"/>){
            throw org.opensplice.dds.dcps.Utilities.createException(
                        org.opensplice.dds.dcps.Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
        return this.__<xsl:value-of select="$branchName"/>;
    }

    public void <xsl:value-of select="$branchName"/>(<xsl:value-of select="$javaTypeWithBrackets"/> val){
        this.__<xsl:value-of select="$branchName"/> = val;
        this._d = <xsl:value-of select="$firstCaseDiscriminatorValue"/>;
    }

    public void <xsl:value-of select="$branchName"/> (<xsl:value-of select="$discrimintatorJavaType"/> _d, <xsl:value-of select="$javaTypeWithBrackets"/> val){
        if (<xsl:value-of select="$discriminatCheck"/>){
            throw org.opensplice.dds.dcps.Utilities.createException(
                        org.opensplice.dds.dcps.Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
        this.__<xsl:value-of select="$branchName"/> = val;
        this._d = _d;
    }
</xsl:for-each>

<xsl:if test="@defaultCaseAllowed='true' and @hasDefaultCase='false'">
    public void __default(){
        this._d = <xsl:value-of select="$defaultCaseValueVar"/>;
    }

    public void __default (<xsl:value-of select="$discrimintatorJavaType"/> d){
        if (<xsl:call-template name="get-possible-discriminator-values"><xsl:with-param name="discrimintatorJavaType" select="$discrimintatorJavaType"/><xsl:with-param name="baseIDLType" select="$baseIDLType"/></xsl:call-template>){
            throw org.opensplice.dds.dcps.Utilities.createException(
                        org.opensplice.dds.dcps.Utilities.EXCEPTION_TYPE_BAD_OPERATION, null);
        }
        this._d = d;
    }
</xsl:if>
<xsl:call-template name="generate-clone-operation">
    <xsl:with-param name="union_nameFQ" select="$union_nameFQ"/>
    <xsl:with-param name="defaultCaseValueVarNumberOnly" select="$defaultCaseValueVarNumberOnly"/>
    <xsl:with-param name="discrimintatorJavaType" select="$discrimintatorJavaType"/>
    <xsl:with-param name="baseIDLType" select="$baseIDLType"/>
</xsl:call-template>

}
    </UNION-out>
    </redirect:write>
    </xsl:if>
</xsl:template>

<xsl:template name="get-possible-discriminator-values">
    <xsl:param name="discrimintatorJavaType"/>
    <xsl:param name="baseIDLType"/>

    <xsl:for-each select="BRANCH/CASE">
        <xsl:if test="string-length(@isDefault)= 0">
            <xsl:if test="position() != '1'">
                <xsl:text> || </xsl:text>
            </xsl:if>
            <xsl:text>d == </xsl:text>
            <xsl:call-template name="get-case-discriminator-value">
                <xsl:with-param name="discrimintatorJavaType" select="$discrimintatorJavaType"/>
                <xsl:with-param name="baseIDLType" select="$baseIDLType"/>
                <xsl:with-param name="value" select="@VALUE"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

<xsl:template name="get-case-discriminator-value">
    <xsl:param name="discrimintatorJavaType"/>
    <xsl:param name="baseIDLType"/>
    <xsl:param name="value"/>

    <xsl:choose>
        <xsl:when test="//ENUM[$baseIDLType=@NAME]"><!-- enum type -->
            <xsl:value-of select="$discrimintatorJavaType"/><xsl:text>.from_int(</xsl:text><xsl:value-of select="$value"/><xsl:text>)</xsl:text>
        </xsl:when>
        <xsl:when test="$discrimintatorJavaType='char'"><!-- char type -->
            <xsl:text>(char)</xsl:text><xsl:value-of select="$value"/>
        </xsl:when>
        <xsl:when test="$discrimintatorJavaType='boolean'"><!-- boolean type -->
            <xsl:choose>
                <xsl:when test="$value='1'">
                    <xsl:text>true</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>false</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:when>
        <xsl:otherwise><!-- integer type -->
            <xsl:value-of select="$value"/>
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>

<xsl:template name="generate-clone-operation">
    <xsl:param name="union_nameFQ"/>
    <xsl:param name="defaultCaseValueVarNumberOnly"/>
    <xsl:param name="discrimintatorJavaType"/>
    <xsl:param name="baseIDLType"/>

    <xsl:text>    /* Creates a deep clone of this union </xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>     *</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>     * If one of the contained classes does not support cloning then null is returned.</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>     * Does not perform cycle detection.</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>     */</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>    public Object clone(){</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>        try{</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>            </xsl:text><xsl:value-of select="$union_nameFQ"/><xsl:text> object = (</xsl:text><xsl:value-of select="$union_nameFQ"/><xsl:text>)super.clone();</xsl:text><xsl:value-of select="$NL"/>
    <xsl:for-each select="BRANCH">
        <xsl:variable name="branchName">
            <xsl:call-template name="java-name">
                <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="cloneCode">
            <xsl:call-template name="struct-or-union-clone_generateMemberClone">
                <xsl:with-param name="memberName">__<xsl:value-of select="$branchName"/></xsl:with-param>
                <xsl:with-param name="whiteSpaceStart"><xsl:text>                </xsl:text></xsl:with-param>
            </xsl:call-template>
        </xsl:variable>
        <xsl:if test="string-length($cloneCode) != 0">
            <xsl:variable name="caseDiscriminatorValueold">
                <xsl:variable name="disValue">
                    <xsl:choose>
                        <xsl:when test="string-length(@isDefault)=0">
                            <xsl:value-of select="@VALUE"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="$defaultCaseValueVarNumberOnly"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:call-template name="get-case-discriminator-value">
                    <xsl:with-param name="discrimintatorJavaType" select="$discrimintatorJavaType"/>
                    <xsl:with-param name="baseIDLType" select="$baseIDLType"/>
                    <xsl:with-param name="value" select="$disValue"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="caseDiscriminatorValue">
                <xsl:for-each select="CASE">
                    <xsl:variable name="caseValue">
                        <xsl:variable name="disValue">
                            <xsl:choose>
                                <xsl:when test="string-length(@isDefault)=0">
                                    <xsl:value-of select="@VALUE"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:value-of select="$defaultCaseValueVarNumberOnly"/>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:call-template name="get-case-discriminator-value">
                            <xsl:with-param name="discrimintatorJavaType" select="$discrimintatorJavaType"/>
                            <xsl:with-param name="baseIDLType" select="$baseIDLType"/>
                            <xsl:with-param name="value" select="$disValue"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:text>(this._d == (</xsl:text><xsl:value-of select="$caseValue"/><xsl:text>))</xsl:text><xsl:if test="position()!=last()"> || </xsl:if>
                </xsl:for-each>
            </xsl:variable>
            <xsl:text>            if(</xsl:text><xsl:value-of select="$caseDiscriminatorValue"/><xsl:text>){</xsl:text><xsl:value-of select="$NL"/>
            <xsl:value-of select="$cloneCode"/>
            <xsl:text>            }</xsl:text><xsl:value-of select="$NL"/>
        </xsl:if>
    </xsl:for-each>
    <xsl:text>            return object;</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>        } catch (java.lang.CloneNotSupportedException e){</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>            return null;</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>        }</xsl:text><xsl:value-of select="$NL"/>
    <xsl:text>  }</xsl:text><xsl:value-of select="$NL"/>
</xsl:template>

<xsl:template name="should-prefix-discriminator-name">
    <xsl:param name="prefixedName"/>

    <xsl:choose>
        <xsl:when test="$prefixedName='discriminator'">
            <xsl:text>true</xsl:text>
        </xsl:when>
        <xsl:otherwise>
            <xsl:for-each select="CASE">
                <xsl:variable name="branchName">
                    <xsl:call-template name="java-name">
                        <xsl:with-param name="name" select="DECLARATOR/@NAME"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:if test="$branchName='discriminator'">
                    <xsl:text>true</xsl:text>
                </xsl:if>
            </xsl:for-each>
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>

</xsl:stylesheet>
