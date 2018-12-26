

package hudson.util;

import hudson.console.ConsoleNote;
import hudson.model.TaskListener;
import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;



public class LogTaskListener extends AbstractTaskListener implements TaskListener, Closeable {

        private final TaskListener delegate;

    public LogTaskListener(Logger logger, Level level) {
        delegate = new StreamTaskListener(new LogOutputStream(logger, level, new Throwable().getStackTrace()[1]));
    }

    @Override
    public PrintStream getLogger() {
        return delegate.getLogger();
    }

    @Override
    @SuppressWarnings("rawtypes")
    public void annotate(ConsoleNote ann) {
            }

    @Override
    public void close() {
        delegate.getLogger().close();
    }

    private static class LogOutputStream extends OutputStream {

        private final Logger logger;
        private final Level level;
        private final StackTraceElement caller;
        private final ByteArrayOutputStream baos = new ByteArrayOutputStream();

        LogOutputStream(Logger logger, Level level, StackTraceElement caller) {
            this.logger = logger;
            this.level = level;
            this.caller = caller;
        }

        @Override
        public void write(int b) throws IOException {
            if (b == '\r' || b == '\n') {
                flush();
            } else {
                baos.write(b);
            }
        }

        @Override
        public void flush() throws IOException {
            if (baos.size() > 0) {
                LogRecord lr = new LogRecord(level, baos.toString());
                lr.setLoggerName(logger.getName());
                lr.setSourceClassName(caller.getClassName());
                lr.setSourceMethodName(caller.getMethodName());
                logger.log(lr);
            }
            baos.reset();
        }

        @Override
        public void close() throws IOException {
            flush();
        }

    }

    private static final long serialVersionUID = 1L;
}
