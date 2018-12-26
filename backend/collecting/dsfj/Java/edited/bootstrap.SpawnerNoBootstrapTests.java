

package org.elasticsearch.bootstrap;

import org.apache.lucene.util.Constants;
import org.apache.lucene.util.LuceneTestCase;
import org.elasticsearch.Version;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.env.TestEnvironment;
import org.elasticsearch.plugins.PluginTestUtil;
import org.elasticsearch.plugins.Platforms;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystemException;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.Path;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.function.Function;

import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.hasSize;
import static org.hamcrest.Matchers.hasToString;
import static org.hamcrest.Matchers.instanceOf;


public class SpawnerNoBootstrapTests extends LuceneTestCase {

    private static final String CONTROLLER_SOURCE = "#!/bin/bash\n"
            + "\n"
            + "echo I am alive\n"
            + "\n"
            + "read SOMETHING\n";

    
    public void testNoControllerSpawn() throws IOException, InterruptedException {
        Path esHome = createTempDir().resolve("esHome");
        Settings.Builder settingsBuilder = Settings.builder();
        settingsBuilder.put(Environment.PATH_HOME_SETTING.getKey(), esHome.toString());
        Settings settings = settingsBuilder.build();

        Environment environment = TestEnvironment.newEnvironment(settings);

                Path plugin = environment.pluginsFile().resolve("a_plugin");
        Files.createDirectories(environment.modulesFile());
        Files.createDirectories(plugin);
        PluginTestUtil.writePluginProperties(
                plugin,
                "description", "a_plugin",
                "version", Version.CURRENT.toString(),
                "elasticsearch.version", Version.CURRENT.toString(),
                "name", "a_plugin",
                "java.version", "1.8",
                "classname", "APlugin",
                "has.native.controller", "false");

        try (Spawner spawner = new Spawner()) {
            spawner.spawnNativePluginControllers(environment);
            assertThat(spawner.getProcesses(), hasSize(0));
        }
    }

    
    public void testControllerSpawn() throws Exception {
        assertControllerSpawns(Environment::pluginsFile);
        assertControllerSpawns(Environment::modulesFile);
    }

