package jenkins.security;

import hudson.Extension;
import hudson.Lookup;
import hudson.init.InitMilestone;
import hudson.util.Secret;
import jenkins.model.Jenkins;
import org.kohsuke.MetaInfServices;

import javax.annotation.CheckForNull;
import javax.annotation.Nonnull;
import java.io.IOException;
import java.security.SecureRandom;
import java.util.Iterator;
import java.util.ServiceConfigurationError;
import java.util.ServiceLoader;
import java.util.logging.Level;
import java.util.logging.Logger;


public abstract class ConfidentialStore {
    
    protected abstract void store(ConfidentialKey key, byte[] payload) throws IOException;

    
    protected abstract @CheckForNull byte[] load(ConfidentialKey key) throws IOException;

    
    public abstract byte[] randomBytes(int size);

    
    public static @Nonnull ConfidentialStore get() {
        if (TEST!=null) return TEST.get();

        Jenkins j = Jenkins.getInstance();
        Lookup lookup = j.lookup;
        ConfidentialStore cs = lookup.get(ConfidentialStore.class);
        if (cs==null) {
            try {
                Iterator<ConfidentialStore> it = ServiceLoader.load(ConfidentialStore.class, ConfidentialStore.class.getClassLoader()).iterator();
                if (it.hasNext()) {
                    cs = it.next();
                }
            } catch (ServiceConfigurationError e) {
                LOGGER.log(Level.WARNING, "Failed to list up ConfidentialStore implementations",e);
                            }

            if (cs==null)
                try {
                    cs = new DefaultConfidentialStore();
                } catch (Exception e) {
                                        throw new Error(e);
                }

            cs = lookup.setIfNull(ConfidentialStore.class,cs);
        }
        return cs;
    }

    
     static ThreadLocal<ConfidentialStore> TEST = null;

    private static final Logger LOGGER = Logger.getLogger(ConfidentialStore.class.getName());
}
