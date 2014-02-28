/*

COPYRIGHT

Copyright 1992, 1993, 1994 Sun Microsystems, Inc.  Printed in the United
States of America.  All Rights Reserved.

This product is protected by copyright and distributed under the following
license restricting its use.

The Interface Definition Language Compiler Front End (CFE) is made
available for your use provided that you include this license and copyright
notice on all media and documentation and the software program in which
this product is incorporated in whole or part. You may copy and extend
functionality (but may not remove functionality) of the Interface
Definition Language CFE without charge, but you are not authorized to
license or distribute it to anyone else except as part of a product or
program developed by you or with the express written consent of Sun
Microsystems, Inc. ("Sun").

The names of Sun Microsystems, Inc. and any of its subsidiaries or
affiliates may not be used in advertising or publicity pertaining to
distribution of Interface Definition Language CFE as permitted herein.

This license is effective until terminated by Sun for failure to comply
with this license.  Upon termination, you shall destroy or return all code
and documentation for the Interface Definition Language CFE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED AS IS WITH NO WARRANTIES OF
ANY KIND INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS
FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, OR ARISING FROM A COURSE OF
DEALING, USAGE OR TRADE PRACTICE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED WITH NO SUPPORT AND WITHOUT
ANY OBLIGATION ON THE PART OF Sun OR ANY OF ITS SUBSIDIARIES OR AFFILIATES
TO ASSIST IN ITS USE, CORRECTION, MODIFICATION OR ENHANCEMENT.

SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES SHALL HAVE NO LIABILITY WITH
RESPECT TO THE INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY
INTERFACE DEFINITION LANGUAGE CFE OR ANY PART THEREOF.

IN NO EVENT WILL SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES BE LIABLE FOR
ANY LOST REVENUE OR PROFITS OR OTHER SPECIAL, INDIRECT AND CONSEQUENTIAL
DAMAGES, EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Use, duplication, or disclosure by the government is subject to
restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
Technical Data and Computer Software clause at DFARS 252.227-7013 and FAR
52.227-19.

Sun, Sun Microsystems and the Sun logo are trademarks or registered
trademarks of Sun Microsystems, Inc.

SunSoft, Inc.  
2550 Garcia Avenue 
Mountain View, California  94043

NOTE:

SunOS, SunSoft, Sun, Solaris, Sun Microsystems or the Sun logo are
trademarks or registered trademarks of Sun Microsystems, Inc.

 */


/*
 * idl.ll - Lexical scanner for IDL 1.1
 */
%{
#include "os_stdlib.h"
#include "os_heap.h"
#include "idl.h"
#include "idl_extern.h"
#include "utl_incl.h"

#include "fe_private.h"

#ifdef VERSION
#undef VERSION
#endif

#include "y_tab.h"
#include <string.h>

#include "preprocess.h"

#undef input
#define input() ((yytchar=yysptr>yysbuf?U(*--yysptr):preprocess_getc())==10?(yylineno++,yytchar):yytchar)

#define YY_INPUT(buf,result,max_size) \
    { \
    int c = preprocess_getc(); \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
    }

static char	idl_escape_reader(char *);
static double	idl_atof(char *);
static long	idl_atoi(char *, long);
static void	idl_parse_line_and_file(char *);

#ifdef __cplusplus
extern "C" int fstackdepth;
#else
extern int fstackdepth;
#endif

// HPUX has yytext typed to unsigned char *. We make sure here that
// we'll always use char *
#define CPPGEN_YYTEXT ((char *)yytext)

%}

%s NORMAL_STATE PRAGMA_STATE

%{
	/* This lets us get exclusive states */
	static int first_time = 1;
%}

/* This tells flex to read only one input file */
%option noyywrap
/* This removes the need to wrap unistd.h in a preprocessor if block, but the
   version of flex we use does not support it. Should we upgrade to a newer
   version, please uncomment this option to remove the need for the
   lex_yy.cpp.patch file. */
/* %option nounistd */

%%

%{
	if(first_time)
	{
		/* ensures that we start off in the NORMAL state */
		BEGIN NORMAL_STATE;
		first_time = 0;
	}
%}


