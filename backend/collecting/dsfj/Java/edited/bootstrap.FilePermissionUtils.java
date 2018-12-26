

package org.elasticsearch.bootstrap;

import org.elasticsearch.common.SuppressForbidden;

import java.io.FilePermission;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.Permissions;

public class FilePermissionUtils {

    
    private FilePermissionUtils() {}

    private static final boolean VERSION_IS_AT_LEAST_JAVA_9 = JavaVersion.current().compareTo(JavaVersion.parse("9")) >= 0;

    
    @SuppressForbidden(reason = "only place where creating Java-9 compatible FilePermission objects is possible")
    public static void addSingleFilePath(Permissions policy, Path path, String permissions) throws IOException {
        policy.add(new FilePermission(path.toString(), permissions));
        if (VERSION_IS_AT_LEAST_JAVA_9 && Files.exists(path)) {
                                    Path realPath = path.toRealPath();
            if (path.toString().equals(realPath.toString()) == false) {
                policy.add(new FilePermission(realPath.toString(), permissions));
            }
        }
    }

    
    @SuppressForbidden(reason = "only place where creating Java-9 compatible FilePermission objects is possible")
    public static void addDirectoryPath(Permissions policy, String configurationName, Path path, String permissions) throws IOException {
                try {
            Security.ensureDirectoryExists(path);
        } catch (IOException e) {
            throw new IllegalStateException("Unable to access '" + configurationName + "' (" + path + ")", e);
        }

                policy.add(new FilePermission(path.toString(), permissions));
        policy.add(new FilePermission(path.toString() + path.getFileSystem().getSeparator() + "-", permissions));
        if (VERSION_IS_AT_LEAST_JAVA_9) {
                                    Path realPath = path.toRealPath();
            if (path.toString().equals(realPath.toString()) == false) {
                policy.add(new FilePermission(realPath.toString(), permissions));
                policy.add(new FilePermission(realPath.toString() + realPath.getFileSystem().getSeparator() + "-", permissions));
            }
        }
    }
}
