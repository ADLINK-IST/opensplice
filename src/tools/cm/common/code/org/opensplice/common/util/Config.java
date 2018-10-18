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

/**
 * Contains all SPLICE DDS C&M Tooling utilities.
 */
package org.opensplice.common.util;

import java.awt.Color;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Iterator;
import java.util.Properties;

import org.opensplice.common.view.CommonFileChooser;

/**
 * Supplies a generic configuration utility for all kinds of applications.
 * Before using the load or loadDefault function must be called.
 * 
 * @date Oct 25, 2004
 */
public class Config {

    /**
     * The current configuration.
     */
    private Properties config = null;

    /**
     * The location of the configuration.
     */
    private File configFile = null;

    /**
     * The validator.
     */
    private ConfigValidator validator = null;

    private CommonFileChooser chooser = null;

    private static volatile Config instance       = null;

    private static Color warningColor = null;
    private static Color errorColor = null;
    private static Color sensitiveColor = null;
    private static Color inactiveColor = null;
    private static Color activeColor = null;
    private static Color inputColor = null;
    private static Color incorrectColor = null;
    private static Color customColor1 = null;

    private Config(){}

    public static Config getInstance(){
        if(instance == null){
            instance = new Config();
        }
        return instance;
    }

    /**
     * Assigns the supplied validator to the configuration. It will use this
     * validator when calling: - load --> call validateValue on each found key.
     * - loadDefault --> call validateValue on each found key. - getProperty ->
     * call getDefaultValue when a supplied kay has no value. - setProperty -->
     * call isValueValid on the supplied key/value combination.
     * 
     * @param _validator
     *            The validator to assign.
     */
    public void setValidator(ConfigValidator _validator){
        validator = _validator;
    }

    /**
     * Provides access to the current configuration validator.
     * 
     * @return The current validator or null if none has been assigned.
     */
    public ConfigValidator getValidator(){
        return validator;
    }

    /**
     * Provides access to the current configuration properties.
     * 
     * @return The current config properties or null if none has been assigned.
     */
    public Properties getConfig() {
        return config;
    }

    /**
     * Loads the release information from a file named RELEASEINFO located in
     * the etc directory of the installation.
     * 
     * @return true if the release information could be loaded, false otherwise.
     */
    public boolean loadReleaseInfo(){
        FileInputStream fis = null;
        if (config == null){
            config = new Properties ();
        }
        boolean result = true;
        String homeDir = System.getenv("OSPL_HOME");
        String separator = System.getProperty("file.separator");
        File releaseFile = new File (homeDir + separator + "etc" + separator + "RELEASEINFO");
        if(releaseFile.exists()) {
            try {
                fis = new FileInputStream(releaseFile);
                config.load(fis);
            } catch (FileNotFoundException e) {
                result = false;
            } catch (IOException e) {
                result = false;
            } finally {
                if (fis != null) {
                    try {
                        fis.close();
                    } catch (IOException ie) {
                        result = false;
                    }
                }
            }
        }

        // If we don't have a value for the version property then try
        // to load the value from the development tree RELEASE file
        String version = config.getProperty("PACKAGE_VERSION");
        if (version == null) {
            releaseFile = new File (homeDir + separator + "release_info" + separator + "RELEASE");
            if(releaseFile.exists()) {
                try {
                    fis = new FileInputStream(releaseFile);
                    config.load(fis);
                } catch (FileNotFoundException e) {
                    result = false;
                } catch (IOException e) {
                    result = false;
                } finally {
                    if (fis != null) {
                        try {
                            fis.close();
                        } catch (IOException ie) {
                            result = false;
                        }
                    }
                }
            }
        }
        if(!result){
            config = null;
        }
        return result;
    }

