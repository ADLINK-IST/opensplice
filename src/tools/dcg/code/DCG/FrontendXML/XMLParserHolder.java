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
package DCG.FrontendXML;

import DCG.Core.ParserHolderable;
//import org.apache.xerces.parsers.DOMParser;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import java.io.IOException;
import org.xml.sax.ErrorHandler;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;

/**
 * The XMLParserHolder class is the holder class for the external XML parser
 * defined in the External Xerces DOM XML parser layer. This class is needed
 * because the parserHolderable class needs to be implemented to allow the XML
 * parser to be compatible with the Core layer without making changes to the
 * existing XML parser. The ParserHolderable class must be implemented to allow
 * multiple frontends to be used by the Core without giving the core knowledge of
 * the exact implementation of a parser.
 */
public class XMLParserHolder implements ParserHolderable, ErrorHandler {

    /**
     * Default constructor
     * @roseuid 4074152801FE
     */
	public static boolean complaintMessageShown=false;
	private boolean verbose=false;
    private ErrorHandler handler = null;

	public XMLParserHolder(
        boolean verbose,
        ErrorHandler handler)
    {
		this.verbose = verbose;
        this.handler = handler;
        if(!XMLParserHolder.complaintMessageShown && verbose)
        {
			System.out.println("- Notification: XML Parser is XML version 1.0 compliant.");
			XMLParserHolder.complaintMessageShown=true;
		}

	}
    /**
     * Implemented parser interface method. Accesses the external parser package and
     * instructs it to start parsing.
     *
     * @param targetFile The target file to be parsed
     * @param targetValidationFile The optional validation file for the target file to
     * be parsed. May be NULL
     * @return java.lang.Object
     * @roseuid 407294E70198
     */
	public Object parseFile(java.io.File targetFile, java.io.File targetValidationFile, java.util.Vector includePaths) throws Exception{
        Document document;

		try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setValidating(true);
            //factory.setFeature("http://xml.org/sax/features/validation", true);
            DocumentBuilder builder = factory.newDocumentBuilder();
            String osplHome;
            if(handler != null)
            {
                builder.setErrorHandler(handler);
            } else
            {
                builder.setErrorHandler(this);
            }
            if(verbose){
                System.out.println("- Notification: Using the following parser for XML parsing: "+ builder.getClass().getName());
            }

            osplHome = System.getProperty("OSPL_HOME");
            if(osplHome == null)
            {
                osplHome = System.getenv ("OSPL_HOME");
                if(osplHome == null)
                {
                    throw new Exception("Unable to locate the enviroment variable 'OSPL_HOME'. " +
                                    "Make sure this variable is set in your enviroment and points to your OpenSplice installation.");
                }
            }
            String seperator = java.io.File.separator;
            java.io.File dtdFile = new java.io.File(osplHome+seperator+"etc"+seperator+"dcg"+seperator+"Dlrl.dtd");

            try
            {
                // try to verify whether the dtd file exists under $OSPL_HOME
                verifyFile(dtdFile);
            }
            catch (Exception osplHomeException)
            {
                throw osplHomeException;
            }

            document = builder.parse(new java.io.FileInputStream(targetFile), dtdFile.toURI().toURL().toString());//dont use toURL directory
		} catch (SAXException se) {
			throw se;
		} catch (IOException ioe) {
			throw ioe;
		}
		return (Object)document;
	}

    // Verify the specified file.
    private void verifyFile(java.io.File aFile) throws Exception{
        if(!aFile.isFile()){
            throw new Exception("The file represented by path ("+aFile.getAbsolutePath()+") is not a file");
        } else if(!aFile.exists()){
            throw new Exception("The file represented by path ("+aFile.getAbsolutePath()+") does not exist");
        } else if(!aFile.canRead()){
            throw new Exception("The file represented by path ("+aFile.getAbsolutePath()+") can not be read");
        }
    }


    //  Warning Event Handler
    public void warning (SAXParseException e)
        throws SAXException
    {
        if(verbose)
        {
            System.out.println ("- Warning:  "+e);
        }
    }

    //  Error Event Handler
    public void error (SAXParseException e)
        throws SAXException
    {
	   throw e;
    }

    //  Fatal Error Event Handler
    public void fatalError (SAXParseException e)
        throws SAXException {
	  throw e;
    }
}
