
package hudson.slaves;

import hudson.ExtensionPoint;
import hudson.Extension;
import hudson.model.*;
import hudson.remoting.Channel;
import hudson.util.DescriptorList;
import hudson.util.StreamTaskListener;

import java.io.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.apache.tools.ant.util.DeweyDecimal;


public abstract class ComputerLauncher extends AbstractDescribableImpl<ComputerLauncher> implements ExtensionPoint {
    
    public boolean isLaunchSupported() {
        return true;
    }

    
    public void launch(SlaveComputer computer, TaskListener listener) throws IOException , InterruptedException {
                launch(computer,cast(listener));
    }

    
    @Deprecated
    public void launch(SlaveComputer computer, StreamTaskListener listener) throws IOException , InterruptedException {
        throw new UnsupportedOperationException(getClass()+" must implement the launch method");
    }

    
    public void afterDisconnect(SlaveComputer computer, TaskListener listener) {
                afterDisconnect(computer,cast(listener));
    }

    
    @Deprecated
    public void afterDisconnect(SlaveComputer computer, StreamTaskListener listener) {
    }

    
    public void beforeDisconnect(SlaveComputer computer, TaskListener listener) {
                beforeDisconnect(computer,cast(listener));
    }

    
    @Deprecated
    public void beforeDisconnect(SlaveComputer computer, StreamTaskListener listener) {
    }

    private StreamTaskListener cast(TaskListener listener) {
        if (listener instanceof StreamTaskListener)
            return (StreamTaskListener) listener;
        return new StreamTaskListener(listener.getLogger());
    }

    
    @Deprecated
    public static final DescriptorList<ComputerLauncher> LIST = new DescriptorList<ComputerLauncher>(ComputerLauncher.class);

    
    protected static void checkJavaVersion(final PrintStream logger, String javaCommand,
                                    final BufferedReader r)
            throws IOException {
        String line;
        Pattern p = Pattern.compile("(?i)(?:java|openjdk) version \"([0-9.]+).*\"");
        while (null != (line = r.readLine())) {
            Matcher m = p.matcher(line);
            if (m.matches()) {
                final String versionStr = m.group(1);
                logger.println(Messages.ComputerLauncher_JavaVersionResult(javaCommand, versionStr));
                try {
                    if (new DeweyDecimal(versionStr).isLessThan(new DeweyDecimal("1.8"))) {
                        throw new IOException(Messages
                                .ComputerLauncher_NoJavaFound(line));
                    }
                } catch (NumberFormatException x) {
                    throw new IOException(Messages.ComputerLauncher_NoJavaFound(line));
                }
                return;
            }
        }
        logger.println(Messages.ComputerLauncher_UnknownJavaVersion(javaCommand));
        throw new IOException(Messages.ComputerLauncher_UnknownJavaVersion(javaCommand));
    }
}
