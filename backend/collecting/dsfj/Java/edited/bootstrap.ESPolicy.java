

package org.elasticsearch.bootstrap;

import org.elasticsearch.common.SuppressForbidden;

import java.io.FilePermission;
import java.io.IOException;
import java.net.SocketPermission;
import java.net.URL;
import java.security.CodeSource;
import java.security.Permission;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.security.Policy;
import java.security.ProtectionDomain;
import java.util.Collections;
import java.util.Map;
import java.util.Set;
import java.util.function.Predicate;


final class ESPolicy extends Policy {

    
    static final String POLICY_RESOURCE = "security.policy";
    
    static final String UNTRUSTED_RESOURCE = "untrusted.policy";

    final Policy template;
    final Policy untrusted;
    final Policy system;
    final PermissionCollection dynamic;
    final Map<String,Policy> plugins;

    ESPolicy(Map<String, URL> codebases, PermissionCollection dynamic, Map<String,Policy> plugins, boolean filterBadDefaults) {
        this.template = Security.readPolicy(getClass().getResource(POLICY_RESOURCE), codebases);
        this.untrusted = Security.readPolicy(getClass().getResource(UNTRUSTED_RESOURCE), Collections.emptyMap());
        if (filterBadDefaults) {
            this.system = new SystemPolicy(Policy.getPolicy());
        } else {
            this.system = Policy.getPolicy();
        }
        this.dynamic = dynamic;
        this.plugins = plugins;
    }

    @Override @SuppressForbidden(reason = "fast equals check is desired")
    public boolean implies(ProtectionDomain domain, Permission permission) {
        CodeSource codeSource = domain.getCodeSource();
                if (codeSource == null) {
            return false;
        }

        URL location = codeSource.getLocation();
                        if (location != null) {
                        if (BootstrapInfo.UNTRUSTED_CODEBASE.equals(location.getFile())) {
                return untrusted.implies(domain, permission);
            }
                                    Policy plugin = plugins.get(location.getFile());
            if (plugin != null && plugin.implies(domain, permission)) {
                return true;
            }
        }

                        if (permission instanceof FilePermission && "<<ALL FILES>>".equals(permission.getName())) {
            for (StackTraceElement element : Thread.currentThread().getStackTrace()) {
                if ("org.apache.hadoop.util.Shell".equals(element.getClassName()) &&
                      "runCommand".equals(element.getMethodName())) {
                                                            rethrow(new IOException("no hadoop, you cannot do this."));
                }
            }
        }

                return template.implies(domain, permission) || dynamic.implies(permission) || system.implies(domain, permission);
    }

    
    private static class Rethrower<T extends Throwable> {
        private void rethrow(Throwable t) throws T {
            throw (T) t;
        }
    }

    
    private void rethrow(Throwable t) {
        new Rethrower<Error>().rethrow(t);
    }

    @Override
    public PermissionCollection getPermissions(CodeSource codesource) {
                                for (StackTraceElement element : Thread.currentThread().getStackTrace()) {
            if ("sun.rmi.server.LoaderHandler".equals(element.getClassName()) &&
                    "loadClass".equals(element.getMethodName())) {
                return new Permissions();
            }
        }
                return super.getPermissions(codesource);
    }

    
    
    private static class BadDefaultPermission extends Permission {

        private final Permission badDefaultPermission;
        private final Predicate<Permission> preImplies;

        
        BadDefaultPermission(final Permission badDefaultPermission, final Predicate<Permission> preImplies) {
            super(badDefaultPermission.getName());
            this.badDefaultPermission = badDefaultPermission;
            this.preImplies = preImplies;
        }

        @Override
        public final boolean implies(Permission permission) {
            return preImplies.test(permission) && badDefaultPermission.implies(permission);
        }

        @Override
        public final boolean equals(Object obj) {
            return badDefaultPermission.equals(obj);
        }

        @Override
        public int hashCode() {
            return badDefaultPermission.hashCode();
        }

        @Override
        public String getActions() {
            return badDefaultPermission.getActions();
        }

    }

                        private static final Permission BAD_DEFAULT_NUMBER_ONE = new BadDefaultPermission(new RuntimePermission("stopThread"), p -> true);

                    private static final Permission BAD_DEFAULT_NUMBER_TWO =
        new BadDefaultPermission(
            new SocketPermission("localhost:0", "listen"),
                        p -> p instanceof SocketPermission && p.getActions().contains("listen"));

    
    static class SystemPolicy extends Policy {
        final Policy delegate;

        SystemPolicy(Policy delegate) {
            this.delegate = delegate;
        }

        @Override
        public boolean implies(ProtectionDomain domain, Permission permission) {
            if (BAD_DEFAULT_NUMBER_ONE.implies(permission) || BAD_DEFAULT_NUMBER_TWO.implies(permission)) {
                return false;
            }
            return delegate.implies(domain, permission);
        }
    }
}
