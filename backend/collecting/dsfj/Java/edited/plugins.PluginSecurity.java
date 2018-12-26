

package org.elasticsearch.plugins;

import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.cli.ExitCodes;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.cli.Terminal.Verbosity;
import org.elasticsearch.cli.UserException;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.NoSuchAlgorithmException;
import java.security.Permission;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.security.Policy;
import java.security.URIParameter;
import java.security.UnresolvedPermission;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;
import java.util.function.Supplier;
import java.util.stream.Collectors;

class PluginSecurity {

    
    static void confirmPolicyExceptions(Terminal terminal, Set<String> permissions,
                                        boolean needsNativeController, boolean batch) throws UserException {
        List<String> requested = new ArrayList<>(permissions);
        if (requested.isEmpty()) {
            terminal.println(Verbosity.VERBOSE, "plugin has a policy file with no additional permissions");
        } else {

                        Collections.sort(requested);

            terminal.println(Verbosity.NORMAL, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
            terminal.println(Verbosity.NORMAL, "@     WARNING: plugin requires additional permissions     @");
            terminal.println(Verbosity.NORMAL, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
                        for (String permission : requested) {
                terminal.println(Verbosity.NORMAL, "* " + permission);
            }
            terminal.println(Verbosity.NORMAL, "See http:            terminal.println(Verbosity.NORMAL, "for descriptions of what these permissions allow and the associated risks.");
            prompt(terminal, batch);
        }

        if (needsNativeController) {
            terminal.println(Verbosity.NORMAL, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
            terminal.println(Verbosity.NORMAL, "@        WARNING: plugin forks a native controller        @");
            terminal.println(Verbosity.NORMAL, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
            terminal.println(Verbosity.NORMAL, "This plugin launches a native controller that is not subject to the Java");
            terminal.println(Verbosity.NORMAL, "security manager nor to system call filters.");
            prompt(terminal, batch);
        }
    }

    private static void prompt(final Terminal terminal, final boolean batch) throws UserException {
        if (!batch) {
            terminal.println(Verbosity.NORMAL, "");
            String text = terminal.readText("Continue with installation? [y/N]");
            if (!text.equalsIgnoreCase("y")) {
                throw new UserException(ExitCodes.DATA_ERROR, "installation aborted by user");
            }
        }
    }

    
    static String formatPermission(Permission permission) {
        StringBuilder sb = new StringBuilder();

        String clazz = null;
        if (permission instanceof UnresolvedPermission) {
            clazz = ((UnresolvedPermission) permission).getUnresolvedType();
        } else {
            clazz = permission.getClass().getName();
        }
        sb.append(clazz);

        String name = null;
        if (permission instanceof UnresolvedPermission) {
            name = ((UnresolvedPermission) permission).getUnresolvedName();
        } else {
            name = permission.getName();
        }
        if (name != null && name.length() > 0) {
            sb.append(' ');
            sb.append(name);
        }

        String actions = null;
        if (permission instanceof UnresolvedPermission) {
            actions = ((UnresolvedPermission) permission).getUnresolvedActions();
        } else {
            actions = permission.getActions();
        }
        if (actions != null && actions.length() > 0) {
            sb.append(' ');
            sb.append(actions);
        }
        return sb.toString();
    }

    
    public static Set<String> parsePermissions(Path file, Path tmpDir) throws IOException {
                                
        Path emptyPolicyFile = Files.createTempFile(tmpDir, "empty", "tmp");
        final Policy emptyPolicy;
        try {
            emptyPolicy = Policy.getInstance("JavaPolicy", new URIParameter(emptyPolicyFile.toUri()));
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
        IOUtils.rm(emptyPolicyFile);

                final Policy policy;
        try {
            policy = Policy.getInstance("JavaPolicy", new URIParameter(file.toUri()));
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
        PermissionCollection permissions = policy.getPermissions(PluginSecurity.class.getProtectionDomain());
                if (permissions == Policy.UNSUPPORTED_EMPTY_COLLECTION) {
            throw new UnsupportedOperationException("JavaPolicy implementation does not support retrieving permissions");
        }
        PermissionCollection actualPermissions = new Permissions();
        for (Permission permission : Collections.list(permissions.elements())) {
            if (!emptyPolicy.implies(PluginSecurity.class.getProtectionDomain(), permission)) {
                actualPermissions.add(permission);
            }
        }
        return Collections.list(actualPermissions.elements()).stream().map(PluginSecurity::formatPermission).collect(Collectors.toSet());
    }
}
