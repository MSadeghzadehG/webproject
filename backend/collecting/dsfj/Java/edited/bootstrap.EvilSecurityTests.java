

package org.elasticsearch.bootstrap;

import org.apache.lucene.util.Constants;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.env.TestEnvironment;
import org.elasticsearch.test.ESTestCase;

import java.io.FilePermission;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.util.Set;

import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.hasToString;

@SuppressForbidden(reason = "modifies system properties and attempts to create symbolic links intentionally")
public class EvilSecurityTests extends ESTestCase {

    
    public void testGeneratedPermissions() throws Exception {
        Path path = createTempDir();
                Path esHome = path.resolve("esHome");
        Settings.Builder settingsBuilder = Settings.builder();
        settingsBuilder.put(Environment.PATH_HOME_SETTING.getKey(), esHome.toString());
        Settings settings = settingsBuilder.build();

        Path fakeTmpDir = createTempDir();
        String realTmpDir = System.getProperty("java.io.tmpdir");
        Permissions permissions;
        try {
            System.setProperty("java.io.tmpdir", fakeTmpDir.toString());
            Environment environment = TestEnvironment.newEnvironment(settings);
            permissions = Security.createPermissions(environment);
        } finally {
            System.setProperty("java.io.tmpdir", realTmpDir);
        }

                assertNoPermissions(esHome, permissions);
                assertNoPermissions(esHome.getParent(), permissions);
                assertNoPermissions(esHome.getParent().resolve("other"), permissions);
                assertNoPermissions(PathUtils.get(realTmpDir), permissions);
    }

    
    @SuppressWarnings("deprecation")     @SuppressForbidden(reason = "to create FilePermission object")
    public void testEnvironmentPaths() throws Exception {
        Path path = createTempDir();
                Path esHome = path.resolve("esHome");

        Settings.Builder settingsBuilder = Settings.builder();
        settingsBuilder.put(Environment.PATH_HOME_SETTING.getKey(), esHome.resolve("home").toString());
        settingsBuilder.putList(Environment.PATH_DATA_SETTING.getKey(), esHome.resolve("data1").toString(),
                esHome.resolve("data2").toString());
        settingsBuilder.put(Environment.PATH_SHARED_DATA_SETTING.getKey(), esHome.resolve("custom").toString());
        settingsBuilder.put(Environment.PATH_LOGS_SETTING.getKey(), esHome.resolve("logs").toString());
        settingsBuilder.put(Environment.PIDFILE_SETTING.getKey(), esHome.resolve("test.pid").toString());
        Settings settings = settingsBuilder.build();

        Path fakeTmpDir = createTempDir();
        String realTmpDir = System.getProperty("java.io.tmpdir");
        Permissions permissions;
        Environment environment;
        try {
            System.setProperty("java.io.tmpdir", fakeTmpDir.toString());
            environment = new Environment(settings, esHome.resolve("conf"));
            permissions = Security.createPermissions(environment);
        } finally {
            System.setProperty("java.io.tmpdir", realTmpDir);
        }

                assertNoPermissions(esHome, permissions);
                assertNoPermissions(esHome.getParent(), permissions);
                assertNoPermissions(esHome.getParent().resolve("other"), permissions);
                assertNoPermissions(PathUtils.get(realTmpDir), permissions);

        
                assertExactPermissions(new FilePermission(environment.binFile().toString(), "read,readlink"), permissions);
                assertExactPermissions(new FilePermission(environment.libFile().toString(), "read,readlink"), permissions);
                assertExactPermissions(new FilePermission(environment.modulesFile().toString(), "read,readlink"), permissions);
                assertExactPermissions(new FilePermission(environment.configFile().toString(), "read,readlink"), permissions);
                assertExactPermissions(new FilePermission(environment.pluginsFile().toString(), "read,readlink"), permissions);

                for (Path dataPath : environment.dataFiles()) {
            assertExactPermissions(new FilePermission(dataPath.toString(), "read,readlink,write,delete"), permissions);
        }
        for (Path dataPath : environment.dataWithClusterFiles()) {
            assertExactPermissions(new FilePermission(dataPath.toString(), "read,readlink,write,delete"), permissions);
        }
        assertExactPermissions(new FilePermission(environment.sharedDataFile().toString(), "read,readlink,write,delete"), permissions);
                assertExactPermissions(new FilePermission(environment.logsFile().toString(), "read,readlink,write,delete"), permissions);
                assertExactPermissions(new FilePermission(fakeTmpDir.toString(), "read,readlink,write,delete"), permissions);
                assertExactPermissions(new FilePermission(environment.pidFile().toString(), "delete"), permissions);
    }

