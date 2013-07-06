<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:include href="../common/common_dlrl.xsl"/>

    <!-- Always the same standard module structure -->
    <xsl:template match="IDL|MODULE">
        <xsl:apply-templates select="MODULE|VALUEDEF|ENUM|STRUCT|UNION" />
    </xsl:template>

    <!-- the package <name> declaration, if any. Any java keywords will be prefixed-->
    <xsl:template name="package-name">
        <xsl:if test="ancestor::MODULE">
            <xsl:text>package </xsl:text>
            <xsl:for-each select="ancestor::MODULE">
                <xsl:variable name="prefixedName">
                    <xsl:call-template name="java-name">
                        <xsl:with-param name="name" select="@NAME"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:value-of select="$prefixedName"/>
                <xsl:if test="position() != last()">
                    <xsl:text>.</xsl:text>
                </xsl:if>
            </xsl:for-each>
            <xsl:text>;</xsl:text>
        </xsl:if>
    </xsl:template>

    <!-- Support for operations, including parameters and return types and values -->

    <xsl:template name="operation-signature">
        <xsl:text>public </xsl:text>

        <!-- return type -->
        <xsl:call-template name="java-type" />
        <xsl:text> </xsl:text>

        <!-- operation name type -->
        <xsl:call-template name="java-name">
            <xsl:with-param name="name" select="@NAME"/>
        </xsl:call-template>

        <!-- parameters -->
        <xsl:text>(</xsl:text>
        <xsl:apply-templates select="PARAMETER" />
        <xsl:text>)</xsl:text>
    </xsl:template>

    <xsl:template match="PARAMETER">
        <xsl:call-template name="java-type" />
        <xsl:text> </xsl:text>
        <xsl:call-template name="java-name">
            <xsl:with-param name="name" select="@NAME"/>
        </xsl:call-template>
        <xsl:if test="position() &lt; last()">
            <xsl:text>, </xsl:text>
        </xsl:if>
    </xsl:template>

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
                            <xsl:call-template name="java-name">
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
                            <xsl:call-template name="java-name">
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
                                <xsl:call-template name="java-name">
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
                                <xsl:call-template name="java-name">
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

    <xsl:template name="default-return">
        <xsl:choose>
            <xsl:when test="TYPEREF/@TYPE = 'void'" />
            <xsl:when test="TYPEREF/@TYPE = 'boolean'">return false;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'char'">return 0;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'float'">return 0.0;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'double'">return 0.0;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'short'">return 0;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'long'">return 0;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'long long'">return 0;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'unsigned short'">return 0;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'unsigned long'">return 0;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'unsigned long long'">return 0;</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'octet'">return 0;</xsl:when>
            <xsl:otherwise>return null;</xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!-- Support for IDL-to-Java type conversion -->
    <xsl:template name="java-type">

        <xsl:choose>
            <xsl:when test="TYPEREF/@TYPE = 'void'">void</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'any'">java.lang.Object</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'boolean'">boolean</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'char'">char</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'float'">float</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'double'">double</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'short'">short</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'long'">int</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'long long'">long</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'unsigned short'">short</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'unsigned long'">int</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'unsigned long long'">long</xsl:when>
            <xsl:when test="TYPEREF/@TYPE = 'octet'">byte</xsl:when>
            <xsl:when test="STRING or WSTRING">java.lang.String</xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="java-type_user-defined-type"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="java-type_user-defined-type">
        <xsl:variable name="type" select="TYPEREF/@TYPE" />

        <xsl:choose>
            <xsl:when test="SEQUENCE">
                <xsl:apply-templates select="SEQUENCE"/>
            </xsl:when>
            <xsl:otherwise>
                <!-- maybe it's a typedef, find out and store the result in the variable -->
                <xsl:variable name="typedefType">
                    <xsl:call-template name="java-type_resolve-typedef">
                        <xsl:with-param name="type" select="$type"/>
                    </xsl:call-template>
                </xsl:variable>
                <!-- if we didnt find a typedef, then we can go and search for an enum/struct/union or valuetype -->
                <xsl:if test="string-length($typedefType)=0">
                    <xsl:for-each select="//ENUM[$type=@NAME]|//STRUCT[$type=@NAME]|//UNION[$type=@NAME]|//VALUEDEF[$type=@NAME]">
                        <xsl:call-template name="string-search-replace">
                            <xsl:with-param name="text" select="$type"/>
                            <xsl:with-param name="from" select="'::'"/>
                            <xsl:with-param name="to" select="'.'"/>
                            <xsl:with-param name="prefixKeywords" select="'yes'"/>
                        </xsl:call-template>
                    </xsl:for-each>
                </xsl:if>
                <!-- always print the result of the typedef, if it was
                    null this wont mean anything, but avoids an evaluation...
                 -->
                <xsl:value-of select="$typedefType"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="SEQUENCE">
        <xsl:call-template name="java-type"/>
    </xsl:template>

    <xsl:template name="java-type_resolve-typedef">
        <xsl:param name="type"/>

        <xsl:for-each select="//TYPEDEF[$type=DECLARATOR/@NAME]">
            <xsl:choose>
                <xsl:when test="SEQUENCE">
                    <xsl:apply-templates select="SEQUENCE"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:call-template name="java-type"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="package-prefix">
        <xsl:for-each select="ancestor::node()">
            <xsl:if test="@NAME != ''">
                <xsl:value-of select="@NAME" />
                <xsl:text>.</xsl:text>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>




    <xsl:template name="java-name">
        <xsl:param name="name" />
        <xsl:choose>
            <xsl:when test="$name = 'abstract'   or $name = 'boolean'    or $name = 'break'
                         or $name = 'byte'       or $name = 'case'       or $name = 'catch'
                         or $name = 'char'       or $name = 'class'      or $name = 'const'
                         or $name = 'continue'   or $name = 'default'    or $name = 'do'
                         or $name = 'double'     or $name = 'else'       or $name = 'extends'
                         or $name = 'false'      or $name = 'final'      or $name = 'finally'
                         or $name = 'float'      or $name = 'for'        or $name = 'goto'
                         or $name = 'if'         or $name = 'implements' or $name = 'import'
                         or $name = 'instanceof' or $name = 'int'        or $name = 'interface'
                         or $name = 'long'       or $name = 'native'     or $name = 'new'
                         or $name = 'null'       or $name = 'package'    or $name = 'private'
                         or $name = 'protected'  or $name = 'public'     or $name = 'return'
                         or $name = 'short'      or $name = 'static'     or $name = 'strictfp'
                         or $name = 'super'      or $name = 'switch'     or $name = 'synchronized'
                         or $name = 'this'       or $name = 'throw'      or $name = 'throws'
                         or $name = 'transient'  or $name = 'true'       or $name = 'try'
                         or $name = 'void'       or $name = 'volatile'   or $name = 'while'
                         or $name = 'assert'     or $name = 'enum'
                           ">_<xsl:value-of select="$name"/></xsl:when>
            <xsl:otherwise><xsl:value-of select="$name"/></xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="struct-or-union-clone_generateMemberClone">
        <xsl:param name="memberName"/>
        <xsl:param name="whiteSpaceStart"/>

        <xsl:variable name="javaType">
            <xsl:call-template name="java-type"/>
        </xsl:variable>
        <xsl:variable name="arrayBrackets">
            <xsl:call-template name="get-array-brackets">
                <xsl:with-param name="action">count</xsl:with-param>
                <xsl:with-param name="userData">0</xsl:with-param>
            </xsl:call-template>
        </xsl:variable>
        <xsl:if test="$arrayBrackets!='0'">
            <xsl:call-template name="struct-or-union-clone_generateArrayClone">
                <xsl:with-param name="classVar" select="$memberName"/>
                <xsl:with-param name="baseVar">this.<xsl:value-of select="$memberName"/></xsl:with-param>
                <xsl:with-param name="baseType" select="$javaType"/>
                <xsl:with-param name="baseTypeIDLStyle">
                    <xsl:call-template name="get-base-IDL-type-non-primitive-for-union-enum-struct"/>
                </xsl:with-param>
                <xsl:with-param name="currentPosition">1</xsl:with-param>
                <xsl:with-param name="whiteSpaceStart" select="$whiteSpaceStart"/>
                <xsl:with-param name="numberOfArrayBrackets" select="$arrayBrackets"/>
                <xsl:with-param name="maxNumberOfArrayBrackets" select="$arrayBrackets"/>
            </xsl:call-template>
        </xsl:if>
        <xsl:if test="$arrayBrackets = '0'">
            <xsl:choose>
                <xsl:when test="TYPEREF">
                    <xsl:variable name="memberTypeIDL">
                        <xsl:call-template name="get-base-IDL-type-non-primitive-for-union-enum-struct"/>
                    </xsl:variable>
                    <xsl:for-each select="//ENUM[@NAME=$memberTypeIDL]|//STRUCT[@NAME=$memberTypeIDL]|//UNION[@NAME=$memberTypeIDL]">
                        <xsl:variable name="nodeName" select="name()"/>
                        <xsl:variable name="memberTypeJava">
                            <xsl:call-template name="string-search-replace">
                                <xsl:with-param name="text" select="$memberTypeIDL"/>
                                <xsl:with-param name="from" select="'::'"/>
                                <xsl:with-param name="to" select="'.'"/>
                                <xsl:with-param name="prefixKeywords" select="'yes'"/>
                            </xsl:call-template>
                        </xsl:variable>
                        <xsl:choose>
                            <xsl:when test="$nodeName='ENUM'">
                                <xsl:value-of select="$whiteSpaceStart"/><xsl:text>if(this.</xsl:text><xsl:value-of select="$memberName"/><xsl:text> != null){</xsl:text><xsl:value-of select="$NL"/>
                                <xsl:value-of select="$whiteSpaceStart"/><xsl:text>    object.</xsl:text><xsl:value-of select="$memberName"/><xsl:text> = (</xsl:text><xsl:value-of select="$memberTypeJava"/><xsl:text>)this.</xsl:text><xsl:value-of select="$memberName"/><xsl:text>;</xsl:text><xsl:value-of select="$NL"/>
                                <xsl:value-of select="$whiteSpaceStart"/><xsl:text>}</xsl:text><xsl:value-of select="$NL"/>
                            </xsl:when>
                            <xsl:when test="$nodeName='STRUCT'">
                                <xsl:value-of select="$whiteSpaceStart"/><xsl:text>if(this.</xsl:text><xsl:value-of select="$memberName"/><xsl:text> != null){</xsl:text><xsl:value-of select="$NL"/>
                                <xsl:value-of select="$whiteSpaceStart"/><xsl:text>    object.</xsl:text><xsl:value-of select="$memberName"/><xsl:text> = (</xsl:text><xsl:value-of select="$memberTypeJava"/><xsl:text>)this.</xsl:text><xsl:value-of select="$memberName"/><xsl:text>.clone();</xsl:text><xsl:value-of select="$NL"/>
                                <xsl:value-of select="$whiteSpaceStart"/><xsl:text>}</xsl:text><xsl:value-of select="$NL"/>
                            </xsl:when>
                            <xsl:when test="$nodeName='UNION'">
                                <xsl:value-of select="$whiteSpaceStart"/><xsl:text>if(this.</xsl:text><xsl:value-of select="$memberName"/><xsl:text> != null){</xsl:text><xsl:value-of select="$NL"/>
                                <xsl:value-of select="$whiteSpaceStart"/><xsl:text>    object.</xsl:text><xsl:value-of select="$memberName"/><xsl:text> = (</xsl:text><xsl:value-of select="$memberTypeJava"/><xsl:text>)this.</xsl:text><xsl:value-of select="$memberName"/><xsl:text>.clone();</xsl:text><xsl:value-of select="$NL"/>
                                <xsl:value-of select="$whiteSpaceStart"/><xsl:text>}</xsl:text><xsl:value-of select="$NL"/>
                            </xsl:when>
                        </xsl:choose>
                    </xsl:for-each>
                </xsl:when>
            </xsl:choose>
        </xsl:if>
    </xsl:template>

    <xsl:template name="struct-or-union-clone_generateArrayClone">
        <xsl:param name="classVar"/>
        <xsl:param name="baseVar"/>
        <xsl:param name="baseType"/>
        <xsl:param name="baseTypeIDLStyle"/>
        <xsl:param name="currentPosition"/>
        <xsl:param name="whiteSpaceStart"/>
        <xsl:param name="maxNumberOfArrayBrackets"/>
        <xsl:param name="numberOfArrayBrackets"/>

        <xsl:variable name="arrayBrackets">
            <xsl:call-template name="struct-or-union-clone_getArrayBracketsForElement">
                <xsl:with-param name="number" select="$numberOfArrayBrackets"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:variable name="whiteSpace">
            <xsl:call-template name="struct-or-union-clone_getWhiteSpaceForElement">
                <xsl:with-param name="number" select="$numberOfArrayBrackets"/>
                <xsl:with-param name="maxNumber" select="$maxNumberOfArrayBrackets"/>
            </xsl:call-template>
            <xsl:value-of select="$whiteSpaceStart"/>
        </xsl:variable>

        <xsl:variable name="arrayBracketsElement">
            <xsl:call-template name="struct-or-union-clone_getArrayBracketsForElement">
                <xsl:with-param name="number" select="$numberOfArrayBrackets - 1"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:variable name="continue"><!-- only continue if there are more array dimensions-->
            <xsl:if test="$numberOfArrayBrackets != 0">
                <xsl:text>true</xsl:text>
            </xsl:if>
        </xsl:variable>

        <xsl:variable name="cloneable">
            <xsl:choose>
                <xsl:when test="$continue='true'">
                    <xsl:text>true</xsl:text>
                </xsl:when>
                <xsl:otherwise><!-- if its a union or struct, return true, else false(or nothing)-->
                    <xsl:if test="//STRUCT[@NAME=$baseTypeIDLStyle]|//UNION[@NAME=$baseTypeIDLStyle]">
                        <xsl:text>true</xsl:text>
                    </xsl:if>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:if test="$cloneable!='true'">
                <xsl:value-of select="$whiteSpace"/>
                <xsl:text>object.</xsl:text>
                <xsl:value-of select="$classVar"/>
                <xsl:text> = </xsl:text>
                <xsl:value-of select="$baseVar"/>
                <xsl:text>;</xsl:text>
                <xsl:value-of select="$NL"/>
        </xsl:if>
        <xsl:if test="$cloneable='true'">
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>if(</xsl:text>
            <xsl:value-of select="$baseVar"/>
            <xsl:text> != null){</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    object.</xsl:text>
            <xsl:value-of select="$classVar"/>
            <xsl:text> = (</xsl:text>
            <xsl:value-of select="$baseType"/>
            <xsl:value-of select="$arrayBrackets"/>
            <xsl:text>)</xsl:text>
            <xsl:value-of select="$baseVar"/>
            <xsl:text>.clone()</xsl:text>
            <xsl:text>;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:if test="$continue='true'">
                <xsl:value-of select="$whiteSpace"/>
                <xsl:text>    for(int count</xsl:text>
                <xsl:value-of select="$currentPosition"/>
                <xsl:text> = 0; count</xsl:text>
                <xsl:value-of select="$currentPosition"/>
                <xsl:text> &lt; </xsl:text>
                <xsl:value-of select="$baseVar"/>
                <xsl:text>.length; count</xsl:text>
                <xsl:value-of select="$currentPosition"/>
                <xsl:text>++){</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$whiteSpace"/>
                <xsl:text>        </xsl:text>
                <xsl:value-of select="$baseType"/>
                <xsl:value-of select="$arrayBracketsElement"/>
                <xsl:text> var</xsl:text>
                <xsl:value-of select="$currentPosition"/>
                <xsl:text> = </xsl:text>
                <xsl:value-of select="$baseVar"/>
                <xsl:text>[count</xsl:text>
                <xsl:value-of select="$currentPosition"/>
                <xsl:text>];</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:call-template name="struct-or-union-clone_generateArrayClone">
                    <xsl:with-param name="classVar"><xsl:value-of select="$classVar"/>[count<xsl:value-of select="$currentPosition"/>]</xsl:with-param>
                    <xsl:with-param name="baseVar">var<xsl:value-of select="$currentPosition"/></xsl:with-param>
                    <xsl:with-param name="baseType" select="$baseType"/>
                    <xsl:with-param name="currentPosition" select="$currentPosition+1"/>
                    <xsl:with-param name="whiteSpaceStart"><xsl:value-of select="$whiteSpaceStart"/><xsl:text>    </xsl:text></xsl:with-param>
                    <xsl:with-param name="baseTypeIDLStyle" select="$baseTypeIDLStyle"/>
                    <xsl:with-param name="numberOfArrayBrackets" select="$numberOfArrayBrackets - 1"/>
                    <xsl:with-param name="maxNumberOfArrayBrackets" select="$maxNumberOfArrayBrackets"/>
                </xsl:call-template>
                <xsl:value-of select="$whiteSpace"/><xsl:text>    }</xsl:text><xsl:value-of select="$NL"/>
            </xsl:if>
            <xsl:value-of select="$whiteSpace"/><xsl:text>}</xsl:text><xsl:value-of select="$NL"/>
        </xsl:if>
    </xsl:template>

    <xsl:template name="struct-or-union-clone_getArrayBracketsForElement">
        <xsl:param name="number"/>

        <xsl:if test="$number &gt; 0">
            <xsl:text>[]</xsl:text>
            <xsl:call-template name="struct-or-union-clone_getArrayBracketsForElement">
                <xsl:with-param name="number" select="$number - 1"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <xsl:template name="get-base-IDL-type-non-primitive-for-union-enum-struct">
        <xsl:for-each select="SEQUENCE">
            <xsl:call-template name="get-base-IDL-type-non-primitive-for-union-enum-struct"/>
        </xsl:for-each>
        <xsl:if test="not (SEQUENCE)">
            <xsl:variable name="type" select="TYPEREF/@TYPE" />
            <xsl:for-each select="//STRUCT[$type=@NAME]|//UNION[$type=@NAME]|//ENUM[$type=@NAME]">
                <xsl:value-of select="$type"/>
            </xsl:for-each>
            <xsl:for-each select="//TYPEDEF[$type=DECLARATOR/@NAME]"><!-- wont happen if we found a struct or union or enum -->
                <xsl:call-template name="get-base-IDL-type-non-primitive-for-union-enum-struct" />
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <xsl:template name="struct-or-union-clone_getWhiteSpaceForElement">
        <xsl:param name="number"/>
        <xsl:param name="maxNumber"/>

        <xsl:if test="$number &lt; $maxNumber">
            <xsl:text>    </xsl:text>
            <xsl:call-template name="struct-or-union-clone_getWhiteSpaceForElement">
                <xsl:with-param name="number" select="$number + 1"/>
                <xsl:with-param name="maxNumber" select="$maxNumber"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

