

package org.elasticsearch.bootstrap;

import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.io.PathUtils;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.Arrays;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.function.Consumer;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.Manifest;


public class JarHell {

    
    private JarHell() {}

    
    @SuppressForbidden(reason = "command line tool")
    public static void main(String args[]) throws Exception {
        System.out.println("checking for jar hell...");
        checkJarHell(System.out::println);
        System.out.println("no jar hell found");
    }

    
    public static void checkJarHell(Consumer<String> output) throws IOException, URISyntaxException {
        ClassLoader loader = JarHell.class.getClassLoader();
        output.accept("java.class.path: " + System.getProperty("java.class.path"));
        output.accept("sun.boot.class.path: " + System.getProperty("sun.boot.class.path"));
        if (loader instanceof URLClassLoader) {
            output.accept("classloader urls: " + Arrays.toString(((URLClassLoader)loader).getURLs()));
        }
        checkJarHell(parseClassPath(), output);
    }

    
    public static Set<URL> parseClassPath()  {
        return parseClassPath(System.getProperty("java.class.path"));
    }

    
    @SuppressForbidden(reason = "resolves against CWD because that is how classpaths work")
    static Set<URL> parseClassPath(String classPath) {
        String pathSeparator = System.getProperty("path.separator");
        String fileSeparator = System.getProperty("file.separator");
        String elements[] = classPath.split(pathSeparator);
        Set<URL> urlElements = new LinkedHashSet<>();         for (String element : elements) {
                                                                                                if (element.isEmpty()) {
                throw new IllegalStateException("Classpath should not contain empty elements! (outdated shell script from a previous" +
                    " version?) classpath='" + classPath + "'");
            }
                                                            if (element.startsWith("/") && "\\".equals(fileSeparator)) {
                                                element = element.replace("/", "\\");
                                if (element.length() >= 3 && element.charAt(2) == ':') {
                    element = element.substring(1);
                }
            }
                        try {
                URL url = PathUtils.get(element).toUri().toURL();
                if (urlElements.add(url) == false) {
                    throw new IllegalStateException("jar hell!" + System.lineSeparator() +
                        "duplicate jar [" + element + "] on classpath: " + classPath);
                }
            } catch (MalformedURLException e) {
                                throw new RuntimeException(e);
            }
        }
        return Collections.unmodifiableSet(urlElements);
    }

    
    @SuppressForbidden(reason = "needs JarFile for speed, just reading entries")
    public static void checkJarHell(Set<URL> urls, Consumer<String> output) throws URISyntaxException, IOException {
                                String javaHome = System.getProperty("java.home");
        output.accept("java.home: " + javaHome);
        final Map<String,Path> clazzes = new HashMap<>(32768);
        Set<Path> seenJars = new HashSet<>();
        for (final URL url : urls) {
            final Path path = PathUtils.get(url.toURI());
                        if (path.startsWith(javaHome)) {
                output.accept("excluding system resource: " + path);
                continue;
            }
            if (path.toString().endsWith(".jar")) {
                if (!seenJars.add(path)) {
                    throw new IllegalStateException("jar hell!" + System.lineSeparator() +
                                                    "duplicate jar on classpath: " + path);
                }
                output.accept("examining jar: " + path);
                try (JarFile file = new JarFile(path.toString())) {
                    Manifest manifest = file.getManifest();
                    if (manifest != null) {
                        checkManifest(manifest, path);
                    }
                                        Enumeration<JarEntry> elements = file.entries();
                    while (elements.hasMoreElements()) {
                        String entry = elements.nextElement().getName();
                        if (entry.endsWith(".class")) {
                                                        entry = entry.replace('/', '.').substring(0, entry.length() - 6);
                            checkClass(clazzes, entry, path);
                        }
                    }
                }
            } else {
                output.accept("examining directory: " + path);
                                final Path root = PathUtils.get(url.toURI());
                final String sep = root.getFileSystem().getSeparator();
                Files.walkFileTree(root, new SimpleFileVisitor<Path>() {
                    @Override
                    public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                        String entry = root.relativize(file).toString();
                        if (entry.endsWith(".class")) {
                                                        entry = entry.replace(sep, ".").substring(0,  entry.length() - ".class".length());
                            checkClass(clazzes, entry, path);
                        }
                        return super.visitFile(file, attrs);
                    }
                });
            }
        }
    }

    
    private static void checkManifest(Manifest manifest, Path jar) {
                String targetVersion = manifest.getMainAttributes().getValue("X-Compile-Target-JDK");
        if (targetVersion != null) {
            checkVersionFormat(targetVersion);
            checkJavaVersion(jar.toString(), targetVersion);
        }
    }

    public static void checkVersionFormat(String targetVersion) {
        if (!JavaVersion.isValid(targetVersion)) {
            throw new IllegalStateException(
                    String.format(
                            Locale.ROOT,
                            "version string must be a sequence of nonnegative decimal integers separated by \".\"'s and may have " +
                                "leading zeros but was %s",
                            targetVersion
                    )
            );
        }
    }

    
    public static void checkJavaVersion(String resource, String targetVersion) {
        JavaVersion version = JavaVersion.parse(targetVersion);
        if (JavaVersion.current().compareTo(version) < 0) {
            throw new IllegalStateException(
                    String.format(
                            Locale.ROOT,
                            "%s requires Java %s:, your system: %s",
                            resource,
                            targetVersion,
                            JavaVersion.current().toString()
                    )
            );
        }
    }

    private static void checkClass(Map<String, Path> clazzes, String clazz, Path jarpath) {
        Path previous = clazzes.put(clazz, jarpath);
        if (previous != null) {
            if (previous.equals(jarpath)) {
                if (clazz.startsWith("org.apache.xmlbeans")) {
                    return;                 }
                                                                throw new IllegalStateException("jar hell!" + System.lineSeparator() +
                        "class: " + clazz + System.lineSeparator() +
                        "exists multiple times in jar: " + jarpath + " !!!!!!!!!");
            } else {
                throw new IllegalStateException("jar hell!" + System.lineSeparator() +
                        "class: " + clazz + System.lineSeparator() +
                        "jar1: " + previous + System.lineSeparator() +
                        "jar2: " + jarpath);
            }
        }
    }
}
