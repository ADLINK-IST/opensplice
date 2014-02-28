/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "cpp_malloc.h"
#include "is.h"
#include "io.h"
#include "symtbl.h"
#include "expr.h"

#include <ctype.h>

#define DEBUG_EXPR/**/

#define TWOCHAR(c1,c2) (((c1)<<8)|(c2))
#define LSH TWOCHAR('<','<')
#define RSH TWOCHAR('>','>')
#define LEQ TWOCHAR('<','=')
#define GEQ TWOCHAR('>','=')
#define EQL TWOCHAR('=','=')
#define NEQ TWOCHAR('!','=')
#define AND TWOCHAR('&','&')
#define OR  TWOCHAR('|','|')

#define ALLBINS         \
        BIN('*',*)      \
        BIN('/',/)      \
        BIN('%',%)      \
        BIN('+',+)      \
        BIN(LSH,<<)     \
        BIN(RSH,>>)     \
        BIN('<',<)      \
        BIN('>',>)      \
        BIN(LEQ,<=)     \
        BIN(GEQ,>=)     \
        BIN(EQL,==)     \
        BIN(NEQ,!=)     \
        BIN('&',&)      \
        BIN('^',^)      \
        BIN('|',|)      \
        BIN(AND,&&)     \
        BIN(OR ,||)

static EORB_CPP_node *expr;
int expr_sharp;
#define sharp expr_sharp
static int complain;

#ifdef DEBUG_EXPR
extern int debugging;
static void dump_expr (EORB_CPP_node *);
#endif

static void free_expr (EORB_CPP_node * n)
{
   switch (n->op)
   {
      case 0:
         break;
      case '-':
         if (n->left)
         {
            free_expr(n->left);
         }
         free_expr(n->right);
         break;
      case '!':
      case '~':
         free_expr(n->right);
         break;
      case 'd':
         os_free(n->name);
         break;
      default:
         free_expr(n->left);
         free_expr(n->right);
         break;
   }
   OLD(n);
}

static int exec_free (EORB_CPP_node * n)
{
   int rv = 0;
   int l;
   int r;

   switch (n->op)
   {
      case 0:
         rv = n->leaf;
         break;
      case '-':
         if (n->left)
         {
            rv = exec_free(n->left);
         }
         else
         {
            rv = 0;
         }
         rv -= exec_free(n->right);
         break;
      case '!':
         rv = ! exec_free(n->right);
         break;
      case '~':
         rv = ~ exec_free(n->right);
         break;
      case 'd':
         rv = !! find_def(n->name);
         os_free(n->name);
         break;
#define BIN(key,op) case key:l=exec_free(n->left);r=exec_free(n->right);rv=l op r;break;

         ALLBINS
#undef BIN

   }
   OLD(n);
   return (rv);
}

static int exec_nofree (EORB_CPP_node * n)
{
   int rv;

   switch (n->op)
   {
      case 0:
         rv = n->leaf;
         break;
      case '-':
         if (n->left)
         {
            rv = exec_nofree(n->left);
         }
         else
         {
            rv = 0;
         }
         rv -= exec_nofree(n->right);
         break;
      case '!':
         rv = ! exec_nofree(n->right);
         break;
      case '~':
         rv = ~ exec_nofree(n->right);
         break;
      case 'd':
         rv = !! find_def(n->name);
         os_free(n->name);
         break;
#define BIN(key,op) case key:rv=(exec_nofree(n->left)op exec_nofree(n->right));break;

         ALLBINS
#undef BIN

   }
   return (rv);
}

static EORB_CPP_node * newnode (EORB_CPP_node * l, int c, EORB_CPP_node * r)
{
   EORB_CPP_node *n;

   n = NEW(EORB_CPP_node);
   check_os_malloc(n);
   n->left = l;
   n->right = r;
   n->op = c;
   return (n);
}

static EORB_CPP_node * newleaf (int v)
{
   EORB_CPP_node *n;

   n = NEW(EORB_CPP_node);
   check_os_malloc(n);
   n->op = 0;
   n->left = 0;
   n->right = 0;
   n->leaf = v;
   return (n);
}

static EORB_CPP_node *newname (char * name)
{
   EORB_CPP_node *n;

   n = NEW(EORB_CPP_node);
   check_os_malloc(n);
   n->op = 'd';
   n->name = copyofstr(name);
   check_os_malloc(n->name);
   return (n);
}

