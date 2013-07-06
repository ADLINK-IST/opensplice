/*
* Copyright (C) The Community OpenORB Project. All rights reserved.
*
* This software is published under the terms of The OpenORB Community Software
* License version 1.0, a copy of which has been included with this distribution
* in the LICENSE.txt file.
*/

package org.openorb.compiler.parser;

/**
 * This  class represents IDL grammar symbols.
 *
 * @author Jerome Daniel
 * @version $Revision: 1.1 $ $Date: 2006-11-24 07:46:15 $
 */

public class Token
{

    public final static int t_plus = 0;  // translate
    public final static int t_moins = 1;  // translate
    public final static int t_mul = 2;
    public final static int t_div = 3;
    public final static int t_acc_ouverte = 4;  // translate
    public final static int t_acc_fermee = 5;  // translate
    public final static int t_cro_ouvert = 6;  // translate
    public final static int t_cro_ferme = 7;  // translate
    public final static int t_par_ouverte = 8;  // translate
    public final static int t_par_fermee = 9;  // translate
    public final static int t_egal = 10;  // translate
    public final static int t_virgule = 11;  // translate
    public final static int t_point = 12;  // translate
    public final static int t_point_virgule = 13;  // translate
    public final static int t_deux_points = 14;  // translate
    public final static int t_inf = 15;  // translate
    public final static int t_sup = 16;  // translate
    public final static int t_inf_egal = 17;  // translate
    public final static int t_sup_egal = 18;  // translate
    public final static int t_and = 19;
    public final static int t_souligne = 20;  // translate
    public final static int t_ident = 21;
    public final static int t_any = 22;
    public final static int t_attribute = 23;
    public final static int t_boolean = 24;
    public final static int t_case = 25;
    public final static int t_char = 26;
    public final static int t_const = 27;
    public final static int t_context = 28;
    public final static int t_default = 29;
    public final static int t_double = 30;
    public final static int t_enum = 31;
    public final static int t_exception = 32;
    public final static int t_false = 33;
    public final static int t_float = 34;
    public final static int t_in = 35;
    public final static int t_inout = 36;
    public final static int t_interface = 37;
    public final static int t_long = 38;
    public final static int t_module = 39;
    public final static int t_object = 40;
    public final static int t_octet = 41;
    public final static int t_oneway = 42;
    public final static int t_out = 43;
    public final static int t_raises = 44;
    public final static int t_readonly = 46;
    public final static int t_sequence = 46;
    public final static int t_short = 47;
    public final static int t_string = 48;
    public final static int t_struct = 49;
    public final static int t_switch = 50;
    public final static int t_true = 51;
    public final static int t_typedef = 52;
    public final static int t_unsigned = 53;
    public final static int t_union = 54;
    public final static int t_void = 55;
    public final static int t_quatre_pts = 56;  // translate
    public final static int t_tilde = 58;
    public final static int t_none = 59;
    public final static int t_mod = 60;
    public final static int t_integer = 61;
    public final static int t_real = 62;
    public final static int t_chaine = 63;  // translate
    public final static int t_caractere = 64;  // translate
    public final static int t_ulong = 65;
    public final static int t_ushort = 66;
    public final static int t_diese = 67;
    public final static int t_pragma = 68;
    public final static int t_include = 69;
    public final static int t_typecode = 70;
    public final static int t_wchar = 71;
    public final static int t_wstring = 72;
    public final static int t_longlong = 73;
    public final static int t_ulonglong = 74;
    public final static int t_longdouble = 75;
    public final static int t_native = 76;
    public final static int t_define = 77;
    public final static int t_ifndef = 78;
    public final static int t_else = 79;
    public final static int t_endif = 80;
    public final static int t_undef = 81;
    public final static int t_enum_member = 82;
    public final static int t_struct_member = 83;
    public final static int t_union_member = 84;
    public final static int t_or = 85;
    public final static int t_fixed = 86;
    public final static int t_abstract = 87;
    public final static int t_ValueBase = 88;
    public final static int t_valuetype = 89;
    public final static int t_supports = 90;
    public final static int t_custom = 91;
    public final static int t_truncatable = 92;
    public final static int t_private = 93;
    public final static int t_public = 94;
    public final static int t_factory = 95;
    public final static int t_ifdef = 96;
    public final static int t_import = 97;
    public final static int t_local = 98;
    public final static int t_typeId = 99;
    public final static int t_typePrefix = 100;
    public final static int t_fin_fichier = 254;  // translate
    public final static int t_fin_ligne = 255;  // translate
    public final static int t_rshift = 300;//ES
    public final static int t_lshift = 301;//ES
}
