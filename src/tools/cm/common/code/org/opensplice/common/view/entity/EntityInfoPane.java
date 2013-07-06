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
package org.opensplice.common.view.entity;

import javax.swing.JEditorPane;
import javax.swing.SwingUtilities;
import javax.swing.text.EditorKit;


/**
 * Pane that is capable of displaying several representations of an object 
 * depending on the view type.
 * 
 * Currently supported views types:
 * - text/plain
 * - text/html
 * 
 * Currently supported objects:
 * - MetaType
 * - String
 * 
 * @date Jun 7, 2004
 */
public class EntityInfoPane extends JEditorPane{
    /**
     * Creates a graphical pane where object information can be showed in.
     * The information depends on the view type.
     * 
     * @param contentType The content type that specifies in which format the
     *                    information must be shown.  
     */
    public EntityInfoPane(String contentType){
        this.setEditable(false);
        curObject = null;
        EditorKit ek = this.getEditorKitForContentType(contentType);
        this.setEditorKit(ek);
        
        curContentType = ek.getContentType();
        
        if("text/plain".equals(curContentType)){
            infoFormatter = new EntityInfoFormatterText();
        }
        else if("text/html".equals(curContentType)){
            infoFormatter = new EntityInfoFormatterHTML();
        }
        else{
            infoFormatter = new EntityInfoFormatterText();
        }
        this.setText(infoFormatter.getValue(curObject));
    }
    
    
    /**
     * Sets the content type to the supplied type.
     * 
     * @param contentType The content type that specifies in which format the
     *                    information must be shown.
     * @return true if succeeded, false if not .
     */
    public boolean setViewType(String contentType){
        boolean result = true;
        
        if(!(curContentType.equals(contentType))){
            final EditorKit ek = this.getEditorKitForContentType(contentType);
            final EntityInfoPane ep = this;
            
            /*
             * Because this is an expensive operation it is executed in a
             * new thread that is invoked later by the event-dispatching thread.
             * This makes the GUI keep responding while processing.
             */
            Runnable worker = new Runnable() {
                @Override
                public void run() {
                    ep.setEditorKit(ek);
                    curContentType = ek.getContentType();
                    if("text/plain".equals(curContentType)){
                        infoFormatter = new EntityInfoFormatterText();
                    }
                    else if("text/html".equals(curContentType)){
                        infoFormatter = new EntityInfoFormatterHTML();
                    }
                    else{
                        infoFormatter = new EntityInfoFormatterText();
                    }
                    synchronized(curObject){
                        ep.setText(infoFormatter.getValue(curObject));
                    }
                }
            };
            SwingUtilities.invokeLater(worker);
            /*
            ep.setEditorKit(ek);
            curContentType = ek.getContentType();
            if("text/plain".equals(curContentType)){
                infoFormatter = new EntityInfoFormatterText();
            }
            else if("text/html".equals(curContentType)){
                infoFormatter = new EntityInfoFormatterHTML();
            }
            else{
                infoFormatter = new EntityInfoFormatterText();
            }
            ep.setText(infoFormatter.getValue(curObject));
            */
        }
        return result;
    }
    
    /**
     * Sets selection to supplied object. This is done by asking the current 
     * formatter to format the object.
     * 
     * @param object The object to select.
     */
    public synchronized void setSelection(Object object){
        this.setText(infoFormatter.getValue(object));
        curObject = object;
    }
    
    /**
     * Clears the cache of MetaType objects.
     */
    public void clearCache(){
        infoFormatter.clearCache();
    }
    
    /**
     * Current entity formatter.
     */
    private transient EntityInfoFormatter infoFormatter;
    
    /**
     * Current content type. Currently supported types:
     * - text/plain
     * - text/html
     */
    private String curContentType;
    
    /**
     * The currently selected object.
     */
    private Object curObject;
}
