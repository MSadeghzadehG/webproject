

package org.elasticsearch.env;

import org.elasticsearch.cluster.ClusterName;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Setting.Property;
import org.elasticsearch.common.settings.Settings;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.file.FileStore;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.function.Function;


@SuppressForbidden(reason = "configures paths for the system")
public class Environment {

    private static final Path[] EMPTY_PATH_ARRAY = new Path[0];

    public static final Setting<String> PATH_HOME_SETTING = Setting.simpleString("path.home", Property.NodeScope);
    public static final Setting<List<String>> PATH_DATA_SETTING =
            Setting.listSetting("path.data", Collections.emptyList(), Function.identity(), Property.NodeScope);
    public static final Setting<String> PATH_LOGS_SETTING =
            new Setting<>("path.logs", "", Function.identity(), Property.NodeScope);
    public static final Setting<List<String>> PATH_REPO_SETTING =
        Setting.listSetting("path.repo", Collections.emptyList(), Function.identity(), Property.NodeScope);
    public static final Setting<String> PATH_SHARED_DATA_SETTING = Setting.simpleString("path.shared_data", Property.NodeScope);
    public static final Setting<String> PIDFILE_SETTING = Setting.simpleString("pidfile", Property.NodeScope);

    private final Settings settings;

    private final Path[] dataFiles;

    private final Path[] dataWithClusterFiles;

    private final Path[] repoFiles;

    private final Path configFile;

    private final Path pluginsFile;

    private final Path modulesFile;

    private final Path sharedDataFile;

    
    private final Path binFile;

    
    private final Path libFile;

    private final Path logsFile;

    
    private final Path pidFile;

    
    private final Path tmpFile;

    public Environment(final Settings settings, final Path configPath) {
        this(settings, configPath, PathUtils.get(System.getProperty("java.io.tmpdir")));
    }

        Environment(final Settings settings, final Path configPath, final Path tmpPath) {
        final Path homeFile;
        if (PATH_HOME_SETTING.exists(settings)) {
            homeFile = PathUtils.get(PATH_HOME_SETTING.get(settings)).normalize();
        } else {
            throw new IllegalStateException(PATH_HOME_SETTING.getKey() + " is not configured");
        }

        if (configPath != null) {
            configFile = configPath.normalize();
        } else {
            configFile = homeFile.resolve("config");
        }

        tmpFile = Objects.requireNonNull(tmpPath);

        pluginsFile = homeFile.resolve("plugins");

        List<String> dataPaths = PATH_DATA_SETTING.get(settings);
        final ClusterName clusterName = ClusterName.CLUSTER_NAME_SETTING.get(settings);
        if (DiscoveryNode.nodeRequiresLocalStorage(settings)) {
            if (dataPaths.isEmpty() == false) {
                dataFiles = new Path[dataPaths.size()];
                dataWithClusterFiles = new Path[dataPaths.size()];
                for (int i = 0; i < dataPaths.size(); i++) {
                    dataFiles[i] = PathUtils.get(dataPaths.get(i));
                    dataWithClusterFiles[i] = dataFiles[i].resolve(clusterName.value());
                }
            } else {
                dataFiles = new Path[]{homeFile.resolve("data")};
                dataWithClusterFiles = new Path[]{homeFile.resolve("data").resolve(clusterName.value())};
            }
        } else {
            if (dataPaths.isEmpty()) {
                dataFiles = dataWithClusterFiles = EMPTY_PATH_ARRAY;
            } else {
                final String paths = String.join(",", dataPaths);
                throw new IllegalStateException("node does not require local storage yet path.data is set to [" + paths + "]");
            }
        }
        if (PATH_SHARED_DATA_SETTING.exists(settings)) {
            sharedDataFile = PathUtils.get(PATH_SHARED_DATA_SETTING.get(settings)).normalize();
        } else {
            sharedDataFile = null;
        }
        List<String> repoPaths = PATH_REPO_SETTING.get(settings);
        if (repoPaths.isEmpty()) {
            repoFiles = EMPTY_PATH_ARRAY;
        } else {
            repoFiles = new Path[repoPaths.size()];
            for (int i = 0; i < repoPaths.size(); i++) {
                repoFiles[i] = PathUtils.get(repoPaths.get(i));
            }
        }

                if (PATH_LOGS_SETTING.exists(settings)) {
            logsFile = PathUtils.get(PATH_LOGS_SETTING.get(settings)).normalize();
        } else {
            logsFile = homeFile.resolve("logs");
        }

        if (PIDFILE_SETTING.exists(settings)) {
            pidFile = PathUtils.get(PIDFILE_SETTING.get(settings)).normalize();
        } else {
            pidFile = null;
        }

        binFile = homeFile.resolve("bin");
        libFile = homeFile.resolve("lib");
        modulesFile = homeFile.resolve("modules");

        Settings.Builder finalSettings = Settings.builder().put(settings);
        finalSettings.put(PATH_HOME_SETTING.getKey(), homeFile);
        if (PATH_DATA_SETTING.exists(settings)) {
            finalSettings.putList(PATH_DATA_SETTING.getKey(), dataPaths);
        }
        finalSettings.put(PATH_LOGS_SETTING.getKey(), logsFile.toString());
        this.settings = finalSettings.build();
    }

    
    public Settings settings() {
        return this.settings;
    }

    
    public Path[] dataFiles() {
        return dataFiles;
    }

    
    public Path sharedDataFile() {
        return sharedDataFile;
    }

    
    @Deprecated
    public Path[] dataWithClusterFiles() {
        return dataWithClusterFiles;
    }

    
    public Path[] repoFiles() {
        return repoFiles;
    }

    
    public Path resolveRepoFile(String location) {
        return PathUtils.get(repoFiles, location);
    }

    
    public URL resolveRepoURL(URL url) {
        try {
            if ("file".equalsIgnoreCase(url.getProtocol())) {
                if (url.getHost() == null || "".equals(url.getHost())) {
                                        Path path = PathUtils.get(repoFiles, url.toURI());
                    if (path == null) {
                                                return null;
                    }
                                        return path.toUri().toURL();
                }
                return null;
            } else if ("jar".equals(url.getProtocol())) {
                String file = url.getFile();
                int pos = file.indexOf("!/");
                if (pos < 0) {
                    return null;
                }
                String jarTail = file.substring(pos);
                String filePath = file.substring(0, pos);
                URL internalUrl = new URL(filePath);
                URL normalizedUrl = resolveRepoURL(internalUrl);
                if (normalizedUrl == null) {
                    return null;
                }
                return new URL("jar", "", normalizedUrl.toExternalForm() + jarTail);
            } else {
                                return null;
            }
        } catch (MalformedURLException ex) {
                        return null;
        } catch (URISyntaxException ex) {
            return null;
        }
    }

        
    public Path configFile() {
        return configFile;
    }

