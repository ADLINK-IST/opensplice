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
package org.opensplice.config.swing;

import java.io.StringWriter;

import javax.swing.JEditorPane;

import org.opensplice.config.meta.MetaAttribute;
import org.opensplice.config.meta.MetaElement;
import org.opensplice.config.meta.MetaNode;

@SuppressWarnings("serial")
public class MetaNodeDocPane extends JEditorPane {
    private boolean showElementName;
    private boolean showChildrenAttributes;
    private static final String fontStyle = "font-family:Sans-serif";
    
    public MetaNodeDocPane(){
        super("text/html", "No selection");
        this.init(true, true);
    }
    
    public MetaNodeDocPane(boolean showElementName, boolean showChildrenAttributes){
        super("text/html", "No selection");
        this.init(showElementName, showChildrenAttributes);
    }
    
    private void init(boolean showElementName, boolean showChildrenAttributes){
        this.setEditable(false);
        this.showElementName = showElementName;
        this.showChildrenAttributes = showChildrenAttributes;
    }
    
    public void setNode(MetaNode node){
        String doc;
        MetaNode[] children;
        MetaAttribute attribute;
        
        int attributeChildrenCount = 0;
        
        StringWriter writer = new StringWriter();
        
        if(node == null){
            writer.write("No element selected.");
        } else {
            doc = this.layoutDoc(node.getDoc());
            
            if(node instanceof MetaElement){
                if(this.showElementName){
                    int maxOcc = ((MetaElement)node).getMaxOccurrences();
                    int minOcc = ((MetaElement)node).getMinOccurrences();
                    String occ;
                    
                    if(maxOcc == Integer.MAX_VALUE){
                        occ = "occurrences: " + minOcc + " - " + "*";
                    } else {
                        occ = "occurrences: " + minOcc + " - " + maxOcc;
                    }
                    writer.write("<h1>" + ((MetaElement)node).getName());
                    writer.write(" <font size=\"-1\">(" + occ + ")</font></h1>");
                }
                writer.write("<font style=\"" + MetaNodeDocPane.fontStyle + "\">" + doc + "</font>");
                children = ((MetaElement)node).getChildren();
                
                for(MetaNode mn: children){
                    if(mn instanceof MetaAttribute){
                        attributeChildrenCount++;
                    }
                }
                if(this.showChildrenAttributes){
                    if(attributeChildrenCount > 0){
                        writer.write("<h2>Attributes</h2>");
                        
                        for(MetaNode mn: children){
                            if(mn instanceof MetaAttribute){
                                this.writeNode(writer, mn);
                            }
                        }
                    } else {
                        writer.write("<h2>Attributes</h2>Element has no attributes");
                    }
                }
            } else if(node instanceof MetaAttribute){
                attribute = (MetaAttribute)node;
                
                if(this.showElementName){
                    writer.write("<h1>" + attribute.getName() + "(");
                    
                    if(attribute.isRequired()){
                        writer.write("required)</h1> - ");
                    } else {
                        writer.write("optional)</h1> - ");
                    }
                }
                writer.write("<font style=\"" + MetaNodeDocPane.fontStyle + "\">" + doc + "</font>");
            } else {
                writer.write("<font style=\"" + MetaNodeDocPane.fontStyle + "\">" + doc + "</font>");
            }
        }
        this.setText(writer.toString());
        this.setCaretPosition(0);
    }
    
    
    private void writeNode(StringWriter writer, MetaNode node){
        MetaElement element;
        MetaAttribute attribute;
        String doc;
        
        if(node instanceof MetaElement){
            element = (MetaElement)node;
            writer.write("<h3>" + element.getName());
            writer.write(" (occurrences : " + element.getMinOccurrences() + " - " + element.getMaxOccurrences() + ")</h3>");
        } else if(node instanceof MetaAttribute){
            attribute = (MetaAttribute)node;
            writer.write("<h3>" + attribute.getName() + " ("); 
            
            if(attribute.isRequired()){
                writer.write("required)</h3>");
            } else {
                writer.write("optional)</h3>");
            }
        }
        doc = this.layoutDoc(node.getDoc());
        writer.write("<font style=\"" + MetaNodeDocPane.fontStyle + "\">" + doc + "</font>");
    }
    
    private String layoutDoc(String doc){
        String temp;
        char c;
        StringWriter writer = new StringWriter();
        
        if(doc == null){
            temp = "No description available.";
        } else {
            temp = doc;
        }
        temp = temp.trim();
        
        if(temp.length() == 0){
            temp = "No description available.";
        } else {
            if(temp.startsWith("<p>")){
                temp = temp.substring(3);
                temp = temp.replaceFirst("</p>", " ");
            }
            
            for(int i=0; i<temp.length(); i++){
               c = temp.charAt(i);
               
               if((c != '\n') && (c != '\t')){
                   if(c == ' '){
                       if((i+1)<temp.length()){
                           if(temp.charAt(i+1) != ' '){
                               writer.write(c);
                           }
                       } else {
                           writer.write(c);
                       }
                   } else {
                       writer.write(c);
                   }
               }
            }
        }
        return writer.toString();
    }
}
