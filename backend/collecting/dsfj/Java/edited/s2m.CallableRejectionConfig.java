package jenkins.security.s2m;

import com.google.common.collect.ImmutableSet;
import jenkins.model.Jenkins;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;


public class CallableRejectionConfig extends ConfigFile<Class,Set<Class>> {
    private final CallableWhitelistConfig whitelist;

    CallableRejectionConfig(File file, CallableWhitelistConfig whitelist) {
        super(file);
        this.whitelist = whitelist;
    }

    @Override
    protected Set<Class> create() {
        return new HashSet<Class>();
    }

    @Override
    protected Set<Class> readOnly(Set<Class> base) {
        return ImmutableSet.copyOf(base);
    }

    @Override
    protected Class parse(String line) {
        try {
            line = line.trim();
            if (whitelist.contains(line))   return null;    
            return Jenkins.getInstance().pluginManager.uberClassLoader.loadClass(line);
        } catch (ClassNotFoundException e) {
                        return null;
        }
    }

    
    void report(Class c) {
        if (!get().contains(c)) {
            try {
                append(c.getName());
            } catch (IOException e) {
                LOGGER.log(Level.WARNING, "Failed to persist " + file, e);
            }
        }
    }

    
    public List<RejectedCallable> describe() {
        List<RejectedCallable> l = new ArrayList<RejectedCallable>();
        for (Class c : get()) {
            if (!whitelist.contains(c.getName()))
                l.add(new RejectedCallable(c));
        }
        return l;
    }


    private static final Logger LOGGER = Logger.getLogger(CallableRejectionConfig.class.getName());
}