module		return MODULE;
raises		return RAISES;
readonly	return READONLY;
attribute	return ATTRIBUTE;
exception	return EXCEPTION;
context		return CONTEXT;
local   	return LOCAL;
interface	return INTERFACE;
const		return CONST;
typedef		return TYPEDEF;
struct		return STRUCT;
enum		return ENUM;
string		return STRING;
wstring	return WSTRING;
sequence	return SEQUENCE;
union		return UNION;
switch		return SWITCH;
case		return CASE;
default		return DEFAULT;
float		return FLOAT;
double		return DOUBLE;
long		return LONG;
short		return SHORT;
unsigned	return UNSIGNED;
char		return CHAR;
wchar		return WCHAR;
boolean		return BOOLEAN;
octet		return OCTET;
void		return VOID;

custom          return CUSTOM;
valuetype       return VALUETYPE;
truncatable     return TRUNCATABLE;
supports        return SUPPORTS;
public          return PUBLIC;
private         return PRIVATE;
factory         return FACTORY;
abstract        return ABSTRACT;

TRUE		return TRUETOK;
FALSE		return FALSETOK;

inout		return INOUT;
in		return IN;
out		return OUT;
oneway		return ONEWAY;
opaque          return OPAQUE;

<PRAGMA_STATE>from return PFROM;

\<\<		return LEFT_SHIFT;
\>\>		return RIGHT_SHIFT;

\:\:		{
		  yylval.strval = os_strdup ("::");
		  return SCOPE_DELIMITOR;
		}


_?[a-zA-Z][a-zA-Z0-9_]*	{
    char *z = (char *) os_malloc(strlen(CPPGEN_YYTEXT) + 1);
    os_strcpy(z, CPPGEN_YYTEXT);
    yylval.strval = z;
    return IDENTIFIER;
}

<PRAGMA_STATE>-?[0-9]+"."[0-9]+ {
							char *z = (char *) os_malloc(strlen(CPPGEN_YYTEXT) + 1);
							strcpy(z, CPPGEN_YYTEXT);
							yylval.strval = z;
							BEGIN NORMAL_STATE;
							return VERSION;
					}

<NORMAL_STATE>-?[0-9]+"."[0-9]*([eE][+-]?[0-9]+)?[lLfF]? {
                  yylval.strval = CPPGEN_YYTEXT;
                  return FLOATING_PT_LITERAL;
                }
-?[0-9]+[eE][+-]?[0-9]+[lLfF]?  {
                  yylval.strval = CPPGEN_YYTEXT;
                  return FLOATING_PT_LITERAL;
                }

-?[1-9][0-9]*	{
		  yylval.ival = idl_atoi(CPPGEN_YYTEXT, 10);
		  return INTEGER_LITERAL;
	        }
-?0[xX][a-fA-F0-9]+ {
		  yylval.ival = idl_atoi(CPPGEN_YYTEXT, 16);
		  return INTEGER_LITERAL;
	        }
-?0[0-7]*	{
		  yylval.ival = idl_atoi(CPPGEN_YYTEXT, 8);
		  return INTEGER_LITERAL;
	      	}

"\""(\\\"|[^\"])*"\""	{
		  CPPGEN_YYTEXT[strlen(CPPGEN_YYTEXT)-1] = '\0';
		  yylval.sval = new UTL_String(CPPGEN_YYTEXT + 1);
		  return STRING_LITERAL;
	      	}
"'"."'"		{
		  yylval.cval = CPPGEN_YYTEXT[1];
		  return CHARACTER_LITERAL;
	      	}
"'"\\([0-7]{1,3})"'"	{
		  // octal character constant
		  yylval.cval = idl_escape_reader(CPPGEN_YYTEXT + 1);
		  return CHARACTER_LITERAL;
		}
"'"\\."'"	{
		  yylval.cval = idl_escape_reader(CPPGEN_YYTEXT + 1);
		  return CHARACTER_LITERAL;
		}
^[ \t]*#[ \t]*pragma[ \t]*cppgen_include {/* pragma include */
			BEGIN PRAGMA_STATE;
			return PRAGMA_INCLUDE;
		}
