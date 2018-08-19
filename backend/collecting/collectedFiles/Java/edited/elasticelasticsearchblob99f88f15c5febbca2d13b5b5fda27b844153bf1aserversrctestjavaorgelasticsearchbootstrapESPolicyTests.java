

package org.elasticsearch.bootstrap;

import org.elasticsearch.test.ESTestCase;

import java.security.AccessControlContext;
import java.security.AccessController;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.security.PrivilegedAction;
import java.security.ProtectionDomain;


public class ESPolicyTests extends ESTestCase {

    
    public void testRestrictPrivileges() {
        assumeTrue("test requires security manager", System.getSecurityManager() != null);
        try {
            System.getProperty("user.home");
        } catch (SecurityException e) {
            fail("this test needs to be fixed: user.home not available by policy");
        }

        PermissionCollection noPermissions = new Permissions();
        AccessControlContext noPermissionsAcc = new AccessControlContext(
            new ProtectionDomain[] {
                new ProtectionDomain(null, noPermissions)
            }
        );
        try {
            AccessController.doPrivileged(new PrivilegedAction<Void>() {
                public Void run() {
                    System.getProperty("user.home");
                    fail("access should have been denied");
                    return null;
                }
            }, noPermissionsAcc);
        } catch (SecurityException expected) {
                    }
    }
}
