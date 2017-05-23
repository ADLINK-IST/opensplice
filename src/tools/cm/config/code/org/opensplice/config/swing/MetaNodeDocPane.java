/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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

                    String dimension = node.getDimension();

                    if (dimension != null) {
                        writer.write(" <font size=\"-1\">(" + occ
                                + " and dimension: " + dimension
                                + ")</font></h1>");
                    } else {
                        writer.write(" <font size=\"-1\">(" + occ
                                + ")</font></h1>");
                    }
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
                        writer.write("required");
                    } else {
                        writer.write("optional");
                    }
                    String dimension = attribute.getDimension();

                    if (dimension != null) {
                        writer.write(" and dimension: ");
                        writer.write(dimension);
                    }
                    writer.write(")</h1> -");
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
        String dimension = node.getDimension();
        
        if(node instanceof MetaElement){
            element = (MetaElement)node;
            writer.write("<h3>" + element.getName());
            writer.write(" (occurrences : " + element.getMinOccurrences()
                    + " - " + element.getMaxOccurrences());

            if (dimension != null) {
                writer.write(" dimension: " + dimension);
            }
            writer.write(")</h3>");
        } else if(node instanceof MetaAttribute){
            attribute = (MetaAttribute)node;
            writer.write("<h3>" + attribute.getName() + " ("); 
            
            if(attribute.isRequired()){
                writer.write("required");
            } else {
                writer.write("optional");
            }
            if (dimension != null) {
                writer.write(" and dimension: " + dimension);
            }
            writer.write(")</h3>");
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