    /**
     * Loads the default configuration. This located in the home directory of
     * the user in the '.splice_tooling.properties' file.
     * 
     * @return true if the default configuration could be loaded, false
     *         otherwise.
     */
    public boolean loadDefault(){
        String key, value;
        boolean result = true;
        String homeDir = System.getProperty("user.home");
        String separator = System.getProperty("file.separator");
        config = new Properties();
        FileInputStream fis = null;

        // Load the release information so that we have the PACKAGE_VERSION property.
        result = loadReleaseInfo();

        if (result) {
            String version = config.getProperty("PACKAGE_VERSION");
            if (version != null) {
                version = version.replaceAll("\"", "");
                version = new String ("." + version);
            } else {
                version = new String ("");
            }
            configFile = new File(homeDir + separator + ".ospl_tooling.properties" + version);
        } else {
            configFile = new File(homeDir + separator + ".ospl_tooling.properties");
            // Reset the result flag.
            result = true;
        }

        if(!(configFile.exists())) {
            try {
                configFile.createNewFile();
            } catch (IOException e) {
                result = false;
            }
        } else {
            try {
                fis = new FileInputStream(configFile);
                config.load(fis);
                Properties newConfig = new Properties(config);
                if(validator != null){
                    Iterator<Object> iter = config.keySet().iterator();

                    while(iter.hasNext()){
                        key = (String)iter.next();
                        value = config.getProperty(key);
                        value = validator.getValidatedValue(key, value);

                        if(value == null){
                            newConfig.remove(key);
                        } else {
                            newConfig.setProperty(key, value);
                        }
                    }
                }
                config = newConfig;
            } catch (FileNotFoundException e) {
                result = false;
            } catch (IOException e) {
                result = false;
            } finally {
                if (fis != null) {
                    try {
                        fis.close();
                    } catch (IOException ie) {
                        result = false;
                    }
                }
            }
        }
        if(!result){
            config = null;
            configFile = null;
        }
        return result;
    }



