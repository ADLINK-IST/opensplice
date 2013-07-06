<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:include href="../common/common_dlrl.xsl"/>
    <!--Example (from = '::' to = '.'):
        A::B::C::Bar -> A.B.C.Bar
    -->

    <xsl:template name="get_corba_module_name">
        <xsl:text>DDS</xsl:text>
    </xsl:template>

    <xsl:template name="string-search-replace">
        <xsl:param name="text"/><!-- the text in which we will search for the 'from' token-->
        <xsl:param name="from"/><!-- the token we will search for in the text -->
        <xsl:param name="to"/><!-- the string with which we will replace each token we find in the text -->
        <xsl:param name="prefixKeywords"/><!-- yes if C++ keywords must be prefixed, any other value if not -->

        <xsl:choose>
            <xsl:when test="contains($text, $from)">
                <xsl:variable name="before" select="substring-before($text, $from)"/>
                <xsl:variable name="after"  select="substring-after($text, $from)"/>

                <xsl:choose>
                    <xsl:when test="$prefixKeywords='yes'">
                        <xsl:variable name="prefixedName">
                            <xsl:call-template name="ccpp-name">
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
                            <xsl:call-template name="ccpp-name">
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
                                <xsl:call-template name="ccpp-name">
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
                                <xsl:call-template name="ccpp-name">
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

    <!-- 03-06-03 C++ language mapping specification keywords -->
    <xsl:template name="ccpp-name">
        <xsl:param name="name"/>

        <xsl:choose>
            <xsl:when test="   $name = 'and'        or $name = 'and_eq'     or $name = 'asm'
                            or $name = 'auto'       or $name = 'bitand'     or $name = 'bitor'
                            or $name = 'bool'       or $name = 'break'      or $name = 'case'
                            or $name = 'catch'      or $name = 'char'       or $name = 'class'
                            or $name = 'compl'      or $name = 'const'      or $name = 'const_cast'
                            or $name = 'continue'   or $name = 'default'    or $name = 'delete'
                            or $name = 'do'         or $name = 'double'     or $name = 'dynamic_cast'
                            or $name = 'else'       or $name = 'enum'       or $name = 'explicit'
                            or $name = 'export'     or $name = 'extern'     or $name = 'false'
                            or $name = 'float'      or $name = 'for'        or $name = 'friend'
                            or $name = 'goto'       or $name = 'if'         or $name = 'inline'
                            or $name = 'int'        or $name = 'long'       or $name = 'mutable'
                            or $name = 'namespace'  or $name = 'new'        or $name = 'not'
                            or $name = 'not_eq'     or $name = 'operator'   or $name = 'or'
                            or $name = 'or_eq'      or $name = 'private'    or $name = 'protected'
                            or $name = 'public'     or $name = 'register'   or $name = 'reinterpret_cast'
                            or $name = 'return'     or $name = 'short'      or $name = 'signed'
                            or $name = 'sizeof'     or $name = 'static'     or $name = 'static_cast'
                            or $name = 'struct'     or $name = 'switch'     or $name = 'template'
                            or $name = 'this'       or $name = 'throw'      or $name = 'true'
                            or $name = 'try'        or $name = 'typedef'    or $name = 'typeid'
                            or $name = 'typename'   or $name = 'union'      or $name = 'unsigned'
                            or $name = 'using'      or $name = 'virtual'    or $name = 'void'
                            or $name = 'volatile'   or $name = 'wchar_t'    or $name = 'while'
                            or $name = 'xor'        or $name = 'xor_eq'">
                <xsl:text>_cxx_</xsl:text>
                <xsl:value-of select="$name"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$name"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="ccpp-type">
        <xsl:variable name="type" select="TYPEREF/@TYPE"/>

        <xsl:choose>
            <xsl:when test="SEQUENCE">
                <xsl:text>sequence&lt;</xsl:text>
                <xsl:for-each select="SEQUENCE"><!-- move into the sequence element -->
                    <xsl:call-template name="ccpp-type"/>
                </xsl:for-each>
                <xsl:if test="not(SEQUENCE/@VALUE=0)">
                    <xsl:text>, </xsl:text>
                    <xsl:value-of select="SEQUENCE/@VALUE"/>
                </xsl:if>
                <xsl:text>> </xsl:text>
            </xsl:when>
            <xsl:when test="//TYPEDEF[DECLARATOR/@NAME=$type]">
                <xsl:for-each select="//TYPEDEF[DECLARATOR/@NAME=$type]">
                    <xsl:call-template name="ccpp-type"/>
                </xsl:for-each>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="VALUEFORWARDDEF[@NAME=$type]">
                        <xsl:value-of select="@itemType"/><xsl:value-of select="@pattern"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:variable name="tmp">
                            <xsl:call-template name="ccppType">
                                <xsl:with-param name="idlType" select="$type"/>
                            </xsl:call-template>
                        </xsl:variable>
                        <xsl:choose>
                            <xsl:when test="string-length($tmp)!=0">
                                <xsl:value-of select="$tmp"/>
                            </xsl:when>
                            <xsl:when test="STRING">
                                <xsl:text>char*</xsl:text>
                            </xsl:when>
                            <xsl:when test="WSTRING">
                                <xsl:call-template name="get_corba_module_name"/>
                                <xsl:text>::WChar*</xsl:text>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="$type"/>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="ccppType">
        <xsl:param name="idlType"/>

        <xsl:choose>
            <xsl:when test="$idlType = 'void'">
                <xsl:text>void</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'any'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::Any</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'boolean'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::Boolean</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'char'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::Char</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'float'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::Float</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'double'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::Double</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'short'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::Short</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'long'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::Long</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'long long'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::LongLong</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'unsigned short'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::UShort</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'unsigned long'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::ULong</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'unsigned long long'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::ULongLong</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'octet'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::Octet</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'long double'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::LongDouble</xsl:text>
            </xsl:when>
            <xsl:when test="$idlType = 'wchar'">
                <xsl:call-template name="get_corba_module_name"/>
                <xsl:text>::WChar</xsl:text>
            </xsl:when>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="get-ccpp-type-for-resolved-idl-type">
        <xsl:param name="idlType"/>

        <xsl:variable name="tmp">
            <xsl:call-template name="ccppType">
                <xsl:with-param name="idlType" select="$idlType"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:choose>
            <xsl:when test="string-length($tmp)!=0">
                 <xsl:value-of select="$tmp"/>
            </xsl:when>
            <xsl:when test="$idlType='STRING' or $idlType='WSTRING'">
                <xsl:text>char*</xsl:text>
            </xsl:when>
            <!-- if we got nothing back from the ccppType template then it must
                be some scoped name which we must convert to java name
            -->
            <xsl:otherwise>
               <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="$idlType"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/>
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!-- fixed and object types are not supported. It works as following:
        - First it checks if we detect an array
        - then it checks if we detect a sequence
        - then we check if we are dealing with a simple type & string/wstring
        - then we first try and resolve a typedef
        - then we check consequentively if its an enum, valuetype, struct, union
    -->

    <!-- NOTE: array and sequence not yet functional -->
    <xsl:template name="get-cpp-type-for-statemember-or-operation">
        <xsl:param name="type"/>
        <xsl:param name="attributeName"/>
        <!-- true or false -->
        <xsl:param name="varLength"/>
        <!-- 'in', 'out', 'inout', 'return' or 'none' -->
        <xsl:param name="idlParamArg"/>
        <!-- true or no value at all, true will mean arrays found at the
            current level wont be processed
        -->
        <xsl:param name="ignoreOneLevelOfArray"/>
        <!-- when we encounter a typedef we need to treat it in a special way.
             as 'name' for the type we want to use the typedef name, but to find
             out the special option for the 'in', 'out', 'inout' and 'return'
             types we need to recurse further into this template and we thus
             need to store this name so we can place it inbetween any pre and
             post values of a specific type. To illustrate an example:
             we encounter the following statemember:
             public SomeTypedef example;
             The type of this statemember is 'SomeTypedef', which is a typedef
             so we can not determine what we need to do for this statemember
             type just based upon this information. we need to take a look at
             the statemember and determine it's concrete type. Imagine the
             typedef is defined as following:
             typedef sequence<long> SomeTypedef;
             Now also imagine we are dealing with an in parameter type, as
             would be the case when we translate the beforementioned
             statemember to a setter operation. So the code we want to generate
             would look like:
             void set_example(const SomeTypedef& value){...}
             But to find out we need to prepend the 'const ' and append the '&'
             we do need to find out the sequence part! So we do need to
             recursive into this template, but we want to use the 'SomeTypedef'
             name inbetween those pre- and ap-pended values! So this is what
             we use the 'typedefTypeName' param &
             'typedefPackagePrefixed' param for, if it doesn't exist (ie has a
             length of 0) we ignore it, otherwise we use it! Long story? yes
             kinda, but hopefully it explains this abit, if not shame on one of
             us :)
        -->
        <xsl:param name="typedefTypeName"/>
        <xsl:param name="typedefPackagePrefixed"/>

        <xsl:variable name="corba-module">
            <xsl:call-template name="get_corba_module_name"/>
        </xsl:variable>

        <xsl:choose>
            <xsl:when test="DECLARATOR/ARRAY and not($ignoreOneLevelOfArray='true')">
                <xsl:if test="$idlParamArg='in'">
                    <xsl:text>const </xsl:text>
                </xsl:if>
                <xsl:choose>
                    <xsl:when test="($idlParamArg='return') or ($idlParamArg='out' and $varLength='true')">
                        <xsl:if test="string-length($typedefPackagePrefixed)!=0">
                            <xsl:value-of select="$typedefPackagePrefixed"/>
                            <xsl:text>::</xsl:text>
                        </xsl:if>
                        <xsl:if test="string-length($typedefTypeName)=0">
                            <xsl:text>_</xsl:text>
                        </xsl:if>
                        <xsl:value-of select="$attributeName"/>
                        <xsl:if test="$idlParamArg='return'">
                            <xsl:text>_slice*</xsl:text>
                        </xsl:if>
                        <xsl:if test="$idlParamArg='out' and $varLength='true'">
                            <xsl:text>_slice*&amp;</xsl:text>
                        </xsl:if>
                    </xsl:when>
                    <xsl:otherwise>
                        <!-- get the type in c++, make sure arrays are ignored for this run!! -->
                        <xsl:call-template name="get-cpp-type-for-statemember-or-operation">
                            <xsl:with-param name="type" select="$type"/>
                            <xsl:with-param name="attributeName" select="$attributeName"/>
                            <xsl:with-param name="varLength" select="$varLength"/>
                            <!-- we already identified the current param arg type,
                             so set it to none for the rest of the algorithm.
                            -->
                            <xsl:with-param name="idlParamArg" select="'none'"/>
                            <!-- we are recursing into this template again, but we dont want
                             the arrays to be processed again ofcourse, so we tell the template
                             to ignore arrays for the next recursion
                             -->
                            <xsl:with-param name="ignoreOneLevelOfArray" select="'true'"/>
                            <xsl:with-param name="typedefTypeName" select="$typedefTypeName"/>
                            <xsl:with-param name="typedefPackagePrefixed" select="$typedefPackagePrefixed"/>
                        </xsl:call-template>
                        <xsl:if test="string-length($typedefTypeName)=0">
                            <!-- append the correct number of brackets -->
                            <xsl:for-each select="DECLARATOR/ARRAY">
                                <xsl:text>[]</xsl:text>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="SEQUENCE">
                        <xsl:if test="$idlParamArg='in'">
                            <xsl:text>const </xsl:text>
                        </xsl:if>
                       <xsl:if test="string-length($typedefTypeName)!=0">
                            <xsl:value-of select="$typedefTypeName"/>
                        </xsl:if>
                        <xsl:if test="string-length($typedefTypeName)=0">
                            <xsl:text>_</xsl:text>
                            <xsl:value-of select="$attributeName"/>
                            <xsl:text>_seq</xsl:text>
                        </xsl:if>
                        <xsl:if test="$idlParamArg='out' or $idlParamArg='return'">
                            <xsl:text>*</xsl:text>
                        </xsl:if>
                        <xsl:if test="$idlParamArg='in' or $idlParamArg='out' or $idlParamArg='inout'">
                            <xsl:text>&amp;</xsl:text>
                        </xsl:if>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:choose>
                            <xsl:when test="STRING">
                                <xsl:if test="$idlParamArg='in'">
                                    <xsl:text>const </xsl:text>
                                </xsl:if>
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::Char*</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="WSTRING">
                                <xsl:if test="$idlParamArg='in'">
                                    <xsl:text>const </xsl:text>
                                </xsl:if>
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::WChar*</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='short'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::Short</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='long'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::Long</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='long long'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::LongLong</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='unsigned short'">
                                <xsl:value-of select="$corba-module"/>
                                <xsl:text>::UShort</xsl:text>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='unsigned long'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::ULong</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='unsigned long long'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::ULongLong</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='float'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::Float</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='double'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::Double</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='boolean'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::Boolean</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='char'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::Char</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='octet'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:value-of select="$corba-module"/>
                                    <xsl:text>::Octet</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='long double'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:text>CORBA::LongDouble</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='wchar'">
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                    <xsl:text>CORBA::WChar</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:when test="$type='any'">
                                <xsl:if test="$idlParamArg='in'">
                                    <xsl:text>const </xsl:text>
                                </xsl:if>
                               <xsl:if test="string-length($typedefTypeName)!=0">
                                    <xsl:value-of select="$typedefTypeName"/>
                                </xsl:if>
                                <xsl:if test="string-length($typedefTypeName)=0">
                                        <xsl:text>CORBA::Any</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='inout' or $idlParamArg='return'">
                                    <xsl:text>*</xsl:text>
                                </xsl:if>
                                <xsl:if test="$idlParamArg='in' or $idlParamArg='out' or $idlParamArg='inout'">
                                    <xsl:text>&amp;</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:choose>
                                    <xsl:when test="//TYPEDEF[DECLARATOR/@NAME=$type]">
                                        <!-- use for-each to actually enter the element -->
                                        <xsl:for-each select="//TYPEDEF[DECLARATOR/@NAME=$type]">
                                            <xsl:call-template name="get-cpp-type-for-statemember-or-operation">
                                                <xsl:with-param name="type" select="TYPEREF/@TYPE"/>
                                                <xsl:with-param name="varLength" select="$varLength"/>
                                                <xsl:with-param name="idlParamArg" select="$idlParamArg"/>
                                                <!-- we need to replace the attribute name with the name of the typedef, only do this the first time -->
                                                <xsl:with-param name="attributeName">
                                                       <xsl:if test="string-length($typedefTypeName)!=0">
                                                            <xsl:value-of select="$attributeName"/>
                                                        </xsl:if>
                                                        <xsl:if test="string-length($typedefTypeName)=0">
                                                            <xsl:call-template name="string-return-text-after-last-token">
                                                                <xsl:with-param name="text" select="DECLARATOR/@NAME"/>
                                                                <xsl:with-param name="token" select="'::'"/>
                                                            </xsl:call-template>
                                                        </xsl:if>
                                                </xsl:with-param>
                                                <xsl:with-param name="typedefTypeName">
                                                    <!-- for the first typedef we encounter provide the
                                                         correct name of the typedef for later use
                                                      -->
                                                   <xsl:if test="string-length($typedefTypeName)!=0">
                                                        <xsl:value-of select="$typedefTypeName"/>
                                                    </xsl:if>
                                                    <xsl:if test="string-length($typedefTypeName)=0">
                                                        <xsl:call-template name="string-search-replace">
                                                            <xsl:with-param name="text" select="$type"/>
                                                            <xsl:with-param name="from" select="'::'"/>
                                                            <xsl:with-param name="to" select="'::'"/>
                                                            <xsl:with-param name="prefixKeywords" select="'yes'"/>
                                                        </xsl:call-template>
                                                    </xsl:if>
                                                </xsl:with-param>
                                                <xsl:with-param name="typedefPackagePrefixed">
                                                    <!-- for the first typedef we encounter provide the
                                                         correct package name of the typedef for later use
                                                      -->
                                                   <xsl:if test="string-length($typedefTypeName)!=0">
                                                        <xsl:value-of select="$typedefPackagePrefixed"/>
                                                    </xsl:if>
                                                    <xsl:if test="string-length($typedefTypeName)=0">
                                                        <xsl:call-template name="string-search-replace-except-last">
                                                            <xsl:with-param name="text" select="$type"/>
                                                            <xsl:with-param name="from" select="'::'"/>
                                                            <xsl:with-param name="to" select="'::'"/>
                                                            <xsl:with-param name="prefixKeywords" select="'yes'"/>
                                                        </xsl:call-template>
                                                    </xsl:if>
                                                </xsl:with-param>
                                            </xsl:call-template>
                                        </xsl:for-each>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <xsl:choose>
                                            <xsl:when test="//ENUM[@NAME=$type]">
                                               <xsl:if test="string-length($typedefTypeName)!=0">
                                                    <xsl:value-of select="$typedefTypeName"/>
                                                </xsl:if>
                                                <xsl:if test="string-length($typedefTypeName)=0">
                                                    <!-- its just an enum type, for this we only need to replace
                                                         any c++ keyword we might find
                                                     -->
                                                    <xsl:call-template name="string-search-replace">
                                                        <xsl:with-param name="text" select="$type"/>
                                                        <xsl:with-param name="from" select="'::'"/>
                                                        <xsl:with-param name="to" select="'::'"/>
                                                        <xsl:with-param name="prefixKeywords" select="'yes'"/>
                                                    </xsl:call-template>
                                                </xsl:if>
                                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                                    <xsl:text>&amp;</xsl:text>
                                                </xsl:if>
                                            </xsl:when>
                                            <xsl:otherwise>
                                                <xsl:choose>
                                                    <!-- union and structs have the same rules -->
                                                    <xsl:when test="//STRUCT[@NAME=$type] or //UNION[@NAME=$type]">
                                                        <xsl:if test="$idlParamArg='in'">
                                                            <xsl:text>const </xsl:text>
                                                        </xsl:if>
                                                       <xsl:if test="string-length($typedefTypeName)!=0">
                                                            <xsl:value-of select="$typedefTypeName"/>
                                                        </xsl:if>
                                                        <xsl:if test="string-length($typedefTypeName)=0">
                                                            <xsl:call-template name="string-search-replace">
                                                                <xsl:with-param name="text" select="$type"/>
                                                                <xsl:with-param name="from" select="'::'"/>
                                                                <xsl:with-param name="to" select="'::'"/>
                                                                <xsl:with-param name="prefixKeywords" select="'yes'"/>
                                                            </xsl:call-template>
                                                        </xsl:if>
                                                        <xsl:if test="$varLength='true' and ($idlParamArg='out' or $idlParamArg='return')">
                                                            <xsl:text>*</xsl:text>
                                                        </xsl:if>
                                                        <xsl:if test="$idlParamArg='in' or $idlParamArg='out' or $idlParamArg='inout'">
                                                            <xsl:text>&amp;</xsl:text>
                                                        </xsl:if>
                                                    </xsl:when>
                                                    <xsl:otherwise>
                                                        <xsl:choose>
                                                            <xsl:when test="//VALUEDEF[@NAME=$type]">
                                                               <xsl:if test="string-length($typedefTypeName)!=0">
                                                                    <xsl:value-of select="$typedefTypeName"/>
                                                                </xsl:if>
                                                                <xsl:if test="string-length($typedefTypeName)=0">
                                                                    <xsl:call-template name="string-search-replace">
                                                                        <xsl:with-param name="text" select="$type"/>
                                                                        <xsl:with-param name="from" select="'::'"/>
                                                                        <xsl:with-param name="to" select="'::'"/>
                                                                        <xsl:with-param name="prefixKeywords" select="'yes'"/>
                                                                    </xsl:call-template>
                                                                </xsl:if>
                                                                <xsl:if test="$idlParamArg='in' or $idlParamArg='out' or $idlParamArg='inout' or $idlParamArg='return'">
                                                                    <xsl:text>*</xsl:text>
                                                                </xsl:if>
                                                                <xsl:if test="$idlParamArg='out' or $idlParamArg='inout'">
                                                                    <xsl:text>&amp;</xsl:text>
                                                                </xsl:if>
                                                            </xsl:when>
                                                        </xsl:choose>
                                                    </xsl:otherwise>
                                                </xsl:choose>
                                            </xsl:otherwise>
                                        </xsl:choose>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>