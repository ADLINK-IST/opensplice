package org.opensplice.common.util;

import java.io.File;

public class ConfigModeIntializer {

    public static final String COMMUNITY = "COMMUNITY";
    public static final String COMMERCIAL = "COMMERCIAL";
    public static final int  COMMUNITY_MODE = 1;
    public static final int  COMMERCIAL_MODE = 2;
    public static final int  COMMUNITY_MODE_FILE_OPEN = 3;
    public static int          CONFIGURATOR_MODE        = COMMUNITY_MODE;

    public static void setMode(int mode) {
        CONFIGURATOR_MODE = mode;
    }

    public int getMode() {
        return CONFIGURATOR_MODE;
    }
}
