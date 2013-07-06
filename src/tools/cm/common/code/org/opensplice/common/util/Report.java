package org.opensplice.common.util;

import java.io.IOException;
import java.util.logging.FileHandler;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;

public class Report {

    private static Report reportObject = new Report();
    static Logger         errorLog;
    static Logger         infoLog;

    private String        errorLogFile  = null;
    private int           errorSize     = 0;
    private int           errorCount    = 0;
    private boolean       errorAppend  = false;

    private String        infoLogFile   = null;
    private int           infoSize      = 0;
    private int           infoCount     = 0;
    private boolean       infoAppend   = false;

    private Handler       errorHandler;
    private Handler       infoHandler;

    private boolean       consoleOutput = false;

    /**
     * A private Constructor prevents any other class from instantiating.
     *
     * @return
     */
    private Report() {
        errorLog = Logger.getAnonymousLogger();
        infoLog = Logger.getAnonymousLogger();
    }

    public static Report getInstance() {
        return reportObject;
    }

    public void initializeInfo(String infoLogFile, int size, int nrOfLogFiles, boolean append) {
        this.infoLogFile = infoLogFile;
        this.infoSize = size;
        this.infoCount = nrOfLogFiles;
        this.infoAppend = append;
    }

    public void initializeError(String errorLogFile, int size, int nrOfLogFiles, boolean append) {
        this.errorLogFile = errorLogFile;
        this.errorSize = size;
        this.errorCount = nrOfLogFiles;
        this.errorAppend = append;
    }

    public void initializeConsole() {
        consoleOutput = true;
    }

    public void writeInfoLog(String message) {
        if (!consoleOutput) {
            if (infoHandler == null && infoLogFile != null) {
                try {
                    infoHandler = new FileHandler(infoLogFile, infoSize, infoCount, infoAppend);
                    infoHandler.setFormatter(new SimpleFormatter());
                    infoLog.addHandler(infoHandler);
                    LogManager logManager = LogManager.getLogManager();
                    logManager.reset();
                } catch (IOException e) {
                    System.err.println("Could not redirect error and/or info output.");
                }

            }
            infoLog.info(message);
        } else {
            System.out.println(message);
        }

    }

    public void writeErrorLog(String message) {
        if (!consoleOutput) {
            if (errorHandler == null && errorLogFile != null) {
                try {
                    errorHandler = new FileHandler(errorLogFile, errorSize, errorCount, errorAppend);
                    errorHandler.setFormatter(new SimpleFormatter());
                    errorLog.addHandler(errorHandler);
                    LogManager logManager = LogManager.getLogManager();
                    logManager.reset();
                } catch (IOException e) {
                    System.err.println("Could not redirect error and/or info output.");
                }
            }
            errorLog.log(Level.SEVERE, message);
        } else {
            System.err.println(message);
        }
    }

    public void CloseHandlers() {
        if (errorHandler != null) {
            errorHandler.close();
        }
        if (infoHandler != null) {
            infoHandler.close();
        }
    }

}
