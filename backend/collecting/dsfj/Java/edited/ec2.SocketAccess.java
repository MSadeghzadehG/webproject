

package org.elasticsearch.discovery.ec2;

import org.elasticsearch.SpecialPermission;

import java.io.IOException;
import java.net.SocketPermission;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;


final class SocketAccess {

    private SocketAccess() {}

    public static <T> T doPrivileged(PrivilegedAction<T> operation) {
        SpecialPermission.check();
        return AccessController.doPrivileged(operation);
    }

    public static <T> T doPrivilegedIOException(PrivilegedExceptionAction<T> operation) throws IOException {
        SpecialPermission.check();
        try {
            return AccessController.doPrivileged(operation);
        } catch (PrivilegedActionException e) {
            throw (IOException) e.getCause();
        }
    }

}
