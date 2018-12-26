package jenkins.slaves.restarter;

import com.sun.akuma.Daemon;
import com.sun.akuma.JavaVMArguments;
import com.sun.jna.Native;
import com.sun.jna.StringArray;
import hudson.Extension;

import java.io.File;
import java.io.IOException;
import java.util.logging.Logger;

import static hudson.util.jna.GNUCLibrary.*;
import static java.util.logging.Level.*;


@Extension
public class UnixSlaveRestarter extends SlaveRestarter {
    private transient JavaVMArguments args;

    @Override
    public boolean canWork() {
        try {
            if (File.pathSeparatorChar!=':')
                return false;     
            args = JavaVMArguments.current();

                        LIBC.getdtablesize();
            int v = LIBC.fcntl(99999, F_GETFD);
            LIBC.fcntl(99999, F_SETFD, v);

            Daemon.getCurrentExecutable();
            LIBC.execv("positively/no/such/executable", new StringArray(new String[]{"a","b","c"}));

            return true;
        } catch (UnsupportedOperationException e) {
            LOGGER.log(FINE, getClass()+" unsuitable", e);
            return false;
        } catch (LinkageError e) {
            LOGGER.log(FINE, getClass()+" unsuitable", e);
            return false;
        } catch (IOException e) {
            LOGGER.log(FINE, getClass()+" unsuitable", e);
            return false;
        }
    }

    public void restart() throws Exception {
                int sz = LIBC.getdtablesize();
        for (int i = 3; i < sz; i++) {
            int flags = LIBC.fcntl(i, F_GETFD);
            if (flags < 0) continue;
            LIBC.fcntl(i, F_SETFD, flags | FD_CLOEXEC);
        }

                String exe = Daemon.getCurrentExecutable();
        LIBC.execv(exe, new StringArray(args.toArray(new String[args.size()])));
        throw new IOException("Failed to exec '" + exe + "' " + LIBC.strerror(Native.getLastError()));
    }

    private static final Logger LOGGER = Logger.getLogger(UnixSlaveRestarter.class.getName());

    private static final long serialVersionUID = 1L;
}