^[ \t]*#[ \t]*pragma[ \t]*cppgen_async_client	{/* pragma async */
			BEGIN PRAGMA_STATE;
			return PRAGMA_ASYNC_CLIENT;
		}
^[ \t]*#[ \t]*pragma[ \t]*cppgen_async_server	{/* pragma async */
			BEGIN PRAGMA_STATE;
			return PRAGMA_ASYNC_SERVER;
		}
^[ \t]*#[ \t]*pragma[ \t]*ID	{/* pragma ID */
			return PRAGMA_ID;
		}
^[ \t]*#[ \t]*pragma[ \t]*prefix	{/* pragma prefix */
			return PRAGMA_PREFIX;
		}
^[ \t]*#[ \t]*pragma[ \t]*version	{/* pragma version */
			BEGIN PRAGMA_STATE;
			return PRAGMA_VERSION;
		}
^[ \t]*#[ \t]*pragma	{/* remember pragma */
        /* error! unrecognized pragma */
        return PRAGMA;
		}
^#[ \t]*line[ \t]*[0-9]*[ \t].*\n {
            idl_parse_line_and_file(CPPGEN_YYTEXT);
      }
^#[ \t]*[0-9]*" ""\""[^\"]*"\""" "[0-9]*\n		{
		  idl_parse_line_and_file(CPPGEN_YYTEXT);
		}
^#[ \t]*[0-9]*" ""\""[^\"]*"\""" "[0-9]*" "[0-9]*\n      {
        idl_parse_line_and_file(CPPGEN_YYTEXT);
      }
^#[ \t]*[0-9]*" ""\""[^\"]*"\""\n			{
		  idl_parse_line_and_file(CPPGEN_YYTEXT);
		}
^#[ \t]*[0-9]*\n	{
		  idl_parse_line_and_file(CPPGEN_YYTEXT);
	        }
^#[ \t]*ident.*\n	{
		  /* ignore cpp ident */
  		  idl_global->set_lineno(idl_global->lineno() + 1);
		}
\/\/.*\n	{
		  /* ignore comments */
  		  idl_global->set_lineno(idl_global->lineno() + 1);
		}
"/*"		{
		  for(;;) {
		    char c = yyinput();
		    if (c == '*') {
		      char next = yyinput();
		      if (next == '/')
			break;
		      else
			unput(c);
	              if (c == '\n') 
		        idl_global->set_lineno(idl_global->lineno() + 1);
		    }
	          }
	        }
[ \t\r]*		;
<PRAGMA_STATE>\n	{
  		  idl_global->set_lineno(idl_global->lineno() + 1);
			BEGIN NORMAL_STATE;
			return PRAGMA_END;
					}
<NORMAL_STATE>\n	{
  		  idl_global->set_lineno(idl_global->lineno() + 1);
		}
.		return CPPGEN_YYTEXT[0];

%%
	/* subroutines */

/*
 * Strip down a name to the last component, i.e. everything after the last
 * '/' character
 */
static char *
stripped_name(UTL_String *fn)
{
    char	*n = fn->get_string();
    long	l;

    if (n == NULL)
	return NULL;
    l = strlen(n);
    for (n += l; l > 0 && *n != '/' && *n != '\\'; l--, n--);
    if (*n == '/' || *n == '\\') n++;
    return n;
}

/*
 * Parse a #line statement generated by the C preprocessor
 */
