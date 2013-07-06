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
package DCG.Core;


/**
 * The AbstractSyntaxTreeHolder class is a holder class for an abstract syntax 
 * tree, as the name for the class already hints. This class can be seen as a 
 * wrapper for any kind of syntax language. In the DCG program for the IDL and XML 
 * syntax languages. 
 * The holder class provides functionality to retrieve the root element of the 
 * syntax, so other (specialized) classes can navigate their way through the 
 * syntax language model, which will contain the contents of a parsed file for 
 * example. 
 * This holder class has no knowledge of how the syntax tree is built or what it 
 * represents. As said before, it must simple be seen as a handy wrapper for 
 * containing the tree.
 */
public class AbstractSyntaxTreeHolder {
    
    /**
     * The root element of the syntax tree contained within this holder. The root 
     * element is stored as a java Object to ensure the holder class has no knowledge 
     * of the exact implementation of the syntax tree and to ensure it can be used as 
     * a holder for any syntax tree.
     */
    private Object root;
    
    /**
     * Default constructor
     * @roseuid 40741529026C
     */
    public AbstractSyntaxTreeHolder() {
		//does nothing     
    }
    
    /**
     * Returns the root access point for this AST
     * @return Object
     * @roseuid 407106F50292
     */
	public Object getRoot(){
		return root;
	}
    
    /**
     * Sets the root access point for this AST
     * 
     * @param root The root object allowing central access to the underlying syntax 
     * tree.
     * @roseuid 407107250081
     */
	public void setRoot(Object root){
		this.root = root;
	}
}
