/*
* Copyright (C) The Community OpenORB Project. All rights reserved.
*
* This software is published under the terms of The OpenORB Community Software
* License version 1.0, a copy of which has been included with this distribution
* in the LICENSE.txt file.
*/

package org.openorb.compiler.parser;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.io.PushbackReader;
import java.io.Reader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Hashtable;
import java.util.Vector;

import org.openorb.compiler.CompileListEntry;
import org.openorb.compiler.CompilerProperties;
import org.openorb.compiler.object.IdlComment;
import org.openorb.compiler.object.IdlCommentField;
import org.openorb.compiler.object.IdlCommentSection;
import org.openorb.compiler.object.IdlInclude;
import org.openorb.compiler.object.IdlModule;
import org.openorb.compiler.object.IdlObject;
import org.openorb.compiler.object.IdlRoot;
import org.openorb.compiler.object.IdlSimple;

/**
 * The IDL parser
 *
 * @author Jerome Daniel
 * @version $Revision: 1.4 $ $Date: 2008-08-12 10:13:29 $
 */

public class IdlParser
{
    // ---------
    // Attributes
    // ---------

    /**
    * Compilation graph root node
    */
    public IdlObject root = null;

    /**
     * Reference to the current container ( default is root )
     */
    protected IdlObject container = null;

    /**
     * Error output
     */
    protected static java.io.PrintStream error_output = java.lang.System.out;

    /**
     * File extension for parsing input
     */
    public String [] file_extension = { ".idl" };

    /**
    * Total errors count
    */
    private int totalError = 0;

    /**
    * Current compilation context
    */
    public CompilationContext ctx  = null;

    /**
    * Stop symbols when error
    */
    public Vector StopList = new Vector();

    /**
    * Including depth
    */
    protected int include_level = 0;

    /**
     * IDL Comment
     */
    protected IdlComment idl_comment = null;

    /**
    * Compilation contexts list (when including)
    */
    public Vector compilationList = new Vector();

    /**
    * Included IDL files list
    */
    public Vector idlIncluded = new Vector();

    /**
    * Indicates the conditional (if) compilation state.
    */
    public int pass = 0;

    /**
    * Should stop compitlation on else.
    */
    public int stop_else = 0;

    /**
    * Macro expansion is turned on, read_word expands macros.
    */
    public boolean macros_enabled = true;

    /**
    * Maximum number of errors before compilation is abandoned.
    */
    private static final int MAX_MACRO = 512;

    /**
    * Maximum number of errors before compilation is abandoned.
    */
    private static final int MAX_ERRORS = 100;

    private ErrorHandler handler = null;

    private CompilerProperties m_cp = null;

    /**
     * Maps #defined macros to their expansions
     */
    public Hashtable m_definedMacros = new Hashtable();

    /**
     * Prefix of the IDL object.
     */
    private String m_idlPrefix = null;

    /* a list of all readers used to open files */
    public Vector usedReaders = new Vector();


    // ------------
    // Constructor
    // ------------
    /**
    * Creates a parser
    */
    public IdlParser( CompilerProperties cp )
    {
        root = new IdlRoot( cp, this );
        IdlModule module = new IdlModule( root );
        module.name( "CORBA" );
        module._prefix = "omg.org";
        root.addIdlObject( module );
        IdlSimple.initIdlSimple( this );
        module.addIdlObject( IdlSimple.typecode_type );
        IdlSimple.typecode_type.name( "TypeCode" );
        IdlSimple.typecode_type._upper = module;
        module._map = true;
        IdlSimple.typecode_type._map = true;
        m_cp = cp;
        m_definedMacros = (Hashtable) cp.getM_macros().clone();
    }

    public void setErrorOutput( PrintStream pw )
    {
        error_output = pw;
    }

    public void setHandler(ErrorHandler handler)
    {
        this.handler = handler;
    }

    public void setMainPrefix( String prefix )
    {
        root.root()._mainPrefix = prefix;
    }

    // -------------------
    // ERRORS MANAGEMENT
    // -------------------
    /**
    * Prints an error
    *
    * @param msg  error message
    */
    public void show_error( String msg )
    {
        ctx.nberrors++;
        totalError++;
        if(handler != null)
        {
            handler.error(ctx.name, ctx.line, msg);
        } else
        {
            error_output.println( ctx.name + ":" + ctx.line + ": " + msg );
        }

        if ( ctx.nberrors > MAX_ERRORS )
        {
            if(handler != null)
            {
                handler.maxErrorsExceeded(ctx.name, MAX_ERRORS);
            }
            else
            {
                error_output.println( "Compilation stopped : too many errors" );
            }
            throw new org.openorb.compiler.parser.CompilationException();
        }
    }

    /**
     * Returns the properties associated with this compiler
     * @return CompilerProperties
	 */
    public CompilerProperties getCompilerProperties()
    {
        return m_cp;
    }

    // --------------------
    // WARNINGS
    // --------------------
    /**
    * Print a warning
    *
    * @param msg  warning text
    */
    public void warning( String msg )
    {
        if(handler != null)
        {
            handler.error(ctx.name, ctx.line, msg);
        }
        else
        {
            error_output.println( ctx.name + ":" + ctx.line + ": warning: " + msg );
        }
    }

    public int getTotalErrors()
    {
        return totalError;
    }

    // -------------------------
    // RESERVED WORDS
    // -------------------------

    /**
    * Cette methode compare un mot avec les identificateurs de la liste
    * des mots reserves et retourne VRAI dans le cas ou celui-ci est
    * trouve
    */
    public boolean is_reserved_word()
    {
        int i = 0;
        SymboleDef s;

        while ( i < Symbole.liste_mots_reserves.size() )
        {
            s = ( SymboleDef ) Symbole.liste_mots_reserves.elementAt( i );

            if ( s.symbole_name.equals( ctx.value ) )
                return true;

            i++;

        }

        return false;
    }

