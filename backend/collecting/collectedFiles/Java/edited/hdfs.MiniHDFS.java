

package hdfs;

import java.io.File;
import java.lang.management.ManagementFactory;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.CommonConfigurationKeysPublic;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileUtil;
import org.apache.hadoop.fs.permission.AclEntry;
import org.apache.hadoop.fs.permission.AclEntryType;
import org.apache.hadoop.fs.permission.FsAction;
import org.apache.hadoop.hdfs.DFSConfigKeys;
import org.apache.hadoop.hdfs.MiniDFSCluster;
import org.apache.hadoop.hdfs.MiniDFSNNTopology;
import org.apache.hadoop.hdfs.client.HdfsClientConfigKeys;
import org.apache.hadoop.hdfs.server.namenode.ha.HATestUtil;
import org.apache.hadoop.security.UserGroupInformation;


public class MiniHDFS {

    private static String PORT_FILE_NAME = "ports";
    private static String PID_FILE_NAME = "pid";

    public static void main(String[] args) throws Exception {
        if (args.length != 1 && args.length != 3) {
            throw new IllegalArgumentException("Expected: MiniHDFS <baseDirectory> [<kerberosPrincipal> <kerberosKeytab>], " +
                "got: " + Arrays.toString(args));
        }
        boolean secure = args.length == 3;

                Path baseDir = Paths.get(args[0]);
                if (System.getenv("HADOOP_HOME") == null) {
            Path hadoopHome = baseDir.resolve("hadoop-home");
            Files.createDirectories(hadoopHome);
            System.setProperty("hadoop.home.dir", hadoopHome.toAbsolutePath().toString());
        }
                Path hdfsHome = baseDir.resolve("hdfs-data");

                Configuration cfg = new Configuration();
        cfg.set(MiniDFSCluster.HDFS_MINIDFS_BASEDIR, hdfsHome.toAbsolutePath().toString());
                cfg.set(DFSConfigKeys.DFS_DATANODE_DATA_DIR_PERMISSION_KEY, "766");

                if (secure) {
            String kerberosPrincipal = args[1];
            String keytabFile = args[2];

            cfg.set(CommonConfigurationKeysPublic.HADOOP_SECURITY_AUTHENTICATION, "kerberos");
            cfg.set(CommonConfigurationKeysPublic.HADOOP_SECURITY_AUTHORIZATION, "true");
            cfg.set(DFSConfigKeys.DFS_NAMENODE_KERBEROS_PRINCIPAL_KEY, kerberosPrincipal);
            cfg.set(DFSConfigKeys.DFS_DATANODE_KERBEROS_PRINCIPAL_KEY, kerberosPrincipal);
            cfg.set(DFSConfigKeys.DFS_WEB_AUTHENTICATION_KERBEROS_PRINCIPAL_KEY, kerberosPrincipal);
            cfg.set(DFSConfigKeys.DFS_NAMENODE_KEYTAB_FILE_KEY, keytabFile);
            cfg.set(DFSConfigKeys.DFS_DATANODE_KEYTAB_FILE_KEY, keytabFile);
            cfg.set(DFSConfigKeys.DFS_NAMENODE_ACLS_ENABLED_KEY, "true");
            cfg.set(DFSConfigKeys.DFS_BLOCK_ACCESS_TOKEN_ENABLE_KEY, "true");
            cfg.set(DFSConfigKeys.IGNORE_SECURE_PORTS_FOR_TESTING_KEY, "true");
            cfg.set(DFSConfigKeys.DFS_ENCRYPT_DATA_TRANSFER_KEY, "true");
        }

        UserGroupInformation.setConfiguration(cfg);

                MiniDFSCluster.Builder builder = new MiniDFSCluster.Builder(cfg);
        if (secure) {
            builder.nameNodePort(9998);
        } else {
            builder.nameNodePort(9999);
        }

                String haNameService = System.getProperty("ha-nameservice");
        boolean haEnabled = haNameService != null;
        if (haEnabled) {
            MiniDFSNNTopology.NNConf nn1 = new MiniDFSNNTopology.NNConf("nn1").setIpcPort(10001);
            MiniDFSNNTopology.NNConf nn2 = new MiniDFSNNTopology.NNConf("nn2").setIpcPort(10002);
            MiniDFSNNTopology.NSConf nameservice = new MiniDFSNNTopology.NSConf(haNameService).addNN(nn1).addNN(nn2);
            MiniDFSNNTopology namenodeTopology = new MiniDFSNNTopology().addNameservice(nameservice);
            builder.nnTopology(namenodeTopology);
        }

        MiniDFSCluster dfs = builder.build();

                org.apache.hadoop.fs.Path esUserPath = new org.apache.hadoop.fs.Path("/user/elasticsearch");

        FileSystem fs;
        if (haEnabled) {
            dfs.transitionToActive(0);
            fs = HATestUtil.configureFailoverFs(dfs, cfg);
        } else {
            fs = dfs.getFileSystem();
        }

        try {
                        fs.mkdirs(esUserPath);
            if (UserGroupInformation.isSecurityEnabled()) {
                List<AclEntry> acls = new ArrayList<>();
                acls.add(new AclEntry.Builder().setType(AclEntryType.USER).setName("elasticsearch").setPermission(FsAction.ALL).build());
                fs.modifyAclEntries(esUserPath, acls);
            }

                        String directoryName = "readonly-repository";
            String archiveName = directoryName + ".tar.gz";
            URL readOnlyRepositoryArchiveURL = MiniHDFS.class.getClassLoader().getResource(archiveName);
            if (readOnlyRepositoryArchiveURL != null) {
                Path tempDirectory = Files.createTempDirectory(MiniHDFS.class.getName());
                File readOnlyRepositoryArchive = tempDirectory.resolve(archiveName).toFile();
                FileUtils.copyURLToFile(readOnlyRepositoryArchiveURL, readOnlyRepositoryArchive);
                FileUtil.unTar(readOnlyRepositoryArchive, tempDirectory.toFile());

                fs.copyFromLocalFile(true, true,
                    new org.apache.hadoop.fs.Path(tempDirectory.resolve(directoryName).toAbsolutePath().toUri()),
                    esUserPath.suffix("/existing/" + directoryName)
                );

                FileUtils.deleteDirectory(tempDirectory.toFile());
            }
        } finally {
            fs.close();
        }

                Path tmp = Files.createTempFile(baseDir, null, null);
        String pid = ManagementFactory.getRuntimeMXBean().getName().split("@")[0];
        Files.write(tmp, pid.getBytes(StandardCharsets.UTF_8));
        Files.move(tmp, baseDir.resolve(PID_FILE_NAME), StandardCopyOption.ATOMIC_MOVE);

                String portFileContent = Integer.toString(dfs.getNameNodePort(0));
        if (haEnabled) {
            portFileContent = portFileContent + "\n" + Integer.toString(dfs.getNameNodePort(1));
        }
        tmp = Files.createTempFile(baseDir, null, null);
        Files.write(tmp, portFileContent.getBytes(StandardCharsets.UTF_8));
        Files.move(tmp, baseDir.resolve(PORT_FILE_NAME), StandardCopyOption.ATOMIC_MOVE);
    }
}