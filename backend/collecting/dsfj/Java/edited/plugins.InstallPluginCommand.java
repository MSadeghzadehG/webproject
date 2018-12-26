

package org.elasticsearch.plugins;

import joptsimple.OptionSet;
import joptsimple.OptionSpec;

import org.apache.lucene.search.spell.LevensteinDistance;
import org.apache.lucene.util.CollectionUtil;
import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.Version;
import org.elasticsearch.bootstrap.JarHell;
import org.elasticsearch.cli.EnvironmentAwareCommand;
import org.elasticsearch.cli.ExitCodes;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.cli.UserException;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.collect.Tuple;
import org.elasticsearch.common.hash.MessageDigests;
import org.elasticsearch.common.settings.KeyStoreWrapper;
import org.elasticsearch.env.Environment;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.nio.file.DirectoryStream;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.StandardCopyOption;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFileAttributes;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.TreeSet;
import java.util.stream.Collectors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import static org.elasticsearch.cli.Terminal.Verbosity.VERBOSE;


class InstallPluginCommand extends EnvironmentAwareCommand {

    private static final String PROPERTY_STAGING_ID = "es.plugins.staging";

        
    static final int PLUGIN_EXISTS = 1;
    
    static final int PLUGIN_MALFORMED = 2;


    
    static final Set<String> MODULES;
    static {
        try (InputStream stream = InstallPluginCommand.class.getResourceAsStream("/modules.txt");
            BufferedReader reader = new BufferedReader(new InputStreamReader(stream, StandardCharsets.UTF_8))) {
            Set<String> modules = new HashSet<>();
            String line = reader.readLine();
            while (line != null) {
                modules.add(line.trim());
                line = reader.readLine();
            }
            MODULES = Collections.unmodifiableSet(modules);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    
    static final Set<String> OFFICIAL_PLUGINS;
    static {
        try (InputStream stream = InstallPluginCommand.class.getResourceAsStream("/plugins.txt");
            BufferedReader reader = new BufferedReader(new InputStreamReader(stream, StandardCharsets.UTF_8))) {
            Set<String> plugins = new TreeSet<>();             String line = reader.readLine();
            while (line != null) {
                plugins.add(line.trim());
                line = reader.readLine();
            }
            plugins.add("x-pack");
            OFFICIAL_PLUGINS = Collections.unmodifiableSet(plugins);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private final OptionSpec<Void> batchOption;
    private final OptionSpec<String> arguments;

    static final Set<PosixFilePermission> BIN_DIR_PERMS;
    static final Set<PosixFilePermission> BIN_FILES_PERMS;
    static final Set<PosixFilePermission> CONFIG_DIR_PERMS;
    static final Set<PosixFilePermission> CONFIG_FILES_PERMS;
    static final Set<PosixFilePermission> PLUGIN_DIR_PERMS;
    static final Set<PosixFilePermission> PLUGIN_FILES_PERMS;

    static {
                BIN_DIR_PERMS = Collections.unmodifiableSet(PosixFilePermissions.fromString("rwxr-xr-x"));

                BIN_FILES_PERMS = BIN_DIR_PERMS;

                CONFIG_DIR_PERMS = Collections.unmodifiableSet(PosixFilePermissions.fromString("rwxr-x---"));

                CONFIG_FILES_PERMS = Collections.unmodifiableSet(PosixFilePermissions.fromString("rw-rw----"));

                PLUGIN_DIR_PERMS = BIN_DIR_PERMS;

                PLUGIN_FILES_PERMS = Collections.unmodifiableSet(PosixFilePermissions.fromString("rw-r--r--"));
    }

    InstallPluginCommand() {
        super("Install a plugin");
        this.batchOption = parser.acceptsAll(Arrays.asList("b", "batch"),
                "Enable batch mode explicitly, automatic confirmation of security permission");
        this.arguments = parser.nonOptions("plugin id");
    }

    @Override
    protected void printAdditionalHelp(Terminal terminal) {
        terminal.println("The following official plugins may be installed by name:");
        for (String plugin : OFFICIAL_PLUGINS) {
            terminal.println("  " + plugin);
        }
        terminal.println("");
    }

    @Override
    protected void execute(Terminal terminal, OptionSet options, Environment env) throws Exception {
        String pluginId = arguments.value(options);
        boolean isBatch = options.has(batchOption) || System.console() == null;
        execute(terminal, pluginId, isBatch, env);
    }

        void execute(Terminal terminal, String pluginId, boolean isBatch, Environment env) throws Exception {
        if (pluginId == null) {
            throw new UserException(ExitCodes.USAGE, "plugin id is required");
        }

        Path pluginZip = download(terminal, pluginId, env.tmpFile());
        Path extractedZip = unzip(pluginZip, env.pluginsFile());
        install(terminal, isBatch, extractedZip, env);
    }

    
    private Path download(Terminal terminal, String pluginId, Path tmpDir) throws Exception {
        if (OFFICIAL_PLUGINS.contains(pluginId)) {
            final String url = getElasticUrl(terminal, getStagingHash(), Version.CURRENT, pluginId, Platforms.PLATFORM_NAME);
            terminal.println("-> Downloading " + pluginId + " from elastic");
            return downloadZipAndChecksum(terminal, url, tmpDir, false);
        }

                String[] coordinates = pluginId.split(":");
        if (coordinates.length == 3 && pluginId.contains("/") == false && pluginId.startsWith("file:") == false) {
            String mavenUrl = getMavenUrl(terminal, coordinates, Platforms.PLATFORM_NAME);
            terminal.println("-> Downloading " + pluginId + " from maven central");
            return downloadZipAndChecksum(terminal, mavenUrl, tmpDir, true);
        }

                if (pluginId.contains(":") == false) {
                        List<String> plugins = checkMisspelledPlugin(pluginId);
            String msg = "Unknown plugin " + pluginId;
            if (plugins.isEmpty() == false) {
                msg += ", did you mean " + (plugins.size() == 1 ? "[" + plugins.get(0) + "]": "any of " + plugins.toString()) + "?";
            }
            throw new UserException(ExitCodes.USAGE, msg);
        }
        terminal.println("-> Downloading " + URLDecoder.decode(pluginId, "UTF-8"));
        return downloadZip(terminal, pluginId, tmpDir);
    }

        String getStagingHash() {
        return System.getProperty(PROPERTY_STAGING_ID);
    }

    
    private String getElasticUrl(Terminal terminal, String stagingHash, Version version,
                                        String pluginId, String platform) throws IOException {
        final String baseUrl;
        if (stagingHash != null) {
            baseUrl = String.format(Locale.ROOT,
                "https:        } else {
            baseUrl = String.format(Locale.ROOT,
                "https:        }
        final String platformUrl = String.format(Locale.ROOT, "%s/%s-%s-%s.zip", baseUrl, pluginId, platform, version);
        if (urlExists(terminal, platformUrl)) {
            return platformUrl;
        }
        return String.format(Locale.ROOT, "%s/%s-%s.zip", baseUrl, pluginId, version);
    }

    
    private String getMavenUrl(Terminal terminal, String[] coordinates, String platform) throws IOException {
        final String groupId = coordinates[0].replace(".", "/");
        final String artifactId = coordinates[1];
        final String version = coordinates[2];
        final String baseUrl = String.format(Locale.ROOT, "https:        final String platformUrl = String.format(Locale.ROOT, "%s/%s-%s-%s.zip", baseUrl, artifactId, platform, version);
        if (urlExists(terminal, platformUrl)) {
            return platformUrl;
        }
        return String.format(Locale.ROOT, "%s/%s-%s.zip", baseUrl, artifactId, version);
    }

    
        @SuppressForbidden(reason = "Make HEAD request using URLConnection.connect()")
    boolean urlExists(Terminal terminal, String urlString) throws IOException {
        terminal.println(VERBOSE, "Checking if url exists: " + urlString);
        URL url = new URL(urlString);
        assert "https".equals(url.getProtocol()) : "Only http urls can be checked";
        HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();
        urlConnection.addRequestProperty("User-Agent", "elasticsearch-plugin-installer");
        urlConnection.setRequestMethod("HEAD");
        urlConnection.connect();
        return urlConnection.getResponseCode() == 200;
    }

    
    private List<String> checkMisspelledPlugin(String pluginId) {
        LevensteinDistance ld = new LevensteinDistance();
        List<Tuple<Float, String>> scoredKeys = new ArrayList<>();
        for (String officialPlugin : OFFICIAL_PLUGINS) {
            float distance = ld.getDistance(pluginId, officialPlugin);
            if (distance > 0.7f) {
                scoredKeys.add(new Tuple<>(distance, officialPlugin));
            }
        }
        CollectionUtil.timSort(scoredKeys, (a, b) -> b.v1().compareTo(a.v1()));
        return scoredKeys.stream().map((a) -> a.v2()).collect(Collectors.toList());
    }

    
        @SuppressForbidden(reason = "We use getInputStream to download plugins")
    Path downloadZip(Terminal terminal, String urlString, Path tmpDir) throws IOException {
        terminal.println(VERBOSE, "Retrieving zip from " + urlString);
        URL url = new URL(urlString);
        Path zip = Files.createTempFile(tmpDir, null, ".zip");
        URLConnection urlConnection = url.openConnection();
        urlConnection.addRequestProperty("User-Agent", "elasticsearch-plugin-installer");
        int contentLength = urlConnection.getContentLength();
        try (InputStream in = new TerminalProgressInputStream(urlConnection.getInputStream(), contentLength, terminal)) {
                        Files.copy(in, zip, StandardCopyOption.REPLACE_EXISTING);
        }
        return zip;
    }

    
    private class TerminalProgressInputStream extends ProgressInputStream {

        private final Terminal terminal;
        private int width = 50;
        private final boolean enabled;

        TerminalProgressInputStream(InputStream is, int expectedTotalSize, Terminal terminal) {
            super(is, expectedTotalSize);
            this.terminal = terminal;
            this.enabled = expectedTotalSize > 0;
        }

        @Override
        public void onProgress(int percent) {
            if (enabled) {
                int currentPosition = percent * width / 100;
                StringBuilder sb = new StringBuilder("\r[");
                sb.append(String.join("=", Collections.nCopies(currentPosition, "")));
                if (currentPosition > 0 && percent < 100) {
                    sb.append(">");
                }
                sb.append(String.join(" ", Collections.nCopies(width - currentPosition, "")));
                sb.append("] %s   ");
                if (percent == 100) {
                    sb.append("\n");
                }
                terminal.print(Terminal.Verbosity.NORMAL, String.format(Locale.ROOT, sb.toString(), percent + "%"));
            }
        }
    }

    
        @SuppressForbidden(reason = "We use openStream to download plugins")
    private Path downloadZipAndChecksum(Terminal terminal, String urlString, Path tmpDir, boolean allowSha1) throws Exception {
        Path zip = downloadZip(terminal, urlString, tmpDir);
        pathsToDeleteOnShutdown.add(zip);
        String checksumUrlString = urlString + ".sha512";
        URL checksumUrl = openUrl(checksumUrlString);
        String digestAlgo = "SHA-512";
        if (checksumUrl == null && allowSha1) {
                        terminal.println("Warning: sha512 not found, falling back to sha1. This behavior is deprecated and will be removed in a " +
                             "future release. Please update the plugin to use a sha512 checksum.");
            checksumUrlString = urlString + ".sha1";
            checksumUrl = openUrl(checksumUrlString);
            digestAlgo = "SHA-1";
        }
        if (checksumUrl == null) {
            throw new UserException(ExitCodes.IO_ERROR, "Plugin checksum missing: " + checksumUrlString);
        }
        final String expectedChecksum;
        try (InputStream in = checksumUrl.openStream()) {
            
            if (digestAlgo.equals("SHA-1")) {
                final BufferedReader checksumReader = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8));
                expectedChecksum = checksumReader.readLine();
                if (checksumReader.readLine() != null) {
                    throw new UserException(ExitCodes.IO_ERROR, "Invalid checksum file at " + checksumUrl);
                }
            } else {
                final BufferedReader checksumReader = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8));
                final String checksumLine = checksumReader.readLine();
                final String[] fields = checksumLine.split(" {2}");
                if (fields.length != 2) {
                    throw new UserException(ExitCodes.IO_ERROR, "Invalid checksum file at " + checksumUrl);
                }
                expectedChecksum = fields[0];
                final String[] segments = URI.create(urlString).getPath().split("/");
                final String expectedFile = segments[segments.length - 1];
                if (fields[1].equals(expectedFile) == false) {
                    final String message = String.format(
                            Locale.ROOT,
                            "checksum file at [%s] is not for this plugin, expected [%s] but was [%s]",
                            checksumUrl,
                            expectedFile,
                            fields[1]);
                    throw new UserException(ExitCodes.IO_ERROR, message);
                }
                if (checksumReader.readLine() != null) {
                    throw new UserException(ExitCodes.IO_ERROR, "Invalid checksum file at " + checksumUrl);
                }
            }
        }

        byte[] zipbytes = Files.readAllBytes(zip);
        String gotChecksum = MessageDigests.toHexString(MessageDigest.getInstance(digestAlgo).digest(zipbytes));
        if (expectedChecksum.equals(gotChecksum) == false) {
            throw new UserException(ExitCodes.IO_ERROR,
                digestAlgo + " mismatch, expected " + expectedChecksum + " but got " + gotChecksum);
        }

        return zip;
    }

    
        URL openUrl(String urlString) throws Exception {
        URL checksumUrl = new URL(urlString);
        HttpURLConnection connection = (HttpURLConnection)checksumUrl.openConnection();
        if (connection.getResponseCode() == 404) {
            return null;
        }
        return checksumUrl;
    }

    private Path unzip(Path zip, Path pluginsDir) throws IOException, UserException {
        
        final Path target = stagingDirectory(pluginsDir);
        pathsToDeleteOnShutdown.add(target);

        try (ZipInputStream zipInput = new ZipInputStream(Files.newInputStream(zip))) {
            ZipEntry entry;
            byte[] buffer = new byte[8192];
            while ((entry = zipInput.getNextEntry()) != null) {
                if (entry.getName().startsWith("elasticsearch/")) {
                    throw new UserException(PLUGIN_MALFORMED, "This plugin was built with an older plugin structure." +
                        " Contact the plugin author to remove the intermediate \"elasticsearch\" directory within the plugin zip.");
                }
                Path targetFile = target.resolve(entry.getName());

                                                                                                if (targetFile.normalize().startsWith(target) == false) {
                    throw new UserException(PLUGIN_MALFORMED, "Zip contains entry name '" +
                        entry.getName() + "' resolving outside of plugin directory");
                }

                                                if (!Files.isSymbolicLink(targetFile.getParent())) {
                    Files.createDirectories(targetFile.getParent());
                }
                if (entry.isDirectory() == false) {
                    try (OutputStream out = Files.newOutputStream(targetFile)) {
                        int len;
                        while ((len = zipInput.read(buffer)) >= 0) {
                            out.write(buffer, 0, len);
                        }
                    }
                }
                zipInput.closeEntry();
            }
        } catch (UserException e) {
            IOUtils.rm(target);
            throw e;
        }
        Files.delete(zip);
        return target;
    }

    private Path stagingDirectory(Path pluginsDir) throws IOException {
        try {
            return Files.createTempDirectory(pluginsDir, ".installing-", PosixFilePermissions.asFileAttribute(PLUGIN_DIR_PERMS));
        } catch (IllegalArgumentException e) {
                                                            final StackTraceElement[] elements = e.getStackTrace();
            if (elements.length >= 1 &&
                elements[0].getClassName().equals("com.google.common.jimfs.AttributeService") &&
                elements[0].getMethodName().equals("setAttributeInternal")) {
                return stagingDirectoryWithoutPosixPermissions(pluginsDir);
            } else {
                throw e;
            }
        } catch (UnsupportedOperationException e) {
            return stagingDirectoryWithoutPosixPermissions(pluginsDir);
        }
    }

    private Path stagingDirectoryWithoutPosixPermissions(Path pluginsDir) throws IOException {
        return Files.createTempDirectory(pluginsDir, ".installing-");
    }

        private void verifyPluginName(Path pluginPath, String pluginName, Path candidateDir) throws UserException, IOException {
        final Path destination = pluginPath.resolve(pluginName);
        if (Files.exists(destination)) {
            final String message = String.format(
                Locale.ROOT,
                "plugin directory [%s] already exists; if you need to update the plugin, " +
                    "uninstall it first using command 'remove %s'",
                destination.toAbsolutePath(),
                pluginName);
            throw new UserException(PLUGIN_EXISTS, message);
        }
                try (DirectoryStream<Path> stream = Files.newDirectoryStream(pluginPath)) {
            for (Path plugin : stream) {
                if (candidateDir.equals(plugin.resolve(pluginName))) {
                    continue;
                }
                if (MetaPluginInfo.isMetaPlugin(plugin) && Files.exists(plugin.resolve(pluginName))) {
                    final MetaPluginInfo info = MetaPluginInfo.readFromProperties(plugin);
                    final String message = String.format(
                        Locale.ROOT,
                        "plugin name [%s] already exists in a meta plugin; if you need to update the meta plugin, " +
                            "uninstall it first using command 'remove %s'",
                        plugin.resolve(pluginName).toAbsolutePath(),
                        info.getName());
                    throw new UserException(PLUGIN_EXISTS, message);
                }
            }
        }
    }

    
    private PluginInfo loadPluginInfo(Terminal terminal, Path pluginRoot, boolean isBatch, Environment env) throws Exception {
        final PluginInfo info = PluginInfo.readFromProperties(pluginRoot);
        PluginsService.verifyCompatibility(info);

                verifyPluginName(env.pluginsFile(), info.getName(), pluginRoot);

        PluginsService.checkForFailedPluginRemovals(env.pluginsFile());

        terminal.println(VERBOSE, info.toString());

                        if (MODULES.contains(info.getName())) {
            throw new UserException(ExitCodes.USAGE, "plugin '" + info.getName() +
                "' cannot be installed like this, it is a system module");
        }

                jarHellCheck(info, pluginRoot, env.pluginsFile(), env.modulesFile());

        return info;
    }

    
    void jarHellCheck(PluginInfo candidateInfo, Path candidateDir, Path pluginsDir, Path modulesDir) throws Exception {
                final Set<URL> jars = new HashSet<>(JarHell.parseClassPath());


                Set<PluginsService.Bundle> bundles = new HashSet<>(PluginsService.getPluginBundles(pluginsDir));
        bundles.addAll(PluginsService.getModuleBundles(modulesDir));
        bundles.add(new PluginsService.Bundle(candidateInfo, candidateDir));
        List<PluginsService.Bundle> sortedBundles = PluginsService.sortBundles(bundles);

                        Map<String, Set<URL>> transitiveUrls = new HashMap<>();
        for (PluginsService.Bundle bundle : sortedBundles) {
            PluginsService.checkBundleJarHell(bundle, transitiveUrls);
        }

                    }

    private void install(Terminal terminal, boolean isBatch, Path tmpRoot, Environment env) throws Exception {
        List<Path> deleteOnFailure = new ArrayList<>();
        deleteOnFailure.add(tmpRoot);
        try {
            if (MetaPluginInfo.isMetaPlugin(tmpRoot)) {
                installMetaPlugin(terminal, isBatch, tmpRoot, env, deleteOnFailure);
            } else {
                installPlugin(terminal, isBatch, tmpRoot, env, deleteOnFailure);
            }
        } catch (Exception installProblem) {
            try {
                IOUtils.rm(deleteOnFailure.toArray(new Path[0]));
            } catch (IOException exceptionWhileRemovingFiles) {
                installProblem.addSuppressed(exceptionWhileRemovingFiles);
            }
            throw installProblem;
        }
    }

    
    private void installMetaPlugin(Terminal terminal, boolean isBatch, Path tmpRoot,
                                   Environment env, List<Path> deleteOnFailure) throws Exception {
        final MetaPluginInfo metaInfo = MetaPluginInfo.readFromProperties(tmpRoot);
        verifyPluginName(env.pluginsFile(), metaInfo.getName(), tmpRoot);

        final Path destination = env.pluginsFile().resolve(metaInfo.getName());
        deleteOnFailure.add(destination);
        terminal.println(VERBOSE, metaInfo.toString());

        final List<Path> pluginPaths = new ArrayList<>();
        try (DirectoryStream<Path> paths = Files.newDirectoryStream(tmpRoot)) {
                        for (Path plugin : paths) {
                if (MetaPluginInfo.isPropertiesFile(plugin)) {
                    continue;
                }
                final PluginInfo info = PluginInfo.readFromProperties(plugin);
                PluginsService.verifyCompatibility(info);
                verifyPluginName(env.pluginsFile(), info.getName(), plugin);
                pluginPaths.add(plugin);
            }
        }

        
        Set<String> permissions = new HashSet<>();
        final List<PluginInfo> pluginInfos = new ArrayList<>();
        boolean hasNativeController = false;
        for (Path plugin : pluginPaths) {
            final PluginInfo info = loadPluginInfo(terminal, plugin, isBatch, env);
            pluginInfos.add(info);

            hasNativeController |= info.hasNativeController();

            Path policy = plugin.resolve(PluginInfo.ES_PLUGIN_POLICY);
            if (Files.exists(policy)) {
                permissions.addAll(PluginSecurity.parsePermissions(policy, env.tmpFile()));
            }
        }
        PluginSecurity.confirmPolicyExceptions(terminal, permissions, hasNativeController, isBatch);

                for (int i = 0; i < pluginPaths.size(); ++i) {
            Path pluginPath = pluginPaths.get(i);
            PluginInfo info = pluginInfos.get(i);
            installPluginSupportFiles(info, pluginPath, env.binFile().resolve(metaInfo.getName()),
                                      env.configFile().resolve(metaInfo.getName()), deleteOnFailure);
                        if (pluginPath.getFileName().toString().equals(info.getName()) == false) {
                Files.move(pluginPath, pluginPath.getParent().resolve(info.getName()), StandardCopyOption.ATOMIC_MOVE);
            }
        }
        movePlugin(tmpRoot, destination);
        String[] plugins = pluginInfos.stream().map(PluginInfo::getName).toArray(String[]::new);
        terminal.println("-> Installed " + metaInfo.getName() + " with: " + Strings.arrayToCommaDelimitedString(plugins));
    }

    
    private void installPlugin(Terminal terminal, boolean isBatch, Path tmpRoot,
                               Environment env, List<Path> deleteOnFailure) throws Exception {
        final PluginInfo info = loadPluginInfo(terminal, tmpRoot, isBatch, env);
                Path policy = tmpRoot.resolve(PluginInfo.ES_PLUGIN_POLICY);
        if (Files.exists(policy)) {
            Set<String> permissions = PluginSecurity.parsePermissions(policy, env.tmpFile());
            PluginSecurity.confirmPolicyExceptions(terminal, permissions, info.hasNativeController(), isBatch);
        }

        final Path destination = env.pluginsFile().resolve(info.getName());
        deleteOnFailure.add(destination);

        installPluginSupportFiles(info, tmpRoot, env.binFile().resolve(info.getName()),
                                  env.configFile().resolve(info.getName()), deleteOnFailure);
        movePlugin(tmpRoot, destination);
        terminal.println("-> Installed " + info.getName());
    }

    
    private void installPluginSupportFiles(PluginInfo info, Path tmpRoot,
                                           Path destBinDir, Path destConfigDir, List<Path> deleteOnFailure) throws Exception {
        Path tmpBinDir = tmpRoot.resolve("bin");
        if (Files.exists(tmpBinDir)) {
            deleteOnFailure.add(destBinDir);
            installBin(info, tmpBinDir, destBinDir);
        }

        Path tmpConfigDir = tmpRoot.resolve("config");
        if (Files.exists(tmpConfigDir)) {
                                    installConfig(info, tmpConfigDir, destConfigDir);
        }
    }

    
    private void movePlugin(Path tmpRoot, Path destination) throws IOException {
        Files.move(tmpRoot, destination, StandardCopyOption.ATOMIC_MOVE);
        Files.walkFileTree(destination, new SimpleFileVisitor<Path>() {
            @Override
            public FileVisitResult visitFile(final Path file, final BasicFileAttributes attrs) throws IOException {
                if ("bin".equals(file.getParent().getFileName().toString())) {
                    setFileAttributes(file, BIN_FILES_PERMS);
                } else {
                    setFileAttributes(file, PLUGIN_FILES_PERMS);
                }
                return FileVisitResult.CONTINUE;
            }

            @Override
            public FileVisitResult postVisitDirectory(final Path dir, final IOException exc) throws IOException {
                setFileAttributes(dir, PLUGIN_DIR_PERMS);
                return FileVisitResult.CONTINUE;
            }
        });
    }

    
    private void installBin(PluginInfo info, Path tmpBinDir, Path destBinDir) throws Exception {
        if (Files.isDirectory(tmpBinDir) == false) {
            throw new UserException(PLUGIN_MALFORMED, "bin in plugin " + info.getName() + " is not a directory");
        }
        Files.createDirectories(destBinDir);
        setFileAttributes(destBinDir, BIN_DIR_PERMS);

        try (DirectoryStream<Path> stream = Files.newDirectoryStream(tmpBinDir)) {
            for (Path srcFile : stream) {
                if (Files.isDirectory(srcFile)) {
                    throw new UserException(PLUGIN_MALFORMED, "Directories not allowed in bin dir " +
                        "for plugin " + info.getName() + ", found " + srcFile.getFileName());
                }

                Path destFile = destBinDir.resolve(tmpBinDir.relativize(srcFile));
                Files.copy(srcFile, destFile);
                setFileAttributes(destFile, BIN_FILES_PERMS);
            }
        }
        IOUtils.rm(tmpBinDir);     }

    
    private void installConfig(PluginInfo info, Path tmpConfigDir, Path destConfigDir) throws Exception {
        if (Files.isDirectory(tmpConfigDir) == false) {
            throw new UserException(PLUGIN_MALFORMED,
                "config in plugin " + info.getName() + " is not a directory");
        }

        Files.createDirectories(destConfigDir);
        setFileAttributes(destConfigDir, CONFIG_DIR_PERMS);
        final PosixFileAttributeView destConfigDirAttributesView =
            Files.getFileAttributeView(destConfigDir.getParent(), PosixFileAttributeView.class);
        final PosixFileAttributes destConfigDirAttributes =
            destConfigDirAttributesView != null ? destConfigDirAttributesView.readAttributes() : null;
        if (destConfigDirAttributes != null) {
            setOwnerGroup(destConfigDir, destConfigDirAttributes);
        }

        try (DirectoryStream<Path> stream = Files.newDirectoryStream(tmpConfigDir)) {
            for (Path srcFile : stream) {
                if (Files.isDirectory(srcFile)) {
                    throw new UserException(PLUGIN_MALFORMED,
                        "Directories not allowed in config dir for plugin " + info.getName());
                }

                Path destFile = destConfigDir.resolve(tmpConfigDir.relativize(srcFile));
                if (Files.exists(destFile) == false) {
                    Files.copy(srcFile, destFile);
                    setFileAttributes(destFile, CONFIG_FILES_PERMS);
                    if (destConfigDirAttributes != null) {
                        setOwnerGroup(destFile, destConfigDirAttributes);
                    }
                }
            }
        }
        IOUtils.rm(tmpConfigDir);     }

    private static void setOwnerGroup(final Path path, final PosixFileAttributes attributes) throws IOException {
        Objects.requireNonNull(attributes);
        PosixFileAttributeView fileAttributeView = Files.getFileAttributeView(path, PosixFileAttributeView.class);
        assert fileAttributeView != null;
        fileAttributeView.setOwner(attributes.owner());
        fileAttributeView.setGroup(attributes.group());
    }

    
    private static void setFileAttributes(final Path path, final Set<PosixFilePermission> permissions) throws IOException {
        PosixFileAttributeView fileAttributeView = Files.getFileAttributeView(path, PosixFileAttributeView.class);
        if (fileAttributeView != null) {
            Files.setPosixFilePermissions(path, permissions);
        }
    }

    private final List<Path> pathsToDeleteOnShutdown = new ArrayList<>();

    @Override
    public void close() throws IOException {
        IOUtils.rm(pathsToDeleteOnShutdown.toArray(new Path[pathsToDeleteOnShutdown.size()]));
    }

}
