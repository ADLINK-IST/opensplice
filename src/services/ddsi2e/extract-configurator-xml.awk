#!/usr/bin/gawk -f

# NOTES:
# - limited formatting freedom
# - default value may not contain a semicolon
# - absolutely no HTML-tag-like things in descriptions
#
# UGLINESSES:
# - knowledge of conversion functions in here
# - hard definitions of enums in here
# - negated_boolean is A BIT WEIRD and special-cased

BEGIN {
  if (version != "COMMUNITY" && version != "COMMERCIAL") {
    print "script error: variable 'version' must be set to COMMUNITY or COMMERCIAL" > "/dev/stderr";
    exit 1;
  }

  typehint2xmltype["____"] = "____";
  typehint2xmltype["networkAddress"] = "String";
  typehint2xmltype["partitionAddress"] = "String";
  typehint2xmltype["networkAddresses"] = "String";
  typehint2xmltype["ipv4"] = "String";
  typehint2xmltype["boolean"] = "Boolean";
  typehint2xmltype["negated_boolean"] = "Boolean";
  typehint2xmltype["boolean_default"] = "Enum";
  typehint2xmltype["string"] = "String";
  typehint2xmltype["tracingOutputFileName"] = "String";
  typehint2xmltype["verbosity"] = "Enum";
  typehint2xmltype["logcat"] = "String";
  typehint2xmltype["peer"] = "String";
  typehint2xmltype["float"] = "Float";
  typehint2xmltype["int"] = "Int";
  typehint2xmltype["int32"] = "Int";
  typehint2xmltype["uint"] = "Int";
  typehint2xmltype["uint32"] = "Int";
  typehint2xmltype["natint"] = "Int";
  typehint2xmltype["natint_255"] = "Int";
  typehint2xmltype["domainId"] = "Int";
  typehint2xmltype["participantIndex"] = "String";
  typehint2xmltype["port"] = "Int";
  typehint2xmltype["dyn_port"] = "Int";
  typehint2xmltype["duration_inf"] = "String";
  typehint2xmltype["duration_ms_1hr"] = "String";
  typehint2xmltype["duration_ms_1s"] = "String";
  typehint2xmltype["duration_us_1s"] = "String";
  typehint2xmltype["memsize"] = "String";
  typehint2xmltype["bandwidth"] = "String";
  typehint2xmltype["standards_conformance"] = "Enum";
  typehint2xmltype["locators"] = "Enum";
  typehint2xmltype["service_name"] = "String";
  typehint2xmltype["sched_class"] = "Enum";
  typehint2xmltype["cipher"] = "Enum";
  typehint2xmltype["besmode"] = "Enum";
  typehint2xmltype["retransmit_merging"] = "Enum";
  typehint2xmltype["sched_prio_class"] = "Enum";
  typehint2xmltype["sched_class"] = "Enum";
  typehint2xmltype["maybe_int32"] = "String";
  typehint2xmltype["maybe_memsize"] = "String";
  typehint2xmltype["maybe_duration_inf"] = "String";
  typehint2xmltype["allow_multicast"] = "String";

  typehint2unit["duration_inf"] = "duration_inf";
  typehint2unit["duration_ms_1hr"] = "duration";
  typehint2unit["duration_ms_1s"] = "duration";
  typehint2unit["duration_us_1s"] = "duration";
  typehint2unit["bandwidth"] = "bandwidth";
  typehint2unit["memsize"] = "memsize";
  typehint2unit["maybe_memsize"] = "memsize";
  typehint2unit["maybe_duration_inf"] = "duration_inf";

  enum_values["locators"] = "local;none";
  enum_values["standards_conformance"] = "lax;strict;pedantic";
  enum_values["verbosity"] = "finest;finer;fine;config;info;warning;severe;none";
  enum_values["besmode"] = "full;writers;minimal";
  enum_values["retransmit_merging"] = "never;adaptive;always";
  enum_values["sched_prio_class"] = "relative;absolute";
  enum_values["sched_class"] = "realtime;timeshare;default";
  enum_values["cipher"] = "null;blowfish;aes128;aes192;aes256";
  enum_values["boolean_default"] = "false;true;default";

  range["port"] = "1;65535";
  range["dyn_port"] = "-1;65535";
  range["domainId"] = "0;229";
  range["general_cfgelems/startupmodeduration"] = "0;60000";
  range["natint_255"] = "0;255";
  range["duration_ms_1hr"] = "0;1hr";
  range["duration_ms_1s"] = "0;1s";
  range["duration_us_1s"] = "0;1s";

  unit_blurb["bandwidth"] = "\n<p>The unit must be specified explicitly. Recognised units: <i>X</i>b/s, <i>X</i>bps for bits/s or <i>X</i>B/s, <i>X</i>Bps for bytes/s; where <i>X</i> is an optional prefix: k for 10<sup>3</sup>, Ki for 2<sup>10</sup>, M for 10<sup>6</sup>, Mi for 2<sup>20</sup>, G for 10<sup>9</sup>, Gi for 2<sup>30</sup>.</p>";
  unit_blurb["memsize"] = "\n<p>The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2<sup>10</sup> bytes), MB & MiB (2<sup>20</sup> bytes), GB & GiB (2<sup>30</sup> bytes).</p>";
  unit_blurb["duration"] = "\n<p>The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.</p>";
  unit_blurb["duration_inf"] = "\n<p>Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.</p>";

  # Some default overrides are set in the END section, as they are
  # dependent on some configuration items
  #default_overrides["ddsi2_cfgattrs/name"]

  for(x in typehint2xmltype) {
    if(typehint2xmltype[x] == "Enum") {
      if(enum_values[x] == "") {
        print "script error: values of enum type "x" unknown" > "/dev/stderr";
        exit 1;
      }
    }
  }
}