    /**
     * Loads the configuration from the supplied URI.
     * 
     * @param uri
     *            The URI to load the configuration from.
     * @return true if the configuration was successfully loaded, false
     *         otherwise.
     */
    public boolean load(String uri) {
        String value;
        boolean result = false;
        FileInputStream fis = null;

        try {
            uri = uri.replaceAll(" ", "%20");
            URI location = new URI(uri);
            configFile = new File(location);
            config = new Properties();
            fis = new FileInputStream(configFile);
            config.load(fis);
            Properties newConfig = new Properties();
            if(validator != null){

                for (Object key : config.keySet()) {
                    value = config.getProperty((String) key);
                    value = validator.getValidatedValue((String) key, value);
                    if (value != null) {
                        newConfig.setProperty((String) key, value);
                    }
                }
            }
            config.putAll(newConfig);
            result = true;
        } catch (FileNotFoundException e1) {
            Report.getInstance().writeErrorLog("Configuration file could not be found.");
        } catch (IOException e1) {
            Report.getInstance().writeErrorLog("Configuration file could not be read.");
        } catch (URISyntaxException e) {
            Report.getInstance().writeErrorLog("Supplied URI not valid.");
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException ie) {
                    Report.getInstance().writeErrorLog("Configuration file could not be read.");
                }
            }
        }
        return result;
    }

    /**
     * Provides access to the value of the supplied property.
     * 
     * @param key
     *            The name of the property.
     * @return The value of the property or null if it was not available.
     */
    public String getProperty(String key){
        String result = null;

        if(config != null){
            if(config.containsKey(key)){
                result = config.getProperty(key);
            } else if(validator != null){
                result = validator.getDefaultValue(key);

                if(result != null){
                    config.setProperty(key, result);
                }
            }
        }
        return result;
    }

    /**
     * Assigns the supplied value to the supplied property.
     * 
     * @param key
     *            The name of the property.
     * @param value
     *            The value of the property.
     * @return true if it could be assigned, false otherwise.
     */
    public boolean setProperty(String key, String value){
        boolean result = false;

        if(config != null){
            if(validator != null){
                result = validator.isValueValid(key, value);

                if(result){
                    config.setProperty(key, value);
                    this.store();
                }
            } else {
                config.setProperty(key, value);
                this.store();
            }
            result = true;
        }
        return result;
    }

    public boolean isPropertyValid(String key, String value){
        boolean result = false;

        if((config != null) && (validator != null)){
            result = validator.isValueValid(key, value);
        }
        return result;
    }

    /**
     * Removes the supplied property from the configuration.
     * 
     * @param key
     *            The name of the property to remove.
     * @return true if succeeded, false otherwise.
     */
    public boolean removeProperty(String key){
        boolean result = false;

        if(config != null){
            Object value = config.remove(key);

            if(value != null){
                result = true;
            }
        }
        return result;
    }

    /**
     * Prints the configuration to screen.
     */
    public void list(){
        if(config != null){
            config.list(System.out);
        }
    }

    /**
     * Stores the configuration to disk.
     * 
     * @return true if succeeded, false otherwise.
     */
    public boolean store(){
        boolean result = false;
        FileOutputStream fos = null;

        if(config != null){
            try {
                fos = new FileOutputStream(configFile);
                config.store(fos, null);
                result = true;
            } catch (IOException e) {
                Report.getInstance().writeErrorLog("Configuration could not be saved.");
            } finally {
                if (fos != null) {
                    try {
                        fos.close();
                    } catch (IOException ie) {
                        Report.getInstance().writeErrorLog("Configuration could not be saved.");
                    }
                }
            }
        }
        return result;
    }

    /**
     * Stores the configuration to the supplied uri.
     * 
     * @return true if succeeded, false otherwise.
     */
    public boolean store(String uri){
        boolean result = false;
        FileOutputStream fos = null;

        if(config != null){
            try {
                Report.getInstance().writeInfoLog("Storing configuration to URI: " +
                        configFile.toURI().getScheme() +
                        configFile.toURI().getPath() + ".");
                uri = uri.replaceAll(" ", "%20");
                URI outURI = new URI(uri);
                File outFile = new File(outURI);
                boolean fileCreated = true;

                if(!(outFile.exists())){
                    fileCreated = outFile.createNewFile();
                }
                if (fileCreated) {
                    fos = new FileOutputStream(outFile);
                    config.store(fos, null);
                } else {
                    Report.getInstance().writeErrorLog("Configuration could not be saved. file could not be created");
                    result = false;
                }
            } catch (IOException e) {
                Report.getInstance().writeErrorLog("Configuration could not be saved.");
            } catch (URISyntaxException e) {
                Report.getInstance().writeErrorLog("Configuration could not be saved.");
            } finally {
                if (fos != null) {
                    try {
                        fos.close();
                    } catch (IOException ie) {
                        Report.getInstance().writeErrorLog("Configuration could not be saved.");
                    }
                }
            }
        }
        return result;
    }

    public synchronized CommonFileChooser getFileChooser(){
        if(this.chooser == null){
            this.chooser = new CommonFileChooser(".");
        }
        return this.chooser;
    }

    public static Color getErrorColor(){
        if(errorColor == null){
            int r = Integer.parseInt("a0", 16);
            int g = Integer.parseInt("20", 16);
            int b = Integer.parseInt("20", 16);

            r = Integer.parseInt("ff", 16);
            g = Integer.parseInt("40", 16);
            b = Integer.parseInt("40", 16);

            errorColor = new Color(r, g, b);
        }
        return errorColor;
    }

    public static Color getWarningColor(){
        if(warningColor == null){
            int r = Integer.parseInt("c0", 16);
            int g = Integer.parseInt("80", 16);
            int b = Integer.parseInt("00", 16);
            warningColor = new Color(r, g, b);
        }
        return warningColor;
    }

    public static Color getSensitiveColor(){
        if(sensitiveColor == null){
            int r = Integer.parseInt("70", 16);
            int g = Integer.parseInt("e0", 16);
            int b = Integer.parseInt("70", 16);
            sensitiveColor = new Color(r, g, b);
        }
        return sensitiveColor;
    }

    public static Color getInactiveColor(){
        if(inactiveColor == null){
            int r = Integer.parseInt("70", 16);
            int g = Integer.parseInt("70", 16);
            int b = Integer.parseInt("7a", 16);
            inactiveColor = new Color(r, g, b);
        }
        return inactiveColor;
    }

    public static Color getActiveColor(){
        if(activeColor == null){
            int r = Integer.parseInt("50", 16);
            int g = Integer.parseInt("90", 16);
            int b = Integer.parseInt("50", 16);
            activeColor = new Color(r, g, b);
        }
        return activeColor;
    }

    public static Color getInputColor(){
        if(inputColor == null){
            int r = Integer.parseInt("f0", 16);
            int g = Integer.parseInt("90", 16);
            int b = Integer.parseInt("00", 16);
            inputColor = new Color(r, g, b);
        }
        return inputColor;
    }

    public static Color getIncorrectColor(){
        if(incorrectColor == null){
            int r = Integer.parseInt("ff", 16);
            int g = Integer.parseInt("40", 16);
            int b = Integer.parseInt("40", 16);
            incorrectColor = new Color(r, g, b);
        }
        return incorrectColor;
    }

    public static Color getCustomColor1(){
        if(customColor1 == null){
            int r = Integer.parseInt("40", 16);
            int g = Integer.parseInt("e0", 16);
            int b = Integer.parseInt("e0", 16);
            customColor1 = new Color(r, g, b);
        }
        return customColor1;
    }
}