    /**
    * Cette methode compare un mot avec les identificateurs de la liste
    * des mots reserves java et retourne VRAI dans le cas ou celui-ci est
    * trouve
    */
    public boolean is_java_reserved_word()
    {
        int i = 0;
        SymboleDef s;

        while ( i < SymboleJava.liste_mots_reserves.size() )
        {
            s = ( SymboleDef ) SymboleJava.liste_mots_reserves.elementAt( i );

            if ( s.symbole_name.equals( ctx.value ) )
                return true;

            i++;

        }

        return false;
    }

    /**
    * Retourne un token de mot reserve dans le cas de l'identificateur
    * trouve dans la liste des mots reserves
    */
    public void to_reserved_word()
    {
        int i = 0;
        SymboleDef s;

        while ( i < Symbole.liste_mots_reserves.size() )
        {
            s = ( SymboleDef ) Symbole.liste_mots_reserves.elementAt( i );

            if ( s.symbole_name.equals( ctx.value ) )
            {
                ctx.symb = s.symbole_token;
                return;
            }

            i++;
        }
    }

    // -----------------------
    // NEW COMPILATION CONTEXT
    // -----------------------
    /**
    * Cette methode cree un nouveau contexte de compilation.
    * Un tel contexte est obligatoire pour compiler un fichier.
    *
    * @return retourne un nouveau contexte de compilation
    * @see  CompilationContext
    */
    public CompilationContext new_compilation_context()
    {
        CompilationContext context = new CompilationContext();

        context.nberrors = 0;
        context.nbwarning = 0;
        context.line = 1;
        context.symb = Token.t_none;

        return context;
    }

