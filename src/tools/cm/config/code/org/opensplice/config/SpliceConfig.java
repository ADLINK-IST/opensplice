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
package org.opensplice.config;

import org.opensplice.common.util.Initializer;
import org.opensplice.common.util.Report;
import org.opensplice.config.swing.ConfigWindow;

public class SpliceConfig extends Initializer {

    /**
     * Starts Configurator. This function is the main class of the Configurator;
     * - It checks for the right version of the JVM, this must be > 1.5.0.
     * - It passes on the commandline arguments. Arguments that are supported
     *   are:
     *      -# <java_properties_file>
     *
     * @param args These are passed on to the initialize function of the
     *             SpliceConfig object.
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
