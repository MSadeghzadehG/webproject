package hudson.node_monitors;

import hudson.Extension;
import hudson.model.Computer;
import hudson.model.TaskListener;
import hudson.slaves.ComputerListener;
import hudson.util.Futures;
import jenkins.model.Jenkins;

import java.io.IOException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import jenkins.util.Timer;


@Extension
public class NodeMonitorUpdater extends ComputerListener {

    private static final Runnable MONITOR_UPDATER = new Runnable() {
        @Override
        public void run() {
            for (NodeMonitor nm : Jenkins.getInstance().getComputer().getMonitors()) {
                nm.triggerUpdate();
            }
        }
    };

    private Future<?> future = Futures.precomputed(null);

    
    @Override
    public void onOnline(Computer c, TaskListener listener) throws IOException, InterruptedException {
        synchronized(this) {
            future.cancel(false);
            future = Timer.get().schedule(MONITOR_UPDATER, 5, TimeUnit.SECONDS);
        }
    }
}
