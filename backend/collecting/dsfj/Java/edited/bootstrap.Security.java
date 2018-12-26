

package org.elasticsearch.bootstrap;

import org.elasticsearch.cli.Command;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.common.network.NetworkModule;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.http.HttpTransportSettings;
import org.elasticsearch.plugins.PluginInfo;
import org.elasticsearch.plugins.PluginsService;
import org.elasticsearch.secure_sm.SecureSM;
import org.elasticsearch.transport.TcpTransport;

import java.io.IOException;
import java.net.SocketPermission;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.file.AccessMode;
import java.nio.file.DirectoryStream;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.Files;
import java.nio.file.NotDirectoryException;
import java.nio.file.Path;
import java.security.NoSuchAlgorithmException;
import java.security.Permissions;
import java.security.Policy;
import java.security.URIParameter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import static org.elasticsearch.bootstrap.FilePermissionUtils.addDirectoryPath;
import static org.elasticsearch.bootstrap.FilePermissionUtils.addSingleFilePath;


final class Security {
    
    private Security() {}

    
    static void configure(Environment environment, boolean filterBadDefaults) throws IOException, NoSuchAlgorithmException {

                Map<String, URL> codebases = getCodebaseJarMap(JarHell.parseClassPath());
        Policy.setPolicy(new ESPolicy(codebases, createPermissions(environment), getPluginPermissions(environment), filterBadDefaults));

                final String[] classesThatCanExit =
                new String[]{
                                                ElasticsearchUncaughtExceptionHandler.PrivilegedHaltAction.class.getName().replace("$", "\\$"),
                        Command.class.getName()};
        System.setSecurityManager(new SecureSM(classesThatCanExit));

                selfTest();
    }

    
    @SuppressForbidden(reason = "find URL path")
    static Map<String, URL> getCodebaseJarMap(Set<URL> urls) {
        Map<String, URL> codebases = new LinkedHashMap<>();         for (URL url : urls) {
            try {
                String fileName = PathUtils.get(url.toURI()).getFileName().toString();
                if (fileName.endsWith(".jar") == false) {
                                        continue;
                }
                codebases.put(fileName, url);
            } catch (URISyntaxException e) {
                throw new RuntimeException(e);
            }
        }
        return codebases;
    }

    
    @SuppressForbidden(reason = "proper use of URL")
    static Map<String,Policy> getPluginPermissions(Environment environment) throws IOException, NoSuchAlgorithmException {
        Map<String,Policy> map = new HashMap<>();
                Set<Path> pluginsAndModules = new LinkedHashSet<>(PluginsService.findPluginDirs(environment.pluginsFile()));
        pluginsAndModules.addAll(PluginsService.findPluginDirs(environment.modulesFile()));

                for (Path plugin : pluginsAndModules) {
            Path policyFile = plugin.resolve(PluginInfo.ES_PLUGIN_POLICY);
            if (Files.exists(policyFile)) {
                                                Set<URL> codebases = new LinkedHashSet<>();                 try (DirectoryStream<Path> jarStream = Files.newDirectoryStream(plugin, "*.jar")) {
                    for (Path jar : jarStream) {
                        URL url = jar.toRealPath().toUri().toURL();
                        if (codebases.add(url) == false) {
                            throw new IllegalStateException("duplicate module/plugin: " + url);
                        }
                    }
                }

                                Policy policy = readPolicy(policyFile.toUri().toURL(), getCodebaseJarMap(codebases));

                                for (URL url : codebases) {
                    if (map.put(url.getFile(), policy) != null) {
                                                throw new IllegalStateException("per-plugin permissions already granted for jar file: " + url);
                    }
                }
            }
        }

        return Collections.unmodifiableMap(map);
    }

    
    @SuppressForbidden(reason = "accesses fully qualified URLs to configure security")
    static Policy readPolicy(URL policyFile, Map<String, URL> codebases) {
        try {
            List<String> propertiesSet = new ArrayList<>();
            try {
                                for (Map.Entry<String,URL> codebase : codebases.entrySet()) {
                    String name = codebase.getKey();
                    URL url = codebase.getValue();

                                                                                String property = "codebase." + name;
                    String aliasProperty = "codebase." + name.replaceFirst("-\\d+\\.\\d+.*\\.jar", "");
                    if (aliasProperty.equals(property) == false) {
                        propertiesSet.add(aliasProperty);
                        String previous = System.setProperty(aliasProperty, url.toString());
                        if (previous != null) {
                            throw new IllegalStateException("codebase property already set: " + aliasProperty + " -> " + previous +
                                                            ", cannot set to " + url.toString());
                        }
                    }
                    propertiesSet.add(property);
                    String previous = System.setProperty(property, url.toString());
                    if (previous != null) {
                        throw new IllegalStateException("codebase property already set: " + property + " -> " + previous +
                                                        ", cannot set to " + url.toString());
                    }
                }
                return Policy.getInstance("JavaPolicy", new URIParameter(policyFile.toURI()));
            } finally {
                                for (String property : propertiesSet) {
                    System.clearProperty(property);
                }
            }
        } catch (NoSuchAlgorithmException | URISyntaxException e) {
            throw new IllegalArgumentException("unable to parse policy file `" + policyFile + "`", e);
        }
    }

    
    static Permissions createPermissions(Environment environment) throws IOException {
        Permissions policy = new Permissions();
        addClasspathPermissions(policy);
        addFilePermissions(policy, environment);
        addBindPermissions(policy, environment.settings());
        return policy;
    }

    
    @SuppressForbidden(reason = "accesses fully qualified URLs to configure security")
    static void addClasspathPermissions(Permissions policy) throws IOException {
                        for (URL url : JarHell.parseClassPath()) {
            Path path;
            try {
                path = PathUtils.get(url.toURI());
            } catch (URISyntaxException e) {
                throw new RuntimeException(e);
            }
                        if (Files.isDirectory(path)) {
                addDirectoryPath(policy, "class.path", path, "read,readlink");
            } else {
                addSingleFilePath(policy, path, "read,readlink");
            }
        }
    }

    
    static void addFilePermissions(Permissions policy, Environment environment) throws IOException {
                addDirectoryPath(policy, Environment.PATH_HOME_SETTING.getKey(), environment.binFile(), "read,readlink");
        addDirectoryPath(policy, Environment.PATH_HOME_SETTING.getKey(), environment.libFile(), "read,readlink");
        addDirectoryPath(policy, Environment.PATH_HOME_SETTING.getKey(), environment.modulesFile(), "read,readlink");
        addDirectoryPath(policy, Environment.PATH_HOME_SETTING.getKey(), environment.pluginsFile(), "read,readlink");
        addDirectoryPath(policy, "path.conf'", environment.configFile(), "read,readlink");
                addDirectoryPath(policy, "java.io.tmpdir", environment.tmpFile(), "read,readlink,write,delete");
        addDirectoryPath(policy, Environment.PATH_LOGS_SETTING.getKey(), environment.logsFile(), "read,readlink,write,delete");
        if (environment.sharedDataFile() != null) {
            addDirectoryPath(policy, Environment.PATH_SHARED_DATA_SETTING.getKey(), environment.sharedDataFile(),
                "read,readlink,write,delete");
        }
        final Set<Path> dataFilesPaths = new HashSet<>();
        for (Path path : environment.dataFiles()) {
            addDirectoryPath(policy, Environment.PATH_DATA_SETTING.getKey(), path, "read,readlink,write,delete");
            
            try {
                final Path realPath = path.toRealPath();
                if (!dataFilesPaths.add(realPath)) {
                    throw new IllegalStateException("path [" + realPath + "] is duplicated by [" + path + "]");
                }
            } catch (final IOException e) {
                throw new IllegalStateException("unable to access [" + path + "]", e);
            }
        }
        for (Path path : environment.repoFiles()) {
            addDirectoryPath(policy, Environment.PATH_REPO_SETTING.getKey(), path, "read,readlink,write,delete");
        }
        if (environment.pidFile() != null) {
                        addSingleFilePath(policy, environment.pidFile(), "delete");
        }
    }

    
    private static void addBindPermissions(Permissions policy, Settings settings) {
        addSocketPermissionForHttp(policy, settings);
        addSocketPermissionForTransportProfiles(policy, settings);
        addSocketPermissionForTribeNodes(policy, settings);
    }

    
    private static void addSocketPermissionForHttp(final Permissions policy, final Settings settings) {
                final String httpRange = HttpTransportSettings.SETTING_HTTP_PORT.get(settings).getPortRangeString();
        addSocketPermissionForPortRange(policy, httpRange);
    }

    
    private static void addSocketPermissionForTransportProfiles(final Permissions policy, final Settings settings) {
                Set<TcpTransport.ProfileSettings> profiles = TcpTransport.getProfileSettings(settings);
        Set<String> uniquePortRanges = new HashSet<>();
                for (final TcpTransport.ProfileSettings profile : profiles) {
            if (uniquePortRanges.add(profile.portOrRange)) {
                                addSocketPermissionForPortRange(policy, profile.portOrRange);
            }
        }
    }

    
    private static void addSocketPermissionForTransport(final Permissions policy, final Settings settings) {
        final String transportRange = TcpTransport.PORT.get(settings);
        addSocketPermissionForPortRange(policy, transportRange);
    }

