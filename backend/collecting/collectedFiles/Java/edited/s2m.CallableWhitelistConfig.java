package jenkins.security.s2m;

import com.google.common.collect.ImmutableSet;
import hudson.Util;
import org.jenkinsci.remoting.RoleSensitive;

import java.io.File;
import java.util.HashSet;
import java.util.Set;


class CallableWhitelistConfig extends ConfigDirectory<String,Set<String>> {
    CallableWhitelistConfig(File file) {
        super(file);
    }

    @Override
    protected Set<String> create() {
        return new HashSet<String>();
    }

    @Override
    protected Set<String> readOnly(Set<String> base) {
        return ImmutableSet.copyOf(base);
    }

    @Override
    protected String parse(String line) {
        return Util.fixEmptyAndTrim(line);
    }

    public boolean contains(String name) {
        return get().contains(name);
    }
}