    public Path pluginsFile() {
        return pluginsFile;
    }

    public Path binFile() {
        return binFile;
    }

    public Path libFile() {
        return libFile;
    }

    public Path modulesFile() {
        return modulesFile;
    }

    public Path logsFile() {
        return logsFile;
    }

    
    public Path pidFile() {
        return pidFile;
    }

    
    public Path tmpFile() {
        return tmpFile;
    }

    
    public void validateTmpFile() throws IOException {
        if (Files.exists(tmpFile) == false) {
            throw new FileNotFoundException("Temporary file directory [" + tmpFile + "] does not exist or is not accessible");
        }
        if (Files.isDirectory(tmpFile) == false) {
            throw new IOException("Configured temporary file directory [" + tmpFile + "] is not a directory");
        }
    }

    public static FileStore getFileStore(final Path path) throws IOException {
        return new ESFileStore(Files.getFileStore(path));
    }

    
    public static void assertEquivalent(Environment actual, Environment expected) {
        assertEquals(actual.dataWithClusterFiles(), expected.dataWithClusterFiles(), "dataWithClusterFiles");
        assertEquals(actual.repoFiles(), expected.repoFiles(), "repoFiles");
        assertEquals(actual.configFile(), expected.configFile(), "configFile");
        assertEquals(actual.pluginsFile(), expected.pluginsFile(), "pluginsFile");
        assertEquals(actual.binFile(), expected.binFile(), "binFile");
        assertEquals(actual.libFile(), expected.libFile(), "libFile");
        assertEquals(actual.modulesFile(), expected.modulesFile(), "modulesFile");
        assertEquals(actual.logsFile(), expected.logsFile(), "logsFile");
        assertEquals(actual.pidFile(), expected.pidFile(), "pidFile");
        assertEquals(actual.tmpFile(), expected.tmpFile(), "tmpFile");
    }

    private static void assertEquals(Object actual, Object expected, String name) {
        assert Objects.deepEquals(actual, expected) : "actual " + name + " [" + actual + "] is different than [ " + expected + "]";
    }
}
