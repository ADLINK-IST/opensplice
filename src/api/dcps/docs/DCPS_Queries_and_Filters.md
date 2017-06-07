Queries and Filters            {#DCPS_Queries_and_Filters}
===================

A subset of SQL syntax is used in several parts of OpenSplice:
- the filter_expression in the ContentFilteredTopic
- the topic_expression in the MultiTopic
- the query_expression in the QueryReadCondition.

Those expressions may use a subset of SQL, extended with the possibility to use
program variables in the SQL expression. The allowed SQL expressions are defined
with the BNF-grammar below. The following notational conventions are made:
- the NonTerminals are typeset in italics
- the ‘Terminals’ are quoted and typeset in a fixed width font
- the TOKENS are typeset in small caps
- the notation (element // ‘,’) represents a non-empty comma-separated list of elements.


### SQL Grammar in BNF

Expression::= FilterExpression<br>
&nbsp;&nbsp;&nbsp;&nbsp;| TopicExpression<br>
&nbsp;&nbsp;&nbsp;&nbsp;| QueryExpression

FilterExpression::= Condition

TopicExpression::= SelectFrom {Where } ‘;’

QueryExpression::= {Condition}{‘ORDER BY’ (FIELDNAME // ‘,’) }

SelectFrom::= ‘SELECT’ Aggregation ‘FROM’ Selection

Aggregation::= ‘*’<br>
&nbsp;&nbsp;&nbsp;&nbsp;| (SubjectFieldSpec // ‘,’)

SubjectFieldSpec::= FIELDNAME<br>
&nbsp;&nbsp;&nbsp;&nbsp;| FIELDNAME ‘AS’ FIELDNAME<br>
&nbsp;&nbsp;&nbsp;&nbsp;| FIELDNAME FIELDNAME

Selection::= TOPICNAME<br>
&nbsp;&nbsp;&nbsp;&nbsp;| TOPICTNAME NaturalJoin JoinItem

JoinItem::= TOPICNAME<br>
&nbsp;&nbsp;&nbsp;&nbsp;| TOPICNAME NaturalJoin JoinItem<br>
&nbsp;&nbsp;&nbsp;&nbsp;| ‘(’ TOPICNAME NaturalJoin JoinItem ‘)’

NaturalJoin::= ‘INNER NATURAL JOIN’<br>
&nbsp;&nbsp;&nbsp;&nbsp;| ‘NATURAL JOIN’<br>
&nbsp;&nbsp;&nbsp;&nbsp;| ‘NATURAL INNER JOIN’

Where::= ‘WHERE’ Condition

Condition::= Predicate<br>
&nbsp;&nbsp;&nbsp;&nbsp;| Condition ‘AND’ Condition<br>
&nbsp;&nbsp;&nbsp;&nbsp;| Condition ‘OR’ Condition<br>
&nbsp;&nbsp;&nbsp;&nbsp;| ‘NOT’ Condition<br>
&nbsp;&nbsp;&nbsp;&nbsp;| ‘(’ Condition ‘)’

Predicate::= ComparisonPredicate<br>
&nbsp;&nbsp;&nbsp;&nbsp;| BetweenPredicate

ComparisonPredicate::= FIELDNAME RelOp Parameter<br>
&nbsp;&nbsp;&nbsp;&nbsp;| Parameter RelOp FIELDNAME

BetweenPredicate::= FIELDNAME ‘BETWEEN’ Range<br>
&nbsp;&nbsp;&nbsp;&nbsp;| FIELDNAME ‘NOT BETWEEN’ Range

RelOp::= ‘=’ | ‘>’ | ‘>=’ | ‘<’ | ‘<=’ | ‘<>’ | like

Range::= Parameter ‘AND’ Parameter

Parameter::= INTEGERVALUE<br>
&nbsp;&nbsp;&nbsp;&nbsp;| FLOATVALUE<br>
&nbsp;&nbsp;&nbsp;&nbsp;| STRING<br>
&nbsp;&nbsp;&nbsp;&nbsp;| ENUMERATEDVALUE<br>
&nbsp;&nbsp;&nbsp;&nbsp;| PARAMETER

Note: INNER NATURAL JOIN, NATURAL JOIN, and NATURAL INNER JOIN are
all aliases, in the sense that they have the same semantics. The aliases are all
supported because they all are part of the SQL standard.



### SQL Token Expression

The syntax and meaning of the tokens used in the SQL grammar is described as
follows:

- FIELDNAME - A fieldname is a reference to a field in the data-structure. The dot
‘.’ is used to navigate through nested structures. The number of dots that may be
used in a fieldname is unlimited. The field-name can refer to fields at any depth in
the data structure. The names of the field are those specified in the IDL definition of
the corresponding structure, which may or may not match the field names that
appear on the C mapping of the structure

- TOPICNAME - A topic name is an identifier for a topic, and is defined as any series
of characters ‘a’, ..., ‘z’, ‘A’, ..., ‘Z’, ‘0’, ..., ‘9’, ‘_’ but may not
start with a digit

- INTEGERVALUE - Any series of digits, optionally preceded by a plus or minus sign,
representing a decimal integer value within the range of the system. A hexadecimal
number is preceded by 0x and must be a valid hexadecimal expression

- FLOATVALUE - Any series of digits, optionally preceded by a plus or minus sign and
optionally including a floating point (‘.’). A power-of-ten expression may be
post-fixed, which has the syntax en, where n is a number, optionally preceded by a
plus or minus sign

- STRING - Any series of characters encapsulated in single quotes, except a new-line
character or a right quote. A string starts with a left or right quote, but ends with a
right quote

- ENUMERATEDVALUE - An enumerated value is a reference to a value declared within
an enumeration. The name of the value must correspond to the names specified in
the IDL definition of the enumeration, and must be encapsulated in single quotes.
An enum value starts with a left or right quote, but ends with a right quote.

- PARAMETER - A parameter is of the form %n, where n represents a natural number
(zero included) smaller than 100. It refers to the n + 1th argument in the given
context.

Note: when RelOp is ‘like’, Unix filename wildcards must be used for strings
instead of the normal SQL wildcards. This means any one character is ‘?’, any zero
or more characters is ‘*’



### SQL Examples

Assuming Topic “Location” has as an associated type a structure with fields
“flight_name, x, y, z”, and Topic “FlightPlan” has as fields “flight_id, source,
destination”. The following are examples of using these expressions.

Example of a topic_expression:<br>
&nbsp;&nbsp;SELECT flight_name, x, y, z AS height FROM ‘Location’ NATURAL JOIN ‘FlightPlan’ WHERE height < 1000 AND x < 23

Example of a query_expression or a filter_expression:<br>
&nbsp;&nbsp;height < 1000 AND x <23

