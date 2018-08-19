
package org.elasticsearch.test;

import java.util.EnumSet;
import java.util.Set;
import org.junit.Assert;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;


public class PosixPermissionsResetter implements AutoCloseable {
    private final PosixFileAttributeView attributeView;
    private final Set<PosixFilePermission> permissions;
    public PosixPermissionsResetter(Path path) throws IOException {
        attributeView = Files.getFileAttributeView(path, PosixFileAttributeView.class);
        Assert.assertNotNull(attributeView);
        permissions = attributeView.readAttributes().permissions();
    }
    @Override
    public void close() throws IOException {
        attributeView.setPermissions(permissions);
    }
    public void setPermissions(Set<PosixFilePermission> newPermissions) throws IOException {
        attributeView.setPermissions(newPermissions);
    }

    public Set<PosixFilePermission> getCopyPermissions() {
        return EnumSet.copyOf(permissions);
    }
}
