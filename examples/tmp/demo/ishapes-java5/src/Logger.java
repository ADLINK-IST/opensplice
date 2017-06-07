

import java.util.logging.Level;

public class Logger {
    private boolean enabled;
    private Level level;

    private Logger() {

        this.enabled = Boolean.parseBoolean(System
                .getenv("OSPL_ISHAPES_LOGGING"));

        try {
            this.level = Level.parse(System
                    .getenv("OSPL_ISHAPES_LOGGING_LEVEL"));
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
            this.log(
                    "    at " + ste.getClass().getName() + "."
                            + ste.getMethodName() + "(" + ste.getFileName()
                            + ":" + ste.getLineNumber() + ")\n", Level.SEVERE);
        }
    }
}
