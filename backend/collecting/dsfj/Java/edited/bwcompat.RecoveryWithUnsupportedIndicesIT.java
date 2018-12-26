
package org.elasticsearch.bwcompat;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

import org.apache.lucene.util.LuceneTestCase;
import org.apache.lucene.util.TestUtil;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.env.NodeEnvironment;
import org.elasticsearch.test.ESIntegTestCase;

import static org.hamcrest.Matchers.containsString;

@LuceneTestCase.SuppressCodecs("*")
@ESIntegTestCase.ClusterScope(scope = ESIntegTestCase.Scope.TEST, numDataNodes = 0, minNumDataNodes = 0, maxNumDataNodes = 0)
public class RecoveryWithUnsupportedIndicesIT extends ESIntegTestCase {

    
    protected Settings prepareBackwardsDataDir(Path backwardsIndex) throws IOException {
        Path indexDir = createTempDir();
        Path dataDir = indexDir.resolve("data");
        try (InputStream stream = Files.newInputStream(backwardsIndex)) {
            TestUtil.unzip(stream, indexDir);
        }
        assertTrue(Files.exists(dataDir));

                final Path[] list;
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(dataDir)) {
            List<Path> dirs = new ArrayList<>();
            for (Path p : stream) {
                if (!p.getFileName().toString().startsWith("extra")) {
                    dirs.add(p);
                }
            }
            list = dirs.toArray(new Path[0]);
        }

        if (list.length != 1) {
            StringBuilder builder = new StringBuilder("Backwards index must contain exactly one cluster\n");
            for (Path line : list) {
                builder.append(line.toString()).append('\n');
            }
            throw new IllegalStateException(builder.toString());
        }
        Path src = list[0].resolve(NodeEnvironment.NODES_FOLDER);
        Path dest = dataDir.resolve(NodeEnvironment.NODES_FOLDER);
        assertTrue(Files.exists(src));
        Files.move(src, dest);
        assertFalse(Files.exists(src));
        assertTrue(Files.exists(dest));
        Settings.Builder builder = Settings.builder()
            .put(Environment.PATH_DATA_SETTING.getKey(), dataDir.toAbsolutePath());

        return builder.build();
    }

    public void testUpgradeStartClusterOn_0_20_6() throws Exception {
        String indexName = "unsupported-0.20.6";

        logger.info("Checking static index {}", indexName);
        Settings nodeSettings = prepareBackwardsDataDir(getBwcIndicesPath().resolve(indexName + ".zip"));
        try {
            internalCluster().startNode(nodeSettings);
            fail();
        } catch (Exception ex) {
            assertThat(ex.getCause().getCause().getMessage(), containsString(" was created before v2.0.0.beta1 and wasn't upgraded"));
        }
    }
}