function clean_description(desc) {
  desc = gensub (/^ *"/, "", 1, desc);
  desc = gensub (/ *" *(\} *, *$)?$/, "", 1, desc);
  desc = gensub (/\\"/, "\"", "g", desc);
  desc = gensub (/\\n\\/, "\n", "g", desc);
  desc = gensub (/\\\\/, "\\", "g", desc);
  return desc;
}

function store_entry() {
  name = gensub (/\|.*/, "", 1, name); # aliases are not visible in osplconf
  ltable = tolower(table);
  lname = tolower(name);
  if(tab2elems[ltable] == "") {
    tab2elems[ltable] = name;
  } else {
    tab2elems[ltable] = tab2elems[table]";"name
  }
  elem[ltable"/"lname] = kind";"subtable";"multiplicity";"defaultvalue";"typehint;
  desc[ltable"/"lname] = clean_description(description)""unit_blurb[typehint2unit[typehint]];
  if(typehint2xmltype[typehint] == "") {
    print FILENAME":near "NR": error: no mapping defined for type "typehint > "/dev/stderr";
    exit 1;
  }
  typehint_seen[typehint] = 1;
  #print ltable"/"lname" - "elem[ltable"/"lname];
  #typehint = "";
}

function print_description(desc,indent, ls,i) { # ls,i: locals
  print indent"  <comment><![CDATA[";
  print desc;
  print indent"    ]]></comment>";
}

function kind_to_kstr(kind,typehint,table,name) {
  if(kind == "GROUP" || kind == "MGROUP") {
    return "element";
  } else if(kind == "ATTR") {
    return "attribute"typehint2xmltype[typehint];
  } else if(kind == "LEAF") {
    return "leaf"typehint2xmltype[typehint];
  } else {
    print FILENAME": error: fs_to_kstr: "kind" unrecognized kind ("table"/"name")" > "/dev/stderr";
    exit 1;
  }
}

function transform_default(fs, tmp,map) { # tmp, map: local
  tmp = gensub(/^"(.*)"$/, "\\1", 1, fs[4]);
  if(fs[5] != "negated_boolean") {
    return tmp;
  } else {
    map["true"] = "false";
    map["false"] = "true";
    return map[tolower(tmp)];
  }
}

function conv_to_xml(table,name,indent,prefix, fs,vs,vsn,kstr,ts,tsi,tsn,i,min_occ,max_occ,rr,req) { # fs,vs,kstr,... are locals
  split(elem[tolower(table"/"name)], fs, ";");
  #print table"/"name" - "; for(x in fs) { print "  - "x" "fs[x]; } 
  kstr = kind_to_kstr(fs[1], fs[5], table, name);
  if(fs[3] == 0) {
    min_occ = max_occ = 0;
  } else if(fs[3] == 1) { # multiplicity = 1 => required if no default
    if(fs[1] == "GROUP" || fs[1] == "MGROUP") {
      min_occ = 0; # unless it's a group
    } else if(fs[4] == "" || fs[4] == "NULL") {
      min_occ = 1;
    } else {
      min_occ = 0;
    }
    max_occ = 1;
  } else {
    min_occ = 0; max_occ = fs[3];
  }
  if(fs[1] == "ATTR") {
    req = (min_occ == 0) ? "false" : "true";
    print indent"<"kstr" name=\""name"\" required=\""req"\" version=\""version"\">";
  } else {
    print indent"<"kstr" name=\""name"\" minOccurrences=\""min_occ"\" maxOccurrences=\""max_occ"\" version=\""version"\">";
  }
  print_description(prefix""desc[tolower(table"/"name)], indent);
  # enum, int ranges
  if(enum_values[fs[5]]) {
    vsn=split(enum_values[fs[5]], vs, ";");
    for(i = 1; i <= vsn; i++) {
      print indent"  <value>"vs[i]"</value>";
    }
  }
  rr = range[tolower(table"/"name)];
  if(rr == "" && range[fs[5]] != "") { rr = range[fs[5]]; }
  if(rr != "") {
    split(rr, vs, ";");
    print indent"  <minimum>"vs[1]"</minimum>";
    print indent"  <maximum>"vs[2]"</maximum>";
  }
  # remarkably, osplconf can't deal with strings for which no maximum
  # length is specified, even though it accepts unlimited length
  # strings ...
  if(typehint2xmltype[fs[5]] == "String") {
    print indent"  <maxLength>0</maxLength>";
  }
  # default not applicable to GROUPs
  if(fs[1] != "GROUP" && fs[1] != "MGROUP") {
    defover = default_overrides[tolower(table"/"name)];
    if(defover != "") {
      print indent"  <default>"defover"</default>";
    } else if(fs[4] == "" || fs[4] == "NULL") {
      print indent"  <default></default>";
    } else {
      print indent"  <default>"transform_default(fs)"</default>";
    }
  }
  # recurse into subtables if any
  if(fs[2] != "") {
    split(fs[2], ts, ",");
    tsn = asort(ts,tsi);
    for(i = 1; i <= tsn; i++) {
      conv_table_to_xml(tsi[i], indent"  ", prefix);
    }
  }
  print indent"</"kstr">";
}

function conv_table_to_xml(table,indent,prefix, ns,nsi,nsn,i) { # ns,i are locals
  split(tab2elems[table], ns, ";");
  nsn = asort(ns,nsi);
  for(i = 1; i <= nsn; i++) {
    conv_to_xml(table, nsi[i], indent, (table == "unsupp_cfgelems") ? "<b>Internal</b>" : prefix);
  }
}

gobbling_description {
  description = description""$0;
  #print "  .. "$0;
}

gobbling_description && /(^|") *\} *, *$/ {
  gobbling_description = 0;
  store_entry();
  next;
}

gobbling_description {
  next;
}

/^[ \t]*#[ \t]*(if[ \t]+LITE($|[^A-Za-z_0-9]))/ {
  # skip LITE, but note that we ONLY handle #if LITE ... (#else ...)
  # #endif, no support for comments or anything similarly fancy.
  skip_lite=1;
}
skip_lite && /^[ \t]*#[ \t]*(else|endif)[ \t]*$/ {
  skip_lite=0;
}

