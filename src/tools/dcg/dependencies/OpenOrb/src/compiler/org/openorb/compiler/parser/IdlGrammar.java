/*
* Copyright (C) The Community OpenORB Project. All rights reserved.
*
* This software is published under the terms of The OpenORB Community Software
* License version 1.0, a copy of which has been included with this distribution
* in the LICENSE.txt file.
*/

package org.openorb.compiler.parser;

import org.openorb.compiler.CompilerProperties;
import org.openorb.compiler.object.IdlArray;
import org.openorb.compiler.object.IdlAttribute;
import org.openorb.compiler.object.IdlConst;
import org.openorb.compiler.object.IdlContext;
import org.openorb.compiler.object.IdlEnum;
import org.openorb.compiler.object.IdlEnumMember;
import org.openorb.compiler.object.IdlExcept;
import org.openorb.compiler.object.IdlFactory;
import org.openorb.compiler.object.IdlFactoryMember;
import org.openorb.compiler.object.IdlFixed;
import org.openorb.compiler.object.IdlIdent;
import org.openorb.compiler.object.IdlImport;
import org.openorb.compiler.object.IdlInterface;
import org.openorb.compiler.object.IdlModule;
import org.openorb.compiler.object.IdlNative;
import org.openorb.compiler.object.IdlObject;
import org.openorb.compiler.object.IdlOp;
import org.openorb.compiler.object.IdlParam;
import org.openorb.compiler.object.IdlRaises;
import org.openorb.compiler.object.IdlSequence;
import org.openorb.compiler.object.IdlSimple;
import org.openorb.compiler.object.IdlStateMember;
import org.openorb.compiler.object.IdlString;
import org.openorb.compiler.object.IdlStruct;
import org.openorb.compiler.object.IdlStructMember;
import org.openorb.compiler.object.IdlTypeDef;
import org.openorb.compiler.object.IdlUnion;
import org.openorb.compiler.object.IdlUnionMember;
import org.openorb.compiler.object.IdlValue;
import org.openorb.compiler.object.IdlValueBox;
import org.openorb.compiler.object.IdlValueInheritance;
import org.openorb.compiler.object.IdlWString;

/**
 * Cette classe represente la grammaire IDL
 *
 * @author Jerome Daniel
 * @version $Revision: 1.2 $ $Date: 2006-12-11 15:51:32 $
 */

public class IdlGrammar
{

    // ---------
    // Attributs
    // ---------

    /**
    * Permet d'acceder au parser pour parcourir le fichier IDL
    */
    public IdlParser m_parser = null;

    /**
     * Permet d'accer ? l'IR importeur
     */
    //public IRImport irImport = null;

    private CompilerProperties m_cp = null;

    // ------------
    // CONSTRUCTEUR
    // ------------
    /**
    * Construit une nouvelle class pour la gestion de la grammaire IDL
    */
    public IdlGrammar( IdlParser p )
    {
        m_parser = p;
        m_cp = p.getCompilerProperties();
        //irImport = loadIRImport( p );
    }

    /**
     * This operation is used to load the IR Import tool.
    public IRImport loadIRImport( IdlParser p )
    {
        IRImport ir_import = null;

        try
        {
            java.lang.Class clz = Thread.currentThread().getContextClassLoader().loadClass( "org.openorb.compiler.ir.IdlFromIR" );

            java.lang.Object obj = clz.newInstance();

            ir_import = ( IRImport ) obj;

            ir_import.set_parser( p );
        }
        catch ( java.lang.Throwable ex )
        {}

        return ir_import;
    }
     */

    // -----------
    // HEXA TO DEC
    // -----------
    /**
     * Cette fonction retourne la valeur decimale d'un symbole hexadecimal
     *
     * @param hexa le symbole hexadecimal
     * @return la valeur decimale correspondante
     */
    public int hexaToDec( char hexa )
    {
        switch ( hexa )
        {

        case '0' :
            return 0;

        case '1' :
            return 1;

        case '2' :
            return 2;

        case '3' :
            return 3;

        case '4' :
            return 4;

        case '5' :
            return 5;

        case '6' :
            return 6;

        case '7' :
            return 7;

        case '8' :
            return 8;

        case '9' :
            return 9;

        case 'a' :
            return 10;

        case 'b' :
            return 11;

        case 'c' :
            return 12;

        case 'd' :
            return 13;

        case 'e' :
            return 14;

        case 'f' :
            return 15;
        }

        return 0;
    }

    // ------------------
    // CONVERT TO DECIMAL
    // ------------------
    /**
     * Cette fonction convertie un nombre hexadecimal en nombre decimal
     * Le nombre est stocke dans ctx.value sous forme d'une chaine.
     */
    public void convertToDecimal()
    {
        String significatif = m_parser.ctx.value.substring( m_parser.ctx.value.indexOf( 'x' ) + 1 );
        int value = 0;

        for ( int i = 0; i < significatif.length(); i++ )
        {
            value = value * 16;
            value = value + hexaToDec ( significatif.charAt( i ) );
        }

        m_parser.ctx.value = new Integer( value ).toString();
    }

    /**
     * Inverse l'ordre d'un prefixe : omg.org -> org.omg
     *
     * @param prefix  le prefixe a inverser
     * @return le prefixe inverse
     */
    public String inversedPrefix ( String prefix )
    {
        int index = 0;
        int previous_index = 0;
        java.util.Vector seq = new java.util.Vector();
        String inversed = new String( "" );

        try
        {
            while ( index != -1 )
            {
                index = prefix.indexOf( '.', previous_index );

                if ( index != -1 )
                {
                    seq.addElement( new String( prefix.substring( previous_index, index ) ) );
                    previous_index = index + 1;
                }
            }
        }
        catch ( StringIndexOutOfBoundsException ex )
        { }

        seq.addElement( new String( prefix.substring( previous_index, prefix.length() ) ) );

        for ( int i = seq.size() - 1; i >= 0; i-- )
        {
            if ( !inversed.equals( "" ) )
                inversed = inversed + ".";

            inversed = inversed + ( String ) seq.elementAt( i );
        }

        return inversed;
    }

    /**
     * Retourne le nom complet d'un objet CORBA
     *
     * @param obj l'objet dont on doit trouver le nom complet
     * @return le nom complet
     */
    public String fullname ( IdlObject obj )
    {
        java.util.Vector v = new java.util.Vector();
        IdlObject obj2 = obj;
        String name = new String( "" );
        String s;
        boolean first = false;

        while ( obj2 != null )
        {
            if ( first )
            {
                if ( obj2.kind() == IdlType.e_interface )
                {
                    if ( obj.kind() != IdlType.e_const )
                        v.addElement( ( obj2.name() + "Package" ) );
                    else
                        v.addElement( obj2.name() );
                }
                else
                    v.addElement( obj2.name() );
            }
            else
                v.addElement( obj2.name() );


            //if ( m_cp.m_use_package == false )
            //{
            if ( obj2.upper() != null )
                if ( obj2.upper().kind() == IdlType.e_root )
                    break;

            //}

            obj2 = obj2.upper();

            first = true;
        }

        if ( m_cp.getM_packageName() != null )
        {
            if ( !m_cp.getM_packageName().equals( "" ) )
            {
                if ( !( ( m_cp.getM_packageName().equals( "generated" ) ) && ( m_cp.getM_use_package() == false ) ) )
                    name = org.openorb.compiler.idl.util.tools.adaptToDot( m_cp.getM_packageName() );
            }
        }

        if ( ( obj.getPrefix() != null ) && ( m_cp.getM_usePrefix() == true ) )
        {
            if ( !name.equals( "" ) )
                name = name + ".";

            name = name + inversedPrefix( obj.getPrefix() );
        }

        for ( int i = v.size() - 1; i >= 0; i-- )
        {
            s = ( String ) v.elementAt( i );

            if ( s != null )
            {
                if ( !name.equals( "" ) )
                    name = name + ".";

                name = name + s;
            }
        }

        return name;
    }

    /**
     * Remove the first '_' char
     */
    private String removePrefix( String name )
    {
        if ( name.charAt( 0 ) == '_' )
            return name.substring( 1 );
        else
            return name;
    }

    // ---------------
    // < SCOPED NAME >
    // ---------------
    /**
     * (11)
     * Analyse un nom eventuellement compose
     */
    public String scoped_name()
    {
        String name = new String();
        boolean stop = false;

        while ( stop == false )
        {
            if ( m_parser.ctx.symb == Token.t_quatre_pts )
            {
                name = name + "::";
                m_parser.symbole();
            }

            if ( m_parser.ctx.symb != Token.t_ident )
            {
                m_parser.show_error( "Bad identifier" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return name;
            }

            name = name + removePrefix( m_parser.ctx.value );

            m_parser.symbole();

            if ( m_parser.ctx.symb != Token.t_quatre_pts )
                stop = true;

        }

        return name;
    }

    // ---------------
    // < PRIMARY EXP >
    // ---------------
    /**
     * (23)
     * Analyse une expression constante
     */
    public long primary_exp( StringContainer str, IdlObject obj, DoubleContainer fv )
    {
        String name;
        IdlObject found;
        long value = 0;

        switch ( m_parser.ctx.symb )
        {

        case Token.t_quatre_pts :

        case Token.t_ident :
            name = scoped_name();

            if ( obj.isVisible( name, false ) == false )
            {
                m_parser.show_error( "Undefined identifier : " + name );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }
            else
            {
                found = obj.returnVisibleObject( name, false );

                if ( found.kind() == IdlType.e_const )
                {
                    value = ( ( IdlConst ) found ).intValue();
                    fv.value = value;
                    str.value = str.value + fullname( found ) + " ";
                }
                else
                    if ( found.kind() == IdlType.e_enum_member )
                    {
                        value = ( ( IdlEnumMember ) found ).getValue();
                        fv.value = value;
                        str.value = str.value + fullname( found ) + "@ ";
                    }
                    else
                    {
                        m_parser.show_error( "This identifier " + name + " doesn't correspond to a constant" );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                        m_parser.stopAt( m_parser.StopList );
                    }
            }

            break;

        case Token.t_par_ouverte :
            m_parser.symbole();
            str.value = str.value + "( ";
            value = const_exp( str, obj, fv );

            if ( m_parser.ctx.symb != Token.t_par_fermee )
            {
                m_parser.show_error( "')' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }

            str.value = str.value + ") ";
            m_parser.symbole();
            break;

        case Token.t_chaine :
            str.value = str.value + "\"" + m_parser.ctx.value + "\" ";
            m_parser.symbole();
            break;

        case Token.t_caractere :
            str.value = str.value + "'" + m_parser.ctx.value + "' ";
            m_parser.symbole();
            break;

        case Token.t_integer :
            str.value = str.value + m_parser.ctx.value;

            if ( m_parser.ctx.base == 1 )
                convertToDecimal();

            Long i = new Long( m_parser.ctx.value );

            value = i.longValue();

            fv.value = value;

            m_parser.symbole();

            break;

        case Token.t_real :
            str.value = str.value + m_parser.ctx.value + " ";

            Double d = new Double( m_parser.ctx.value );

            fv.value = d.doubleValue();

            m_parser.symbole();

            break;

        case Token.t_true :
            str.value = str.value + "true ";

            value = 1;

            m_parser.symbole();

            break;

        case Token.t_false :
            str.value = str.value + "false ";

            m_parser.symbole();

            value = 0;

            break;

        default :
            m_parser.show_error( "Incorrect expression" );

            m_parser.StopList.removeAllElements();

            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );

            m_parser.stopAt( m_parser.StopList );

            break;
        }

        return value;
    }

    // -------------
    // < UNARY EXP >
    // -------------
    /**
     * (22)
     * Ananlyse une expression constante
     */
    public long unary_exp( StringContainer str, IdlObject obj, DoubleContainer fv )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_moins :
            m_parser.symbole();
            str.value = str.value + "- ";
            return primary_exp( str, obj, fv );

        case Token.t_plus :
            m_parser.symbole();
            str.value = str.value + "+ ";
            return primary_exp( str, obj, fv );

        case Token.t_div :
            m_parser.symbole();
            str.value = str.value + "/ ";
            return primary_exp( str, obj, fv );

        case Token.t_mod :
            m_parser.symbole();
            str.value = str.value + "% ";
            return primary_exp( str, obj, fv );

        case Token.t_mul :
            m_parser.symbole();
            str.value = str.value + "* ";
            return primary_exp( str, obj, fv );

        case Token.t_tilde :
            m_parser.symbole();
            str.value = str.value + "~ ";
            return primary_exp( str, obj, fv );

        case Token.t_and :
            m_parser.symbole();
            str.value = str.value + "& ";
            return primary_exp( str, obj, fv );

        case Token.t_or :
            m_parser.symbole();
            str.value = str.value + "| ";
            return primary_exp( str, obj, fv );
        case Token.t_lshift :
            m_parser.symbole();
            str.value = str.value + "<< ";
            return primary_exp( str, obj, fv );
        case Token.t_rshift :
            m_parser.symbole();
            str.value = str.value + ">> ";
            return primary_exp( str, obj, fv );

        default :
            return primary_exp( str, obj, fv );
        }
    }

