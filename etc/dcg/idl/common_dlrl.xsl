<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:include href="../common/common_dlrl.xsl"/>
    <!--Example (from = '::' to = '.'):
        A::B::C::Bar -> A.B.C.Bar
    -->
    <xsl:template name="string-search-replace">
        <xsl:param name="text"/><!-- the text in which we will search for the 'from' token-->
        <xsl:param name="from"/><!-- the token we will search for in the text -->
        <xsl:param name="to"/><!-- the string with which we will replace each token we find in the text -->
        <xsl:param name="prefixKeywords"/><!-- yes if java keywords must be prefixed, any other value if not -->

        <xsl:choose>
            <xsl:when test="contains($text, $from)">
                <xsl:variable name="before" select="substring-before($text, $from)"/>
                <xsl:variable name="after"  select="substring-after($text, $from)"/>

                <xsl:choose>
                    <xsl:when test="$prefixKeywords='yes'">
                        <xsl:variable name="prefixedName">
                            <xsl:call-template name="idl-name">
                                <xsl:with-param name="name" select="$before"/>
                            </xsl:call-template>
                        </xsl:variable>
                        <xsl:value-of select="$prefixedName"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="$before"/>
                    </xsl:otherwise>
                </xsl:choose>

                <xsl:value-of select="$to"/>
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="$after"/>
                    <xsl:with-param name="from" select="$from"/>
                    <xsl:with-param name="to" select="$to"/>
                    <xsl:with-param name="prefixKeywords" select="$prefixKeywords"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$prefixKeywords='yes'">
                        <xsl:variable name="prefixedName">
                            <xsl:call-template name="idl-name">
                                <xsl:with-param name="name" select="$text"/>
                            </xsl:call-template>
                        </xsl:variable>
                        <xsl:value-of select="$prefixedName"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="$text"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>


    <!--Example:
        A::B::C::Bar -> Bar
    -->
    <xsl:template name="string-return-text-after-last-token">
        <xsl:param name="text"/><!-- the text in which we will search for the 'from' token-->
        <xsl:param name="token"/><!-- the token we will search for in the text -->

        <xsl:variable name="after" select="substring-after($text, $token)"/>
        <xsl:choose>
            <xsl:when test="contains($text, $token)">
                <xsl:call-template name="string-return-text-after-last-token">
                    <xsl:with-param name="text" select="$after"/>
                    <xsl:with-param name="token" select="$token"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$text"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!--Example (token = '::' to = '.'):
        A::B::C::Bar -> A.B.C
    -->
    <xsl:template name="string-search-replace-except-last">
        <xsl:param name="text"/>
        <xsl:param name="from"/>
        <xsl:param name="to"/>
        <xsl:param name="prefixKeywords"/>

        <xsl:if test="contains($text, $from)">
            <xsl:variable name="before" select="substring-before($text, $from)"/>
            <xsl:variable name="after" select="substring-after($text, $from)"/>
            <xsl:choose>
                <xsl:when test="contains($after, $from)">
                    <xsl:choose>
                        <xsl:when test="$prefixKeywords='yes'">
                            <xsl:variable name="prefixedName">
                                <xsl:call-template name="idl-name">
                                    <xsl:with-param name="name" select="$before"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:value-of select="$prefixedName"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="$before"/>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:value-of select="$to"/>
                    <xsl:call-template name="string-search-replace-except-last">
                        <xsl:with-param name="text" select="$after"/>
                        <xsl:with-param name="from" select="$from"/>
                        <xsl:with-param name="to" select="$to"/>
                        <xsl:with-param name="prefixKeywords" select="$prefixKeywords"/>
                    </xsl:call-template>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:choose>
                        <xsl:when test="$prefixKeywords='yes'">
                            <xsl:variable name="prefixedName">
                                <xsl:call-template name="idl-name">
                                    <xsl:with-param name="name" select="$before"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:value-of select="$prefixedName"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="$before"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>

    <!-- keywords of idl grammar v2.4.2 -->
    <xsl:template name="idl-name">
        <xsl:param name="name" />
        <xsl:choose>
            <xsl:when test="   $name = 'abstract'  or $name = 'double'     or $name = 'local'
                            or $name = 'raises'    or $name = 'typedef'    or $name = 'any'
                            or $name = 'exception' or $name = 'long'       or $name = 'readonly'
                            or $name = 'unsigned'  or $name = 'attribute'  or $name = 'enum'
                            or $name = 'module'    or $name = 'sequence'   or $name = 'union'
                            or $name = 'boolean'   or $name = 'factory'    or $name = 'native'
                            or $name = 'short'     or $name = 'ValueBase'  or $name = 'case'
                            or $name = 'FALSE'     or $name = 'Object'     or $name = 'string'
                            or $name = 'valuetype' or $name = 'public'     or $name = 'char'
                            or $name = 'fixed'     or $name = 'octet'      or $name = 'struct'
                            or $name = 'void'      or $name = 'const'      or $name = 'float'
                            or $name = 'oneway'    or $name = 'supports'   or $name = 'wchar'
                            or $name = 'context'   or $name = 'in'         or $name = 'out'
                            or $name = 'switch'    or $name = 'wstring'    or $name = 'custom'
                            or $name = 'inout'     or $name = 'private'    or $name = 'TRUE'
                            or $name = 'default'   or $name = 'interface'  or $name = 'truncatable'">
                <xsl:text>_</xsl:text>
                <xsl:value-of select="$name"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$name"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="idl-type">
        <xsl:variable name="type" select="TYPEREF/@TYPE"/>

        <xsl:choose>
            <xsl:when test="SEQUENCE">
                <xsl:text>sequence&lt;</xsl:text>
                <xsl:for-each select="SEQUENCE"><!-- move into the sequence element -->
                    <xsl:call-template name="idl-type"/>
                </xsl:for-each>
                <xsl:if test="not(SEQUENCE/@VALUE=0)">
                    <xsl:text>, </xsl:text>
                    <xsl:value-of select="SEQUENCE/@VALUE"/>
                </xsl:if>
                <xsl:text>> </xsl:text>
            </xsl:when>
            <xsl:when test="//TYPEDEF[DECLARATOR/@NAME=$type]">
                <xsl:for-each select="//TYPEDEF[DECLARATOR/@NAME=$type]">
                    <xsl:call-template name="idl-type"/>
                </xsl:for-each>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="VALUEFORWARDDEF[@NAME=$type]">
                        <xsl:value-of select="@itemType"/><xsl:value-of select="@pattern"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="$type"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>