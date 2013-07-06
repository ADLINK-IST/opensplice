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
#include "stdincs.h"
#include "cpp_malloc.h"
#include "if.h"
#include "io.h"
#include "is.h"
#include "expr.h"
#include "symtbl.h"

/* #define DEBUG_IF */

#ifdef DEBUG_IF
extern int debugging;
#endif

static void iffalse (void);
static void iftrue (void);

void do_if (int sharp)
{
   char c;
   char d;

#ifdef DEBUG_IF

   if (debugging)
   {
      outputc('<');
      outputc(sharp ? '#' : '@');
      outputs("if: ");
      // fflush(outfile);
   }
#endif
   if (in_false_if())
   {
      n_skipped_ifs ++;
#ifdef DEBUG_IF

      if (debugging)
      {
         outputs("in-false, skipped>");
         // fflush(outfile);
      }
#endif
      if (sharp)
      {
         d = '\0';
         do
         {
            c = d;
            d = Get();
         }
         while ((c == '\\') || (d != '\n'));
      }
      return ;
   }
   if (! sharp)
   {
      c = getnonspace();
      if (c != '(')
      {
         err_head();
         fprintf(stderr, "@if must have ()s\n");
         Push(c);
         iffalse();
#ifdef DEBUG_IF

         if (debugging)
         {
            outputc('>');
            // fflush(outfile);
         }
#endif
         return ;
      }
   }
   if (eval_expr(sharp, 0))
   {
      iftrue();
   }
   else
   {
      iffalse();
   }
#ifdef DEBUG_IF
   if (debugging)
   {
      outputc('>');
      // fflush(outfile);
   }
#endif
}

void do_ifdef (int sharp)
{
   char *w;

#ifdef DEBUG_IF

   if (debugging)
   {
      outputc('<');
      outputc(sharp ? '#' : '@');
      outputs("ifdef: ");
      // fflush(outfile);
   }
#endif
   if (in_false_if())
   {
      n_skipped_ifs ++;
#ifdef DEBUG_IF

      if (debugging)
      {
         outputs("in-false, skipped>");
         // fflush(outfile);
      }
#endif

   }
   else
   {
      w = read_ident();
      if (! w)
      {
#ifdef DEBUG_IF
         if (debugging)
         {
            outputs("no ident ");
            // fflush(outfile);
         }
#endif
         iffalse();
      }
      else
      {
#ifdef DEBUG_IF
         if (debugging)
         {
            outputs(w);
            outputc(' ');
            // fflush(outfile);
         }
#endif
         if (find_def(w))
         {
            iftrue();
         }
         else
         {
            iffalse();
         }
         os_free(w);
      }
#ifdef DEBUG_IF
      if (debugging)
      {
         outputc('>');
         // fflush(outfile);
      }
#endif

   }
   if (sharp)
   {
      flush_sharp_line();
   }
}

void do_ifndef (int sharp)
{
   char *w;

#ifdef DEBUG_IF

   if (debugging)
   {
      outputc('<');
      outputc(sharp ? '#' : '@');
      outputs("ifndef: ");
      // fflush(outfile);
   }
#endif
   if (in_false_if())
   {
      n_skipped_ifs ++;
#ifdef DEBUG_IF

      if (debugging)
      {
         outputs("in-false, skipped>");
         // fflush(outfile);
      }
#endif

   }
   else
   {
      w = read_ident();
      if (! w)
      {
#ifdef DEBUG_IF
         if (debugging)
         {
            outputs("no ident ");
            // fflush(outfile);
         }
#endif
         iftrue();
      }
      else
      {
#ifdef DEBUG_IF
         if (debugging)
         {
            outputs(w);
            outputc(' ');
            // fflush(outfile);
         }
#endif
         if (find_def(w))
         {
            iffalse();
         }
         else
         {
            iftrue();
         }
         os_free(w);
      }
#ifdef DEBUG_IF
      if (debugging)
      {
         outputc('>');
         // fflush(outfile);
      }
#endif

   }
   if (sharp)
   {
      flush_sharp_line();
   }
}

extern void do_else (int sharp)
{
#ifdef DEBUG_IF
   if (debugging)
   {
      outputc('<');
      outputc(sharp ? '#' : '@');
      outputs("else: ");
      // fflush(outfile);
   }
#endif
   if (n_skipped_ifs == 0)
   {
      if (ifstack)
      {
#ifdef DEBUG_IF
         if (debugging)
         {
            outputs("top ");
            output_ifstate(ifstack->condstate);
            // fflush(outfile);
         }
#endif
         switch (ifstack->condstate)
         {
            case IFSTATE_TRUE:
               ifstack->condstate = IFSTATE_STAYFALSE;
               break;
            case IFSTATE_FALSE:
               ifstack->condstate = IFSTATE_TRUE;
               break;
         }
#ifdef DEBUG_IF
         if (debugging)
         {
            outputs(" now ");
            output_ifstate(ifstack->condstate);
            outputc('>');
            // fflush(outfile);
         }
#endif

      }
      else
      {
#ifdef DEBUG_IF
         if (debugging)
         {
            outputs(" no if>");
            // fflush(outfile);
         }
#endif
         err_head();
         fprintf(stderr, "if-less else\n");
      }
   }
   else
   {
#ifdef DEBUG_IF
      if (debugging)
      {
         outputs("in-false, forgetting>");
         // fflush(outfile);
      }
#endif

   }
   if (sharp)
   {
      flush_sharp_line();
   }
}

