

package org.elasticsearch.cloud.gce.util;

import org.elasticsearch.SpecialPermission;

import java.io.IOException;
import java.net.SocketPermission;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;


public final class Access {

    private Access() {}

    public static <T> T doPrivileged(final PrivilegedAction<T> operation) {
        SpecialPermission.check();
        return AccessController.doPrivileged(operation);
    }

    public static void doPrivilegedVoid(final Runnable action) {
        SpecialPermission.check();
        AccessController.doPrivileged((PrivilegedAction<Void>) () -> {
            action.run();
            return null;
        });
    }

    public static <T> T doPrivilegedIOException(final PrivilegedExceptionAction<T> operation)
            throws IOException {
        SpecialPermission.check();
        try {
            return AccessController.doPrivileged(operation);
        } catch (final PrivilegedActionException e) {
            throw (IOException) e.getCause();
        }
    }

}
