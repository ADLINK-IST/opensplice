.. _`MATLAB Generation from IDL`:

##########################
MATLAB Generation from IDL
##########################

The DDS MATLAB Integration supports generation of MATLAB classes from IDL. This chapter describes the details of the IDL-MATLAB binding.

Running IDLPP
*************

Compiling IDL into MATLAB code is done using the ``-l matlab`` switch on idlpp::

    idlpp -l matlab idl-file-to-compile.idl

**Generated Artifacts**

The following table defines the MATLAB artifacts generated from IDL concepts:

===========  ==============  ===============================================
IDL Concept  MATLAB Concept  Comment
===========  ==============  ===============================================
module       package         a MATLAB package is a folder starting with '+'.
enum         class           a MATLAB .m file.
enum value   enum value
struct       class           a MATLAB .m file.
field        class property
typedef                      IDL typedef's are inlined.
union        Unsupported
inheritance  Unsupported
===========  ==============  ===============================================

**Datatype mappings**

The following table shows the MATLAB equivalents to IDL primitive types:

=========== ===========
IDL Type    MATLAB Type
=========== ===========
boolean     logical
char        int8
octet       uint8
short       int16
ushort      uint16
long        int32
ulong       uint32
long long   int64
ulong long  uint64
float       single
double      double
string      char
wchar       Unsupported
wstring     Unsupported
any         Unsupported
long double Unsupported
=========== ===========

**Implementing Arrays and Sequences in MATLAB**

Both IDL arrays and IDL sequences are mapped to MATLAB arrays.
MATLAB supports both native array types, which must have homogenenous contents and *cell arrays*, which may have heterogenous content.
In general, IDLPP prefers native arrays, as they support more straight forward type checking.
However, some situations require cell arrays.
The following table summarizes the cases where IDLPP will generate cell arrays:

==================== ======================== =====================================================
Datatype             Sample Syntax            Reason for using cell array
==================== ======================== =====================================================
sequence of sequence sequence<sequence<T>> f; Nested sequences need not have a homogeneous length
array of sequence    sequence<T> f[N];        Sequences lengths need not be homogeneous
sequence of array    sequence<A> f;           A multi-dim array makes adding elements too difficult
sequence of string   sequence<string> f;      Nested strings need not have a homogeneous length
string array         string f[N];             Nested strings need not have a homogeneous length
==================== ======================== =====================================================

Limitations of MATLAB Support
*****************************

The IDL-to-MATLAB binding has the following limitations:

* IDL unions are not supported
* the following IDL data types are not supported: wchar, wstring, any and long double
* arrays of sequences of structures are not supported

