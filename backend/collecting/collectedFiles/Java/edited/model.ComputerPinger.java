package hudson.model;

import hudson.Extension;
import hudson.ExtensionList;
import hudson.ExtensionPoint;

import java.io.IOException;
import java.net.InetAddress;
import java.util.logging.Logger;


public abstract class ComputerPinger implements ExtensionPoint {
    
    public abstract boolean isReachable(InetAddress ia, int timeout) throws IOException;

    
    public static ExtensionList<ComputerPinger> all() {
        return ExtensionList.lookup(ComputerPinger.class);
    }

    
    public static boolean checkIsReachable(InetAddress ia, int timeout) throws IOException {
        for (ComputerPinger pinger : ComputerPinger.all()) {
            try {
                if (pinger.isReachable(ia, timeout)) {
                    return true;
                }
            } catch (IOException e) {
                LOGGER.fine("Error checking reachability with " + pinger + ": " + e.getMessage());
            }
        }

        return false;
    }
    
    
    @Extension
    public static class BuiltInComputerPinger extends ComputerPinger {
        @Override
        public boolean isReachable(InetAddress ia, int timeout) throws IOException {
            return ia.isReachable(timeout * 1000);
        }
    }

    private static final Logger LOGGER = Logger.getLogger(ComputerPinger.class.getName());
}