static int get_quote_char (void)
{
   char c;
   char d;

   c = Get();
   if (c == '\'')
   {
      return ( -1);
   }
   if (c == '\n')
   {
      err_head();
      fprintf(stderr, "newline in character constant\n");
      return ( -1);
   }
   if (c != '\\')
   {
      return (0xff&(int)c);
   }
   c = Get();
   if ((c >= '0') && (c <= '7'))
   {
      d = c - '0';
      c = Get();
      if ((c >= '0') && (c <= '7'))
      {
         d = (d << 3) + c - '0';
         c = Get();
         if ((c >= '0') && (c <= '7'))
         {
            d = (d << 3) + c - '0';
         }
         else
         {
            Push(c);
         }
      }
      else
      {
         Push(c);
      }
      return (0xff&(int)d);
   }
   switch (c)
   {
      case 'n':
         return ('\n');
         break;
      case 'r':
         return ('\r');
         break;
      case 'e':
         return ('\033');
         break;
      case 'f':
         return ('\f');
         break;
      case 't':
         return ('\t');
         break;
      case 'b':
         return ('\b');
         break;
      case 'v':
         return ('\v');
         break;
      default:
         return (0xff&(int)c);
         break;
   }
}

static EORB_CPP_node *read_expr_11 (void)
{
   char c;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E11:");
   }
#endif
   while (1)
   {
      c = getnhsexpand();
      if (c == '(')
      {
         EORB_CPP_node *n;
#ifdef DEBUG_EXPR

         if (debugging)
         {
            outputs("()");
         }
#endif
         n = read_expr_();
         c = getnhsexpand();
         if (c != ')')
         {
            err_head();
            fprintf(stderr, "expression syntax error -- missing ) supplied\n");
            Push(c);
         }
#ifdef DEBUG_EXPR
         if (debugging)
         {
            outputs("~");
         }
#endif
         return (n);
      }
      else if (isdigit((int) c))
      {
         int base;
         static char digits[] = "0123456789abcdefABCDEF";
         static char values[] =
            "\0\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\12\13\14\15\16\17";
         char *d;
         int v;
#ifdef DEBUG_EXPR

         if (debugging)
         {
            outputs("N");
         }
#endif
         base = 10;
         if (c == '0')
         {
            base = 8;
            c = Get();
            if ((c == 'x') || (c == 'X'))
            {
               base = 16;
               c = Get();
            }
         }
         v = 0;
         while (1)
         {
            d = strchr(digits, c);
            if (d == 0)
            {
               Push(c);
#ifdef DEBUG_EXPR

               if (debugging)
               {
                  outputd(v);
                  outputs("~");
               }
#endif
               return (newleaf(v));
            }
            else if (values[d -digits] >= base)
            {
               err_head();
               fprintf(stderr, "warning: illegal %sdigit `%c'\n",
                       (base == 16) ? "hex " : (base == 8) ? "octal " : "", c);
            }
            v = (v * base) + values[d - digits];
            c = Get();
         }
      }
      else if (c == '\'')
      {
         int i;
         int j;
         int n;
         i = 0;
         n = 0;
         while (1)
         {
            j = get_quote_char();
            if (j < 0)
            {
               break;
            }
            i = (i << 8) | j;
            n ++;
         }
         if (n > 4)
         {
            err_head();
            fprintf(stderr, "warning: too many characters in character constant\n");
         }
         return (newleaf(i));
      }
      else if ((c == '\n') && !sharp)
      {}
      else
      {
         char *id;
         if (complain)
         {
            err_head();
            fprintf(stderr, "expression syntax error -- number expected\n");
         }
         if (isbsymchar(c))
         {
            Push(c);
            id = read_ident();
         }
         else
         {
            id = 0;
         }
#ifdef DEBUG_EXPR
         if (debugging)
         {
            outputs("0(");
            outputc(c);
            outputs(":");
            outputs(id ? id : "(none)");
            outputs(")~");
         }
#endif
         if (id)
         {
            os_free(id);
         }
         return (newleaf(0));
      }
   }
}

static EORB_CPP_node *read_expr_10 (void)
{
   char c;
   char *w;
   EORB_CPP_node *n;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E10:");
   }
