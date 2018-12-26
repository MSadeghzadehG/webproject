

package org.elasticsearch.bootstrap;

import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.test.ESTestCase;

import java.io.FilePermission;
import java.net.SocketPermission;
import java.security.AllPermission;
import java.security.CodeSource;
import java.security.Permission;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.security.ProtectionDomain;
import java.security.cert.Certificate;
import java.util.Collections;


public class ESPolicyUnitTests extends ESTestCase {
    
    @SuppressForbidden(reason = "to create FilePermission object")
    public void testNullCodeSource() throws Exception {
        assumeTrue("test cannot run with security manager", System.getSecurityManager() == null);
                Permission all = new AllPermission();
        PermissionCollection allCollection = all.newPermissionCollection();
        allCollection.add(all);
        ESPolicy policy = new ESPolicy(Collections.emptyMap(), allCollection, Collections.emptyMap(), true);
                PermissionCollection noPermissions = new Permissions();
        assertFalse(policy.implies(new ProtectionDomain(null, noPermissions), new FilePermission("foo", "read")));
    }

    
    @SuppressForbidden(reason = "to create FilePermission object")
    public void testNullLocation() throws Exception {
        assumeTrue("test cannot run with security manager", System.getSecurityManager() == null);
        PermissionCollection noPermissions = new Permissions();
        ESPolicy policy = new ESPolicy(Collections.emptyMap(), noPermissions, Collections.emptyMap(), true);
        assertFalse(policy.implies(new ProtectionDomain(new CodeSource(null, (Certificate[]) null), noPermissions),
                new FilePermission("foo", "read")));
    }

    public void testListen() {
        assumeTrue("test cannot run with security manager", System.getSecurityManager() == null);
        final PermissionCollection noPermissions = new Permissions();
        final ESPolicy policy = new ESPolicy(Collections.emptyMap(), noPermissions, Collections.emptyMap(), true);
        assertFalse(
            policy.implies(
                new ProtectionDomain(ESPolicyUnitTests.class.getProtectionDomain().getCodeSource(), noPermissions),
                new SocketPermission("localhost:" + randomFrom(0, randomIntBetween(49152, 65535)), "listen")));
    }

}
