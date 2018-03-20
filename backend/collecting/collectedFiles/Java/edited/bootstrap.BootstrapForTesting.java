

package org.elasticsearch.bootstrap;

import com.carrotsearch.randomizedtesting.RandomizedRunner;
import org.apache.logging.log4j.Logger;
import org.apache.lucene.util.LuceneTestCase;
import org.elasticsearch.common.Booleans;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.io.FileSystemUtils;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.common.logging.ESLoggerFactory;
import org.elasticsearch.common.network.IfConfig;
import org.elasticsearch.plugins.PluginInfo;
import org.elasticsearch.secure_sm.SecureSM;
import org.junit.Assert;

import java.io.InputStream;
import java.net.SocketPermission;
import java.net.URL;
import java.nio.file.Path;
import java.security.Permission;
import java.security.Permissions;
import java.security.Policy;
import java.security.ProtectionDomain;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Properties;
import java.util.Set;

import static com.carrotsearch.randomizedtesting.RandomizedTest.systemPropertyAsBoolean;


public class BootstrapForTesting {

        
    static {
                Path javaTmpDir = PathUtils.get(Objects.requireNonNull(System.getProperty("java.io.tmpdir"),
                                                               "please set ${java.io.tmpdir} in pom.xml"));
        try {
            Security.ensureDirectoryExists(javaTmpDir);
        } catch (Exception e) {
            throw new RuntimeException("unable to create test temp directory", e);
        }

                final boolean systemCallFilter = Booleans.parseBoolean(System.getProperty("tests.system_call_filter", "true"));
        Bootstrap.initializeNatives(javaTmpDir, true, systemCallFilter, true);

                Bootstrap.initializeProbes();

                BootstrapInfo.getSystemProperties();

                try {
            final Logger logger = ESLoggerFactory.getLogger(JarHell.class);
            JarHell.checkJarHell(logger::debug);
        } catch (Exception e) {
            throw new RuntimeException("found jar hell in test classpath", e);
        }

                IfConfig.logIfNecessary();

                if (systemPropertyAsBoolean("tests.security.manager", true)) {
            try {
                                Permissions perms = new Permissions();
                Security.addClasspathPermissions(perms);
                                FilePermissionUtils.addDirectoryPath(perms, "java.io.tmpdir", javaTmpDir, "read,readlink,write,delete");
                                if (Strings.hasLength(System.getProperty("tests.config"))) {
                    FilePermissionUtils.addSingleFilePath(perms, PathUtils.get(System.getProperty("tests.config")), "read,readlink");
                }
                                final boolean testsCoverage =
                        Booleans.parseBoolean(System.getProperty("tests.coverage", "false"));
                if (testsCoverage) {
                    Path coverageDir = PathUtils.get(System.getProperty("tests.coverage.dir"));
                    FilePermissionUtils.addSingleFilePath(perms, coverageDir.resolve("jacoco.exec"), "read,write");
                                        FilePermissionUtils.addSingleFilePath(perms, coverageDir.resolve("jacoco-it.exec"), "read,write");
                }
                                                if (System.getProperty("tests.gradle") == null) {
                    perms.add(new RuntimePermission("setIO"));
                }

                                                                perms.add(new SocketPermission("localhost:0", "listen,resolve"));
                                                perms.add(new SocketPermission("localhost:1024-", "listen,resolve"));

                                Map<String, URL> codebases = Security.getCodebaseJarMap(JarHell.parseClassPath());
                if (System.getProperty("tests.gradle") == null) {
                                        addClassCodebase(codebases,"plugin-classloader", "org.elasticsearch.plugins.ExtendedPluginsClassLoader");
                    addClassCodebase(codebases,"elasticsearch-nio", "org.elasticsearch.nio.ChannelFactory");
                    addClassCodebase(codebases, "elasticsearch-secure-sm", "org.elasticsearch.secure_sm.SecureSM");
                }
                final Policy testFramework = Security.readPolicy(Bootstrap.class.getResource("test-framework.policy"), codebases);
                final Policy esPolicy = new ESPolicy(codebases, perms, getPluginPermissions(), true);
                Policy.setPolicy(new Policy() {
                    @Override
                    public boolean implies(ProtectionDomain domain, Permission permission) {
                                                return esPolicy.implies(domain, permission) || testFramework.implies(domain, permission);
                    }
                });
                System.setSecurityManager(SecureSM.createTestSecureSM());
                Security.selfTest();

                                                for (URL url : Collections.list(BootstrapForTesting.class.getClassLoader().getResources(PluginInfo.ES_PLUGIN_PROPERTIES))) {
                    Properties properties = new Properties();
                    try (InputStream stream = FileSystemUtils.openFileURLStream(url)) {
                        properties.load(stream);
                    }
                    String clazz = properties.getProperty("classname");
                    if (clazz != null) {
                        Class.forName(clazz);
                    }
                }
            } catch (Exception e) {
                throw new RuntimeException("unable to install test security manager", e);
            }
        }
    }

    
    private static void addClassCodebase(Map<String, URL> codebases, String name, String classname) {
        try {
            Class clazz = BootstrapForTesting.class.getClassLoader().loadClass(classname);
            if (codebases.put(name, clazz.getProtectionDomain().getCodeSource().getLocation()) != null) {
                throw new IllegalStateException("Already added " + name + " codebase for testing");
            }
        } catch (ClassNotFoundException e) {
                                }
    }

    
    @SuppressForbidden(reason = "accesses fully qualified URLs to configure security")
    static Map<String,Policy> getPluginPermissions() throws Exception {
        List<URL> pluginPolicies = Collections.list(BootstrapForTesting.class.getClassLoader().getResources(PluginInfo.ES_PLUGIN_POLICY));
        if (pluginPolicies.isEmpty()) {
            return Collections.emptyMap();
        }

                Set<URL> codebases = new HashSet<>(parseClassPathWithSymlinks());
        Set<URL> excluded = new HashSet<>(Arrays.asList(
                                Bootstrap.class.getProtectionDomain().getCodeSource().getLocation(),
                                BootstrapForTesting.class.getProtectionDomain().getCodeSource().getLocation(),
                                LuceneTestCase.class.getProtectionDomain().getCodeSource().getLocation(),
                                RandomizedRunner.class.getProtectionDomain().getCodeSource().getLocation(),
                                Assert.class.getProtectionDomain().getCodeSource().getLocation()
        ));
        codebases.removeAll(excluded);

                final List<Policy> policies = new ArrayList<>(pluginPolicies.size());
        for (URL policyFile : pluginPolicies) {
            policies.add(Security.readPolicy(policyFile, Security.getCodebaseJarMap(codebases)));
        }

                Map<String,Policy> map = new HashMap<>();
        for (URL url : codebases) {
            map.put(url.getFile(), new Policy() {
                @Override
                public boolean implies(ProtectionDomain domain, Permission permission) {
                                        for (Policy p : policies) {
                        if (p.implies(domain, permission)) {
                            return true;
                        }
                    }
                    return false;
                }
            });
        }
        return Collections.unmodifiableMap(map);
    }

    
    @SuppressForbidden(reason = "does evil stuff with paths and urls because devs and jenkins do evil stuff with paths and urls")
    static Set<URL> parseClassPathWithSymlinks() throws Exception {
        Set<URL> raw = JarHell.parseClassPath();
        Set<URL> cooked = new HashSet<>(raw.size());
        for (URL url : raw) {
            boolean added = cooked.add(PathUtils.get(url.toURI()).toRealPath().toUri().toURL());
            if (added == false) {
                throw new IllegalStateException("Duplicate in classpath after resolving symlinks: " + url);
            }
        }
        return raw;
    }

        public static void ensureInitialized() {}
}
