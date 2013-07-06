<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xalan="http://xml.apache.org/xalan"
    xmlns:redirect="http://xml.apache.org/xalan/redirect"
    extension-element-prefixes="redirect">
    <xsl:param name="output.dir" select="'.'"/>
    <xsl:include href="common_dlrl.xsl"/>

    <xsl:template match="IDL">
        <xsl:variable name="filename">
            <xsl:value-of select="@baseFile"/>
            <xsl:text>Dlrl.idl</xsl:text>
        </xsl:variable>
        <redirect:write file="{$output.dir}/ptdlrltmp/{$filename}">
            <FOODLRL-out>
                <xsl:call-template name="copyright-notice"/>
                <xsl:value-of select="$NL"/>
                <xsl:text>#include "</xsl:text>
                <xsl:value-of select="@baseFile"/>
                <xsl:text>.idl"</xsl:text>
                <xsl:value-of select="$NL"/>
                <xsl:value-of select="$NL"/>
                <xsl:for-each select="VALUEDEF">
                    <xsl:call-template name="VALUEDEF">
                        <xsl:with-param name="whiteSpace"/>
                    </xsl:call-template>
                </xsl:for-each>
                <xsl:for-each select="MODULE">
                    <xsl:call-template name="MODULE">
                        <xsl:with-param name="whiteSpace"/>
                    </xsl:call-template>
                </xsl:for-each>
            </FOODLRL-out>
        </redirect:write>
    </xsl:template>

    <xsl:template name="MODULE">
        <xsl:param name="whiteSpace"/>

        <xsl:variable name="prefixedName">
            <xsl:call-template name="idl-name">
                <xsl:with-param name="name" select="@NAME"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="contains-dlrl-valuetypes">
            <xsl:for-each select="VALUEDEF">
                <xsl:variable name="isNonIncludedSharedValueDef">
                    <xsl:call-template name="does-valuedef-inherit-from-objectroot-and-is-not-included"/>
                </xsl:variable>
                <xsl:if test="$isNonIncludedSharedValueDef='true'">
                    <xsl:text>true</xsl:text>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>

        <xsl:if test="string-length($contains-dlrl-valuetypes)!=0">
            <xsl:text>module </xsl:text>
            <xsl:value-of select="$prefixedName"/>
            <xsl:text>{</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:for-each select="VALUEDEF">
                <xsl:call-template name="VALUEDEF">
                    <xsl:with-param name="whiteSpace">
                        <xsl:text>    </xsl:text>
                        <xsl:value-of select="$whiteSpace"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:for-each>
            <xsl:for-each select="MODULE">
                <xsl:call-template name="MODULE">
                    <xsl:with-param name="whiteSpace">
                        <xsl:text>    </xsl:text>
                        <xsl:value-of select="$whiteSpace"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:for-each>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
        </xsl:if>
    </xsl:template>

    <xsl:template name="VALUEDEF">
        <xsl:param name="whiteSpace"/>

        <xsl:variable name="isNonIncludedSharedValueDef">
            <xsl:call-template name="does-valuedef-inherit-from-objectroot-and-is-not-included"/>
        </xsl:variable>

        <xsl:if test="$isNonIncludedSharedValueDef='true'">
            <xsl:variable name="prefixedFullName">
                <xsl:call-template name="string-search-replace">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="nonPrefixedName">
                <xsl:call-template name="string-return-text-after-last-token">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="token" select="'::'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="prefixedFullNameExceptLast">
                <xsl:call-template name="string-search-replace-except-last">
                    <xsl:with-param name="text" select="@NAME"/>
                    <xsl:with-param name="from" select="'::'"/>
                    <xsl:with-param name="to" select="'::'"/><!-- i.e., dont replace anything we will just prefix-->
                    <xsl:with-param name="prefixKeywords" select="'yes'"/>
                </xsl:call-template>
                <xsl:text>::</xsl:text>
                <xsl:value-of select="$nonPrefixedName"/>
            </xsl:variable>


            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>/****** Generated for type: </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> ******/</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- generate neccesary typedefs -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>typedef sequence&lt;</xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text>> </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Seq;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSelectionListener :
                local interface FooSelectionListener : DDS::SelectionListener {
                    void on_object_modified (in test::Foo the_object);
                    void on_object_in (in test::Foo the_object);
                    void on_object_out (in test::Foo the_object);
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>local interface </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>SelectionListener : DDS::SelectionListener {</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void on_object_modified (in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> the_object);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void on_object_in (in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> the_object);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void on_object_out (in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> the_object);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooListener :
                local interface FooListener : DDS::ObjectListener {
                    boolean on_object_modified (in test::Foo the_object);
                    boolean on_object_created (in test::Foo the_object);
                    boolean on_object_deleted (in test::Foo the_object);
                };

                typedef sequence<test::FooListener> FooListenerSeq;
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>local interface </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Listener : DDS::ObjectListener {</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    boolean on_object_modified (in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> the_object);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    boolean on_object_created (in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> the_object);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    boolean on_object_deleted (in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> the_object);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>typedef sequence&lt;</xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener> </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>ListenerSeq;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooFilterInterface :
                local interface FooFilter : DDS::FilterCriterion {
                    boolean check_object(in test::Foo an_object, in DDS::MembershipState membership_state);
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>local interface </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>FilterInterface : DDS::FilterCriterion {</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    boolean check_object(in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> an_object, in DDS::MembershipState membership_state);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSelection :
                local interface FooSelection : DDS::Selection {
                    test::FooSeq members() raises (DDS::AlreadyDeleted);
                    test::FooSeq get_inserted_members() raises (DDS::AlreadyDeleted);
                    test::FooSeq get_modified_members() raises (DDS::AlreadyDeleted);
                    test::FooSeq get_removed_members() raises (DDS::AlreadyDeleted);
                    test::FooSelectionListener set_listener(in test::FooSelectionListener listener) raises(DDS::AlreadyDeleted);
                    test::FooSelectionListener listener() raises (DDS::AlreadyDeleted);
                };

                typedef sequence<test::FooSelection> FooSelectionSeq;
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>local interface </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Selection : DDS::Selection {</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq members() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq get_inserted_members() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq get_modified_members() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq get_removed_members() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>SelectionListener set_listener(in </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>SelectionListener listener) raises(DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>SelectionListener listener() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>typedef sequence&lt;</xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection> </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>SelectionSeq;</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooSet :
                valuetype FooSet : DDS::Set {
                    void add(in test::Foo value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
                    test::FooSeq added_elements() raises (DDS::AlreadyDeleted);
                    boolean contains(in test::Foo value) raises (DDS::AlreadyDeleted);
                    void remove(in test::Foo value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
                    test::FooSeq values() raises (DDS::AlreadyDeleted);
                    test::FooSeq removed_elements() raises (DDS::AlreadyDeleted);
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>valuetype </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>Set : DDS::Set {</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void add(in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq added_elements() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    boolean contains(in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> value) raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void remove(in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq values() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq removed_elements() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooStrMap :
                valuetype FooStrMap : DDS::StrMap {
                    test::FooSeq values() raises (DDS::AlreadyDeleted);
                    test::Foo get(in string key) raises (DDS::AlreadyDeleted, DDS::NoSuchElement);
                    void put(in string key, in test::Foo value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>valuetype </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>StrMap : DDS::StrMap {</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq values() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> get(in string key) raises (DDS::AlreadyDeleted, DDS::NoSuchElement);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void put(in string key, in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooIntMap :
                valuetype FooIntMap : DDS::IntMap {
                    test::FooSeq values() raises (DDS::AlreadyDeleted);
                    test::Foo get(in long key) raises (DDS::AlreadyDeleted, DDS::NoSuchElement);
                    void put(in long key, in test::Foo value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>valuetype </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>IntMap : DDS::IntMap {</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq values() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> get(in long key) raises (DDS::AlreadyDeleted, DDS::NoSuchElement);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void put(in long key, in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooList :
                valuetype FooList : DDS::List {
                    test::FooSeq values() raises (DDS::AlreadyDeleted);
                    test::Foo get(in long key) raises (DDS::AlreadyDeleted, DDS::NoSuchElement);
                    void add(in test::Foo value) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);
                    void put(in long key, in test::Foo value) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>valuetype </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>List : DDS::List {</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq values() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> get(in long key) raises (DDS::AlreadyDeleted, DDS::NoSuchElement);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void add(in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> value) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void put(in long key, in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> value) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>

            <!-- Generate FooHome :
                local interface FooHomeInterface : DDS::ObjectHome{
                    test::FooListenerSeq listeners() raises (DDS::AlreadyDeleted);
                    boolean attach_listener (in test::FooListener listener, in boolean concerns_contained_objects) raises (DDS::AlreadyDeleted);
                    boolean detach_listener(in test::FooListener listener) raises (DDS::AlreadyDeleted);
                    test::FooSelection create_selection(in DDS::SelectionCriterion criterion, in boolean auto_refresh, in boolean concerns_contained_objects) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
                    void delete_selection(in test::FooSelection a_selection) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
                    test::FooSelectionSeq selections() raises (DDS::AlreadyDeleted);
                    test::FooSeq get_modified_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);
                    test::FooSeq get_deleted_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);
                    test::FooSeq get_created_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);
                    test::FooSeq get_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);
                    test::Foo create_unregistered_object(in DDS::CacheAccess access) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);
                    test::Foo find_object(in DDS::DLRLOid oid, in DDS::CacheBase source) raises (DDS::NotFound, DDS::AlreadyDeleted);
                    void register_object(in test::Foo unregistered_object) raises (DDS::PreconditionNotMet, DDS::AlreadyExisting, DDS::AlreadyDeleted);
                    test::Foo create_object(in DDS::CacheAccess access) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);
                };
            -->
            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>local interface </xsl:text>
            <xsl:value-of select="$nonPrefixedName"/>
            <xsl:text>HomeInterface : DDS::ObjectHome{</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>ListenerSeq listeners() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    boolean attach_listener (in </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener listener, in boolean concerns_contained_objects) raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    boolean detach_listener(in </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Listener listener) raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection create_selection(in DDS::SelectionCriterion criterion, in boolean auto_refresh, in boolean concerns_contained_objects) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void delete_selection(in </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Selection a_selection) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>SelectionSeq selections() raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq get_modified_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq get_deleted_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq get_created_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullNameExceptLast"/>
            <xsl:text>Seq get_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> create_unregistered_object(in DDS::CacheAccess access) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> find_object(in DDS::DLRLOid oid, in DDS::CacheBase source) raises (DDS::NotFound, DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    void register_object(in </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> unregistered_object) raises (DDS::PreconditionNotMet, DDS::AlreadyExisting, DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$prefixedFullName"/>
            <xsl:text> create_object(in DDS::CacheAccess access) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);</xsl:text>
            <xsl:value-of select="$NL"/>

            <xsl:value-of select="$whiteSpace"/>
            <xsl:text>};</xsl:text>
            <xsl:value-of select="$NL"/>
            <xsl:value-of select="$NL"/>
        </xsl:if>
    </xsl:template>
<!--
//example output for a valuetype Foo inheriting from DDS::ObjectRoot defined in module test.
//Defined in file Foo.idl

#include "Foo.idl"

module test{

    local interface FooSelectionListener : DDS::SelectionListener{
        void on_object_modified (in test::Foo the_object);
        void on_object_in (in test::Foo the_object);
        void on_object_out (in test::Foo the_object);
    };

    local interface FooListener : DDS::ObjectListener{
        boolean on_object_modified (in test::Foo the_object);
        boolean on_object_created (in test::Foo the_object);
        boolean on_object_deleted (in test::Foo the_object);
    };

    local interface FooFilter : DDS::FilterCriterion{
        boolean check_object(in test::Foo an_object, in DDS::MembershipState membership_state);
    };

    local interface FooSelection : DDS::Selection{
        test::FooSeq members() raises (DDS::AlreadyDeleted);
        test::FooSeq get_inserted_members() raises (DDS::AlreadyDeleted);
        test::FooSeq get_modified_members() raises (DDS::AlreadyDeleted);
        test::FooSeq get_removed_members() raises (DDS::AlreadyDeleted);
        test::FooSelectionListener set_listener(in test::FooSelectionListener listener) raises(DDS::AlreadyDeleted);
        test::FooSelectionListener listener() raises (DDS::AlreadyDeleted);
    };

    valuetype FooSet : DDS::Set {
        void add(in test::Foo value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
        test::FooSeq added_elements() raises (DDS::AlreadyDeleted);
        boolean contains(in test::Foo value) raises (DDS::AlreadyDeleted);
        void remove(in test::Foo value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
        test::FooSeq values() raises (DDS::AlreadyDeleted);
        test::FooSeq removed_elements() raises (DDS::AlreadyDeleted);
    };

    valuetype FooStrMap : DDS::StrMap {
        test::FooSeq values() raises (DDS::AlreadyDeleted);
        test::Foo get(in string key) raises (DDS::AlreadyDeleted, DDS::NoSuchElement);
        void put(in string key, in test::Foo value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
    };

    valuetype FooIntMap : DDS::IntMap {
        test::FooSeq values() raises (DDS::AlreadyDeleted);
        test::Foo get(in long key) raises (DDS::AlreadyDeleted, DDS::NoSuchElement);
        void put(in long key, in test::Foo value) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
    };

    valuetype FooList : DDS:List {
        test::FooSeq values() raises (DDS::AlreadyDeleted);
        test::Foo get(in long key) raises (DDS::AlreadyDeleted, DDS::NoSuchElement);
        void add(in test::Foo value) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);
        void put(in long key, in test::Foo value) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);
    };

    local interface FooHomeInterface : DDS::ObjectHome{
        test::FooListenerSeq listeners() raises (DDS::AlreadyDeleted);
        boolean attach_listener (in test::FooListener listener, in boolean concerns_contained_objects) raises (DDS::AlreadyDeleted);
        boolean detach_listener(in test::FooListener listener) raises (DDS::AlreadyDeleted);
        test::FooSelection create_selection(in DDS::SelectionCriterion criterion, in boolean auto_refresh, in boolean concerns_contained_objects) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
        void delete_selection(in test::FooSelection a_selection) raises (DDS::AlreadyDeleted, DDS::PreconditionNotMet);
        test::FooSelectionSeq selections() raises (DDS::AlreadyDeleted);
        test::FooSeq get_modified_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);
        test::FooSeq get_deleted_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);
        test::FooSeq get_created_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);
        test::FooSeq get_objects(in DDS::CacheBase source) raises (DDS::AlreadyDeleted);
        test::Foo create_unregistered_object(in DDS::CacheAccess access) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);
        test::Foo find_object(in DDS::DLRLOid oid, in DDS::CacheBase source) raises (DDS::NotFound, DDS::AlreadyDeleted);
        void register_object(in test::Foo unregistered_object) raises (DDS::PreconditionNotMet, DDS::AlreadyExisting, DDS::AlreadyDeleted);
        test::Foo create_object(in DDS::CacheAccess access) raises (DDS::PreconditionNotMet, DDS::AlreadyDeleted);
    };
};
-->
</xsl:stylesheet>