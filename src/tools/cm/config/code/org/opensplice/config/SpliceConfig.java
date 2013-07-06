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
package org.opensplice.config;

import org.opensplice.common.util.Initializer;
import org.opensplice.common.util.Report;
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
        boolean uriSet = false;
        String[] args2;
        String uri = null;

        SpliceConfig t = new SpliceConfig();
        t.validateJVMVersion();

        for (int i = 0; i < args.length; i++) {
            if ("-noredirect".equals(args[i])) {
                redirect = false;
            } else if (args[i].toLowerCase().startsWith("-uri=")) {
                uri = args[i].substring(5); /* '-uri=' is 5 characters */
                uriSet = true;
            } else {
                args2 = new String[1];
                args2[0] = args[i];
            }
        }

        if (redirect) {
            /*
             * initialize logging with a max size of 10M with a max history of
             * 10 runs and do not append
             */
            Report.getInstance().initializeInfo("osplconf-info.log", 10000000, 10, false);
            Report.getInstance().initializeError("osplconf-error.log", 10000000, 10, false);
        } else {
            Report.getInstance().initializeConsole();
            System.out.println("No redirect.");
        }

        Runtime.getRuntime().addShutdownHook(new Thread() {
            @Override
            public void run() {
                Report.getInstance().CloseHandlers();
            }
        });
        ConfigWindow cw = null;
        if (uriSet) {
            cw = new ConfigWindow(uri);
        } else {
            cw = new ConfigWindow();
        }
        cw.setVisible(true);
    }
}
