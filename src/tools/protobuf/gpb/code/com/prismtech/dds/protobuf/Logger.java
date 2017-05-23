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
package com.prismtech.dds.protobuf;

import java.util.logging.Level;

public class Logger {
    private boolean enabled;
    private Level level;


    private Logger() {
        this.enabled = Boolean.parseBoolean(Util.getenv("PROTOBUF_LOGGING"));

        try {
            this.level = Level.parse(Util.getenv("PROTOBUF_LOGGING_LEVEL"));
        } catch (NullPointerException npe) {
            this.level = Level.ALL;
        } catch (IllegalArgumentException iae) {
            this.level = Level.ALL;
        }
    }

    private static class LazyHolder {
        private static final Logger INSTANCE = new Logger();
    }

    public static Logger getInstance() {
        return LazyHolder.INSTANCE;
    }

    public void log(String message, Level level) {
        if (this.enabled) {
            if (level.intValue() >= this.level.intValue()) {
                System.err.print(message);
            }
        }
    }

    public void exception(Exception e) {
        this.log(
                e.getClass().getName() + " occurred: " + e.getMessage() + "\n",
                Level.SEVERE);

        for (StackTraceElement ste : e.getStackTrace()) {
            this.log("    at " + ste.getClassName() + "." + ste.getMethodName()
                    + "(" + ste.getFileName() + ":" + ste.getLineNumber()
                    + ")\n", Level.SEVERE);
        }
    }
}
