package jenkins.slaves.restarter;

import hudson.Extension;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.logging.Logger;

import static java.util.logging.Level.*;
import static org.apache.commons.io.IOUtils.*;


@Extension
public class WinswSlaveRestarter extends SlaveRestarter {
    private transient String exe;

    @Override
    public boolean canWork() {
        try {
            exe = System.getenv("WINSW_EXECUTABLE");
            if (exe==null)
                return false;   
            return exec("status") ==0;
        } catch (InterruptedException e) {
            LOGGER.log(FINE, getClass()+" unsuitable", e);
            return false;
        } catch (IOException e) {
            LOGGER.log(FINE, getClass()+" unsuitable", e);
            return false;
        }
    }

    private int exec(String cmd) throws InterruptedException, IOException {
        ProcessBuilder pb = new ProcessBuilder(exe, cmd);
        pb.redirectErrorStream(true);
        Process p = pb.start();
        p.getOutputStream().close();
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        copy(p.getInputStream(), baos);
        int r = p.waitFor();
        if (r!=0)
            LOGGER.info(exe+" cmd: output:\n"+baos);
        return r;
    }

    public void restart() throws Exception {
                                int r = exec("restart!");
        throw new IOException("Restart failure. '"+exe+" restart' completed with "+r+" but I'm still alive!  "
                               + "See https:                               + " for a possible explanation and solution");
    }

    private static final Logger LOGGER = Logger.getLogger(WinswSlaveRestarter.class.getName());

    private static final long serialVersionUID = 1L;
}