    private void assertControllerSpawns(Function<Environment, Path> pluginsDirFinder) throws Exception {
        
        assumeFalse("This test does not work on Windows", Constants.WINDOWS);

        Path esHome = createTempDir().resolve("esHome");
        Settings.Builder settingsBuilder = Settings.builder();
        settingsBuilder.put(Environment.PATH_HOME_SETTING.getKey(), esHome.toString());
        Settings settings = settingsBuilder.build();

        Environment environment = TestEnvironment.newEnvironment(settings);

                Path plugin = pluginsDirFinder.apply(environment).resolve("test_plugin");
        Files.createDirectories(environment.modulesFile());
        Files.createDirectories(environment.pluginsFile());
        Files.createDirectories(plugin);
        PluginTestUtil.writePluginProperties(
            plugin,
            "description", "test_plugin",
            "version", Version.CURRENT.toString(),
            "elasticsearch.version", Version.CURRENT.toString(),
            "name", "test_plugin",
            "java.version", "1.8",
            "classname", "TestPlugin",
            "has.native.controller", "true");
        Path controllerProgram = Platforms.nativeControllerPath(plugin);
        createControllerProgram(controllerProgram);

                Path otherPlugin = pluginsDirFinder.apply(environment).resolve("other_plugin");
        Files.createDirectories(otherPlugin);
        PluginTestUtil.writePluginProperties(
            otherPlugin,
            "description", "other_plugin",
            "version", Version.CURRENT.toString(),
            "elasticsearch.version", Version.CURRENT.toString(),
            "name", "other_plugin",
            "java.version", "1.8",
            "classname", "OtherPlugin",
            "has.native.controller", "false");

        Spawner spawner = new Spawner();
        spawner.spawnNativePluginControllers(environment);

        List<Process> processes = spawner.getProcesses();
        
        assertThat(processes, hasSize(1));
        Process process = processes.get(0);
        final InputStreamReader in =
            new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8);
        try (BufferedReader stdoutReader = new BufferedReader(in)) {
            String line = stdoutReader.readLine();
            assertEquals("I am alive", line);
            spawner.close();
            
            assertTrue(process.waitFor(1, TimeUnit.SECONDS));
        }
    }

    
    public void testControllerSpawnMetaPlugin() throws IOException, InterruptedException {
        
        assumeFalse("This test does not work on Windows", Constants.WINDOWS);

        Path esHome = createTempDir().resolve("esHome");
        Settings.Builder settingsBuilder = Settings.builder();
        settingsBuilder.put(Environment.PATH_HOME_SETTING.getKey(), esHome.toString());
        Settings settings = settingsBuilder.build();

        Environment environment = TestEnvironment.newEnvironment(settings);

        Path metaPlugin = environment.pluginsFile().resolve("meta_plugin");
        Files.createDirectories(environment.modulesFile());
        Files.createDirectories(metaPlugin);
        PluginTestUtil.writeMetaPluginProperties(
            metaPlugin,
            "description", "test_plugin",
            "name", "meta_plugin",
            "plugins", "test_plugin,other_plugin");

                Path plugin = metaPlugin.resolve("test_plugin");

        Files.createDirectories(plugin);
        PluginTestUtil.writePluginProperties(
            plugin,
            "description", "test_plugin",
            "version", Version.CURRENT.toString(),
            "elasticsearch.version", Version.CURRENT.toString(),
            "name", "test_plugin",
            "java.version", "1.8",
            "classname", "TestPlugin",
            "has.native.controller", "true");
        Path controllerProgram = Platforms.nativeControllerPath(plugin);
        createControllerProgram(controllerProgram);

                Path otherPlugin = metaPlugin.resolve("other_plugin");
        Files.createDirectories(otherPlugin);
        PluginTestUtil.writePluginProperties(
            otherPlugin,
            "description", "other_plugin",
            "version", Version.CURRENT.toString(),
            "elasticsearch.version", Version.CURRENT.toString(),
            "name", "other_plugin",
            "java.version", "1.8",
            "classname", "OtherPlugin",
            "has.native.controller", "false");

        Spawner spawner = new Spawner();
        spawner.spawnNativePluginControllers(environment);

        List<Process> processes = spawner.getProcesses();
        
        assertThat(processes, hasSize(1));
        Process process = processes.get(0);
        final InputStreamReader in =
            new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8);
        try (BufferedReader stdoutReader = new BufferedReader(in)) {
            String line = stdoutReader.readLine();
            assertEquals("I am alive", line);
            spawner.close();
            
            assertTrue(process.waitFor(1, TimeUnit.SECONDS));
        }
    }

    public void testControllerSpawnWithIncorrectDescriptor() throws IOException {
                Path esHome = createTempDir().resolve("esHome");
        Settings.Builder settingsBuilder = Settings.builder();
        settingsBuilder.put(Environment.PATH_HOME_SETTING.getKey(), esHome.toString());
        Settings settings = settingsBuilder.build();

        Environment environment = TestEnvironment.newEnvironment(settings);

        Path plugin = environment.pluginsFile().resolve("test_plugin");
        Files.createDirectories(plugin);
        PluginTestUtil.writePluginProperties(
                plugin,
                "description", "test_plugin",
                "version", Version.CURRENT.toString(),
                "elasticsearch.version", Version.CURRENT.toString(),
                "name", "test_plugin",
                "java.version", "1.8",
                "classname", "TestPlugin",
                "has.native.controller", "false");
        Path controllerProgram = Platforms.nativeControllerPath(plugin);
        createControllerProgram(controllerProgram);

        Spawner spawner = new Spawner();
        IllegalArgumentException e = expectThrows(
                IllegalArgumentException.class,
                () -> spawner.spawnNativePluginControllers(environment));
        assertThat(
                e.getMessage(),
                equalTo("plugin [test_plugin] does not have permission to fork native controller"));
    }

    public void testSpawnerHandlingOfDesktopServicesStoreFiles() throws IOException {
        final Path esHome = createTempDir().resolve("home");
        final Settings settings = Settings.builder().put(Environment.PATH_HOME_SETTING.getKey(), esHome.toString()).build();

        final Environment environment = TestEnvironment.newEnvironment(settings);

        Files.createDirectories(environment.modulesFile());
        Files.createDirectories(environment.pluginsFile());

        final Path desktopServicesStore = environment.pluginsFile().resolve(".DS_Store");
        Files.createFile(desktopServicesStore);

        final Spawner spawner = new Spawner();
        if (Constants.MAC_OS_X) {
                        spawner.spawnNativePluginControllers(environment);
        } else {
                        final FileSystemException e =
                    expectThrows(FileSystemException.class, () -> spawner.spawnNativePluginControllers(environment));
            if (Constants.WINDOWS) {
                assertThat(e, instanceOf(NoSuchFileException.class));
            } else {
                assertThat(e, hasToString(containsString("Not a directory")));
            }
        }
    }

    private void createControllerProgram(final Path outputFile) throws IOException {
        final Path outputDir = outputFile.getParent();
        Files.createDirectories(outputDir);
        Files.write(outputFile, CONTROLLER_SOURCE.getBytes(StandardCharsets.UTF_8));
        final PosixFileAttributeView view =
                Files.getFileAttributeView(outputFile, PosixFileAttributeView.class);
        if (view != null) {
            final Set<PosixFilePermission> perms = new HashSet<>();
            perms.add(PosixFilePermission.OWNER_READ);
            perms.add(PosixFilePermission.OWNER_WRITE);
            perms.add(PosixFilePermission.OWNER_EXECUTE);
            perms.add(PosixFilePermission.GROUP_READ);
            perms.add(PosixFilePermission.GROUP_EXECUTE);
            perms.add(PosixFilePermission.OTHERS_READ);
            perms.add(PosixFilePermission.OTHERS_EXECUTE);
            Files.setPosixFilePermissions(outputFile, perms);
        }
    }

}