    protected boolean isIntoInterface( IdlObject cst )
    {
        IdlObject upper = final_type( cst.upper() );

        if ( ( upper.kind() == IdlType.e_interface ) || ( upper.kind() == IdlType.e_value ) )
            return true;

        return false;
    }

    // -------------
    // < CONST EXP >
    // -------------
    /**
     * (14)
     * Analyse une expression constante
     */
    public long const_exp( StringContainer str, IdlObject obj, DoubleContainer fv )
    {
        long size = 0;
        String name;
        IdlObject found;
        boolean flag = true;
        DoubleContainer fv2 = new DoubleContainer();

        while ( flag == true )
        {
            switch ( m_parser.ctx.symb )
            {

            case Token.t_moins :
                size = size - unary_exp( str, obj, fv2 );
                fv.value = fv.value - fv2.value;
                break;

            case Token.t_plus :
                size = size + unary_exp( str, obj, fv2 );
                fv.value = fv.value + fv2.value;
                break;

            case Token.t_div :
                size = size / unary_exp( str, obj, fv2 );
                fv.value = fv.value / fv2.value;
                break;

            case Token.t_mod :
                size = size % unary_exp( str, obj, fv2 );
                break;

            case Token.t_mul :
                size = size * unary_exp( str, obj, fv2 );
                fv.value = fv.value * fv2.value;
                break;

            case Token.t_tilde :
                size = unary_exp( str, obj, fv );
                break;

            case Token.t_and :
                size = size & unary_exp( str, obj, fv2 );
                break;

            case Token.t_or :
                size = size | unary_exp( str, obj, fv2 );
                break;
            case Token.t_lshift :
                size = size << unary_exp( str, obj, fv2 );
                break;
            case Token.t_rshift :
                size = size << unary_exp( str, obj, fv2 );
                break;
            case Token.t_quatre_pts :

            case Token.t_ident :
                name = scoped_name();

                if ( obj.isVisible( name, false ) == false )
                {
                    m_parser.show_error( "Undefined identifier : " + name );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                    m_parser.stopAt( m_parser.StopList );
                }
                else
                {
                    found = obj.returnVisibleObject( name, false );

                    if ( found.kind() == IdlType.e_const )
                    {
                        size = ( ( IdlConst ) found ).intValue();
                        fv.value = ( ( ( IdlConst ) found ).floatValue() );

                        if ( !isIntoInterface( found ) )
                            str.value = str.value + fullname( found ) + ".value ";
                        else
                            str.value = str.value + fullname( found ) + " ";
                    }
                    else
                        if ( found.kind() == IdlType.e_enum_member )
                        {
                            size = ( ( IdlEnumMember ) found ).getValue();
                            fv.value = size;
                            str.value = str.value + fullname( found ) + "@ ";
                        }
                        else
                        {
                            m_parser.show_error( "This identifier " + name + " doesn't correspond to a constant value" );
                            m_parser.StopList.removeAllElements();
                            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                            m_parser.stopAt( m_parser.StopList );
                        }
                }

                break;

            case Token.t_par_ouverte :
                size = primary_exp( str, obj, fv );
                break;

            case Token.t_chaine :
                str.value = str.value + "\"" + m_parser.ctx.value + "\" ";
                m_parser.symbole();
                break;

            case Token.t_caractere :
                str.value = str.value + "'" + m_parser.ctx.value + "' ";
                m_parser.symbole();
                break;

            case Token.t_integer :
                str.value = str.value + m_parser.ctx.value;

                if ( m_parser.ctx.base == 1 )
                    convertToDecimal();

                size = Long.valueOf( m_parser.ctx.value ).longValue();

                fv.value = size;

                m_parser.symbole();

                break;

            case Token.t_real :
                str.value = str.value + m_parser.ctx.value + " ";

                fv.value = Double.valueOf( m_parser.ctx.value ).doubleValue();

                m_parser.symbole();

                break;

            case Token.t_true :
                str.value = str.value + "true ";

                size = 1;

                m_parser.symbole();

                break;

            case Token.t_false :
                str.value = str.value + "false ";

                size = 0;

                m_parser.symbole();

                break;

            default :
                flag = false;

                break;
            }
        }

        return size;

    }

    // ----------------------
    // < POSITIVE INT CONST >
    // ----------------------
    /**
     * (26)
     * Analyse une expression entiere constante
     */
    public int positive_int_const( IdlObject obj )
    {
        StringContainer s = new StringContainer();
        DoubleContainer f = new DoubleContainer();
        int value = ( int ) const_exp( s, obj, f );

        if ( value < 0 )
            m_parser.show_error( "Positive interger constant expected" );

        return value;
    }

