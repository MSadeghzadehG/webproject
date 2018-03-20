package jenkins.slaves.restarter;

import hudson.Extension;
import hudson.Functions;
import hudson.model.Computer;
import hudson.model.TaskListener;
import hudson.remoting.Engine;
import hudson.remoting.EngineListener;
import hudson.remoting.EngineListenerAdapter;
import hudson.remoting.VirtualChannel;
import hudson.slaves.ComputerListener;
import jenkins.model.Jenkins.MasterComputer;

import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.logging.Logger;

import static java.util.logging.Level.*;
import jenkins.security.MasterToSlaveCallable;


@Extension
public class JnlpSlaveRestarterInstaller extends ComputerListener implements Serializable {
    @Override
    public void onOnline(final Computer c, final TaskListener listener) throws IOException, InterruptedException {
        MasterComputer.threadPoolForRemoting.submit(new Install(c, listener));
    }
    private static class Install implements Callable<Void> {
        private final Computer c;
        private final TaskListener listener;
        Install(Computer c, TaskListener listener) {
            this.c = c;
            this.listener = listener;
        }
        @Override
        public Void call() throws Exception {
            install(c, listener);
            return null;
        }
    }

    private static void install(Computer c, TaskListener listener) {
        try {
            final List<SlaveRestarter> restarters = new ArrayList<SlaveRestarter>(SlaveRestarter.all());

            VirtualChannel ch = c.getChannel();
            if (ch==null) return;  
            List<SlaveRestarter> effective = ch.call(new FindEffectiveRestarters(restarters));

            LOGGER.log(FINE, "Effective SlaveRestarter on {0}: {1}", new Object[] {c.getName(), effective});
        } catch (Throwable e) {
            Functions.printStackTrace(e, listener.error("Failed to install restarter"));
        }
    }
    private static class FindEffectiveRestarters extends MasterToSlaveCallable<List<SlaveRestarter>, IOException> {
        private final List<SlaveRestarter> restarters;
        FindEffectiveRestarters(List<SlaveRestarter> restarters) {
            this.restarters = restarters;
        }
        @Override
        public List<SlaveRestarter> call() throws IOException {
            Engine e = Engine.current();
            if (e == null) return null;    
            try {
                Engine.class.getMethod("addListener", EngineListener.class);
            } catch (NoSuchMethodException _) {
                return null;                }

                        for (Iterator<SlaveRestarter> itr = restarters.iterator(); itr.hasNext(); ) {
                SlaveRestarter r =  itr.next();
                if (!r.canWork())
                    itr.remove();
            }

            e.addListener(new EngineListenerAdapter() {
                @Override
                public void onReconnect() {
                    try {
                        for (SlaveRestarter r : restarters) {
                            try {
                                LOGGER.info("Restarting agent via "+r);
                                r.restart();
                            } catch (Exception x) {
                                LOGGER.log(SEVERE, "Failed to restart agent with "+r, x);
                            }
                        }
                    } finally {
                                                                        restarters.clear();
                    }
                }
            });

            return restarters;
        }
    }

    private static final Logger LOGGER = Logger.getLogger(JnlpSlaveRestarterInstaller.class.getName());

    private static final long serialVersionUID = 1L;
}