    // -------------
    // OPEN IDL FILE
    // -------------
    /**
     * Open an IDL file
     *
     * @param file_name  file name
     */
    public void open_idl_file( String source_name ) throws FileNotFoundException
    {
        if ( m_cp.getM_verbose() )
        {
            System.out.println( "Trying to open idl file '" + source_name + "'..." );
        }

        boolean ready = false;
        for ( int i = 0; i < file_extension.length; i++ )
        {
            if ( source_name.endsWith( file_extension[ i ] ) )
            {
                ready = true;
                break;
            }
        }

        if ( !ready )
        {
            source_name = source_name + file_extension[ 0 ];
        }

        Reader is;
        URL fileURL;
        try
        {
            File file = new File( source_name );
            if ( file.isAbsolute() && file.exists() )
            {
                // check for absolute files
                fileURL = file.toURL();
                is = openURL( fileURL );
            }
            else
            {
                // discover the base URL. This will be the URL of the including file
                // or the current working directory otherwise.
                URL baseURL;
                if ( include_level == 0 )
                {
                    try
                    {
                        baseURL = new File( "" ).getCanonicalFile().toURL();
                    }
                    catch ( IOException ex )
                    {
                        // This should be impossible.
                        ex.printStackTrace();
                        throw new RuntimeException( "Impossible exception occured: " + ex );
                    }
                }
                else
                {
                    baseURL = ctx.sourceURL;
                }

                fileURL = new URL( baseURL, source_name );

                if ( m_cp.getM_verbose() )
                {
                    System.out.println( "fileURL=" + fileURL );
                    System.out.println( "baseURL=" + baseURL );
                    System.out.println( "Trying to open fileURL and looking for include folders..." );
                }

                // Try to open the file in the current working directory
                is = openURL( fileURL );
                URL incURL = null;
                for ( int i = 0; is == null && i < m_cp.getM_includeList().size(); i++ )
                {
                    Object incObj = m_cp.getM_includeList().elementAt( i );
                    if ( incObj != null )
                    {
                        if ( m_cp.getM_verbose() )
                        {
                            System.out.println( "    Include: " + incObj );
                        }

                        if ( incObj instanceof String )
                        {
                            try
                            {
                                incURL = new URL( ( String ) incObj );
                            }
                            catch ( MalformedURLException ex )
                            {
                                try
                                {
                                    incURL = new File( ( String ) incObj ).toURL();
                                }
                                catch ( MalformedURLException ex1 )
                                {
                                    m_cp.getM_includeList().removeElementAt( i-- );
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            incURL = ( URL ) incObj;
                        }

                        if ( !incURL.equals( baseURL ) )
                        {
                            try
                            {
                                fileURL = new URL( incURL, source_name );
                                is = openURL( fileURL );
                            }
                            catch ( Exception ex )
                            {}
                        }

                        // when open via standard file system wasn't successful try to find it in a jar
                        if ( is == null && incObj instanceof String )
                        {
                            is = openResource( ( ( String ) incObj ) + "/" + source_name );
                        }
                    }
                }
            }
        }
        catch ( MalformedURLException ex )
        {
            // This should be impossible.
            throw new FileNotFoundException("Failed to open idl file: " + source_name + " (" + ex + ")" );
        }

        if ( is == null )
        {
            throw new FileNotFoundException("Failed to open idl file: " + source_name);
        }

        ctx = new_compilation_context();
        ctx.prefix = null;
        ctx.is = new PushbackReader( new BufferedReader( is ), MAX_MACRO );
        usedReaders.add(ctx.is);
        ctx.sourceURL = fileURL;
        ctx.name = fileURL.toString();
        setM_idlPrefix( null );

        if ( m_cp.getM_verbose() )
            System.out.println( "Successfully opened idl file '" + source_name + "'." );
    }

    /**
     * open a URL. returns null if fail.
     */
    private Reader openURL( URL url )
    {
        if ( m_cp.getM_verbose() )
            System.out.println( "Trying to open url '" + url + "'..." );

        try
        {
            java.net.URLConnection conn = url.openConnection();
            conn.connect();

            if ( conn instanceof java.net.HttpURLConnection )
            {
                java.net.HttpURLConnection hConn = ( java.net.HttpURLConnection ) url.openConnection();

                if ( hConn.getResponseCode() != HttpURLConnection.HTTP_OK )
                    return null;
            }

            if ( m_cp.getM_verbose() )
                System.out.println( "Successfully opened url '" + url + "'." );
            InputStreamReader tmpInputStreamReader = new InputStreamReader( conn.getInputStream() );
            usedReaders.add(tmpInputStreamReader);
            return tmpInputStreamReader;
        }
        catch ( IOException ex )
        {
            //System.out.println( "URL '" + url + "' not found! (" + ex + ")" );
        }
        if ( m_cp.getM_verbose() )
            System.out.println( "Failed to open url '" + url + "'." );
        return null;
    }

    /**
     * Try to open the specified resource from
     */
    private Reader openResource( String url )
    {
        if ( m_cp.getM_verbose() )
            System.out.println( "Trying to open resource url '" + url + "'..." );

        // try to find the URL in a current classpath element
        ClassLoader clzldr = this.getClass().getClassLoader();
        if ( clzldr instanceof java.net.URLClassLoader )
        {
            java.net.URLClassLoader urlclzldr = ( java.net.URLClassLoader ) clzldr;
            if ( m_cp.getM_verbose() )
            {
                URL urls[] = urlclzldr.getURLs();
                System.out.println( "Searching in " + urls.length + " URLs:" );
                for ( int i = 0; i < urls.length; i++ )
                System.out.println( " -> " + urls[ i ] );
            }

            // The path must be relative here
            URL jarurl = null;
            try
            {
                jarurl = urlclzldr.findResource( url );
                if ( jarurl != null )
                {
                    java.net.URLConnection conn = jarurl.openConnection();
                    conn.connect();
                    if ( m_cp.getM_verbose() )
                        System.out.println( "Successfully opened resource url '" + url + "'." );
                    InputStreamReader tmpInputStreamReader = new InputStreamReader( conn.getInputStream() );
                    usedReaders.add(tmpInputStreamReader);
                    return tmpInputStreamReader;
                }
            }
            catch ( IOException ex )
            {
                //System.out.println( "URL '" + jarurl + "' not found! (" + ex + ")" );
            }
        }
        if ( m_cp.getM_verbose() )
            System.out.println( "Failed to open resource url '" + url + "'." );
        return null;
    }

    /**
     * This operation is used to test if an included file is also to compile.
     */
    private boolean isAlsoToCompile( String name )
    {
        boolean found = false;

        for ( int i = 0; i < file_extension.length; i++ )
        {
            if ( name.endsWith( file_extension[ i ] ) )
                found = true;
        }

        if ( !found )
            name = name + file_extension[ 0 ];

        return m_cp.getM_compileList().contains( new CompileListEntry( name ) );

    }

    // -----------
    // INCLUDE IDL
    // -----------
    /**
    * Cette methode permet la compilation d'un fichier inclus
    */
    public void include_idl()
    {
        int err;
        String tmp;

        boolean old_me = macros_enabled;
        macros_enabled = false;
        symbole();
        macros_enabled = old_me;

        switch ( ctx.symb )
        {

        case Token.t_inf :
            tmp = "";
            scan();

            while ( ctx.car != '>' )
            {
                tmp = tmp + ctx.car;

                if ( ( ctx.car == Token.t_fin_ligne ) || ( ctx.car == Token.t_fin_fichier ) )
                {
                    show_error( "IDL file name expected" );
                    StopList.removeAllElements();
                    StopList.addElement( new Integer( Token.t_module ) );
                    StopList.addElement( new Integer( Token.t_interface ) );
                    StopList.addElement( new Integer( Token.t_typedef ) );
                    StopList.addElement( new Integer( Token.t_union ) );
                    StopList.addElement( new Integer( Token.t_struct ) );
                    StopList.addElement( new Integer( Token.t_enum ) );
                    StopList.addElement( new Integer( Token.t_exception ) );
                    StopList.addElement( new Integer( Token.t_const ) );
                    stopAt( StopList );
                    return;
                }

                scan();
            }

            break;

        case Token.t_chaine :
            tmp = ctx.value;
            break;

        default :
            show_error( "IDL file name to include expected" );
            StopList.removeAllElements();
            StopList.addElement( new Integer( Token.t_module ) );
            StopList.addElement( new Integer( Token.t_interface ) );
            StopList.addElement( new Integer( Token.t_typedef ) );
            StopList.addElement( new Integer( Token.t_union ) );
            StopList.addElement( new Integer( Token.t_struct ) );
            StopList.addElement( new Integer( Token.t_enum ) );
            StopList.addElement( new Integer( Token.t_exception ) );
            StopList.addElement( new Integer( Token.t_const ) );
            stopAt( StopList );
            return;
        }

        ctx.prefix = getM_idlPrefix();
        compilationList.addElement( ctx );
        boolean included = false;

        if ( !isAlsoToCompile( tmp ) && ( !m_cp.getM_map_all() ) )
        {
            included = true;
            include_level++;
        }

        if ( m_cp.getM_verbose() )
            System.out.println( "Include IDL: " + tmp );

        try
        {
            compile_idl( tmp , container );
        }
        catch (FileNotFoundException e)
        {
            throw new CompilationException("Impossible to include IDL " + tmp);
        }

        root.addIdlObject( new IdlInclude( root, tmp ) );

        if ( include_level == 1 )
            idlIncluded.addElement( ctx.name );

        if ( included )
            include_level--;

        err = ctx.nberrors;

        ctx = ( CompilationContext ) compilationList.lastElement();

        compilationList.setSize( compilationList.size() - 1 );

        ctx.nberrors += err;

        setM_idlPrefix( ctx.prefix );

        symbole();
    }

    /**
    * This method allows the compilation of an included file
    */
    public void include_idl_file( String file_name )
    {
        int err;
        ctx.prefix = getM_idlPrefix();

        if ( ctx.sourceURL == null )
        {
            try
            {
                ctx.sourceURL = new File( "" ).getCanonicalFile().toURL();
            }
            catch ( IOException ex )
            {
                // This should be impossible.
                ex.printStackTrace();
                throw new RuntimeException( "Impossible exception occured: " + ex );
            }
        }

        compilationList.addElement( ctx );

        if ( !m_cp.getM_map_all() )
            include_level++;

        if ( m_cp.getM_verbose() )
            System.out.println( "Include IDL file: " + file_name );

        try
        {
            compile_idl( file_name, container );
        }
        catch (FileNotFoundException e)
        {
            throw new CompilationException("Impossible to include IDL file:" + file_name);
        }

        root.addIdlObject( new IdlInclude( root, file_name ) );

        if ( include_level == 1 )
            idlIncluded.addElement( ctx.name );

        if ( !m_cp.getM_map_all() )
            include_level--;

        err = ctx.nberrors;

        ctx = ( CompilationContext ) compilationList.lastElement();

        compilationList.setSize( compilationList.size() - 1 );

        ctx.nberrors += err;

        setM_idlPrefix( ctx.prefix );
    }

    public void precompiler_error()
    {
        scan();

        StringBuffer buf = new StringBuffer();

        while ( ctx.car != ( char ) Token.t_fin_ligne )
        {
            if ( ctx.car == '\\' )
            {
                scan();

                if ( ctx.car == ( char ) Token.t_fin_ligne )
                    buf.append( ' ' );
                else
                    buf.append( '\\' );

                continue;
            }

            buf.append( ctx.car );
            scan();
        }

        show_error( "#error" + buf );
        symbole();
    }

    public void precompiler_warning()
    {
        StringBuffer buf = new StringBuffer();

        scan();

        while ( ctx.car != ( char ) Token.t_fin_ligne )
        {
            if ( ctx.car == '\\' )
            {
                scan();

                if ( ctx.car == ( char ) Token.t_fin_ligne )
                    buf.append( ' ' );
                else
                    buf.append( '\\' );

                continue;
            }

            buf.append( ctx.car );
            scan();
        }

        warning( "#warning" + buf );
        symbole();
    }

    // --------------
    // DEFINE SYMBOLE
    // --------------
    /**
     * Cette methode permet de definir un symbole pour
     * la compilation conditionnelle.
     */
    public void define_symbole()
    {
        boolean old_me = macros_enabled;
        macros_enabled = false;
        symbole();
        macros_enabled = old_me;

        if ( ctx.symb != Token.t_ident )
        {
            show_error( "Identifier expected after #define" );
            StopList.removeAllElements();
            StopList.addElement( new Integer( Token.t_fin_ligne ) );
            stopAt( StopList );
        }
        else
        {
            if ( m_definedMacros.containsKey( ctx.value ) )
                warning( "\'" + ctx.value + "\' redefined" );

            // things to look out for which are not supported:
            // recursivly defined macros eg:
            // #define MACRO MACRO noodle
            // should expand to 'MACRO noodle' rather than infinite regress
            // function macros:
            // #define MACRO(arg) call(arg)
            // tokenize operator:
            // #define MACRO one ## two

            scan();

            if ( ctx.car == '(' )
                show_error( "Tried to define function macro \'" + ctx.value + "\'. Function macros are not implemented." );

            StringBuffer buf = new StringBuffer();

            buf.append( ' ' );

            boolean skipWS = true;

            while ( ctx.car != ( char ) Token.t_fin_ligne )
            {
                switch ( ctx.car )
                {

                case ' ':

                case '\t':

                    if ( !skipWS )
                    {
                        skipWS = true;
                        buf.append( ' ' );
                    }

                    break;

                case '\\':
                    scan();

                    if ( ctx.car == ( char ) Token.t_fin_ligne )
                    {
                        if ( !skipWS )
                        {
                            skipWS = true;
                            buf.append( ' ' );
                        }
                    }
                    else
                    {
                        buf.append( '\\' );
                        skipWS = false;
                        continue;
                    }

                    break;

                case '#':
                    scan();

                    if ( ctx.car == '#' )
                    {
                        // skip back over scanned whitespace
                        if ( buf.charAt( buf.length() - 1 ) == ' ' )
                            buf.setLength( buf.length() - 1 );

                        skipWS = true;
                    }
                    else
                    {
                        buf.append( '#' );
                        skipWS = false;
                        continue;
                    }

                    break;

                default:
                    buf.append( ctx.car );
                    skipWS = false;
                }

                scan();
            }

            buf.append( ' ' );

            if ( buf.length() > MAX_MACRO )
                show_error( "macro length exceeds system limits" );

            m_definedMacros.put( ctx.value, buf.toString() );

            symbole();
        }
    }

    // ----------------
    // UNDEFINE SYMBOLE
    // ----------------
    /**
     * Cette methode permet de supprimer un symbole precedemment
     * defini pour la compilation conditionnelle.
     */
    public void undefine_symbole()
    {
        boolean old_me = macros_enabled;
        macros_enabled = false;
        symbole();
        macros_enabled = old_me;

        if ( ctx.symb != Token.t_ident )
        {
            show_error( "Identifier expected after #undef" );
            StopList.removeAllElements();
            StopList.addElement( new Integer( Token.t_fin_ligne ) );
            stopAt( StopList );
        }
        else
        {
            if ( m_definedMacros.remove( ctx.value ) != null )
                warning( "\'" + ctx.value + "\' was not defined." );

            symbole();
        }
    }

    // --------------
    // IFNDEF SYMBOLE
    // --------------
    /**
    * Cette methode permet la gestion de la compilation conditionnelle
    */
    public void ifndef_symbole()
    {
        int state;
        int count;

        boolean old_me = macros_enabled;
        macros_enabled = false;
        symbole();
        macros_enabled = old_me;

        if ( ctx.symb != Token.t_ident )
        {
            show_error( "Identifier expected after #ifndef" );
            StopList.removeAllElements();
            StopList.addElement( new Integer( Token.t_fin_ligne ) );
            stopAt( StopList );
        }
        else
        {
            if ( m_definedMacros.containsKey( ctx.value ) )
            {
                // On a trouver le symbole, il faut donc passer toutes
                // les instructions jusqu'au prochaine #else ou #endif
                // en eliminant tous les #ifndef ... #endif

                state = 1;
                count = 1;
                pass = 2;

                while ( state != 0 )
                {
                    symbole();

                    switch ( ctx.symb )
                    {

                    case Token.t_endif :

                        switch ( state )
                        {

                        case 1 :

                        case 2 :
                            state = 2;
                            count--;
                            break;

                        case 3 :
                            state = 2;
                            break;
                        }

                        if ( count == 0 )
                            pass = 0;

                        break;

                    case Token.t_else :
                        switch ( state )
                        {

                        case 1 :

                        case 2 :
                            state = 3;
                            count--;
                            break;
                        }

                        if ( count == 0 )
                            pass = 0;

                        break;

                    case Token.t_ifdef :

                    case Token.t_ifndef :
                        switch ( state )
                        {

                        case 1 :

                        case 2 :
                            state = 1;
                            count++;
                            break;

                        case 3 :
                            state = 1;
                            count += 2;
                            break;
                        }

                        if ( count == 0 )
                            pass = 0;

                        break;
                    }

                    if ( count == 0 )
                        state = 0;
                }
            }
            else
            {
                pass = 0;

                if ( stop_else == 0 )
                    stop_else = 1;
            }

            symbole();
        }
    }

    // -------------
    // IFDEF SYMBOLE
    // -------------
    /**
    * Cette methode permet la gestion de la compilation conditionnelle
    */
    public void ifdef_symbole()
    {
        int state;
        int count;

        boolean old_me = macros_enabled;
        macros_enabled = false;
        symbole();
        macros_enabled = old_me;

        if ( ctx.symb != Token.t_ident )
        {
            show_error( "Identifier expected after #ifdef" );
            StopList.removeAllElements();
            StopList.addElement( new Integer( Token.t_fin_ligne ) );
            stopAt( StopList );
        }
        else
        {
            if ( !m_definedMacros.containsKey( ctx.value ) )
            {
                // On a trouver le symbole, il faut donc passer toutes
                // les instructions jusqu'au prochaine #else ou #endif
                // en eliminant tous les #ifndef ... #endif

                state = 1;
                count = 1;
                pass = 2;

                while ( state != 0 )
                {
                    symbole();

                    switch ( ctx.symb )
                    {

                    case Token.t_endif :

                        switch ( state )
                        {

                        case 1 :

                        case 2 :
                            state = 2;
                            count--;
                            break;

                        case 3 :
                            state = 2;
                            break;
                        }

                        if ( count == 0 )
                            pass = 0;

                        break;

                    case Token.t_else :
                        switch ( state )
                        {

                        case 1 :

                        case 2 :
                            state = 3;
                            count--;
                            break;
                        }

                        if ( count == 0 )
                            pass = 0;

                        break;

                    case Token.t_ifdef :

                    case Token.t_ifndef :
                        switch ( state )
                        {

                        case 1 :

                        case 2 :
                            state = 1;
                            count++;
                            break;

                        case 3 :
                            state = 1;
                            count += 2;
                            break;
                        }

                        if ( count == 0 )
                            pass = 0;

                        break;
                    }

                    if ( count == 0 )
                        state = 0;
                }
            }
            else
            {
                pass = 0;

                if ( stop_else == 0 )
                    stop_else = 1;
            }

            symbole();
        }
    }

    // ----------------
    // READ DESCRIPTION
    // ----------------
    /**
     * This function extracts from an IDL comment a section description.
     */
    public boolean read_description()
    {
        boolean not_end = false;
        boolean end = false;

        java.util.Vector description = new java.util.Vector();

        while ( !not_end )
        {
            scan();

            switch ( ctx.car )
            {

            case '@':
                not_end = true;
                break;

            case '*':
                scan();

                if ( ctx.car == '/' )
                {
                    not_end = true;
                    end = true;
                }
                else
                {
                    while ( true )
                    {
                        if ( ctx.car != ' ' )
                        {
                            unscan();
                            break;
                        }

                        scan();
                    }
                }

                break;

            default :

                if ( ctx.car == Token.t_fin_ligne )
                {
                    while ( true )
                    {
                        scan();

                        if ( ctx.car == Token.t_fin_fichier )
                        {
                            show_error( "IDL comment incorrect." );
                            throw new RuntimeException( "IDL comment incorrect!" );
                        }

                        if ( ctx.car == '*' )
                        {
                            unscan();
                            break;
                        }
                    }

                    description.addElement( new Character( '\n' ) );
                }
                else
                    description.addElement( new Character( ctx.car ) );

                break;
            }
        }

        if ( description.size() != 0 )
        {
            char [] text = new char[ description.size() ];

            for ( int i = 0; i < description.size(); i++ )
                text[ i ] = ( ( Character ) description.elementAt( i ) ).charValue();

            ctx.value = new String( text );
        }

        return end;
    }

    // -------------
    // SCAN COMMENTS
    // -------------
    /**
     * This method scan rich comment like java doc
     */
    public void scan_comment()
    {
        boolean not_end = false;
        boolean end = false;
        boolean begin = false;
        java.util.Vector comment = new java.util.Vector();
        IdlCommentSection new_section = null;

        idl_comment = new IdlComment();

        while ( true )
        {
            scan();

            if ( ctx.car == '*' )
            {
                unscan();
                break;
            }
        }

        while ( !not_end )
        {


            switch ( ctx.car )
            {

            case '*' :
                // It may be the comment end
                scan();

                if ( ctx.car == '/' )
                    not_end = true;

                while ( true )
                {
                    if ( ctx.car != ' ' )
                        break;

                    scan();
                }

                break;

            case '@' :
                scan();

                if ( macros_enabled )
                {
                    macros_enabled = false;
                    read_word();
                    macros_enabled = true;
                }
                else
                    read_word();

                begin = true;

                if ( ctx.value.equals( "exception" ) )
                    new_section = new IdlCommentSection( IdlCommentField.exception_field );
                else
                    if ( ctx.value.equals( "author" ) )
                        new_section = new IdlCommentSection( IdlCommentField.author_field );
                    else
                        if ( ctx.value.equals( "version" ) )
                            new_section = new IdlCommentSection( IdlCommentField.version_field );
                        else
                            if ( ctx.value.equals( "param" ) )
                                new_section = new IdlCommentSection( IdlCommentField.param_field );
                            else
                                if ( ctx.value.equals( "return" ) )
                                    new_section = new IdlCommentSection( IdlCommentField.return_field );
                                else
                                    if ( ctx.value.equals( "deprecated" ) )
                                        new_section = new IdlCommentSection( IdlCommentField.deprecated_field );
                                    else
                                        if ( ctx.value.equals( "see" ) )
                                            new_section = new IdlCommentSection( IdlCommentField.see_field );
                                        else
                                        {
                                            new_section = new IdlCommentSection( IdlCommentField.unknown_field );
                                            new_section.set_title( ctx.value );
                                        }

                end = read_description();
                new_section.add_description( ctx.value );
                idl_comment.add_section( new_section );

                if ( end )
                    not_end = true;

                break;

            default :
                if ( ctx.car == Token.t_fin_ligne )
                {
                    while ( true )
                    {

                        scan();

                        if ( ctx.car == '*' )
                        {
                            unscan();
                            break;
                        }
                    }

                    if ( begin == false )
                        comment.addElement( new Character( '\n' ) );
                }
                else
                    if ( begin == false )
                    {
                        comment.addElement( new Character( ctx.car ) );

                    }

                scan();
                break;
            }
        }


        if ( comment.size() != 0 )
        {
            char [] text = new char[ comment.size() ];

            for ( int i = 0; i < comment.size(); i++ )
                text[ i ] = ( ( Character ) comment.elementAt( i ) ).charValue();

            idl_comment.add_description( new String( text ) );
        }

        // CORECTION BUG 2 ---- NICOLAS ---- 30/07/99
        else
            idl_comment = null;

        // END CORRECTION
    }

    // ----
    // SCAN
    // ----
    /**
    * Cette methode scan un fichier
    */
    public void scan()
    {
        try
        {
            int c = ctx.is.read();

            if ( c < 0 )
            {
                ctx.car = ( char ) Token.t_fin_fichier;
            }
            else if ( c == 13 )
                scan();
            else if ( c == 10 )
            {
                ctx.line++;
                ctx.car = ( char ) Token.t_fin_ligne;
            }
            else
                ctx.car = ( char ) c;
        }
        catch ( IOException ex )
        {
            show_error( "IOException occoured" );
        }
    }

    public void unscan()
    {
        if ( ctx.car == ( char ) Token.t_fin_ligne )
        {
            ctx.car = ( char ) 10;
            ctx.line--;
        }

        try
        {
            ctx.is.unread( ( int ) ctx.car );
        }
        catch ( IOException ex )
        {
            show_error( "IOException occoured" );
        }
    }

    // ----------
    // PRE PARSER
    // ----------
    /**
    * Cette methode effectue une tache de pre-compilation
    */
    public void pre_parse()
    {
        int stop;
        int state;
        int count;
        int two = 1;

        scan();

        ctx.symb = Token.t_none;

        switch ( ctx.car )
        {

        case Token.t_fin_fichier :
            ctx.symb = Token.t_fin_fichier;
            scan();
            break;

        case ' ' :

        case '\t' :

        case Token.t_fin_ligne :
            pre_parse();
            break;

        case '#' :
            scan();
            // skip over whitespace
            while ( ctx.car == ' ' || ctx.car == '\t' )
                scan();

            if ( macros_enabled )
            {
                macros_enabled = false;
                read_word();
                macros_enabled = true;
            }
            else
                read_word();

            if ( ctx.value.toUpperCase().equals( "ERROR" ) )
            {
                if ( pass == 0 )
                    precompiler_error();

                break;
            }
            else if ( ctx.value.toUpperCase().equals( "WARNING" ) )
            {
                if ( pass == 0 )
                    precompiler_warning();

                break;
            }

            to_reserved_word();

            switch ( ctx.symb )
            {

            case Token.t_define :

                if ( pass == 0 )
                    define_symbole();

                break;

            case Token.t_ifdef :
                if ( pass == 0 )
                    ifdef_symbole();

                break;

            case Token.t_undef :
                if ( pass == 0 )
                    undefine_symbole();

                break;

            case Token.t_ifndef :
                if ( pass == 0 )
                    ifndef_symbole();

                break;

            case Token.t_pragma :
                break;

            case Token.t_include :
                if ( pass == 0 )
                    include_idl();

                break;

            case Token.t_else :
                if ( ( stop_else == 1 ) && ( pass == 0 ) )
                {
                    state = 1;
                    count = 1;
                    stop_else = -1;
                    pass = 2;

                    while ( state != 0 )
                    {
                        symbole();

                        switch ( ctx.symb )
                        {

                        case Token.t_ifndef :

                            switch ( state )
                            {

                            case 1 :

                            case 2 :

                            case 3 :
                                state = 3;
                                count++;
                                break;
                            }

                            break;

                        case Token.t_endif :

                            switch ( state )
                            {

                            case 1 :

                            case 2 :

                            case 3 :
                                state = 2;
                                count--;
                                break;
                            }

                            break;

                        case Token.t_else :

                            switch ( state )
                            {

                            case 2 :

                            case 3 :
                                state = 1;
                                break;
                            }

                            break;
                        }

                        if ( count == 0 )
                            state = 0;
                    }

                    stop_else = 0;
                    pass = 0;
                }

                if ( pass == 0 )
                {
                    symbole();
                    pass = 0;
                }

                break;

            case Token.t_endif :

                if ( pass == 0 )
                {
                    symbole();
                    pass = 0;
                }

                break;

            default :
                warning( "Compilation flag unknown : " + ctx.value );
                break;
            }

            break;

        case '/' :
            scan();

            if ( ctx.car == '/' )
            {
                while ( true )
                {
                    if ( ( ctx.car == Token.t_fin_ligne ) || ( ctx.car == Token.t_fin_fichier ) )
                        break;

                    scan();
                }

                pre_parse();
            }
            else
                if ( ctx.car == '*' )
                {
                    stop = 1;

                    while ( stop != 0 )
                    {
                        scan();

                        if ( ctx.car == Token.t_fin_fichier )
                        {
                            show_error( "Comment end missing '*/'" );
                            StopList.removeAllElements();
                            StopList.addElement( new Integer( Token.t_fin_ligne ) );
                            stopAt( StopList );
                        }

                        if ( ctx.car == '*' )
                        {
                            two++;
                            scan();

                            if ( ctx.car == '/' )
                                stop = 0;
                            else
                                unscan();
                        }

                        if ( ( two == 2 ) && ( ctx.car == Token.t_fin_ligne ) )
                        {
                            unscan();
                            // scan(); BUG J21#20
                            scan_comment();
                            break;
                        }
                    }

                    pre_parse();
                }
                else
                    ctx.symb = Token.t_div;

            break;
        }
    }

    // ---------
    // READ WORD
    // ---------
    /**
    * Cette methode permet d'extraire un mot d'un flux d'entree
    */
    public void read_word()
    {
        StringBuffer tmp = new StringBuffer();

        while ( Character.isLetterOrDigit( ctx.car ) || ( ctx.car == '_' ) )
        {
            tmp.append( ctx.car );

            scan();

            if ( ctx.car == Token.t_fin_fichier )
                break;

            if ( ctx.car == Token.t_fin_ligne )
                break;
        }

        ctx.value = tmp.toString();

        unscan();

        if ( macros_enabled )
        {
            String macro_exp = ( String ) m_definedMacros.get( ctx.value );

            if ( macro_exp != null )
            {
                try
                {
                    ctx.is.unread( macro_exp.toCharArray() );
                }
                catch ( IOException ex )
                {
                    show_error( "IOException while expanding macro" );
                }

                ctx.value = null;
            }
        }
    }

    // -----------
    // READ NUMBER
    // -----------
    /**
    * Cette methode permet de lire un nombre dans un flux d'entree
    */
    public void read_number()
    {
        int base = 0; // 0 = decimal, 1 = hexadecimal
        int type = 0; // 0 = integer, 1 = real
        int stop = 1;
        int index = 0;
        char[] tmp = new char[ 255 ];

        while ( stop != 0 )
        {
            tmp[ index++ ] = ctx.car;
            scan();

            if ( ctx.car == '.' )
            {
                if ( type == 0 )
                    type = 1;
                else
                    show_error( "Bad number value" );
            }
            else
                if ( ctx.car == 'x' || ctx.car == 'X' )
                {
                    if ( base == 0 )
                        base = 1;
                    else
                        show_error( "Bad number value" );
                }
                else
                    if ( ctx.car == '-' )
                    {
                        show_error( "Token '-' misplaced" );
                        stop = 0;
                    }
                    else
                    {
                        if ( Character.isLetter( ctx.car ) )
                        {
                            if ( ( ctx.car == ' ' ) || ( ctx.car == Token.t_fin_ligne ) || ( ctx.car == Token.t_fin_fichier ) )
                            {
                                stop = 0;
                                unscan();
                            }
                            else
                                if ( base != 1 )
                                {
                                    if ( ctx.car == 'D' || ctx.car == 'd' )
                                        stop = 0;
                                    else
                                    {
                                        System.out.println( "[" + ( int ) ctx.car + "]" );
                                        unscan();
                                        show_error( "Bad hexadecimal value" );
                                        stop = 0;
                                    }
                                }
                        }
                        else
                            if ( !Character.isDigit( ctx.car ) )
                            {
                                unscan();
                                stop = 0;
                            }
                    }
        }

        tmp[ index ] = 0;

        if ( type == 0 )
            ctx.symb = Token.t_integer;
        else
            ctx.symb = Token.t_real;

        ctx.value = new String( tmp, 0, index );
        ctx.base = base;
    }

    // ------
    // PARSER
    // ------
    /**
     * This method parses an IDL file.
     */
    public void parse()
    {
        pre_parse();

        if ( ctx.symb == Token.t_none )
        {
            switch ( ctx.car )
            {

            case '&' :
                ctx.symb = Token.t_and;
                break;

            case '|' :
                ctx.symb = Token.t_or;
                break;

            case '+' :
                ctx.symb = Token.t_plus;
                break;

            case '-' :
                ctx.symb = Token.t_moins;
                break;

            case '=' :
                ctx.symb = Token.t_egal;
                break;

            case '*' :
                ctx.symb = Token.t_mul;
                break;

            case '{' :
                ctx.symb = Token.t_acc_ouverte;
                break;

            case '}' :
                ctx.symb = Token.t_acc_fermee;
                break;

            case '[' :
                ctx.symb = Token.t_cro_ouvert;
                break;

            case ']' :
                ctx.symb = Token.t_cro_ferme;
                break;

            case '.' :
                ctx.symb = Token.t_point;
                break;

            case '~' :
                ctx.symb = Token.t_tilde;
                break;

            case '<' :
                scan();
                if(ctx.car == '<'){
                    ctx.symb = Token.t_lshift;
                } else{
                    ctx.symb = Token.t_inf;
                    unscan();
                }

                break;
            case '>' :
                scan();
                if(ctx.car == Token.t_sup){
                    ctx.symb = Token.t_rshift;
                } else{
                    ctx.symb = Token.t_sup;
                    unscan();
                }
                break;
            case '%' :
                ctx.symb = Token.t_mod;
                break;

            case '(' :
                ctx.symb = Token.t_par_ouverte;
                break;

            case ')' :
                ctx.symb = Token.t_par_fermee;
                break;

            case ';' :
                ctx.symb = Token.t_point_virgule;
                break;

            case ',' :
                ctx.symb = Token.t_virgule;
                break;

            case ':' :
                scan();

                if ( ctx.car == ':' )
                    ctx.symb = Token.t_quatre_pts;
                else
                {
                    ctx.symb = Token.t_deux_points;
                    unscan();
                }

                break;

            case '\"' :

            case '\'':
                {
                    char stop = ctx.car;
                    StringBuffer tab = new StringBuffer();
                    scan();

                    while ( ctx.car != stop )
                    {
                        if ( ctx.car == Token.t_fin_fichier )
                        {
                            show_error( "End of file reached while scanning string" );
                            break;
                        }

                        if ( ctx.car == '\\' )
                        {
                            scan();

                            if ( ctx.car == Token.t_fin_ligne )
                                continue;

                            tab.append( '\\' );
                        }

                        tab.append( ctx.car );
                        scan();
                    }

                    ctx.value = tab.toString();

                    if ( stop == '\"' )
                        ctx.symb = Token.t_chaine;
                    else
                        ctx.symb = Token.t_caractere;
                }

                break;

            default :

                if ( Character.isDigit( ctx.car ) )
                    read_number();
                else
                    if ( Character.isLetter( ctx.car ) || ( ctx.car == '_' ) )
                    {
                        read_word();
                        // if the word expanded to a macro ctx.value will be null
                        if ( ctx.value == null )
                            parse();
                        else if ( is_reserved_word() )
                            to_reserved_word();
                        else
                        {
                            ctx.symb = Token.t_ident;

                            if ( is_java_reserved_word() )
                                ctx.value = "__" + ctx.value;
                        }
                    }
                    else
                    {
                        show_error( "Undefined character : " + ctx.car );
                    }

                break;
            }
        }
    }

    // -------
    // SYMBOLE
    // -------
    /**
    * Cette methode extrait les symboles de la grammaire IDL
    */
    public void symbole()
    {
        if ( ctx.one != Token.t_none )
        {
            ctx.symb = ctx.one;
            ctx.one = Token.t_none;
            return;
        }

        parse();

        switch ( ctx.symb )
        {

        case Token.t_moins :
            parse();

            if ( ( ctx.symb == Token.t_integer ) || ( ctx.symb == Token.t_real ) )
                ctx.value = "-" + ctx.value;
            else
            {
                ctx.one = ctx.symb;
                ctx.symb = Token.t_moins;
            }

            break;

        case Token.t_long :
            parse();

            switch ( ctx.symb )
            {

            case Token.t_long :
                ctx.symb = Token.t_longlong;
                break;

            case Token.t_double :
                ctx.symb = Token.t_longdouble;
                break;

            default :
                ctx.one = ctx.symb;
                ctx.symb = Token.t_long;
                break;
            }

            break;

        case Token.t_unsigned :
            parse();

            switch ( ctx.symb )
            {

            case Token.t_long :
                parse();

                if ( ctx.symb == Token.t_long )
                    ctx.symb = Token.t_ulonglong;
                else
                {
                    ctx.one = ctx.symb;
                    ctx.symb = Token.t_ulong;
                }

                break;

            case Token.t_short :
                ctx.symb = Token.t_ushort;
                break;

            default :
                show_error( "Unexpected combination type with unsigned" );
                break;
            }

            break;
        }
    }

    // -------
    // STOP AT
    // -------
    /**
    * Cette methode deroule un fichier IDL jusqu'a lecture d'un des
    * elements specifies
    *
    * @param list Liste des elements d'arret
    */
    public void stopAt( Vector list )
    {
        int i;
        Integer val;

        while ( true )
        {
            if ( ctx.symb == Token.t_fin_fichier )
                break;

            for ( i = 0; i < list.size(); i++ )
            {
                val = ( Integer ) ( list.elementAt( i ) );

                if ( val.intValue() == ctx.symb )
                    return;
            }

            symbole();
        }
    }

    public void skipLine() // JWL
    {
	while (ctx.car != Token.t_fin_fichier && ctx.car != 10) {
	    //System.out.println("skipLine: " + ((int)ctx.car) + " = '" + ctx.car + "'=" + ctx.symb);
	    symbole();
	}
    } // !JWL

    // -----------
    // COMPILE IDL
    // -----------
    /**
     * Start the compilation of an IDL file
     *
     * @return compilation graph root
     * @param file_name IDL file name
     * @see  IdlObject
     */
    public IdlObject compile_idl( String source_name, IdlObject current_root ) throws FileNotFoundException
    {
        IdlGrammar grammar = new IdlGrammar( this );

        open_idl_file( source_name );

        grammar.idl_specification( current_root );

        return current_root;
    }

    /**
    * Start the compilation of an IDL file
    *
    * @return compilation graph root
    * @param file_name IDL file name
    * @see  IdlObject
    */
    public IdlObject compile_idl( String source_name ) throws FileNotFoundException
    {

        IdlGrammar grammar = new IdlGrammar( this );

        container = root;
        open_idl_file( source_name );
        grammar.idl_specification( root );
        return root;
    }
    /**
     * Returns the include_level.
     * @return int
     */
    public int getInclude_level()
    {
        return include_level;
    }

    /**
     * Sets the include_level.
     * @param include_level The include_level to set
     */
    public void setInclude_level(int include_level)
    {
        this.include_level = include_level;
    }

    /**
     * Returns the idl_comment.
     * @return IdlComment
     */
    public IdlComment getIdl_comment()
    {
        return idl_comment;
    }

    /**
     * Sets the idl_comment.
     * @param idl_comment The idl_comment to set
     */
    public void setIdl_comment(IdlComment idl_comment)
    {
        this.idl_comment = idl_comment;
    }

    /**
     * Returns the m_idlPrefix.
     * @return String
     */
    public String getM_idlPrefix()
    {
        return m_idlPrefix;
    }

    /**
     * Sets the m_idlPrefix.
     * @param m_idlPrefix The m_idlPrefix to set
     */
    public void setM_idlPrefix(String m_idlPrefix)
    {
        this.m_idlPrefix = m_idlPrefix;
    }

}