<xsl:template name="generate_struct_or_union_attribute_initialized">
    <xsl:param name="name"/>
    <xsl:param name="accessorType"/>

    <xsl:variable name="arrayBrackets">
        <xsl:call-template name="get-array-brackets">
            <xsl:with-param name="action">bracket-only</xsl:with-param>
            <xsl:with-param name="userData">0</xsl:with-param>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="arrayBracketsValue">
        <xsl:call-template name="get-array-brackets">
            <xsl:with-param name="action">bracket-value</xsl:with-param>
            <xsl:with-param name="userData">0</xsl:with-param>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="javaType">
        <xsl:call-template name="java-type"/>
    </xsl:variable>
    <xsl:choose>
        <xsl:when test="string-length($arrayBrackets) != 0">
            <xsl:value-of select="$accessorType"/>
            <xsl:value-of select="$javaType"/>
            <xsl:value-of select="$arrayBrackets"/>
            <xsl:text> </xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text> = new </xsl:text>
            <xsl:value-of select="$javaType"/>
            <xsl:value-of select="$arrayBracketsValue"/>
            <xsl:text>;</xsl:text>
            <xsl:value-of select="$NL"/>
        </xsl:when>
        <xsl:when test="$javaType='java.lang.String'">
            <xsl:value-of select="$accessorType"/>
            <xsl:value-of select="$javaType"/>
            <xsl:text> </xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text> = "";</xsl:text>
            <xsl:value-of select="$NL"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:variable name="baseTypeIDLStyle">
                <xsl:call-template name="get-base-IDL-type-non-primitive-for-union-enum-struct"/>
            </xsl:variable>
            <xsl:choose>
                <xsl:when test="//STRUCT[@NAME=$baseTypeIDLStyle]|//UNION[@NAME=$baseTypeIDLStyle]">
                    <xsl:value-of select="$accessorType"/>
                    <xsl:value-of select="$javaType"/>
                    <xsl:text> </xsl:text>
                    <xsl:value-of select="$name"/>
                    <xsl:text> = new </xsl:text>
                    <xsl:value-of select="$javaType"/>
                    <xsl:text>();</xsl:text>
                    <xsl:value-of select="$NL"/>
                </xsl:when>
                <xsl:when test="//ENUM[@NAME=$baseTypeIDLStyle]">
                    <xsl:for-each select="//ENUM[@NAME=$baseTypeIDLStyle]"><!-- must actually go into the enum for it's first label-->
                        <xsl:value-of select="$accessorType"/>
                        <xsl:value-of select="$javaType"/>
                        <xsl:text> </xsl:text>
                        <xsl:value-of select="$name"/>
                        <xsl:text> = </xsl:text>
                        <xsl:value-of select="$javaType"/>
                        <xsl:text>.</xsl:text>
                        <xsl:value-of select="ENUMMEMBER/@NAME"/>
                        <xsl:text>;</xsl:text>
                        <xsl:value-of select="$NL"/>
                    </xsl:for-each>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$accessorType"/><xsl:value-of select="$javaType"/><xsl:value-of select="$arrayBrackets"/><xsl:text> </xsl:text><xsl:value-of select="$name"/><xsl:text>;</xsl:text><xsl:value-of select="$NL"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>

