

package org.elasticsearch.repositories.azure;

import com.microsoft.azure.storage.StorageException;
import org.elasticsearch.SpecialPermission;

import java.io.IOException;
import java.net.SocketPermission;
import java.net.URISyntaxException;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;


public final class SocketAccess {

    private SocketAccess() {}

    public static <T> T doPrivilegedIOException(PrivilegedExceptionAction<T> operation) throws IOException {
        SpecialPermission.check();
        try {
            return AccessController.doPrivileged(operation);
        } catch (PrivilegedActionException e) {
            throw (IOException) e.getCause();
        }
    }

    public static <T> T doPrivilegedException(PrivilegedExceptionAction<T> operation) throws StorageException, URISyntaxException {
        SpecialPermission.check();
        try {
            return AccessController.doPrivileged(operation);
        } catch (PrivilegedActionException e) {
            throw (StorageException) e.getCause();
        }
    }

    public static void doPrivilegedVoidException(StorageRunnable action) throws StorageException, URISyntaxException {
        SpecialPermission.check();
        try {
            AccessController.doPrivileged((PrivilegedExceptionAction<Void>) () -> {
                action.executeCouldThrow();
                return null;
            });
        } catch (PrivilegedActionException e) {
            Throwable cause = e.getCause();
            if (cause instanceof StorageException) {
                throw (StorageException) cause;
            } else {
                throw (URISyntaxException) cause;
            }
        }
    }

    @FunctionalInterface
    public interface StorageRunnable {
        void executeCouldThrow() throws StorageException, URISyntaxException, IOException;
    }

}