#ifdef DEBUG_IF
output_ifstate(state)
int state;
{
   switch (state)
   {
      case IFSTATE_TRUE:
         outputs("TRUE");
         break;
      case IFSTATE_FALSE:
         outputs("FALSE");
         break;
      case IFSTATE_STAYFALSE:
         outputs("STAYFALSE");
         break;
      default:
         outputs("BAD");
         outputd(state);
         break;
   }
}
#endif

extern void do_elif (int sharp)
{
   char c;
   char d;
   int e;

#ifdef DEBUG_IF

   if (debugging)
   {
      outputc('<');
      outputc(sharp ? '#' : '@');
      outputs("elif: ");
      // fflush(outfile);
   }
#endif
   if (ifstack == 0)
   {
      err_head();
      fprintf(stderr, "if-less elif converted to normal if\n");
      iffalse();
   }
   if (n_skipped_ifs > 0)
   {
#ifdef DEBUG_IF
      if (debugging)
      {
         outputs("being skipped, ignoring>");
         // fflush(outfile);
      }
#endif
      if (sharp)
      {
         d = '\0';
         do
         {
            c = d;
            d = Get();
         }
         while ((c == '\\') || (d != '\n'));
      }
      return ;
   }
   if (! sharp)
   {
      c = getnonspace();
      if (c != '(')
      {
         err_head();
         fprintf(stderr, "@elif must have ()s\n");
         Push(c);
         ifstack->condstate = IFSTATE_STAYFALSE;
#ifdef DEBUG_IF

         if (debugging)
         {
            outputs("forcing STAYFALSE>");
            // fflush(outfile);
         }
#endif
         return ;
      }
   }
   e = eval_expr(sharp, 0);
#ifdef DEBUG_IF

   if (debugging)
   {
      outputs("expr ");
      outputd(e);
      outputc(' ');
      // fflush(outfile);
   }
#endif
#ifdef DEBUG_IF
   if (debugging)
   {
      outputs(" top ");
      output_ifstate(ifstack->condstate);
      // fflush(outfile);
   }
#endif
   switch (ifstack->condstate)
   {
      case IFSTATE_TRUE:
         ifstack->condstate = IFSTATE_STAYFALSE;
         break;
      case IFSTATE_FALSE:
         if (e)
         {
            ifstack->condstate = IFSTATE_TRUE;
         }
         break;
   }
#ifdef DEBUG_IF
   if (debugging)
   {
      outputs(" now ");
      output_ifstate(ifstack->condstate);
      outputc('>');
      // fflush(outfile);
   }
#endif
}

extern void do_endif (int sharp)
{
   IF *i;

#ifdef DEBUG_IF

   if (debugging)
   {
      outputc('<');
      outputc(sharp ? '#' : '@');
      outputs("endif: ");
      // fflush(outfile);
   }
#endif
   if (n_skipped_ifs > 0)
   {
      n_skipped_ifs --;
#ifdef DEBUG_IF

      if (debugging)
      {
         outputs("n_skipped -->");
         // fflush(outfile);
      }
#endif

   }
   else if (ifstack)
   {
      i = ifstack->next;
      OLD(ifstack);
      ifstack = i;
#ifdef DEBUG_IF

      if (debugging)
      {
         outputs("popping stack>");
         // fflush(outfile);
      }
#endif

   }
   else
   {
      err_head();
      fprintf(stderr, "if-less endif\n");
#ifdef DEBUG_IF

      if (debugging)
      {
         outputc('>');
         // fflush(outfile);
      }
#endif

   }
   if (sharp)
   {
      flush_sharp_line();
   }
}

static void iftrue (void)
{
   IF *i;

   i = NEW(IF);
   check_os_malloc(i);
   i->next = ifstack;
   ifstack = i;
   i->condstate = IFSTATE_TRUE;
#ifdef DEBUG_IF

   if (debugging)
   {
      outputs("IFTRUE");
      // fflush(outfile);
   }
#endif
}

static void iffalse (void)
{
   IF *i;

   i = NEW(IF);
   check_os_malloc(i);
   i->next = ifstack;
   ifstack = i;
   i->condstate = IFSTATE_FALSE;
#ifdef DEBUG_IF

   if (debugging)
   {
      outputs("IFFALSE");
      // fflush(outfile);
   }
#endif
}

extern int in_false_if (void)
{
   return (ifstack && (ifstack->condstate != IFSTATE_TRUE));
}

extern void maybe_print (char c)
{
   extern int incldep;

   if (incldep)
   {
      return ;
   }
   if ((c == '\n') || !in_false_if())
   {
      outputc(c);
   }
}