#endif
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '-':
         case '~':
         case '!':
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputc(c);
            }
#endif
            n = read_expr_10();
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            return (newnode(0, c, n));
            break;
         case 'd':
            Push(c);
            input_mark();
            w = read_ident();
            if (strcmp(w, "defined") == 0)
            {
               c = getnonspace();
               if (c == '(')
               {
                  char *id;
                  id = read_ident();
                  if (id)
                  {
                     c = getnonspace();
                     if (c == ')')
                     {
                        input_unmark();
#ifdef DEBUG_EXPR

                        if (debugging)
                        {
                           outputs("ifdef");
                        }
#endif
                        return (newname(id));
                     }
                  }
               }
               else if (isbsymchar(c))
               {
                  char *id;
                  Push(c);
                  id = read_ident();
                  if (id)
                  {
                     input_unmark();
#ifdef DEBUG_EXPR

                     if (debugging)
                     {
                        outputs("ifdef");
                     }
#endif
                     return (newname(id));
                  }
               }
            }
            input_recover();
            n = read_expr_11();
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            return (n);
            break;
         default:
            Push(c);
            n = read_expr_11();
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            return (n);
            break;
      }
   }
}

static EORB_CPP_node * read_expr_9 (void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E9:");
   }
#endif
   l = read_expr_10();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '*':
         case '%':
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputc(c);
            }
#endif
            r = read_expr_10();
            l = newnode(l, c, r);
            break;
         case '/':
            r = read_expr_10();
            if (!r->op) {
               Push(c);
               return (l);
            }
            break;
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

static EORB_CPP_node *read_expr_8 (void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E8:");
   }
#endif
   l = read_expr_9();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '+':
         case '-':
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputc(c);
            }
#endif
            r = read_expr_9();
            l = newnode(l, c, r);
            break;
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

static EORB_CPP_node *read_expr_7(void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;
   char d;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E7:");
   }
#endif
   l = read_expr_8();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '<':
         case '>':
            d = Get();
            if (d == c)
            {
#ifdef DEBUG_EXPR
               if (debugging)
               {
                  outputc(c);
                  outputc(d);
               }
#endif
               r = read_expr_8();
               l = newnode(l, (c == '<') ? LSH : RSH, r);
               break;
            }
            Push(d);
            /* fall through ... */
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

static EORB_CPP_node *read_expr_6(void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;
   char d;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E6:");
   }
#endif
   l = read_expr_7();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '<':
         case '>':
            d = Get();
            if (d == '=')
            {
#ifdef DEBUG_EXPR
               if (debugging)
               {
                  outputc(c);
                  outputc(d);
               }
#endif
               r = read_expr_7();
               l = newnode(l, (c == '<') ? LEQ : GEQ, r);
            }
            else
            {
#ifdef DEBUG_EXPR
               if (debugging)
               {
                  outputc(c);
               }
#endif
               Push(d);
               r = read_expr_7();
               l = newnode(l, c, r);
            }
            break;
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

static EORB_CPP_node *read_expr_5(void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;
   char d;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E5:");
   }
#endif
   l = read_expr_6();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '=':
         case '!':
            d = Get();
            if (d == '=')
            {
#ifdef DEBUG_EXPR
               if (debugging)
               {
                  outputc(c);
                  outputc(d);
               }
#endif
               r = read_expr_6();
               l = newnode(l, (c == '=') ? EQL : NEQ, r);
               break;
            }
            Push(d);
            /* fall through ... */
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

static EORB_CPP_node *read_expr_4(void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;
   char d;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E4:");
   }
#endif
   l = read_expr_5();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '&':
            d = Get();
            if (d != '&')
            {
#ifdef DEBUG_EXPR
               if (debugging)
               {
                  outputc(c);
                  outputc(d);
               }
#endif
               r = read_expr_5();
               l = newnode(l, c, r);
               break;
            }
            Push(d);
            /* fall through ... */
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

static EORB_CPP_node *read_expr_3(void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E3:");
   }
#endif
   l = read_expr_4();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '^':
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputc(c);
            }
#endif
            r = read_expr_4();
            l = newnode(l, c, r);
            break;
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

static EORB_CPP_node *read_expr_2(void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;
   char d;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E2:");
   }
