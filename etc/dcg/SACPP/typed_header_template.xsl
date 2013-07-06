<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xalan="http://xml.apache.org/xalan"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
    <xsl:param name="output.dir" select="'.'"/>
    <xsl:include href="../CCPP/common_dlrl.xsl"/>

    <xsl:template match="IDL">
        <xsl:variable name="filename">
            <xsl:text>ccpp_</xsl:text>
            <xsl:value-of select="@baseFile"/>
            <xsl:text>.h</xsl:text>
        </xsl:variable>
        <redirect:write file="{$output.dir}/{$filename}">
            <FOODLRL-out>
                <xsl:call-template name="copyright-notice"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#ifndef CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_H</xsl:text>
                <xsl:value-of select="$NL"/>

                <xsl:text>#define CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_H</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>

                <xsl:text>#include "ccpp_dds_dlrl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>Dlrl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "ccpp_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>Dlrl_impl.h"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:if test="string-length(@dcpsBaseFile)!=0">
                    <xsl:text>#include "ccpp_</xsl:text>
                    <xsl:value-of select="@dcpsBaseFile"/>
                    <xsl:text>.h"</xsl:text>
                    <xsl:value-of select="$NL"/>
                </xsl:if>
                <xsl:if test="string-length(@generatedDcpsBaseFile)!=0">
                    <xsl:text>#include "ccpp_</xsl:text>
                    <xsl:value-of select="@generatedDcpsBaseFile"/>
                    <xsl:text>.h"</xsl:text>
                    <xsl:value-of select="$NL"/>
                </xsl:if>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#endif /* CCPP_</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>_H */</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
            </FOODLRL-out>
        </redirect:write>
    </xsl:template>

</xsl:stylesheet>
