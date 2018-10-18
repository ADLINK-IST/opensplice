/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

    public String getCurrentContentType() {
        return curContentType;
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
