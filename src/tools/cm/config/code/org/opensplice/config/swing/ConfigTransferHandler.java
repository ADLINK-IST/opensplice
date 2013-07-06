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

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.io.File;
import java.net.URI;
import java.util.List;

import javax.swing.JComponent;
import javax.swing.TransferHandler;

@SuppressWarnings("serial")
public class ConfigTransferHandler extends TransferHandler {
    private final ConfigWindow view;
    
    public ConfigTransferHandler(ConfigWindow view){
        this.view = view;
    }
    @Override
    public boolean canImport(JComponent comp, DataFlavor[] transferFlavors) {
        for(DataFlavor flavor: transferFlavors){
            if(flavor.equals(DataFlavor.javaFileListFlavor)){
                view.setStatus("Drag here to open " + 
                        flavor.getHumanPresentableName() + " file.", false);
                return true;
            } else if(flavor.equals(DataFlavor.stringFlavor)){
                view.setStatus("Drag here to open " + 
                        flavor.getHumanPresentableName() + " file.", false);
                return true;
            }
        }
        view.setStatus("Warning: Unsupported type.", false);
        
        return false;
    }
    
    @Override
    public boolean importData(JComponent comp, Transferable t) {
        if (!canImport(comp, t.getTransferDataFlavors())) {
            view.setStatus("Warning: Unsupported type", false);
            return false;
        }
        try{
            if(t.isDataFlavorSupported(DataFlavor.javaFileListFlavor)){
                @SuppressWarnings("unchecked")
                List<File> files = (List<File>) t.getTransferData(DataFlavor.javaFileListFlavor);
                
                if(files.size() == 1){
                    File file = files.get(0);
                    view.getController().handleOpen(file);
                } else {
                    return false;
                }
            } else if(t.isDataFlavorSupported(DataFlavor.stringFlavor)){
                String str = (String) t.getTransferData(DataFlavor.stringFlavor);
                
                if(str.startsWith("file:/")){
                    File f = new File(new URI(str));
                    view.getController().handleOpen(f);
                } else {
                    view.setStatus("Warning: Unsupported file.", false);
                }
            } else {
                view.setStatus("Warning: Unsupported drag-and-drop type", false);
            }
        } catch (Exception e) {
            view.setStatus("Warning: " + e.getMessage(), false);
            return false;
        }
        return true;
    }
}