    public void testDuplicateDataPaths() throws IOException {
        final Path path = createTempDir();
        final Path home = path.resolve("home");
        final Path data = path.resolve("data");
        final Path duplicate;
        if (randomBoolean()) {
            duplicate = data;
        } else {
            duplicate = createTempDir().toAbsolutePath().resolve("link");
            Files.createSymbolicLink(duplicate, data);
        }

        final Settings settings =
                Settings
                        .builder()
                        .put(Environment.PATH_HOME_SETTING.getKey(), home.toString())
                        .putList(Environment.PATH_DATA_SETTING.getKey(), data.toString(), duplicate.toString())
                        .build();

        final Environment environment = TestEnvironment.newEnvironment(settings);
        final IllegalStateException e = expectThrows(IllegalStateException.class, () -> Security.createPermissions(environment));
        assertThat(e, hasToString(containsString("path [" + duplicate.toRealPath() + "] is duplicated by [" + duplicate + "]")));
    }

    public void testEnsureSymlink() throws IOException {
        Path p = createTempDir();

        Path exists = p.resolve("exists");
        Files.createDirectory(exists);

                Path linkExists = p.resolve("linkExists");
        try {
            Files.createSymbolicLink(linkExists, exists);
        } catch (UnsupportedOperationException | IOException e) {
            assumeNoException("test requires filesystem that supports symbolic links", e);
        } catch (SecurityException e) {
            assumeNoException("test cannot create symbolic links with security manager enabled", e);
        }
        Security.ensureDirectoryExists(linkExists);
        Files.createTempFile(linkExists, null, null);
    }

    public void testEnsureBrokenSymlink() throws IOException {
        Path p = createTempDir();

                Path brokenLink = p.resolve("brokenLink");
        try {
            Files.createSymbolicLink(brokenLink, p.resolve("nonexistent"));
        } catch (UnsupportedOperationException | IOException e) {
            assumeNoException("test requires filesystem that supports symbolic links", e);
        } catch (SecurityException e) {
            assumeNoException("test cannot create symbolic links with security manager enabled", e);
        }
        try {
            Security.ensureDirectoryExists(brokenLink);
            fail("didn't get expected exception");
        } catch (IOException expected) {}
    }

    
    public void testSymlinkPermissions() throws IOException {
                assumeFalse("windows does not automatically grant permission to the target of symlinks", Constants.WINDOWS);
        Path dir = createTempDir();

        Path target = dir.resolve("target");
        Files.createDirectory(target);

                Path link = dir.resolve("link");
        try {
            Files.createSymbolicLink(link, target);
        } catch (UnsupportedOperationException | IOException e) {
            assumeNoException("test requires filesystem that supports symbolic links", e);
        } catch (SecurityException e) {
            assumeNoException("test cannot create symbolic links with security manager enabled", e);
        }
        Permissions permissions = new Permissions();
        FilePermissionUtils.addDirectoryPath(permissions, "testing", link, "read");
        assertExactPermissions(new FilePermission(link.toString(), "read"), permissions);
        assertExactPermissions(new FilePermission(link.resolve("foo").toString(), "read"), permissions);
        assertExactPermissions(new FilePermission(target.toString(), "read"), permissions);
        assertExactPermissions(new FilePermission(target.resolve("foo").toString(), "read"), permissions);
    }

    
    @SuppressForbidden(reason = "to create FilePermission object")
    static void assertExactPermissions(FilePermission expected, PermissionCollection actual) {
        String target = expected.getName();         Set<String> permissionSet = asSet(expected.getActions().split(","));
        boolean read = permissionSet.remove("read");
        boolean readlink = permissionSet.remove("readlink");
        boolean write = permissionSet.remove("write");
        boolean delete = permissionSet.remove("delete");
        boolean execute = permissionSet.remove("execute");
        assertTrue("unrecognized permission: " + permissionSet, permissionSet.isEmpty());
        assertEquals(read, actual.implies(new FilePermission(target, "read")));
        assertEquals(readlink, actual.implies(new FilePermission(target, "readlink")));
        assertEquals(write, actual.implies(new FilePermission(target, "write")));
        assertEquals(delete, actual.implies(new FilePermission(target, "delete")));
        assertEquals(execute, actual.implies(new FilePermission(target, "execute")));
    }

    
    @SuppressForbidden(reason = "to create FilePermission object")
    static void assertNoPermissions(Path path, PermissionCollection actual) {
        String target = path.toString();
        assertFalse(actual.implies(new FilePermission(target, "read")));
        assertFalse(actual.implies(new FilePermission(target, "readlink")));
        assertFalse(actual.implies(new FilePermission(target, "write")));
        assertFalse(actual.implies(new FilePermission(target, "delete")));
        assertFalse(actual.implies(new FilePermission(target, "execute")));
    }
}