    private static void addSocketPermissionForTribeNodes(final Permissions policy, final Settings settings) {
        for (final Settings tribeNodeSettings : settings.getGroups("tribe", true).values()) {
                        if (NetworkModule.HTTP_ENABLED.exists(tribeNodeSettings) && NetworkModule.HTTP_ENABLED.get(tribeNodeSettings)) {
                addSocketPermissionForHttp(policy, tribeNodeSettings);
            }
            addSocketPermissionForTransport(policy, tribeNodeSettings);
        }
    }

    
    private static void addSocketPermissionForPortRange(final Permissions policy, final String portRange) {
                        policy.add(new SocketPermission("*:" + portRange, "listen,resolve"));
    }

    
    static void ensureDirectoryExists(Path path) throws IOException {
                if (Files.isDirectory(path)) {
                                    path.getFileSystem().provider().checkAccess(path.toRealPath(), AccessMode.READ);
        } else {
                        try {
                Files.createDirectories(path);
            } catch (FileAlreadyExistsException e) {
                                IOException e2 = new NotDirectoryException(path.toString());
                e2.addSuppressed(e);
                throw e2;
            }
        }
    }

    
    @SuppressForbidden(reason = "accesses jvm default tempdir as a self-test")
    static void selfTest() throws IOException {
                try {
            Path p = Files.createTempFile(null, null);
            try {
                Files.delete(p);
            } catch (IOException ignored) {
                            }
        } catch (SecurityException problem) {
            throw new SecurityException("Security misconfiguration: cannot access java.io.tmpdir", problem);
        }
    }
}