static void
idl_parse_line_and_file(char *buf)
{
  char		*r = buf;
  char 		*h;
  UTL_String	*nm;

  /* Skip initial '#' */
  if (*r != '#') {
    return;
  }

  /* Find line number */
  for (r++; *r == ' ' || *r == '\t'; r++);
  h = r;
  for (; *r != '\0' && *r != ' ' && *r != '\t'; r++);
  *r++ = 0;
  idl_global->set_lineno(idl_atoi(h, 10));
  
  /* Find file name, if present */
  for (; *r != '"'; r++) {
    if (*r == '\n' || *r == '\0')
      return;
  }
  h = ++r;
  for (; *r != '"'; r++);
  *r = 0;
  if (*h == '\0')
    idl_global->set_filename(new UTL_String("standard input"));
  else
  {
    // figure out whether to include this file...
    UTL_String *new_file = new UTL_String(h);

    idl_global->set_filename(new_file);
    // hide that fact that we are dealing with a temp file
    if(idl_global->filename()->compare(idl_global->real_filename()))
    {
      idl_global->set_filename(idl_global->main_filename());
      idl_global->set_in_main_file(I_TRUE);
    }
    else
    {
      idl_global->set_in_main_file(I_FALSE);
    }
  }

  /*
   * If it's an import file store the stripped name for the BE to use
   */
  if (!(idl_global->in_main_file()) && idl_global->imported() && fstackdepth < 2) {
    nm = new UTL_String(stripped_name(idl_global->filename()));
    idl_global->store_include_file_name(nm);
  }
}
    
/*
 * idl_atoi - Convert a string of digits into an integer according to base b
 */
static long
idl_atoi(char *s, long b)
{
	long	r = 0;
	long	negative = 0;

	if (*s == '-') {
	  negative = 1;
	  s++;
	}
	if (b == 8 && *s == '0')
	  s++;
	else if (b == 16 && *s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X'))
	  s += 2;

	for (; *s; s++)
	  if (*s <= '9' && *s >= '0')
	    r = (r * b) + (*s - '0');
	  else if (b > 10 && *s <= 'f' && *s >= 'a')
	    r = (r * b) + (*s - 'a' + 10);
	  else if (b > 10 && *s <= 'F' && *s >= 'A')
	    r = (r * b) + (*s - 'A' + 10);
	  else
	    break;

	if (negative)
	  r *= -1;

	return r;
}

/*
 * Convert a string to a float; atof doesn't seem to work, always.
 */
static double
idl_atof(char *s)
{
	double	d = 0.0;
	double	e, k;
	long	neg = 0, negexp = 0;

	if (*s == '-') {
	  neg = 1;
	  s++;
	}
	while (*s >= '0' && *s <= '9') {
		d = (d * 10) + *s - '0';
		s++;
	}
	if (*s == '.') {
		s++;
		e = 10;
		while (*s >= '0' && *s <= '9') {
			d += (*s - '0') / (e * 1.0);
			e *= 10;
			s++;
		}
	}
	if (*s == 'e' || *s == 'E') {
		s++;
		if (*s == '-') {
			negexp = 1;
			s++;
		} else if (*s == '+')
			s++;
		e = 0;
		while (*s >= '0' && *s <= '9') {
			e = (e * 10) + *s - '0';
			s++;
		}
		if (e > 0) {
			for (k = 1; e > 0; k *= 10, e--);
			if (negexp)
				d /= k;
			else
				d *= k;
		}
	}

	if (neg) d *= -1.0;

	return d;
}	

/*
 * Convert (some) escaped characters into their ascii values
 */
static char
idl_escape_reader(
    char *str
)
{
    if (str[0] != '\\') {
	return str[0];
    }

    switch (str[1]) {
      case 'n':
	return '\n';
      case 't':
	return '\t';
      case 'v':
	return '\v';
      case 'b':
	return '\b';
      case 'r':
	return '\r';
      case 'f':
	return '\f';
      case 'a':
	return '\a';
      case '\\':
	return '\\';
      case '\?':
	return '?';
      case '\'':
	return '\'';
      case '"':
	return '"';
      case 'x':
	{
	 int i;
	    // hex value
	    for (i = 2; str[i] != '\0' && isxdigit(str[i]); i++) {
		continue;
	    }
	    char save = str[i];
	    str[i] = '\0';
	    char out = (char)idl_atoi(&str[2], 16);
	    str[i] = save;
	    return out;
	}
	break;
      default:
	// check for octal value
	if (str[1] >= '0' && str[1] <= '7')
	{
	 int i;
	    for (i = 1; str[i] >= '0' && str[i] <= '7'; i++) {
		continue;
	    }
	    char save = str[i];
	    str[i] = '\0';
	    char out = (char)idl_atoi(&str[1], 8);
	    str[i] = save;
	    return out;
	} else {
	  return str[1] - 'a';
	}
	break;
    }
}