skip_lite || /^[ \t]*(#[ \t]*(if|ifdef|ifndef|else|endif).*)?$/ { # skip empty lines, preproc
  next;
}

/^ *END_MARKER *$/ {
  if(!in_table) {
    print FILENAME":near "NR": END_MARKER seen while not in a table" > "/dev/stderr";
  }
  in_table = 0;
  #print "END_MARKER "table;
  next;
}

/^static +const +struct +cfgelem +/ {
  in_table = 1;
  table = gensub(/^static +const +struct +cfgelem +([A-Za-z_0-9]+).*/, "\\1", 1, $0);
  #print "TABLE "table;
  next;
}

in_table && /^ *WILDCARD *, *$|^ *\{ *(MOVED) *\(/ {
  next;
}

# Recognise all "normal" entries: attributes, groups, leaves and
# leaves with attributes. This doesn't recognise the ones used for the
# root groups: those are dealt with by the next pattern
in_table && /^ *\{ *(ATTR|GROUP|MGROUP|LEAF|LEAF_W_ATTRS) *\(/ {
  rest = $0;
  # extract kind
  re = "^ *\\{ *(ATTR|GROUP|MGROUP|LEAF|LEAF_W_ATTRS) *\\( *";
  kind = gensub(re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  # extract name + reference to subtable
  re = "\"([A-Za-z_0-9|]+)\" *";
  name = gensub(re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  subelems = subattrs = "";
  if(kind == "GROUP" || kind == "MGROUP") {
    re = ", *([A-Za-z_0-9]+) *";
    subelems = gensub(re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  }
  if(kind == "LEAF_W_ATTRS" || kind == "MGROUP") {
    re = ", *([A-Za-z_0-9]+) *";
    subattrs = gensub(re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  }

  subtable = "";
  if(subelems != "") { subtable = subelems; }
  if(subattrs != "") {
    if(subtable != "") { subtable = subtable","subattrs; }
    else { subtable = subattrs; }
  }
  rest = gensub(/ *) *, */, "", 1, rest);
  #print "  kind "kind" name "name" subtable "subtable;

  # don't care about the distinction between LEAF and LEAF_W_ATTRS
  # in the remainer of the code: we simply rely on subtable.
  if(kind == "LEAF_W_ATTRS") {
    kind = "LEAF";
  }
}

# Root groups: use a special trick, which allows them to do groups
# with attributes. Which the DDSI2 proper doesn't use, but which the
# service configuration stuff does rely on.
in_table && /^ *\{ *"([A-Za-z_0-9|]+)" *, */ {
  rest = $0;
  # root elements are all groups, formatted as: <name>, <subelems>,
  # <attrs>, NODATA, description. They're therefore pretty easy to
  # parse.
  kind = "GROUP";
  re = "^ *\\{ *\"([A-Za-z_0-9|]+)\" *, *";
  name = gensub (re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  # then follow the sub-elements and the attributes
  re = "([A-Za-z_0-9]+) *, *";
  subelems = gensub(re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  subattrs = gensub(re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  # then we require NODATA (could do this in the pattern also)
  if(!match(rest, /^NODATA *,/)) {
    print FILENAME":near "NR": error: NODATA expected" > "/dev/stderr";
    exit 1;
  }
  # multiplicity is hard coded: we want to allow multiple ddsi2 services
  multiplicity = 0;
  subtable = "";
  if(subelems != "NULL") { subtable = subelems; }
  if(subattrs != "NULL") {
    if(subtable != "") { subtable = subtable","subattrs; }
    else { subtable = subattrs; }
  }
  rest = gensub(re, "", 1, rest);
}

# Extract stuff specific to ATTRs, LEAFs and MGROUPs
in_table && (kind == "ATTR" || kind == "LEAF" || kind == "MGROUP") {
  # extract multiplicity
  re = "([0-9]+) *, *";
  multiplicity = gensub(re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  # extract default value
  re = "(\"([^\"]*)\"|NULL|0) *, *";
  defaultvalue = gensub(re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  if(defaultvalue == "0") { defaultvalue = "NULL"; }
  # skip reference to internal name (either ABSOFF(field),
  # RELOFF(field,field) or <int>,<int> (the latter being used by
  # "verbosity")
  rest = gensub(/(ABSOFF *\( *[A-Za-z_0-9.]+ *\)|RELOFF *\( *[A-Za-z_0-9.]+ *, *[A-Za-z_0-9]+ *\)|[0-9]+ *, *[0-9]+) *, */, "", 1, rest);
  # skip init function
  rest = gensub(/([A-Za-z_0-9]+|0) *, */, "", 1, rest);
  # type hint from conversion function
  re = "(uf_([A-Za-z_0-9]+)|NULL|0) *, *";
  typehint = gensub(re".*", "\\1", 1, rest); rest = gensub(re, "", 1, rest);
  typehint = gensub(/^uf_/, "", 1, typehint);
  # accept typehint = NULL for a LEAF_WITH_ATTRS: there is no defined
  # "syntax" for groups that have only attributes, pretending it is a
  # group because that causes us to emit an "element" and not a
  # "leaf".
  if(typehint == "0" || typehint == "NULL") {
    kind = "GROUP";
    typehint = "____";
  }
  # skip free, print functions
  rest = gensub(/([A-Za-z_0-9]+|0) *, *([A-Za-z_0-9]+|0) *, */, "", 1, rest);
  #print "  .. multiplicity "multiplicity" default "defaultvalue" typehint "typehint;
}

# Extract description (or NULL, if not to be included in the configurator XML)
in_table {
  #print "  .. "rest;
  # description or NULL
  if(match(rest, /NULL *\} *, *$/)) {
    # no description - discard this one/simply continue with next one
  } else if(match(rest, /".*" *\} *, *$/)) {
    # description ending on same line
    description = gensub(/(".*").*/, "\\1", 1, rest);
    store_entry();
  } else {
    # strip the quotes &c. once the full text has been gathered
    description = rest;
    gobbling_description = 1;
  }
  #print "  .. gobbling "gobbling_description;
  next;
}

END {
  #print FILENAME": "tab2elems["root_cfgelems"]
  nrootnames = split(tab2elems["root_cfgelems"], rootnames, ";");
  if(nrootnames != 1) {
    print FILENAME": error: root_cfgelems has no multiple entries, service name not known" > "/dev/stderr";
    exit 1;
  }
  servicename = rootnames[1];

  # Default service name attribute is executable name, which we can
  # derive from the root element, DDSI2Service and DDSI2EService.  If
  # the root element is neither of those, error out (though we could
  # also simply use some other sane default).
  if(servicename != "DDSI2Service" && servicename != "DDSI2EService") {
    print FILENAME": error: top element expected to be DDSI2Service or DDSI2EService " > "/dev/stderr";
    exit 1;
  } else {
    execname = tolower(gensub(/Service/, "", "g", servicename));
  }
  default_overrides["ddsi2_cfgattrs/name"] = execname;

  print "<!-- "servicename" CONFIGURATION AUTOMAGICALLY GENERATED (see src/services/ddsi2e/extract-configuration-xml.awk) -->";
  #print "<!-- "strftime()" -->";
  conv_table_to_xml("root_cfgelems", "  ", "");
  print "<!-- END "servicename" CONFIGURATION AUTOMAGICALLY GENERATED (see src/services/ddsi2e/extract-configuration-xml.awk) -->";

  for(x in typehint_seen) {
    if(x == 0) {
      print "script warning: type mapping defined for "x" but not used" > "/dev/stderr";
    }
  }
}