<xsl:template name="generate_union_or_struct_constructor_content">
    <xsl:param name="name"/>
    <xsl:variable name="arrayBracketsValue">
        <xsl:call-template name="get-array-brackets">
            <xsl:with-param name="action">bracket-value</xsl:with-param>
            <xsl:with-param name="userData">0</xsl:with-param>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="javaType">
        <xsl:call-template name="java-type"/>
    </xsl:variable>
    <xsl:if test="string-length($arrayBracketsValue) != 0">
        <xsl:if test="not(contains($arrayBracketsValue, '[0]'))"><!--do not do anything when sequences are involved -->
            <xsl:variable name="initInstruction">
                <xsl:variable name="baseTypeIDLStyle">
                    <xsl:call-template name="get-base-IDL-type-non-primitive-for-union-enum-struct"/>
                </xsl:variable>
                <xsl:choose>
                    <xsl:when test="//STRUCT[@NAME=$baseTypeIDLStyle]|//UNION[@NAME=$baseTypeIDLStyle]">
                        <xsl:text> = new </xsl:text>
                        <xsl:value-of select="$javaType"/>
                        <xsl:text>();</xsl:text>
                        <xsl:value-of select="$NL"/>
                    </xsl:when>
                    <xsl:when test="//ENUM[@NAME=$baseTypeIDLStyle]">
                        <xsl:for-each select="//ENUM[@NAME=$baseTypeIDLStyle]"><!-- must actually go into the enum for it's first label-->
                            <xsl:text> = </xsl:text>
                            <xsl:value-of select="$javaType"/>
                            <xsl:text>.</xsl:text>
                            <xsl:value-of select="ENUMMEMBER/@NAME"/>
                            <xsl:text>;</xsl:text>
                            <xsl:value-of select="$NL"/>
                        </xsl:for-each>
                    </xsl:when>
                </xsl:choose>
            </xsl:variable>
            <xsl:if test="string-length($initInstruction) != 0"><!-- only continue for an enum, union or struct -->
                <xsl:call-template name="generate_union_or_struct_constructor_content_generate_for_loop">
                    <xsl:with-param name="initInstruction" select="$initInstruction"/>
                    <xsl:with-param name="whiteSpace">
                        <xsl:text>        </xsl:text>
                    </xsl:with-param>
                    <xsl:with-param name="baseVar">
                        <xsl:text>this.</xsl:text>
                        <xsl:value-of select="$name"/>
                    </xsl:with-param>
                    <xsl:with-param name="remainingBrackets" select="$arrayBracketsValue"/>
                    <xsl:with-param name="number" select="1"/>
                </xsl:call-template>
            </xsl:if>
        </xsl:if>
    </xsl:if>
