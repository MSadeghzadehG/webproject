
package org.elasticsearch.repositories.hdfs;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Collections;
import java.util.Map;

import org.apache.hadoop.hdfs.protocolPB.ClientNamenodeProtocolPB;
import org.apache.hadoop.security.KerberosInfo;
import org.apache.hadoop.security.SecurityUtil;
import org.elasticsearch.SpecialPermission;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.env.Environment;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.RepositoryPlugin;
import org.elasticsearch.repositories.Repository;

public final class HdfsPlugin extends Plugin implements RepositoryPlugin {

        static {
        SpecialPermission.check();
        AccessController.doPrivileged((PrivilegedAction<Void>) HdfsPlugin::evilHadoopInit);
        AccessController.doPrivileged((PrivilegedAction<Void>) HdfsPlugin::eagerInit);
    }

    @SuppressForbidden(reason = "Needs a security hack for hadoop on windows, until HADOOP-XXXX is fixed")
    private static Void evilHadoopInit() {
                                                Path hadoopHome = null;
        String oldValue = null;
        try {
            hadoopHome = Files.createTempDirectory("hadoop").toAbsolutePath();
            oldValue = System.setProperty("hadoop.home.dir", hadoopHome.toString());
            Class.forName("org.apache.hadoop.security.UserGroupInformation");
            Class.forName("org.apache.hadoop.util.StringUtils");
            Class.forName("org.apache.hadoop.util.ShutdownHookManager");
            Class.forName("org.apache.hadoop.conf.Configuration");
        } catch (ClassNotFoundException | IOException e) {
            throw new RuntimeException(e);
        } finally {
                        if (oldValue == null) {
                System.clearProperty("hadoop.home.dir");
            } else {
                System.setProperty("hadoop.home.dir", oldValue);
            }
            try {
                                if (hadoopHome != null) {
                    Files.delete(hadoopHome);
                }
            } catch (IOException thisIsBestEffort) {}
        }
        return null;
    }

    private static Void eagerInit() {
        
        ClassLoader oldCCL = Thread.currentThread().getContextClassLoader();
        try {
            Thread.currentThread().setContextClassLoader(HdfsRepository.class.getClassLoader());
            KerberosInfo info = SecurityUtil.getKerberosInfo(ClientNamenodeProtocolPB.class, null);
                        if (info == null) {
                throw new RuntimeException("Could not initialize SecurityUtil: " +
                    "Unable to find services for [org.apache.hadoop.security.SecurityInfo]");
            }
        } finally {
            Thread.currentThread().setContextClassLoader(oldCCL);
        }
        return null;
    }

    @Override
    public Map<String, Repository.Factory> getRepositories(Environment env, NamedXContentRegistry namedXContentRegistry) {
        return Collections.singletonMap("hdfs", (metadata) -> new HdfsRepository(metadata, env, namedXContentRegistry));
    }
}
