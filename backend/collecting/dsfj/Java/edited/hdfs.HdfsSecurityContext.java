
package org.elasticsearch.repositories.hdfs;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.lang.reflect.ReflectPermission;
import java.net.SocketPermission;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.AccessController;
import java.security.Permission;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import java.util.Arrays;
import javax.security.auth.AuthPermission;
import javax.security.auth.PrivateCredentialPermission;
import javax.security.auth.kerberos.ServicePermission;

import org.apache.hadoop.security.UserGroupInformation;
import org.elasticsearch.SpecialPermission;
import org.elasticsearch.env.Environment;


class HdfsSecurityContext {

    private static final Permission[] SIMPLE_AUTH_PERMISSIONS;
    private static final Permission[] KERBEROS_AUTH_PERMISSIONS;
    static {
                SIMPLE_AUTH_PERMISSIONS = new Permission[]{
            new SocketPermission("*", "connect"),
                        new ReflectPermission("suppressAccessChecks"),
                        new AuthPermission("modifyPrivateCredentials"),
                        new PrivateCredentialPermission("org.apache.hadoop.security.Credentials * \"*\"", "read")
        };

                KERBEROS_AUTH_PERMISSIONS = new Permission[] {
            new SocketPermission("*", "connect"),
                        new ReflectPermission("suppressAccessChecks"),
                        new AuthPermission("modifyPrivateCredentials"),
                        new AuthPermission("doAs"),
                        new SocketPermission("localhost:0", "listen,resolve"),
                                    new RuntimePermission("getClassLoader"),
            new RuntimePermission("setContextClassLoader"),
                        new AuthPermission("modifyPrincipals"),
            new PrivateCredentialPermission("org.apache.hadoop.security.Credentials * \"*\"", "read"),
            new PrivateCredentialPermission("javax.security.auth.kerberos.KerberosTicket * \"*\"", "read"),
            new PrivateCredentialPermission("javax.security.auth.kerberos.KeyTab * \"*\"", "read")
                                            };
    }

    
    static Path locateKeytabFile(Environment environment) {
        Path keytabPath = environment.configFile().resolve("repository-hdfs").resolve("krb5.keytab");
        try {
            if (Files.exists(keytabPath) == false) {
                throw new RuntimeException("Could not locate keytab at [" + keytabPath + "].");
            }
        } catch (SecurityException se) {
            throw new RuntimeException("Could not locate keytab at [" + keytabPath + "]", se);
        }
        return keytabPath;
    }

    private final UserGroupInformation ugi;
    private final boolean restrictPermissions;
    private final Permission[] restrictedExecutionPermissions;

    HdfsSecurityContext(UserGroupInformation ugi, boolean restrictPermissions) {
        this.ugi = ugi;
        this.restrictPermissions = restrictPermissions;
        this.restrictedExecutionPermissions = renderPermissions(ugi);
    }

    private Permission[] renderPermissions(UserGroupInformation ugi) {
        Permission[] permissions;
        if (ugi.isFromKeytab()) {
                                    int permlen = KERBEROS_AUTH_PERMISSIONS.length + 1;
            permissions = new Permission[permlen];

            System.arraycopy(KERBEROS_AUTH_PERMISSIONS, 0, permissions, 0, KERBEROS_AUTH_PERMISSIONS.length);

                                    permissions[permissions.length - 1] = new ServicePermission(ugi.getUserName(), "initiate");
        } else {
                        permissions = Arrays.copyOf(SIMPLE_AUTH_PERMISSIONS, SIMPLE_AUTH_PERMISSIONS.length);
        }
        return permissions;
    }

    private Permission[] getRestrictedExecutionPermissions() {
        return restrictedExecutionPermissions;
    }

    <T> T doPrivilegedOrThrow(PrivilegedExceptionAction<T> action) throws IOException {
        SpecialPermission.check();
        try {
            if (restrictPermissions) {
                return AccessController.doPrivileged(action, null, this.getRestrictedExecutionPermissions());
            } else {
                return AccessController.doPrivileged(action);
            }
        } catch (PrivilegedActionException e) {
            throw (IOException) e.getCause();
        }
    }

    void ensureLogin() {
        if (ugi.isFromKeytab()) {
            try {
                ugi.checkTGTAndReloginFromKeytab();
            } catch (IOException ioe) {
                throw new UncheckedIOException("Could not re-authenticate", ioe);
            }
        }
    }
}