</xsl:template>

<xsl:template name="generate_union_or_struct_constructor_content_generate_for_loop">
    <xsl:param name="initInstruction"/>
    <xsl:param name="whiteSpace"/>
    <xsl:param name="baseVar"/>
    <xsl:param name="remainingBrackets"/>
    <xsl:param name="number"/>

    <xsl:variable name="before" select="substring-before($remainingBrackets, ']')"/>
    <xsl:variable name="after"  select="substring-after($remainingBrackets, ']')"/>
    <xsl:variable name="value" select="substring-after($before, '[')"/>

    <xsl:value-of select="$whiteSpace"/>
    <xsl:text>for(int count</xsl:text>
    <xsl:value-of select="$number"/>
    <xsl:text> = 0; count</xsl:text>
    <xsl:value-of select="$number"/>
    <xsl:text> &lt; </xsl:text>
    <xsl:value-of select="$value"/>
    <xsl:text>; count</xsl:text>
    <xsl:value-of select="$number"/>
    <xsl:text>++){</xsl:text>
    <xsl:value-of select="$NL"/>
    <xsl:choose>
        <xsl:when test="contains($after, '[')"><!-- more dimensions following?-->
            <xsl:call-template name="generate_union_or_struct_constructor_content_generate_for_loop">
                <xsl:with-param name="initInstruction" select="$initInstruction"/>
                <xsl:with-param name="whiteSpace">
                    <xsl:text>    </xsl:text>
                    <xsl:value-of select="$whiteSpace"/>
                </xsl:with-param>
                <xsl:with-param name="baseVar">
                    <xsl:value-of select="$baseVar"/>
                    <xsl:text>[count</xsl:text>
                    <xsl:value-of select="$number"/>
                    <xsl:text>]</xsl:text>
                </xsl:with-param>
                <xsl:with-param name="remainingBrackets" select="$after"/>
                <xsl:with-param name="number" select="$number +1"/>
            </xsl:call-template>
        </xsl:when>
        <xsl:otherwise><!-- end of the road, generate the init instruction-->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$baseVar"/>
            <xsl:text>[count</xsl:text>
            <xsl:value-of select="$number"/>
            <xsl:text>]</xsl:text>
            <xsl:value-of select="$initInstruction"/>
        </xsl:otherwise>
    </xsl:choose>
    <xsl:value-of select="$whiteSpace"/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$NL"/>
</xsl:template>


    <xsl:template name="javaType">
        <xsl:param name="idlType"/>
        <xsl:choose>
            <xsl:when test="$idlType='boolean'  or $idlType='char' or
                            $idlType='float'    or $idlType='double'">
                <xsl:value-of select="$idlType"/>
            </xsl:when>
            <xsl:when test="$idlType='short' or $idlType='unsigned short'">
                <xsl:text>short</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType='long' or $idlType='unsigned long'">
                <xsl:text>int</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType='long long' or $idlType='unsigned long long'">
                <xsl:text>long</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType='octet'">
                <xsl:text>byte</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType='any'">
                <xsl:text>java.lang.Object</xsl:text>
            </xsl:when>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="get-java-type-for-resolved-idl-type">
        <xsl:param name="idlType"/>

        <xsl:variable name="tmp">
            <xsl:call-template name="javaType">
                <xsl:with-param name="idlType" select="$idlType"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:choose>
            <xsl:when test="string-length($tmp)!=0">
                 <xsl:value-of select="$tmp"/>
            </xsl:when>
            <xsl:when test="$idlType='STRING' or $idlType='WSTRING'">
                <xsl:text>java.lang.String</xsl:text>
            </xsl:when>
            <!-- if we got nothing back from the javaType template then it must
                be some scoped name which we must convert to java name
            -->
            <xsl:otherwise>
               <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="$idlType"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'.'"/>
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>