    // ---------
    // < FIXED >
    // ---------
    /**
     * (82)
     * Analyse la definition d'un type fixed
     */
    public void fixed_dcl( IdlObject obj )
    {
        int digits;
        int scale;

        if ( m_parser.ctx.symb != Token.t_fixed )
            m_parser.show_error( "'fixed' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_inf )
            m_parser.show_error( "'<' expected after 'fixed'" );

        m_parser.symbole();

        digits = positive_int_const( obj );

        if ( m_parser.ctx.symb != Token.t_virgule )
            m_parser.show_error( "',' expected" );

        m_parser.symbole();

        scale = positive_int_const( obj );

        if ( m_parser.ctx.symb != Token.t_sup )
        {
            m_parser.show_error( "'>' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.StopList.addElement( new Integer( Token.t_sup ) );
            m_parser.stopAt( m_parser.StopList );
        }

        m_parser.symbole();

        obj.addIdlObject( new IdlFixed( digits, scale, obj ) );
    }


    // ---------------
    // < FIXED CONST >
    // ---------------
    /**
     * (82)
     * Analyse la definition d'un type fixed constant
     */
    public void fixed_const_dcl( IdlObject obj )
    {
        if ( m_parser.ctx.symb != Token.t_fixed )
            m_parser.show_error( "'fixed' expected" );

        m_parser.symbole();

        obj.addIdlObject( new IdlFixed( 0, 0, obj ) );
    }


    // -----------
    // < WSTRING >
    // -----------
    /**
     * (67)
     * Analyse la definition d'une chaine de caracteres larges
     */
    public void wstring_dcl( IdlObject obj )
    {
        int size = 0;

        if ( m_parser.ctx.symb != Token.t_wstring )
            m_parser.show_error( "'wstring' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb == Token.t_inf )
        {
            m_parser.symbole();

            size = positive_int_const( obj );

            if ( m_parser.ctx.symb != Token.t_sup )
            {
                m_parser.show_error( "'>' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.StopList.addElement( new Integer( Token.t_sup ) );
                m_parser.stopAt( m_parser.StopList );
            }

            m_parser.symbole();

        }

        obj.addIdlObject( new IdlWString( size, obj ) );
    }

    // ----------
    // < STRING >
    // ----------
    /**
     * (66)
     * Analyse la definition d'une chaine de caracteres
     */
    public void string_dcl( IdlObject obj )
    {
        int size = 0;

        if ( m_parser.ctx.symb != Token.t_string )
            m_parser.show_error( "'string' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb == Token.t_inf )
        {
            m_parser.symbole();

            size = positive_int_const( obj );

            if ( m_parser.ctx.symb != Token.t_sup )
            {
                m_parser.show_error( "'>' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.StopList.addElement( new Integer( Token.t_sup ) );
                m_parser.stopAt( m_parser.StopList );
            }

            m_parser.symbole();

        }

        obj.addIdlObject( new IdlString( size, obj ) );
    }

    // ------------
    // < SEQUENCE >
    // ------------
    /**
     * (65)
     * Analyse la definition d'une sequence
     */
    public void sequence_dcl( IdlObject obj )
    {
        IdlSequence seq_obj;
        int size = 0;

        seq_obj = new IdlSequence( obj );

        seq_obj.attach_comment();

        if ( m_parser.ctx.symb != Token.t_sequence )
            m_parser.show_error( "'sequence' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb == Token.t_inf )
        {
            m_parser.symbole();

            simple_type_spec( seq_obj );

            if ( m_parser.ctx.symb == Token.t_virgule )
            {
                m_parser.symbole();
                size = positive_int_const( obj );
            }

            seq_obj.setSize( size );

            if ( m_parser.ctx.symb != Token.t_sup )
            {
                m_parser.show_error( "'>' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }

            obj.addIdlObject( seq_obj );

        }

        obj.addIdlObject( new IdlString( size, obj ) );
    }

    // --------------
    // < CONST TYPE >
    // --------------
    /**
     * (13)
     * Analyse un type de constante
     */
    public void const_type( IdlObject obj )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_octet :
            obj.addIdlObject( IdlSimple.octet_type );
            break;

        case Token.t_boolean :
            obj.addIdlObject( IdlSimple.boolean_type );
            break;

        case Token.t_float :
            obj.addIdlObject( IdlSimple.float_type );
            break;

        case Token.t_double :
            obj.addIdlObject( IdlSimple.double_type );
            break;

        case Token.t_longdouble :
            obj.addIdlObject( IdlSimple.longdouble_type );
            break;

        case Token.t_short :
            obj.addIdlObject( IdlSimple.short_type );
            break;

        case Token.t_ushort :
            obj.addIdlObject( IdlSimple.ushort_type );
            break;

        case Token.t_long :
            obj.addIdlObject( IdlSimple.long_type );
            break;

        case Token.t_ulong :
            obj.addIdlObject( IdlSimple.ulong_type );
            break;

        case Token.t_longlong :
            obj.addIdlObject( IdlSimple.longlong_type );
            break;

        case Token.t_ulonglong :
            obj.addIdlObject( IdlSimple.ulonglong_type );
            break;

        case Token.t_char :
            obj.addIdlObject( IdlSimple.char_type );
            break;

        case Token.t_wchar :
            obj.addIdlObject( IdlSimple.wchar_type );
            break;

        case Token.t_wstring :
            wstring_dcl( obj );
            return;

        case Token.t_string :
            string_dcl( obj );
            return;

        case Token.t_fixed :
            fixed_const_dcl( obj );
            return;

        case Token.t_quatre_pts :

        case Token.t_ident :
            String name = scoped_name();

            if ( obj.isVisible( name, false ) == false )
                m_parser.show_error( "Undefined idenfitier : " + name );
            else
                obj.addIdlObject( new IdlIdent( name, obj, obj.returnVisibleObject( name, false ) ) );

            return;
        }

        m_parser.symbole();
    }

    // ---------
    // < CONST >
    // ---------
    /**
     * (12)
     * Analyse la definition d'une constante
     * <const> ::= 'const' <const_type> <identifier> '=' <const_exp>
     */
    public void const_dcl( IdlObject obj )
    {
        IdlConst const_obj;
        StringContainer exp = new StringContainer( "" );
        long value;

        const_obj = new IdlConst( obj );

        const_obj.attach_comment();

        if ( m_parser.ctx.symb != Token.t_const )
            m_parser.show_error( "'const' expected" );

        m_parser.symbole();

        const_type( const_obj );

        if ( m_parser.ctx.symb != Token.t_ident )
            m_parser.show_error( "Identifier expected" );
        else
        {
            const_obj.name( m_parser.ctx.value );

            if ( obj.isVisible( m_parser.ctx.value, true ) == true ){
	    	//const can be overrided just for interfaces
		if(!(obj instanceof IdlInterface))
               	    m_parser.show_error( "Identifier already used : "
			    + m_parser.ctx.value );
		if(((IdlInterface)obj).searchObject(m_parser.ctx.value) != null)
               	    m_parser.show_error( "Identifier already used : "
			    + m_parser.ctx.value );
	    }
            //    parser.show_error( "Identifier already used : " + parser.ctx.value );
            m_parser.symbole();
        }

        if ( m_parser.ctx.symb != Token.t_egal )
        {
            m_parser.show_error( "'=' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }
        else
        {
            m_parser.symbole();

            DoubleContainer fvalue = new DoubleContainer( 0 );

            value = const_exp( exp, obj, fvalue );
            const_obj.expression( exp.value );
            const_obj.intValue( value );
            const_obj.floatValue( fvalue.value );
            obj.addIdlObject( const_obj );
            m_parser.ctx.one = m_parser.ctx.symb;
        }

    }

    // --------------------
    // < CONSTR_TYPE_SPEC >
    // --------------------
    /**
     * (33)
     * Ananlyse un type de donnee construit
     */
    public void constr_type_spec( IdlObject obj )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_struct :
            struct_dcl( obj );
            break;

        case Token.t_union :
            union_dcl( obj );
            break;

        case Token.t_enum :
            enum_dcl( obj );
            break;
        }

        m_parser.symbole();
    }

    // --------------------
    // < SIMPLE TYPE SPEC >
    // --------------------
    /**
     * (30)
     * Analyse un type de donnee simple
     */
    public void simple_type_spec( IdlObject obj )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_float :
            obj.addIdlObject( IdlSimple.float_type );
            break;

        case Token.t_double :
            obj.addIdlObject( IdlSimple.double_type );
            break;

        case Token.t_longdouble :
            obj.addIdlObject( IdlSimple.longdouble_type );
            break;

        case Token.t_short :
            obj.addIdlObject( IdlSimple.short_type );
            break;

        case Token.t_ushort :
            obj.addIdlObject( IdlSimple.ushort_type );
            break;

        case Token.t_long :
            obj.addIdlObject( IdlSimple.long_type );
            break;

        case Token.t_ulong :
            obj.addIdlObject( IdlSimple.ulong_type );
            break;

        case Token.t_longlong :
            obj.addIdlObject( IdlSimple.longlong_type );
            break;

        case Token.t_ulonglong :
            obj.addIdlObject( IdlSimple.ulonglong_type );
            break;

        case Token.t_char :
            obj.addIdlObject( IdlSimple.char_type );
            break;

        case Token.t_wchar :
            obj.addIdlObject( IdlSimple.wchar_type );
            break;

        case Token.t_fixed :
            fixed_dcl( obj );
            return;

        case Token.t_wstring :
            wstring_dcl( obj );
            return;

        case Token.t_string :
            string_dcl( obj );
            return;

        case Token.t_quatre_pts :

        case Token.t_ident :
            String name = scoped_name();

            if ( obj.isVisible( name, false ) == false )
                m_parser.show_error( "Undefined identifier : " + name );
            else
                obj.addIdlObject( new IdlIdent( name, obj, obj.returnVisibleObject( name, false ) ) );

            return;

        case Token.t_boolean :
            obj.addIdlObject( IdlSimple.boolean_type );

            break;

        case Token.t_octet :
            obj.addIdlObject( IdlSimple.octet_type );

            break;

        case Token.t_any :
            obj.addIdlObject( IdlSimple.any_type );

            break;

        case Token.t_object :
            obj.addIdlObject( IdlSimple.object_type );

            break;

        case Token.t_sequence :
            sequence_dcl( obj );

            break;

        case Token.t_ValueBase :
            obj.addIdlObject( IdlSimple.valuebase_type );

            break;
        }

        m_parser.symbole();
    }

    // -------------
    // < TYPE SPEC >
    // -------------
    /**
     * (29)
     * Ananlyse un type de donnee
     */
    public void type_spec( IdlObject obj )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_float :

        case Token.t_double :

        case Token.t_longdouble :

        case Token.t_short :

        case Token.t_ushort :

        case Token.t_long :

        case Token.t_ulong :

        case Token.t_longlong :

        case Token.t_ulonglong :

        case Token.t_char :

        case Token.t_wchar :

        case Token.t_boolean :

        case Token.t_octet :

        case Token.t_any :

        case Token.t_sequence :

        case Token.t_fixed :

        case Token.t_wstring :

        case Token.t_string :

        case Token.t_quatre_pts :

        case Token.t_ident :

        case Token.t_object :

        case Token.t_ValueBase :
            simple_type_spec( obj );
            break;

        case Token.t_struct :

        case Token.t_union :

        case Token.t_enum :
            constr_type_spec( obj );
            break;
        }
    }

    // ---------------
    // < DECLARATORS >
    // ---------------
    /**
     * (34)
     * Analyse une declaration de membre
     */
    public void declarators( IdlObject obj )
    {
        java.util.Vector dims;
        int dim;

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }
        else
        {
            if ( obj.upper().isVisible( m_parser.ctx.value, true ) == true )
                m_parser.show_error( "Identifier already used : " + m_parser.ctx.value );

            obj.name( m_parser.ctx.value );

            m_parser.symbole();

            if ( m_parser.ctx.symb == Token.t_cro_ouvert )
            {
                dims = new java.util.Vector();

                while ( true )
                {
                    m_parser.symbole();

                    dim = positive_int_const( obj );
                    dims.addElement( new Integer( dim ) );

                    if ( m_parser.ctx.symb != Token.t_cro_ferme )
                    {
                        m_parser.show_error( "']' expected" );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                        m_parser.stopAt( m_parser.StopList );
                    }

                    m_parser.symbole();

                    if ( m_parser.ctx.symb != Token.t_cro_ouvert )
                        break;
                }


                IdlArray top_level = new IdlArray( obj );

                top_level.setDimension( ( ( Integer ) dims.elementAt( 0 ) ).intValue() );

                IdlArray array_obj = top_level;

                if ( dims.size() > 1 )
                {
                    for ( int i = 1; i < dims.size(); i++ )
                    {
                        IdlArray array_new = new IdlArray( array_obj );

                        array_new.setDimension( ( ( Integer ) dims.elementAt( i ) ).intValue() );

                        array_obj.addIdlObject( array_new );

                        array_obj = array_new;

                    }
                }

                array_obj.addIdlObject( obj.type() );
                obj.type( top_level );
            }
        }
    }

    // ----------
    // < MEMBER >
    // ----------
    /**
     * (56)
     * Analyse un membre d'une structure
     */
    public void member( IdlObject obj, java.util.Hashtable list )
    {
        IdlStructMember first_member_obj;
        IdlStructMember member_obj;

        first_member_obj = new IdlStructMember( obj );

        first_member_obj.attach_comment();

        type_spec( first_member_obj );
        first_member_obj.reset();

        while ( true )
        {
            member_obj = new IdlStructMember( obj );
            member_obj.type( first_member_obj.type() );
            member_obj.attach_comment( first_member_obj.getComment() );
            declarators( member_obj );

            switch ( member_obj.type().kind() )
            {

            case IdlType.e_array :

            case IdlType.e_sequence :
                m_parser.warning( "Anonymous sequences and arrays are deprecated" );
                break;

            case IdlType.e_string :

                if ( ( ( IdlString ) member_obj.type() ).max() != 0 )
                    m_parser.warning( "Anonymous unbounded string are deprecated" );

                break;

            case IdlType.e_wstring :
                if ( ( ( IdlWString ) member_obj.type() ).max() != 0 )
                    m_parser.warning( "Anonymous unbounded string are deprecated" );

                break;

            case IdlType.e_fixed :
                m_parser.warning( "Anonymous fixed are deprecated" );

                break;
            }

            obj.addIdlObject( member_obj );

            if ( list.get( member_obj.name() ) != null )
            {
                m_parser.show_error( "This identifier is already used : " + member_obj.name() );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }
            else
                list.put( member_obj.name(), member_obj );

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;

            m_parser.symbole();
        }
    }

    // ---------------
    // < MEMBER LIST >
    // ---------------
    /**
     * (55)
     * Analyse les membres d'une structure
     */
    public void member_list( IdlObject obj )
    {
        java.util.Hashtable list = new java.util.Hashtable();

        while ( true )
        {
            member( obj, list );

            if ( m_parser.ctx.symb != Token.t_point_virgule )
            {
                m_parser.show_error( "';' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }

            //parser.symbole();
            block_symbole( obj );

            if ( m_parser.ctx.symb == Token.t_acc_fermee )
                break;
        }
    }

    // ----------
    // < STRUCT >
    // ----------
    /**
     * (54)
     * Analyse la definition d'une structure
     */
    public void struct_dcl( IdlObject obj )
    {
        IdlStruct struct_obj;

        struct_obj = new IdlStruct( obj );
        struct_obj.attach_comment();

        m_parser.container = struct_obj;

        if ( m_parser.ctx.symb != Token.t_struct )
            m_parser.show_error( "'struct' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
            m_parser.show_error( "Identifier expected" );
        else
        {

            if ( obj.isVisible( m_parser.ctx.value, true ) )
            {
                // it may be a forward declaration

                IdlObject visible = obj.returnVisibleObject( m_parser.ctx.value, true );

                if ( visible.kind() == IdlType.e_struct )
                {
                    if ( ( ( IdlStruct ) visible ).isForward() )
                        ( ( IdlStruct ) visible ).setDefinition( struct_obj );
                    else
                    {
                        m_parser.show_error( "The '" + m_parser.ctx.value + "' structure is already defined..." );
                    }
                }
                else
                    m_parser.show_error( "Identifier already used : " + m_parser.ctx.value );
            }

            struct_obj.name( m_parser.ctx.value );
            obj.addIdlObject( struct_obj );
            m_parser.symbole();
        }

        if ( m_parser.ctx.symb != Token.t_acc_ouverte )
        {
            if ( m_parser.ctx.symb != Token.t_point_virgule )
            {
                m_parser.show_error( "';' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }

            struct_obj.isForward( true );
            m_parser.ctx.one = m_parser.ctx.symb;
            return;
        }
        else
        {

            block_symbole( obj );
            member_list( struct_obj );

            if ( m_parser.ctx.symb != Token.t_acc_fermee )
            {
                m_parser.show_error( "'}' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }
        }

        m_parser.container = obj;
    }

    // --------------------
    // < SWITCH TYPE SPEC >
    // --------------------
    /**
    * (58)
    * Analyse un type de donnee de discriminant d'union
    */
    public void switch_type_spec( IdlObject obj )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_short :
            obj.addIdlObject( IdlSimple.short_type );
            break;

        case Token.t_ushort :
            obj.addIdlObject( IdlSimple.ushort_type );
            break;

        case Token.t_long :
            obj.addIdlObject( IdlSimple.long_type );
            break;

        case Token.t_ulong :
            obj.addIdlObject( IdlSimple.ulong_type );
            break;

        case Token.t_longlong :
            obj.addIdlObject( IdlSimple.longlong_type );
            break;

        case Token.t_ulonglong :
            obj.addIdlObject( IdlSimple.ulonglong_type );
            break;

        case Token.t_char :
            obj.addIdlObject( IdlSimple.char_type );
            break;

        case Token.t_wchar :
            obj.addIdlObject( IdlSimple.wchar_type );
            break;

        case Token.t_quatre_pts :

        case Token.t_ident :
            String name = scoped_name();

            if ( obj.isVisible( name, false ) == false )
                m_parser.show_error( "Undefined identifier : " + name );
            else
                obj.addIdlObject( new IdlIdent( name, obj, obj.returnVisibleObject( name, false ) ) );

            //obj.addIdlObject( obj.returnVisibleObject(name,false) );
            return;

        case Token.t_boolean :
            obj.addIdlObject( IdlSimple.boolean_type );

            break;

        case Token.t_enum :
            enum_dcl( obj );

            break;

        default :
            m_parser.show_error( "Bad type for union discriminant" );

            break;
        }

        m_parser.symbole();
    }

    // ---------------
    // < SWITCH CASE >
    // ---------------
    /**
     * (60)
     * Analyse un des cas d'une union
     */
    public int switch_case( IdlObject obj, BooleanContainer asNext, java.util.Hashtable list )
    {
        int index = -1;
        StringContainer str = new StringContainer( "" );
        int size = 0;
        DoubleContainer fv = new DoubleContainer( 0 );

        asNext.value = false;

        switch ( m_parser.ctx.symb )
        {

        case Token.t_case :
            m_parser.symbole();
            size = ( int ) const_exp( str, obj, fv );
            break;

        case Token.t_default :
            m_parser.symbole();
            index = 0;
            break;

        default :
            m_parser.show_error( "Reserved words 'case' or 'default' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return index;
        }

        if ( m_parser.ctx.symb != Token.t_deux_points )
        {
            m_parser.show_error( "':' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }
        else
        {
            IdlUnionMember member_obj = new IdlUnionMember( obj );

            m_parser.symbole();

            if ( m_parser.ctx.symb != Token.t_case )
            {
                type_spec( member_obj );
                member_obj.reset();
                // member_obj.current()._upper = obj.upper(); // modify to supported nested items.

                declarators( member_obj );

                switch ( member_obj.type().kind() )
                {

                case IdlType.e_array :

                case IdlType.e_sequence :
                    m_parser.warning( "Anonymous sequences and arrays are deprecated" );
                    break;

                case IdlType.e_string :

                    if ( ( ( IdlString ) member_obj.type() ).max() != 0 )
                        m_parser.warning( "Anonymous unbounded string are deprecated" );

                    break;

                case IdlType.e_wstring :
                    if ( ( ( IdlWString ) member_obj.type() ).max() != 0 )
                        m_parser.warning( "Anonymous unbounded string are deprecated" );

                    break;

                case IdlType.e_fixed :
                    m_parser.warning( "Anonymous fixed are deprecated" );

                    break;
                }
            }
            else
            {
                member_obj.setAsNext();
                asNext.value = true;
            }

            if ( index == 0 )
                member_obj.setAsDefault();

            member_obj.setExpression( str.value );

            member_obj.setValue( size );

            if ( member_obj.name() != null )
            {
                if ( list.get( member_obj.name() ) != null )
                {
                    m_parser.show_error( "This identifier is already used : " + member_obj.name() );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                    m_parser.stopAt( m_parser.StopList );
                }
                else
                    list.put( member_obj.name(), member_obj );
            }

            obj.addIdlObject( member_obj );
        }

        return index;
    }

    // ---------------
    // < SWITCH BODY >
    // ---------------
    /**
     * (59)
     * Analyse le contenu d'une union
     */
    public void switch_body( IdlObject obj )
    {
        int idx = 0;
        int limit;
        boolean found = false;
        boolean stop;
        BooleanContainer asNext = new BooleanContainer();
        java.util.Hashtable list = new java.util.Hashtable();

        // Analyse la declaration de chaque element

        while ( true )
        {
            if ( switch_case( obj, asNext, list ) != -1 )
                ( ( IdlUnion ) obj ).index( idx );

            idx++;

            if ( asNext.value == false )
            {
                if ( m_parser.ctx.symb != Token.t_point_virgule )
                {
                    m_parser.show_error( "';' expected" );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                    m_parser.stopAt( m_parser.StopList );
                }

                //parser.symbole();
                block_symbole( obj );

                if ( m_parser.ctx.symb == Token.t_acc_fermee )
                    break;
            }
        }

        // Fixe le Type si un membre reference un autre membre

        int pos = obj.pos();

        obj.reset();

        while ( obj.end() != true )
        {
            if ( ( ( IdlUnionMember ) ( obj.current() ) ).isAsNext() )
            {
                int back = obj.pos();
                stop = false;
                found = false;

                while ( stop != true )
                {
                    if ( !( ( IdlUnionMember ) ( obj.current() ) ).isAsNext() )
                    {
                        found = true;
                        limit = obj.pos();
                        obj.current().reset();
                        IdlObject type = obj.current().current();
                        String name = obj.current().name();
                        obj.pos( back );

                        for ( int i = back; i < limit; i++ )
                        {
                            ( ( IdlUnionMember ) ( obj.current() ) ).memberTypeAndNameIs( type, name );
                            obj.next();
                        }

                        obj.pos( limit - 1 );
                        stop = true;
                    }

                    obj.next();

                    if ( obj.end() == true )
                        stop = true;
                }

                if ( found == false )
                    m_parser.show_error( "A type declaration is expected for union members" );
            }

            obj.next();
        }


        obj.pos( pos );
    }

    // ---------
    // < UNION >
    // ---------
    /**
     * (57)
     * Analyse la definition d'une union
     */
    public void union_dcl( IdlObject obj )
    {
        IdlUnion union_obj;

        union_obj = new IdlUnion( obj );

        union_obj.attach_comment();

        m_parser.container = union_obj;

        if ( m_parser.ctx.symb != Token.t_union )
            m_parser.show_error( "'union' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }
        else
        {

            if ( obj.isVisible( m_parser.ctx.value, true ) )
            {
                // it may be a forward declaration

                IdlObject visible = obj.returnVisibleObject( m_parser.ctx.value, true );

                if ( visible.kind() == IdlType.e_union )
                {
                    if ( ( ( IdlUnion ) visible ).isForward() )
                        ( ( IdlUnion ) visible ).setDefinition( union_obj );
                    else
                    {
                        m_parser.show_error( "The '" + m_parser.ctx.value + "' union is already defined..." );
                    }
                }
                else
                    m_parser.show_error( "Identifier already used : " + m_parser.ctx.value );
            }

            union_obj.name( m_parser.ctx.value );
            obj.addIdlObject( union_obj );
            m_parser.symbole();

            if ( m_parser.ctx.symb != Token.t_switch )
            {
                if ( m_parser.ctx.symb != Token.t_point_virgule )
                {
                    m_parser.show_error( "'switch' or ';' expected" );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                    m_parser.stopAt( m_parser.StopList );
                    return;
                }

                union_obj.isForward( true );
                m_parser.ctx.one = m_parser.ctx.symb;
                return;
            }
            else
            {
                m_parser.symbole();

                if ( m_parser.ctx.symb != Token.t_par_ouverte )
                {
                    m_parser.show_error( "'(' expected" );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                    m_parser.stopAt( m_parser.StopList );
                }
                else
                {
                    m_parser.symbole();

                    IdlUnionMember d = new IdlUnionMember( obj );
                    d.name( "__d" );
                    switch_type_spec( d );
                    union_obj.addIdlObject( d );

                    if ( m_parser.ctx.symb != Token.t_par_fermee )
                    {
                        m_parser.show_error( "')' expected" );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                        m_parser.stopAt( m_parser.StopList );
                    }
                    else
                    {
                        m_parser.symbole();

                        if ( m_parser.ctx.symb != Token.t_acc_ouverte )
                        {
                            m_parser.show_error( "'{' expected" );
                            m_parser.StopList.removeAllElements();
                            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                            m_parser.stopAt( m_parser.StopList );
                        }
                        else
                        {
                            //parser.symbole();
                            block_symbole( obj );
                            switch_body( union_obj );

                            if ( m_parser.ctx.symb != Token.t_acc_fermee )
                            {
                                m_parser.show_error( "'}' expected" );
                                m_parser.StopList.removeAllElements();
                                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                                m_parser.stopAt( m_parser.StopList );
                            }

                        }
                    }
                }
            }
        }

        m_parser.container = obj;
    }

    // --------
    // < ENUM >
    // --------
    /**
     * (63)
     * Analyse une enumeration
     */
    public void enum_dcl( IdlObject obj )
    {
        IdlEnum enum_obj;

        enum_obj = new IdlEnum( obj );

        enum_obj.attach_comment();

        m_parser.container = enum_obj;

        if ( m_parser.ctx.symb != Token.t_enum )
            m_parser.show_error( "'enum' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
            m_parser.show_error( "Identifier expected" );
        else
        {

            if ( obj.isVisible( m_parser.ctx.value, true ) )
                m_parser.show_error( "Identifier already used : " + m_parser.ctx.value );

            enum_obj.name( m_parser.ctx.value );

            m_parser.symbole();
        }

        obj.addIdlObject( enum_obj );

        if ( m_parser.ctx.symb != Token.t_acc_ouverte )
            m_parser.show_error( "'{ expected" );
        else
        {
            //parser.symbole();
            block_symbole( obj );
        }

        java.util.Hashtable list = new java.util.Hashtable();
        int index = 0;

        while ( true )
        {
            if ( m_parser.ctx.symb != Token.t_ident )
            {
                m_parser.show_error( "Identifier expected" );
                break;
            }
            else
            {
                IdlEnumMember member_obj = new IdlEnumMember( enum_obj );
                member_obj.name( m_parser.ctx.value );
                member_obj.setValue( index++ );

                if ( list.get( member_obj.name() ) != null )
                {
                    m_parser.show_error( "This identifier is already used : " + member_obj.name() );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                    m_parser.stopAt( m_parser.StopList );
                    return;
                }
                else
                    list.put( member_obj.name(), member_obj );

                enum_obj.addIdlObject( member_obj );

                m_parser.symbole();

                if ( m_parser.ctx.symb != Token.t_virgule )
                    break;

                m_parser.symbole();
            }
        }

        if ( m_parser.ctx.symb != Token.t_acc_fermee )
            m_parser.show_error( "'}' expected" );

        m_parser.container = obj;
    }

    // -----------
    // < TYPEDEF >
    // -----------
    /**
     * (27)
     * Analyse la definition d'un nouveau type de donnee
     */
    public void type_dcl( IdlObject obj )
    {
        IdlTypeDef first_type_obj;
        IdlTypeDef type_obj;
        int count = 0;

        first_type_obj = new IdlTypeDef( obj );

        first_type_obj.attach_comment();

        if ( m_parser.ctx.symb != Token.t_typedef )
            m_parser.show_error( "'typedef' expected" );

        m_parser.symbole();

        type_spec( first_type_obj );

        // Add the sub type if it's a constructed type
        switch ( first_type_obj.type().kind() )
        {

        case IdlType.e_union :

        case IdlType.e_struct :

        case IdlType.e_enum :
            obj.addIdlObject( first_type_obj.type() );
            break;

        case IdlType.e_sequence :

            if ( ( ( IdlSequence ) first_type_obj.type() ).current().kind() == IdlType.e_sequence )
                m_parser.warning( "Anonymous sequences and arrays are deprecated" );

            break;
        }

        while ( true )
        {
            type_obj = new IdlTypeDef( obj );

            type_obj.type( first_type_obj.type() );

            type_obj.attach_comment( first_type_obj.getComment() );

            type_obj.setOrder( count++ );

            declarators( type_obj );

            obj.addIdlObject( type_obj );

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;

            m_parser.symbole();
        }

        m_parser.ctx.one = m_parser.ctx.symb;
    }

    // -------------
    // < EXCEPTION >
    // -------------
    /**
     * (71)
     * Analyse la definition d'une exception
     */
    public void except_dcl( IdlObject obj )
    {
        IdlExcept except_obj;

        except_obj = new IdlExcept( obj );

        except_obj.attach_comment();

        if ( m_parser.ctx.symb != Token.t_exception )
            m_parser.show_error( "'exception' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
            m_parser.show_error( "Identifier expected" );
        else
        {

            if ( obj.isVisible( m_parser.ctx.value, true ) )
                m_parser.show_error( "Identifier already used : " + m_parser.ctx.value );

            except_obj.name( m_parser.ctx.value );

            m_parser.symbole();
        }

        if ( m_parser.ctx.symb != Token.t_acc_ouverte )
        {
            m_parser.show_error( "'{' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }
        else
        {
            m_parser.symbole();

            if ( m_parser.ctx.symb != Token.t_acc_fermee )
                member_list( except_obj );

            if ( m_parser.ctx.symb != Token.t_acc_fermee )
            {
                m_parser.show_error( "'}' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }
        }

        obj.addIdlObject( except_obj );

    }

    // ----------
    // < MODULE >
    // ----------
    /**
     * (3)
     * Analyse un module
     */
    public void module_dcl( IdlObject obj )
    {
        IdlModule module_obj;

        module_obj = new IdlModule( obj );

        module_obj.attach_comment();

        m_parser.container = module_obj;

        if ( m_parser.ctx.symb != Token.t_module )
            m_parser.show_error( "'module' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
            m_parser.show_error( "Identifier expected" );
        else
        {
            if ( obj.isVisible( m_parser.ctx.value, true ) )
            {
                module_obj = ( IdlModule ) obj.returnVisibleObject( m_parser.ctx.value, true );
                module_obj.refreshIncluded();
                m_parser.container = module_obj;

                if ( module_obj._import == false )
                {
                    m_parser.show_error( "This module ( " + m_parser.ctx.value + " ) is exposed but not imported, you cannot re-open it !" );
                    return;
                }
            }
            else
            {
                module_obj.name( m_parser.ctx.value );
                obj.addIdlObject( module_obj );
            }

            m_parser.symbole();

        }

        if ( m_parser.ctx.symb != Token.t_acc_ouverte )
            m_parser.show_error( "'{' expected" );
        else
            m_parser.symbole();

        while ( true )
        {
            idl_definition( module_obj );

            if ( m_parser.ctx.symb == Token.t_acc_fermee )
                break;

            if ( m_parser.ctx.symb == Token.t_fin_fichier )
            {
                m_parser.show_error( "End of module expected for '" + module_obj.name() + "'" );
                return;
            }
        }

        m_parser.container = obj;
    }

    /**
    * Returns the final definition of a data type
    *
    * @param obj the object
    * @return the final definition
    */
    public IdlObject final_type( IdlObject obj )
    {
        switch ( obj.kind() )
        {

        case IdlType.e_ident :
            return final_type( ( ( IdlIdent ) obj ).internalObject() );

        case IdlType.e_typedef :

        case IdlType.e_union_member :

        case IdlType.e_struct_member :

        case IdlType.e_param :
            return final_type( obj.current() );

        default :
            return obj;
        }
    }

    // -------------------
    // < PARAM TYPE SPEC >
    // -------------------
    /**
     * (80)
     * Analyse le type d'un parametre
     */
    public void param_type_spec( IdlObject obj )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_float :
            obj.addIdlObject( IdlSimple.float_type );
            break;

        case Token.t_double :
            obj.addIdlObject( IdlSimple.double_type );
            break;

        case Token.t_longdouble :
            obj.addIdlObject( IdlSimple.longdouble_type );
            break;

        case Token.t_short :
            obj.addIdlObject( IdlSimple.short_type );
            break;

        case Token.t_ushort :
            obj.addIdlObject( IdlSimple.ushort_type );
            break;

        case Token.t_long :
            obj.addIdlObject( IdlSimple.long_type );
            break;

        case Token.t_ulong :
            obj.addIdlObject( IdlSimple.ulong_type );
            break;

        case Token.t_longlong :
            obj.addIdlObject( IdlSimple.longlong_type );
            break;

        case Token.t_ulonglong :
            obj.addIdlObject( IdlSimple.ulonglong_type );
            break;

        case Token.t_char :
            obj.addIdlObject( IdlSimple.char_type );
            break;

        case Token.t_wchar :
            obj.addIdlObject( IdlSimple.wchar_type );
            break;

        case Token.t_fixed :
            fixed_dcl( obj );
            return;

        case Token.t_wstring :
            wstring_dcl( obj );
            return;

        case Token.t_string :
            string_dcl( obj );
            return;

        case Token.t_quatre_pts :

        case Token.t_ident :
            String name = scoped_name();

            if ( obj.isVisible( name, false ) == false )
                m_parser.show_error( "Undefined identifier " + name );
            else
            {
                // check the parameter type
                IdlObject ret = obj.returnVisibleObject( name, false );

                if ( final_type( ret ).kind() == IdlType.e_exception )
                {
                    m_parser.show_error( "Invalid type : " + ret.name() );
                }

                obj.addIdlObject( new IdlIdent( name, obj, ret ) );
            }

            return;

        case Token.t_boolean :
            obj.addIdlObject( IdlSimple.boolean_type );
            break;

        case Token.t_octet :
            obj.addIdlObject( IdlSimple.octet_type );
            break;

        case Token.t_any :
            obj.addIdlObject( IdlSimple.any_type );
            break;

        case Token.t_object :
            obj.addIdlObject( IdlSimple.object_type );
            break;
            //-case Token.t_typecode :
            //- obj.addIdlObject( IdlSimple.typecode_type );
            //- break;

        case Token.t_ValueBase :
            obj.addIdlObject( IdlSimple.valuebase_type );
            break;
        }

        m_parser.symbole();
    }

    // --------
    // < ATTR >
    // --------
    /**
     * (70)
     * Analyse la definition d'un attribut
     */
    public void attr_dcl( IdlObject obj )
    {
        IdlAttribute first_attr_obj;
        IdlAttribute attr_obj;
        boolean rd = false;

        first_attr_obj = new IdlAttribute( obj );

        first_attr_obj.attach_comment();

        if ( m_parser.ctx.symb == Token.t_readonly )
        {
            m_parser.symbole();
            rd = true;
        }

        if ( m_parser.ctx.symb != Token.t_attribute )
        {
            m_parser.show_error( "'attribute' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_parser.symbole();

        param_type_spec( first_attr_obj );

        switch ( first_attr_obj.type().kind() )
        {

        case IdlType.e_string :

            if ( ( ( IdlString ) first_attr_obj.type() ).max() != 0 )
                m_parser.warning( "Anonymous unbounded string are deprecated" );

            break;

        case IdlType.e_wstring :
            if ( ( ( IdlWString ) first_attr_obj.type() ).max() != 0 )
                m_parser.warning( "Anonymous unbounded string are deprecated" );

            break;

        case IdlType.e_fixed :
            m_parser.warning( "Anonymous fixed are deprecated" );

            break;
        }

        while ( true )
        {
            if ( m_parser.ctx.symb != Token.t_ident )
            {
                m_parser.show_error( "Identifier expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }

            if ( obj.isVisible( m_parser.ctx.value, true ) == true )
            {
                m_parser.show_error( "Identifier already used : " + m_parser.ctx.value );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }

            attr_obj = new IdlAttribute( obj );
            attr_obj.name( m_parser.ctx.value );
            attr_obj.readOnly( rd );
            attr_obj.type( first_attr_obj.type() );
            attr_obj.attach_comment( first_attr_obj.getComment() );

            obj.addIdlObject( attr_obj );

            m_parser.symbole();

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;

            m_parser.symbole();
        }

        m_parser.ctx.one = m_parser.ctx.symb;
    }

    // ----------------
    // < OP TYPE SPEC >
    // ----------------
    /**
     * (74)
     * Analyse le type de retour d'une operation
     */
    public void op_type_spec( IdlOp obj )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_void :
            obj.addIdlObject( IdlSimple.void_type );
            m_parser.symbole();
            break;

        default :
            param_type_spec( obj );
            break;
        }
    }

    // -------------------
    // < PARAM ATTRIBUTE >
    // -------------------
    /**
     * (77)
     * Analyse l'attribut d'un parametre
     */
    public void param_attribute( IdlParam obj )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_in :
            obj.param_attr( 0 );
            break;

        case Token.t_out :
            obj.param_attr( 1 );
            break;

        case Token.t_inout :
            obj.param_attr( 2 );
            break;

        default :
            m_parser.show_error( "Attribute ( in, out, inout ) expected" );
            return;
        }

        m_parser.symbole();
    }

    // -------------
    // < PARAM DCL >
    // -------------
    /**
     * (76)
     * Analyse la definition d'un parametre d'une operation
     */
    public void param_dcl( IdlOp obj )
    {
        IdlParam param_obj;

        param_obj = new IdlParam( obj );

        param_attribute( param_obj );
        param_type_spec( param_obj );

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected for parameter" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_par_fermee ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        if ( obj.returnVisibleObject( m_parser.ctx.value, true ) != null )
        {
            m_parser.show_error( "Identifier already used" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_par_fermee ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        param_obj.name( m_parser.ctx.value );

        obj.addIdlObject( param_obj );

        m_parser.symbole();
    }

    // ------------------
    // < PARAMETER DCLS >
    // ------------------
    /**
     * (75)
     * Analyse les parametres d'une operation
     */
    public void parameter_dcls( IdlOp obj )
    {
        if ( m_parser.ctx.symb != Token.t_par_ouverte )
        {
            m_parser.show_error( "'(' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_parser.symbole();

        while ( true )
        {
            if ( m_parser.ctx.symb == Token.t_par_fermee )
                break;

            param_dcl( obj );

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;

            m_parser.symbole();

            if ( m_parser.ctx.symb == Token.t_par_fermee )
            {
                m_parser.show_error( "Parameter definition expected" );
                break;
            }

        }

        if ( m_parser.ctx.symb != Token.t_par_fermee )
        {
            m_parser.show_error( "')' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_parser.symbole();
    }

    // ---------------
    // < RAISES EXPR >
    // ---------------
    /**
     * (78)
     * Analyse l'expression "raises" d'une operation
     */
    public void raises_expr( IdlObject obj )
    {
        java.util.Hashtable exceptions = new java.util.Hashtable();
        IdlRaises raises_obj;
        String name;

        raises_obj = new IdlRaises( obj );

        if ( m_parser.ctx.symb != Token.t_raises )
            m_parser.show_error( "'raises' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_par_ouverte )
        {
            m_parser.show_error( "'(' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_context ) );
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_parser.symbole();

        while ( true )
        {
            name = scoped_name();

            if ( obj.isVisible( name, false ) == false )
            {
                m_parser.show_error( "Undefined identifier : " + name );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_context ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }

            if ( obj.returnVisibleObject( name, false ).kind() != IdlType.e_exception )
            {
                m_parser.show_error( "This identifier doesn't correspond to an exception : " + name );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_context ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }

            IdlObject exp = obj.returnVisibleObject( name, false );

            raises_obj.addIdlObject( exp );

            if ( exceptions.get( exp ) != null )
            {
                m_parser.show_error( "An exception has been declared more than one time in the 'raise' clause : " + name );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_context ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }
            else
                exceptions.put( exp, exp );

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;

            m_parser.symbole();
        }

        obj.addIdlObject( raises_obj );

        if ( m_parser.ctx.symb != Token.t_par_fermee )
        {
            m_parser.show_error( "')' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_context ) );
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_parser.symbole();
    }

    // ----------------
    // < CONTEXT EXPR >
    // ----------------
    /**
     * (79)
     * Analyse l'expression "context" d'une operation
     */
    public void context_expr( IdlOp obj )
    {
        IdlContext context_obj;
        context_obj = new IdlContext( obj );
        if ( m_parser.ctx.symb != Token.t_context )
        {
            m_parser.show_error( "'context' expected" );
        }
        m_parser.symbole();
        if ( m_parser.ctx.symb != Token.t_par_ouverte )
        {
            m_parser.show_error( "'(' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_parser.symbole();

        while ( true )
        {

            if ( m_parser.ctx.symb != Token.t_chaine )
            {
                m_parser.show_error( "String expected after 'context'" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }

            context_obj.addValue( m_parser.ctx.value );
            m_parser.symbole();

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;

            m_parser.symbole();
        }

        obj.addIdlObject( context_obj );

        if ( m_parser.ctx.symb != Token.t_par_fermee )
        {
            m_parser.show_error( "')' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_context ) );
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_parser.symbole();
    }

    // ------
    // < OP >
    // ------
    /**
     * (72)
     * Analyse la definition d'une operation
     */
    public void op_dcl( IdlObject obj )
    {
        IdlOp op_obj;
        boolean one = false;

        op_obj = new IdlOp( obj );

        op_obj.attach_comment();

        if ( m_parser.ctx.symb == Token.t_oneway )
        {
            m_parser.symbole();
            one = true;
        }

        op_obj.oneway( one );

        op_type_spec( op_obj );

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected for operation" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        if ( obj.isVisible( m_parser.ctx.value, true ) )
        {
            m_parser.show_error( "Indentifier already used : " + m_parser.ctx.value );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_context ) );
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        op_obj.name( m_parser.ctx.value );
        m_parser.symbole();

        parameter_dcls( op_obj );

        if ( m_parser.ctx.symb == Token.t_raises )
            raises_expr( op_obj );

        if ( m_parser.ctx.symb == Token.t_context )
            context_expr( op_obj );

        obj.addIdlObject( op_obj );

        m_parser.ctx.one = m_parser.ctx.symb;
    }

    // ----------
    // < EXPORT >
    // ----------
    /**
     * (9)
     * Analyse les membres d'une interface
     */
    public void export_dcl( IdlObject obj )
    {
        switch ( m_parser.ctx.symb )
        {

        case Token.t_const :
            const_dcl( obj );
            break;

        case Token.t_exception :
            except_dcl( obj );
            break;

        case Token.t_struct :
            struct_dcl( obj );
            break;

        case Token.t_union :
            union_dcl( obj );
            break;

        case Token.t_enum :
            enum_dcl( obj );
            break;

        case Token.t_typedef :
            type_dcl( obj );
            break;

        case Token.t_native :
            native_dcl( obj );
            break;

        case Token.t_pragma :
            pragma_dcl( obj );
            break;

        case Token.t_typeId :
            type_id_dcl( obj );
            break;

        case Token.t_typePrefix :
            type_prefix_dcl( obj );
            break;

        case Token.t_attribute :

        case Token.t_readonly :
            attr_dcl( obj );
            break;

        case Token.t_fixed :

        case Token.t_float :

        case Token.t_double :

        case Token.t_longdouble :

        case Token.t_short :

        case Token.t_ushort :

        case Token.t_long :

        case Token.t_ulong :

        case Token.t_longlong :

        case Token.t_ulonglong :

        case Token.t_char :

        case Token.t_wchar :

        case Token.t_wstring :

        case Token.t_string :

        case Token.t_quatre_pts :

        case Token.t_ident :

        case Token.t_any :

        case Token.t_void :
            //-case Token.t_typecode :

        case Token.t_object :

        case Token.t_boolean :

        case Token.t_octet :

        case Token.t_ValueBase :

        case Token.t_oneway :
            op_dcl( obj );
            break;

        default :
            m_parser.show_error( "Unexpected or undefined key word in the interface or value body of " + obj.name() );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            break;
        }

        m_parser.symbole();
    }

    // ------------------
    // < INTERFACE BODY >
    // ------------------
    /**
     * (8)
     * Analyse le corps d'une interface
     */
    public void interface_body( IdlInterface obj )
    {
        while ( true )
        {
            if ( m_parser.ctx.symb == Token.t_acc_fermee )
                break;

            export_dcl( obj );

            if ( m_parser.ctx.symb != Token.t_point_virgule )
            {
                m_parser.show_error( "';' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }

            m_parser.symbole();
        }
    }

    // --------------------
    // < INHERITANCE SPEC >
    // --------------------
    /**
     * (10)
     * Analyse les declarations d'heritages d'une interface
     */
    public void inheritance_spec( IdlInterface obj )
    {
        String name;
        IdlObject anObj;

        if ( m_parser.ctx.symb != Token.t_deux_points )
        {
            m_parser.show_error( "':' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_parser.symbole();

        while ( true )
        {
            name = scoped_name();

            if ( obj.isVisible( name, false ) == false )
            {
                m_parser.show_error( "Undeclared interface : " + name );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }

            anObj = obj.returnVisibleObject( name, false );

            if ( ( anObj.final_kind() != IdlType.e_forward_interface ) &&
                    ( anObj.final_kind() != IdlType.e_interface ) )
            {
                m_parser.show_error( "This identifier is not an interface : " + name );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                m_parser.stopAt( m_parser.StopList );
                break;
            }

            if ( anObj.final_kind() == IdlType.e_forward_interface )
            {
                if ( ( ( IdlInterface ) anObj.final_object() ).getInterface() == null )
                {
                    m_parser.show_error( "It is not legal to inherit from a forward interface which is not defined : " + name );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                    m_parser.stopAt( m_parser.StopList );
                    break;
                }
            }

            if ( obj.abstract_interface() )
            {
                if ( ( ( IdlInterface ) anObj.final_object() ).abstract_interface() == false )
                {
                    m_parser.show_error( "An abstract interface can only inherit from an abstract interface : " + name );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                    m_parser.stopAt( m_parser.StopList );
                    break;
                }
            }

            obj.addInheritance( anObj.final_object() );

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;

            m_parser.symbole();
        }
    }

    // -----------------
    // < INTERFACE DCL >
    // -----------------
    /**
     * (5)
     * Analyse la declaration d'une interface
     */
    public void interface_dcl( IdlInterface obj )
    {
        if ( m_parser.ctx.symb == Token.t_deux_points )
            inheritance_spec( obj );

        if ( m_parser.ctx.symb != Token.t_acc_ouverte )
        {
            m_parser.show_error( "'{' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
            m_parser.stopAt( m_parser.StopList );
        }
        else
        {
            m_parser.symbole();
            interface_body( obj );
        }

        if ( m_parser.ctx.symb != Token.t_acc_fermee )
        {
            m_parser.show_error( "'}' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }
    }

    // -----------
    // < FORWARD >
    // -----------
    /**
     * (6)
     * Analyse une definition forward d'une interface
     */
    public void forward_dcl( IdlInterface obj )
    {
        obj.forward();
    }

    // -------------
    // < INTERFACE >
    // -------------
    /**
     * (4)
     * Analyse la definition d'une interface
     */
    public void interface_type( IdlObject obj )
    {
        IdlInterface interface_obj;
        IdlInterface anItf = null;
        IdlObject anObj;

        // Allready
        // 0 non declaree, non forwardee
        // 1 forwardee
        // 2 declaree
        int allready = 0;

        interface_obj = new IdlInterface( obj );

        interface_obj.attach_comment();

        m_parser.container = interface_obj;

        if ( m_parser.ctx.symb == Token.t_abstract )
        {
            interface_obj.abstract_interface( true );
            m_parser.symbole();
        }

        if ( m_parser.ctx.symb == Token.t_local )
        {
            if ( interface_obj.abstract_interface() )
                m_parser.show_error( "An abstract interface cannot be mark as 'local'" );

            interface_obj.local_interface( true );

            m_parser.symbole();
        }

        if ( m_parser.ctx.symb != Token.t_interface )
            m_parser.show_error( "'interface' expected" );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
            m_parser.show_error( "Identifier expected for interface" );
        else
        {
            interface_obj.name( m_parser.ctx.value );

            if ( obj.isVisible( m_parser.ctx.value, true ) == true )
            {
                anObj = obj.returnVisibleObject( m_parser.ctx.value, true );

                if ( ( anObj.kind() != IdlType.e_interface ) &&
                        ( anObj.kind() != IdlType.e_forward_interface ) )
                {
                    m_parser.show_error( "Identifier already used : " + m_parser.ctx.value );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_deux_points ) );
                    m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                    m_parser.stopAt( m_parser.StopList );
                }
                else
                {
                    anItf = ( IdlInterface ) anObj;

                    if ( anItf.isForward() == true )
                        allready = 1;
                    else
                        allready = 2;
                }
            }
        }

        obj.addIdlObject( interface_obj );

        m_parser.symbole();

        switch ( m_parser.ctx.symb )
        {

        case Token.t_point_virgule :

            if ( allready == 1 )
            {
                m_parser.show_error( "This interface is already declared : " + interface_obj.name() );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }

            forward_dcl( interface_obj );
            m_parser.ctx.one = m_parser.ctx.symb;
            m_parser.container = obj;
            break;

        case Token.t_acc_ouverte :

        case Token.t_deux_points :

            if ( allready == 1 )
            {
                if ( anItf.abstract_interface() != interface_obj.abstract_interface() )
                {
                    m_parser.show_error( "This interface is not defined as specified in forward declaraton : " + interface_obj.name() );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                    m_parser.stopAt( m_parser.StopList );
                    m_parser.container = obj;
                    return;
                }
                else
                    interface_obj.defined( anItf );
            }

            if ( allready == 2 )
            {
                m_parser.show_error( "This interface is already declared : " + interface_obj.name() );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.stopAt( m_parser.StopList );
                m_parser.container = obj;
                return;
            }

            interface_dcl( interface_obj );

            m_parser.container = obj;
            break;

        default :
            m_parser.show_error( "':' or  '{' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
            m_parser.stopAt( m_parser.StopList );
            break;
        }
    }

    // ----------
    // < NATIVE >
    // ----------
    /**
     * Analyse la definition d'un type natif
     */
    public void native_dcl( IdlObject obj )
    {
        IdlNative native_obj;

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected after 'native'" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        if ( obj.isVisible( m_parser.ctx.value, false ) == true )
        {
            m_parser.show_error( "Identifier is already used : " + m_parser.ctx.value );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        native_obj = new IdlNative( obj );

        native_obj.attach_comment();

        native_obj.name( m_parser.ctx.value );

        obj.addIdlObject( native_obj );
    }

    // ---------------------
    // < VALUE FORWARD DCL >
    // ---------------------
    public void value_forward_dcl( IdlObject obj, String name, boolean abstract_value )
    {
        IdlValue value = new IdlValue( obj );

        if ( obj.isVisible( name, false ) )
        {
            m_parser.show_error( "Identifier already used" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }

        value.abstract_value( abstract_value );
        value.name( name );
        value.forward( true );

        obj.addIdlObject( value );

        m_parser.ctx.one = m_parser.ctx.symb;
        m_parser.ctx.symb = Token.t_point_virgule;
    }

    // -----------------
    // < VALUE BOX DCL >
    // -----------------
    public void value_box_dcl( IdlObject obj, String name )
    {

        if ( obj.isVisible( name, false ) )
        {
            m_parser.show_error( "Identifier already used" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }

        IdlValueBox value = new IdlValueBox( obj );

        value.attach_comment();

        value.name( name );

        type_spec( value );

        obj.addIdlObject( value );

        m_parser.ctx.one = m_parser.ctx.symb;
        m_parser.ctx.symb = Token.t_point_virgule;
    }

    // ---------------------
    // < VALUE INHERITANCE >
    // ---------------------
    public void value_inheritance( IdlObject obj, boolean custom, boolean statefull )
    {
        String vname = null;
        boolean trunc;
        boolean first = true;
        IdlObject inh = null;

        while ( true )
        {
            m_parser.symbole();

            IdlValueInheritance inheritance = new IdlValueInheritance( obj );

            if ( m_parser.ctx.symb == Token.t_truncatable )
            {
                if ( statefull == false )
                {
                    m_parser.show_error( "'truncatable' cannot be used for an abstract valuetype" );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                    m_parser.stopAt( m_parser.StopList );
                    return;
                }

                if ( custom )
                {
                    m_parser.show_error( "'truncatable' cannot be used if a valuetype is specified with 'custom'" );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                    m_parser.stopAt( m_parser.StopList );
                    return;
                }

                trunc = true;
                m_parser.symbole();
            }
            else
                trunc = false;

            vname = scoped_name();

            if ( obj.isVisible( vname, false ) )
            {
                IdlObject target = obj.returnVisibleObject( vname, false );
                inheritance.truncatable_member( trunc );

                if ( target.final_kind() == IdlType.e_forward_value )
                {
                    if ( ( ( IdlValue ) target ).definedValue() == null )
                    {
                        m_parser.show_error( "Unable to inherit from a forward valuetype which has not been yet defined" );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                        m_parser.stopAt( m_parser.StopList );
                        return;
                    }
                    else
                        inh = ( ( IdlValue ) target.final_object() ).definedValue();
                }
                else
                    inh = target.final_object();

                if ( inh.kind() != IdlType.e_value )
                {
                    m_parser.show_error( "Unable to inherit from a non-valuetype : " + vname );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                    m_parser.stopAt( m_parser.StopList );
                    return;
                }

                if ( !custom )
                {
                    if ( ( ( IdlValue ) inh ).custom_value() )
                    {
                        m_parser.show_error( "A non-custom valuetype cannot inherit from a custom value type" );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                        m_parser.stopAt( m_parser.StopList );
                        return;
                    }
                }

                if ( ( first ) && ( trunc ) )
                {
                    if ( ( ( IdlValue ) inh ).abstract_value() )
                    {
                        m_parser.show_error( "An abstract valuetype cannot be specified with 'truncatable'" );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                        m_parser.stopAt( m_parser.StopList );
                        return;
                    }
                }

                if ( statefull == false )
                {
                    if ( ( ( IdlValue ) inh ).abstract_value() == false )
                    {
                        m_parser.show_error( "An abstract valuetype may only inherit from abstract valuetype" );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                        m_parser.stopAt( m_parser.StopList );
                        return;
                    }
                }

                if ( !first )
                {
                    if ( ( ( IdlValue ) inh ).abstract_value() == false )
                    {
                        m_parser.show_error( "The second and others inheritances must be abstract valuetype" );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_acc_ouverte ) );
                        m_parser.stopAt( m_parser.StopList );
                        return;
                    }
                }

                inheritance.addIdlObject( inh );

                ( ( IdlValue ) obj ).addInheritance( inheritance );

                if ( first )
                    first = false;
            }
            else
            {
                m_parser.show_error( "Undefined identifier : " + vname );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }

            //parser.symbole();

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;
        }
    }

    // --------------------------
    // < VALUE INHERITANCE SPEC >
    // --------------------------
    public void value_inheritance_spec( IdlObject obj, boolean custom, boolean statefull )
    {
        java.util.Vector support_list = new java.util.Vector();


        if ( m_parser.ctx.symb == Token.t_deux_points )
            value_inheritance( obj, custom, statefull );

        if ( m_parser.ctx.symb == Token.t_supports )
        {
            while ( true )
            {
                m_parser.symbole();

                String supports_name = scoped_name();

                if ( obj.isVisible( supports_name , false ) )
                {
                    IdlObject target = obj.returnVisibleObject( supports_name, false );

                    if ( target.kind() == IdlType.e_forward_interface )
                    {
                        if ( ( ( IdlInterface ) target ).getInterface() == null )
                        {
                            m_parser.show_error( "Unable to inherit from an interface which has not been yet defined : " + m_parser.ctx.value );
                            m_parser.StopList.removeAllElements();
                            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                            m_parser.stopAt( m_parser.StopList );
                            return;
                        }
                        else
                            target = ( ( IdlInterface ) target ).getInterface();
                    }

                    if ( target.kind() != IdlType.e_interface )
                    {
                        m_parser.show_error( "This identifier is not an interface : " + m_parser.ctx.value );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                        m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                        m_parser.stopAt( m_parser.StopList );
                        return;
                    }
                    else
                        support_list.addElement( target );
                }
                else
                {
                    m_parser.show_error( "Undefined interface : " + m_parser.ctx.value );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                    m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                    m_parser.stopAt( m_parser.StopList );
                }

                if ( m_parser.ctx.symb != Token.t_virgule )
                    break;
            }

            int nb_non_abstract = 0;

            for ( int i = 0; i < support_list.size(); i++ )
            {
                if ( ( ( IdlInterface ) ( support_list.elementAt( i ) ) ).abstract_interface() == false )
                {
                    nb_non_abstract++;
                }
            }

            if ( nb_non_abstract > 1 )
            {
                m_parser.show_error( "A valuetype can only support one non abstract interface" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }


            ( ( IdlValue ) obj ).supports( support_list );

        }
    }

    // -----------------
    // < VALUE ABS DCL >
    // -----------------
    public void value_abs_dcl( IdlObject obj, String name )
    {
        IdlValue value = new IdlValue( obj );

        value.attach_comment();

        if ( obj.isVisible( name, false ) )
        {
            IdlObject target = obj.returnVisibleObject( name, false );

            if ( target.kind() == IdlType.e_value )
            {
                m_parser.show_error( "This valuetype is already defined : " + m_parser.ctx.value );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }
            else
                if ( target.kind() != IdlType.e_forward_value )
                {
                    m_parser.show_error( "This identifier is not a forward valuetype : " + m_parser.ctx.value );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                    m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                    m_parser.stopAt( m_parser.StopList );
                }
                else
                {
                    if ( ( ( IdlValue ) target ).abstract_value() == false )
                    {
                        m_parser.show_error( "The previous definition of this identifier was not for an abstract valuetype : " + m_parser.ctx.value );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                        m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                        m_parser.stopAt( m_parser.StopList );
                    }
                    else
                    {
                        if ( ( ( IdlValue ) target ).definedValue() != null )
                        {
                            m_parser.show_error( "Abstract valuetype already defined : " + m_parser.ctx.value );
                            m_parser.StopList.removeAllElements();
                            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                            m_parser.stopAt( m_parser.StopList );
                        }
                        else
                            ( ( IdlValue ) target ).definedValue( value );
                    }
                }
        }

        value.name( name );

        value.abstract_value( true );

        value_inheritance_spec( value, false, false );

        obj.addIdlObject( value );

        m_parser.symbole();

        while ( true )
        {
            if ( m_parser.ctx.symb == Token.t_acc_fermee )
                break;

            export_dcl( value );

            if ( m_parser.ctx.symb != Token.t_point_virgule )
            {
                m_parser.show_error( "';' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }

            m_parser.symbole();
        }

    }

    // --------------------
    // < STATE MEMBER DCL >
    // --------------------
    public void state_member_dcl( IdlObject obj )
    {
        IdlStateMember member = new IdlStateMember( obj );

        member.attach_comment();

        switch ( m_parser.ctx.symb )
        {

        case Token.t_public :
            member.public_member( true );
            break;

        case Token.t_private :
            member.public_member( false );
            break;
        }

        m_parser.symbole();

        type_spec( member );

        while ( true )
        {
            IdlStateMember member_obj = new IdlStateMember( obj );
            member_obj.public_member( member.public_member() );
            member_obj.type( member.type() );

            member_obj.attach_comment( member.getComment() );

            declarators( member_obj );

            obj.addIdlObject( member_obj );

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;

            m_parser.symbole();
        }
    }

    // --------------------
    // < INIT PARAM DECLS >
    // --------------------
    public void init_param_decls( IdlObject obj )
    {
        while ( true )
        {
            m_parser.symbole();

            IdlFactoryMember member = new IdlFactoryMember( obj );

            if ( m_parser.ctx.symb != Token.t_in )
            {
                m_parser.show_error( "'in' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }

            m_parser.symbole();

            param_type_spec( member );

            if ( m_parser.ctx.symb != Token.t_ident )
            {
                m_parser.show_error( "Identifier expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }

            member.name( m_parser.ctx.value );

            obj.addIdlObject( member );

            m_parser.symbole();

            if ( m_parser.ctx.symb != Token.t_virgule )
                break;
        }
    }

    // ------------
    // < INIT DCL >
    // ------------
    public void init_dcl( IdlObject obj )
    {
        IdlFactory init = new IdlFactory( obj );

        init.attach_comment();

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected after 'factory'" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        init.name( m_parser.ctx.value );

        m_parser.symbole();

        init_param_decls( init );

        if ( m_parser.ctx.symb != Token.t_par_fermee )
        {
            m_parser.show_error( "')' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_point_virgule )
        {
            m_parser.show_error( "';' expected" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
        }

        obj.addIdlObject( init );
    }

    // -------------
    // < VALUE DCL >
    // -------------
    public void value_dcl( IdlObject obj, String name, boolean custom )
    {

        IdlValue value = new IdlValue( obj );

        value.attach_comment();

        m_parser.container = value;

        if ( obj.isVisible( name, false ) )
        {
            IdlObject target = obj.returnVisibleObject( name, false );

            if ( target.kind() == IdlType.e_value )
            {
                m_parser.show_error( "This valuetype is already defined : " + m_parser.ctx.value );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }
            else
                if ( target.kind() != IdlType.e_forward_value )
                {
                    m_parser.show_error( "This identifier is not a forward valuetype : " + m_parser.ctx.value );
                    m_parser.StopList.removeAllElements();
                    m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                    m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                    m_parser.stopAt( m_parser.StopList );
                }
                else
                {
                    if ( ( ( IdlValue ) target ).abstract_value() )
                    {
                        m_parser.show_error( "The previous definition of this identifier was for an abstract valuetype : " + m_parser.ctx.value );
                        m_parser.StopList.removeAllElements();
                        m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                        m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                        m_parser.stopAt( m_parser.StopList );
                    }
                    else
                    {
                        if ( ( ( IdlValue ) target ).definedValue() != null )
                        {
                            m_parser.show_error( "Valuetype already defined : " + m_parser.ctx.value );
                            m_parser.StopList.removeAllElements();
                            m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                            m_parser.stopAt( m_parser.StopList );
                        }
                        else
                            ( ( IdlValue ) target ).definedValue( value );
                    }
                }
        }

        value.name( name );
        value.custom_value( custom );

        value_inheritance_spec( value, custom, true );

        obj.addIdlObject( value );

        m_parser.symbole();

        while ( true )
        {
            if ( m_parser.ctx.symb == Token.t_acc_fermee )
                break;

            switch ( m_parser.ctx.symb )
            {

            case Token.t_private :

            case Token.t_public :
                state_member_dcl( value );
                break;

            case Token.t_factory :
                init_dcl( value );
                break;

            default :
                export_dcl( value );
                break;
            }

            if ( m_parser.ctx.symb != Token.t_point_virgule )
            {
                m_parser.show_error( "';' expected" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_acc_fermee ) );
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
            }

            m_parser.symbole();
        }

        m_parser.container = obj;
    }

    // ------------------
    // < FULL VALUE DCL >
    // ------------------
    public void full_value_dcl( IdlObject obj )
    {
        boolean abstract_value = false;
        boolean custom = false;

        String value_name;

        if ( m_parser.ctx.symb == Token.t_abstract )
        {
            abstract_value = true;
            m_parser.symbole();
        }

        if ( m_parser.ctx.symb == Token.t_custom )
        {
            if ( abstract_value )
            {
                m_parser.show_error( "A 'abstract' value cannot be 'custom'" );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
                m_parser.stopAt( m_parser.StopList );
                return;
            }
            else
            {
                custom = true;
                m_parser.symbole();
            }
        }

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected after 'valuetype'" );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        value_name = m_parser.ctx.value;

        m_parser.symbole();

        switch ( m_parser.ctx.symb )
        {

        case Token.t_point_virgule :
            value_forward_dcl( obj, value_name, abstract_value );
            return;

        case Token.t_supports :

        case Token.t_deux_points :

        case Token.t_acc_ouverte :

            if ( abstract_value )
                value_abs_dcl( obj, value_name );
            else
                value_dcl( obj, value_name, custom );

            return;

        default :
            value_box_dcl( obj, value_name );

            return;
        }


    }

    // -------------
    // < CHANGE ID >
    // -------------
    /**
     * Change l'ID d'un objet CORBA
     */
    public void changeId( IdlObject current )
    {
        IdlObject obj;

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected after #pragma ID " );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }


        if ( current.isVisible( m_parser.ctx.value, false ) == false )
        {
            m_parser.show_error( "Undefined Identifier : " + m_parser.ctx.value );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        obj = current.returnVisibleObject( m_parser.ctx.value, false );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_chaine )
        {
            m_parser.show_error( "ID definition expected after #pragma " + obj.name() );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        if ( obj.kind() == IdlType.e_forward_interface )
        {
            IdlInterface itf = ( IdlInterface ) obj;

            if ( itf.isForward() == true )
            {
                itf = itf.getInterface();

                if ( itf != null )
                    obj = itf;
            }
        }

        obj.setId( m_parser.ctx.value );
    }

    // ------------------
    // < CHANGE VERSION >
    // ------------------
    /**
     * Change la version d'un ID d'un objet CORBA
     */
    public void changeVersion( IdlObject current )
    {
        IdlObject obj;
        String id;
        String newId;

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected after #pragma VERSION " );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }


        if ( current.isVisible( m_parser.ctx.value, false ) == false )
        {
            m_parser.show_error( "Undefined identifier : " + m_parser.ctx.value );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        obj = current.returnVisibleObject( m_parser.ctx.value, false );

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_real )
        {
            m_parser.show_error( "Bad version number for  : #pragma " + obj.name() );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        id = obj.getId();

        int index = id.lastIndexOf( ':' );

        if ( index != -1 )
            id = new String( id.substring( 0, index + 1 ) );
        else
            id = id + ":";

        newId = new String( id + m_parser.ctx.value );

        if ( obj.kind() == IdlType.e_forward_interface )
        {
            IdlInterface itf = ( IdlInterface ) obj;

            if ( itf.isForward() == true )
            {
                itf = itf.getInterface();

                if ( itf != null )
                {
                    obj.setId( newId );
                    obj = itf;
                }
            }
        }

        obj.setId( newId );
    }

    // -----------------
    // < CHANGE PREFIX >
    // -----------------
    /**
     * Change le prefixe des IDs des objets CORBA
     */
    public void changePrefix()
    {
        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_chaine )
        {
            m_parser.show_error( "String expected after #pragma PREFIX " );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        if ( m_parser.ctx.value.equals( "" ) )
            m_parser.setM_idlPrefix( null );
        else
            if ( checkPrefix( m_parser.ctx.value ) )
                m_parser.setM_idlPrefix( new String( m_parser.ctx.value ) );
    }

    // ----------------
    // < CHECK PREFIX >
    // ----------------

    /**
     * Check the pragma prefix string.
     * Any occurence of the ':' and the '/' character is illegal.
     *
     * From CORBA 2.4.2, 10.6.1
     * The second component is a list of identifiers, separated by '/' characters. These
     * identifiers are arbitrarily long sequences of alphabetic, digit, underscore ('_'),
     * hyphen ('-'), and period ('.') characters. Typically, the first identifier is a unique
     * prefix, and the rest are the OMG IDL Identifiers that make up the scoped name of
     * the definition.
     * Added by Olivier Modica, 4/11/2001
     */
    public boolean checkPrefix( java.lang.String pref )
    {
        for ( int i = 0; i < pref.length(); i++ )
        {

            char c = pref.charAt( i );

            if ( ( !java.lang.Character.isLetterOrDigit( c ) ) &&
                    ( c != '_' ) &&
                    ( c != '-' ) &&
                    ( c != '.' ) )
            {
                m_parser.show_error( "The character '" + c + "' is not allowed in a pragma prefix." );
                m_parser.StopList.removeAllElements();
                m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
                m_parser.stopAt( m_parser.StopList );
                return false;
            }

        }

        return true;
    }

    // ---------
    // < ECHO  >
    // ---------
    /**
     * Change le prefixe des IDs des objets CORBA
     */
    public void echo()
    {
        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_chaine )
        {
            m_parser.show_error( "String expected after #pragma echo " );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        System.out.println( m_parser.ctx.value );
    }

    // ------------------
    // < CHANGE PACKAGE >
    // ------------------
    /**
     * Change le prefixe des IDs des objets CORBA
     */
    public void changePackage()
    {
        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_chaine )
        {
            m_parser.show_error( "String expected after #pragma javaPackage " );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_cp.setM_use_package(true);
        m_cp.setM_packageName(new String( m_parser.ctx.value ));
        //IdlObject.idlPrefix =
    }

    /**
     * Respond to #pragma javaNativeMap
     */
    public void addNativeMap( IdlObject current )
    {
        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Identifier expected after #pragma javaNativeMap " );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        if ( current.isVisible( m_parser.ctx.value, false ) == false )
        {
            m_parser.show_error( "Undefined identifier : " + m_parser.ctx.value );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        IdlObject obj = current.returnVisibleObject( m_parser.ctx.value, false );

        if ( ! ( obj instanceof IdlNative ) )
        {
            m_parser.show_error( "Attempted to apply native mapping to " + m_parser.ctx.value
                           + " which is not of native type." );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        String nativeMap = m_parser.ctx.value;

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_chaine )
        {
            m_parser.show_error( "String expected after #pragma javaNativeMap " + nativeMap );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        m_cp.getM_nativeDefinition().addElement( nativeMap + ":" + m_parser.ctx.value );
    }

    // --------------
    // < PRAGMA DCL >
    // --------------
    /**
     * Analyse les clauses pragma
     */
    public void pragma_dcl( IdlObject obj )
    {
        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_ident )
        {
            m_parser.show_error( "Undefined pragma option : " + m_parser.ctx.value );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
        }
        else
        {
            if ( m_parser.ctx.value.toUpperCase().equals( "ID" ) == true )
                changeId( obj );
            else
                if ( m_parser.ctx.value.toUpperCase().equals( "VERSION" ) == true )
                    changeVersion( obj );
                else
                    if ( m_parser.ctx.value.toUpperCase().equals( "PREFIX" ) == true )
                        changePrefix();
                    else
                        if ( m_parser.ctx.value.toUpperCase().equals( "JAVAPACKAGE" ) == true )
                            changePackage();
                        else
                            if ( m_parser.ctx.value.toUpperCase().equals( "ECHO" ) == true )
                                echo();
                            else
                                if ( m_parser.ctx.value.toUpperCase().equals( "JAVANATIVEMAP" ) == true )
                                    addNativeMap( obj );
                                else
                                {
                                    //JWL - ES added if statement
                                    if(m_parser.ctx.value.toUpperCase().equals( "KEYLIST" ) != true){
                                        m_parser.warning( "Unknown pragma directive : " + m_parser.ctx.value );
                                    }//else ignore
                                    //JWL
				    //m_parser.StopList.removeAllElements();
                                    //m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
                                    //m_parser.stopAt( m_parser.StopList );
				    m_parser.skipLine();
				    //!JWL
                                }
        }

        m_parser.ctx.one = Token.t_point_virgule;
    }

    /**
     * Set a repository ID
     */
    public void type_id_dcl( IdlObject obj )
    {
        m_parser.symbole();

        String ident = scoped_name();

        if ( obj.isVisible( ident, false ) == false )
        {
            m_parser.show_error( "Unknown identifier : " + ident );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        IdlObject target = obj.returnVisibleObject( ident, false );

        if ( m_parser.ctx.symb != Token.t_chaine )
        {
            m_parser.show_error( "ID definition expected after typeId " + ident );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        if ( target.kind() == IdlType.e_forward_interface )
        {
            IdlInterface itf = ( IdlInterface ) target;

            if ( itf.isForward() == true )
            {
                itf = itf.getInterface();

                if ( itf != null )
                    target = itf;
            }
        }

        target.setId( m_parser.ctx.value );
    }

    /**
     * Set repository IDs prefix
     */
    public void type_prefix_dcl( IdlObject obj )
    {
        m_parser.symbole();

        String ident = scoped_name();

        if ( obj.isVisible( ident, false ) == false )
        {
            m_parser.show_error( "Unknown identifier : " + ident );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_point_virgule ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        IdlObject target = obj.returnVisibleObject( ident, false );

        if ( m_parser.ctx.symb != Token.t_chaine )
        {
            m_parser.show_error( "ID definition expected after typeId " + ident );
            m_parser.StopList.removeAllElements();
            m_parser.StopList.addElement( new Integer( Token.t_fin_ligne ) );
            m_parser.stopAt( m_parser.StopList );
            return;
        }

        if ( target.kind() == IdlType.e_forward_interface )
        {
            IdlInterface itf = ( IdlInterface ) target;

            if ( itf.isForward() == true )
            {
                itf = itf.getInterface();

                if ( itf != null )
                    target = itf;
            }
        }

        if ( target.equals( obj ) )
            m_parser.setM_idlPrefix( m_parser.ctx.value );

        target.applyPrefix( m_parser.ctx.value );
    }

    // --------------
    // < DEFINITION >
    // --------------
    /**
     * (1)
     * Analyse une definition IDL
     * <definition> ::= <type_dcl> ';'
     *     |  <const_dcl> ';'
     *        | <except_dcl> ';'
     *     | <interface_dcl> ';'
     *     |  <module_dcl> ';'
     */
    public void idl_definition( IdlObject obj )
    {
        verbose( "<idl_definition> : " + m_parser.ctx.symb );

        switch ( m_parser.ctx.symb )
        {

        case Token.t_interface :
            interface_type( obj );
            break;

        case Token.t_module :
            module_dcl( obj );
            break;

        case Token.t_const :
            const_dcl( obj );
            break;

        case Token.t_exception :
            except_dcl( obj );
            break;

        case Token.t_struct :
            struct_dcl( obj );
            break;

        case Token.t_union :
            union_dcl( obj );
            break;

        case Token.t_enum :
            enum_dcl( obj );
            break;

        case Token.t_typedef :
            type_dcl( obj );
            break;

        case Token.t_pragma :
            pragma_dcl( obj );
            break;

        case Token.t_native :
            native_dcl( obj );
            break;

        case Token.t_local :
            m_parser.symbole();

            if ( m_parser.ctx.symb != Token.t_interface )
                m_parser.show_error( "'interface' key word expected after 'local'" );
            else
            {
                m_parser.ctx.one = m_parser.ctx.symb;
                m_parser.ctx.symb = Token.t_local;
                interface_type( obj );
            }

            break;

        case Token.t_abstract :
            m_parser.symbole();

            switch ( m_parser.ctx.symb )
            {

            case Token.t_valuetype :
                m_parser.ctx.one = m_parser.ctx.symb;
                m_parser.ctx.symb = Token.t_abstract;
                full_value_dcl( obj );
                break;

            case Token.t_interface :
                m_parser.ctx.one = m_parser.ctx.symb;
                m_parser.ctx.symb = Token.t_abstract;
                interface_type( obj );
                break;

            default :
                m_parser.show_error( "Bad definition after 'abstract'" );
                break;
            }

            break;

        case Token.t_custom :

        case Token.t_valuetype :
            full_value_dcl( obj );
            break;

        case Token.t_typeId :
            type_id_dcl( obj );
            break;

        case Token.t_typePrefix :
            type_prefix_dcl( obj );
            break;

        case Token.t_fin_fichier :
            return;

        default :
            m_parser.show_error( "Definition expected" );
            break;
        }

        m_parser.symbole();

        if ( m_parser.ctx.symb != Token.t_point_virgule )
            m_parser.show_error( "';' expected" );

        m_parser.symbole();
    }

    // Next symbole for a block
    private void block_symbole( IdlObject obj )
    {
        verbose( "<block_symbole>" );

        m_parser.symbole();

        if ( m_parser.ctx.symb == Token.t_pragma )
        {
            pragma_dcl( obj );
            m_parser.symbole();
            block_symbole( obj );
        }
    }

    // ------------------
    // IMPORT DECLARATION
    // ------------------
    /**
     * Include external description
     */
    public void import_dcl( IdlObject obj )
    {
        verbose( "<import_dcl>" );

        String name = null;

        while ( true )
        {
            if ( m_parser.ctx.symb == Token.t_import )
            {
                m_parser.symbole();

                name = scoped_name();

                obj.addIdlObject( new IdlImport( m_cp, obj, name ) );

                //irImport.getDescriptionFromIR( name, obj );

                if ( m_parser.ctx.symb != Token.t_point_virgule )
                    m_parser.show_error( "';' expected" );

                m_parser.symbole();
            }
            else
                break;
        }
    }

    // -----------------
    // IDL SPECIFICATION
    // -----------------
    /**
     * Premiere regle de la grammaire
     *
     * @param        obj   objet a partir duquel la compilation s'effectue
     */
    public void idl_specification( IdlObject obj )
    {
        verbose( "<idl_speficication>" );

        try
        {
            m_parser.symbole();

            while ( true )
            {
                if ( m_parser.ctx.symb == Token.t_import )
                    import_dcl( obj );

                idl_definition( obj );

                if ( m_parser.ctx.symb == Token.t_fin_fichier )
                    break;
            }
        }
        catch ( java.lang.Exception ex )
        {
            if ( m_cp.getM_verbose() )
            {
                System.out.println( "--------------------" );
                ex.printStackTrace();
                System.out.println( "--------------------" );
                System.out.println( "" );
            }

            m_parser.show_error(ex.getMessage());
        }
    }

    /**
     * Display a verbose message
     */
    private void verbose( String msg )
    {
        if ( m_cp.getM_verbose() )
            System.out.println( msg );
    }
}