#endif
   l = read_expr_3();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '|':
            d = Get();
            if (d != '|')
            {
               Push(d);
#ifdef DEBUG_EXPR

               if (debugging)
               {
                  outputc(c);
               }
#endif
               r = read_expr_3();
               l = newnode(l, c, r);
               break;
            }
            Push(d);
            /* fall through ... */
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

static EORB_CPP_node *read_expr_1(void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;
   char d;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E1:");
   }
#endif
   l = read_expr_2();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '&':
            d = Get();
            if (d == c)
            {
#ifdef DEBUG_EXPR
               if (debugging)
               {
                  outputc(c);
                  outputc(d);
               }
#endif
               r = read_expr_2();
               l = newnode(l, AND, r);
               break;
            }
            Push(d);
            /* fall through ... */
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

static EORB_CPP_node *read_expr_0(void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;
   char d;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E0:");
   }
#endif
   l = read_expr_1();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '|':
            d = Get();
            if (d == c)
            {
#ifdef DEBUG_EXPR
               if (debugging)
               {
                  outputc(c);
                  outputc(d);
               }
#endif
               r = read_expr_1();
               l = newnode(l, OR, r);
               break;
            }
            Push(d);
            /* fall through ... */
         default:
#ifdef DEBUG_EXPR

            if (debugging)
            {
               outputs("~");
            }
#endif
            Push(c);
            return (l);
            break;
      }
   }
}

EORB_CPP_node *read_expr_ (void)
{
   EORB_CPP_node *l;
   EORB_CPP_node *r;
   char c;

#ifdef DEBUG_EXPR

   if (debugging)
   {
      outputs("~E");
   }
#endif
   l = read_expr_0();
   while (1)
   {
      c = getnhsexpand();
      switch (c)
      {
         case '\n':
         case ')':
            Push(c);
            return (l);
            break;
         case ',':
            r = read_expr_0();
            l = newnode(l, c, r);
            break;
         default:
            err_head();
            fprintf(stderr, "expression syntax error -- bad operator `%c'\n", c);
            return (l);
            break;
      }
   }
}

EORB_CPP_node *read_expr_p (void)
{
   char c;
   EORB_CPP_node *rv;

   sharp = 0;
   complain = 1;
   c = getnonspace();
   if (c != '(')
   {
      Push(c);
      return (0);
   }
   rv = read_expr_();
   c = getnonspace();
   if (c != ')')
   {
      err_head();
      fprintf(stderr, "junk after expression\n");
   }
   return (rv);
}

int eval_expr (int Sharp, int Complain)
{
   char c;
   char d;
   int rv;

   sharp = Sharp;
   complain = Complain;
   expr = read_expr_();
   if (sharp)
   {
      c = getnonhspace();
      d = '\n';
   }
   else
   {
      c = getnonspace();
      d = ')';
   }
   if (c != d)
   {
      if (complain)
      {
         err_head();
         fprintf(stderr, "expression syntax error -- junk after expression\n");
      }
      while (Get() != d)
         ;
   }
#ifdef DEBUG_EXPR
   if (debugging)
   {
      outputc('<');
      dump_expr(expr);
      outputc('=');
      rv = exec_free(expr);
      outputd(rv);
      outputc('>');
   }
   else
   {
      rv = exec_free(expr);
   }
   return (rv);
#else

   return (exec_free(expr));
#endif
}

#ifdef DEBUG_EXPR
static void putx (int i)
{
   char c1 = i&0xff;
   
   if (i > 0xff)
   {
      char c2 = i >> 8;
      outputc(c2);
   }
   outputc(c1);
}

static void dump_expr (EORB_CPP_node * n)
{
   switch (n->op)
   {
      case 0:
         outputd(n->leaf);
         break;
      case '-':
         if (n->left)
         {
            dump_expr(n->left);
         }
         outputc('-');
         dump_expr(n->right);
         break;
      case '!':
         outputc('!');
         dump_expr(n->right);
         break;
      case '~':
         outputc('~');
         dump_expr(n->right);
         break;
      case 'd':
         outputs("defined(");
         outputs(n->name);
         outputc(')');
         break;
#define BIN(key,op) case key:dump_expr(n->left);\
                         putx(key);\
                         dump_expr(n->right);\
                break;

         ALLBINS
#undef BIN

   }
}

#endif
