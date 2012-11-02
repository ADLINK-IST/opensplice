/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.config;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;

import org.opensplice.common.util.Initializer;
import org.opensplice.config.swing.ConfigWindow;

public class SpliceConfig extends Initializer {

    /**
     * Starts Splice Tuner. This function is the main class of Splice Tuner;
     * - It checks for the right version of the JVM, this must be 1.5.0.
     * - It passes on the commandline arguments. Arguments that are supported 
     *   are:
     *      -# <java_properties_file>
     * 
     * @param args These are passed on to the initialize function of the 
     *             SpliceTuner object.
     */
    public static void main(String[] args) {
        boolean redirect = true;
        String[] args2;
        
        SpliceConfig t = new SpliceConfig();
        t.validateJVMVersion();
        
        if (args.length == 1) {
            if ("-noredirect".equals(args[0])) {
                redirect = false;
            }
        }

        if (redirect) {
            args2 = args;

            try {
                File errorFile = new File("osplconf-error.log");
                File infoFile = new File("osplconf-info.log");

                if (!(errorFile.exists())) {
                    errorFile.createNewFile();
                }
                if (!(infoFile.exists())) {
                    infoFile.createNewFile();
                }

                if (errorFile.canWrite()) {
                    System.setErr(new PrintStream(new FileOutputStream(
                            errorFile)));
                } else {
                    System.err.println("Could not redirect error output.");
                }
                if (infoFile.canWrite()) {
                    System.setOut(new PrintStream(
                            new FileOutputStream(infoFile)));
                } else {
                    System.err.println("Could not redirect info output.");
                }

            } catch (IOException e) {
                System.err
                        .println("Could not redirect error and/or info output.");
            }
        } else {
            System.out.println("No redirect.");
            args2 = new String[args.length - 1];

            for (int i = 1; i < args.length; i++) {
                args2[i - 1] = args[i];
            }
        }
        

        
        ConfigWindow cw = new ConfigWindow();
        cw.setVisible(true);
        /*
        MetaConfiguration config = MetaConfiguration.getInstance();
        
        if(config != null){
        
            File file = new File("C:\\Documents and Settings\\niels\\Desktop\\config.xml");
            
            try {
                DataConfiguration dataConfig = null;
                try{
                    dataConfig = new DataConfiguration(config, file, false);
                } catch (DataException de) {
                    System.err.println(de.getMessage());
                    dataConfig = new DataConfiguration(config, file, true);
                }
                String xmlString = dataConfig.toString();
                file = new File("C:\\Documents and Settings\\niels\\Desktop\\config2.xml");
                file.createNewFile();

                FileWriter fw = new FileWriter(file, false);
                fw.write(xmlString);
                fw.flush();
                fw.close();
                  
                ConfigWindow cw = new ConfigWindow();
                cw.setVisible(true);
                cw.setDataConfiguration(dataConfig);
        
                if (!result) {
                    t.showVersionWarning(cw);
                }
            } catch (IOException e) {
                e.printStackTrace();
            } catch (DataException de) {
                de.printStackTrace();
            }
        }
        */
    }
}
