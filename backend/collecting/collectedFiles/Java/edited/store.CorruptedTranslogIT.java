

package org.elasticsearch.index.store;

import com.carrotsearch.randomizedtesting.generators.RandomPicks;
import org.elasticsearch.action.admin.cluster.node.stats.NodesStatsResponse;
import org.elasticsearch.action.index.IndexRequestBuilder;
import org.elasticsearch.action.search.SearchPhaseExecutionException;
import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.routing.GroupShardsIterator;
import org.elasticsearch.cluster.routing.ShardIterator;
import org.elasticsearch.cluster.routing.ShardRouting;
import org.elasticsearch.common.Priority;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.ByteSizeUnit;
import org.elasticsearch.common.unit.ByteSizeValue;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.index.Index;
import org.elasticsearch.index.IndexSettings;
import org.elasticsearch.index.MockEngineFactoryPlugin;
import org.elasticsearch.index.translog.TestTranslog;
import org.elasticsearch.monitor.fs.FsInfo;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.test.ESIntegTestCase;
import org.elasticsearch.test.engine.MockEngineSupport;
import org.elasticsearch.test.transport.MockTransportService;

import java.io.IOException;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import static org.elasticsearch.common.util.CollectionUtils.iterableAsArrayList;
import static org.elasticsearch.index.query.QueryBuilders.matchAllQuery;
import static org.elasticsearch.test.hamcrest.ElasticsearchAssertions.assertAcked;


@ESIntegTestCase.ClusterScope(scope= ESIntegTestCase.Scope.SUITE, numDataNodes = 0)
public class CorruptedTranslogIT extends ESIntegTestCase {
    @Override
    protected Collection<Class<? extends Plugin>> nodePlugins() {
        return Arrays.asList(MockTransportService.TestPlugin.class, MockEngineFactoryPlugin.class);
    }

    public void testCorruptTranslogFiles() throws Exception {
        internalCluster().startNodes(1, Settings.EMPTY);

        assertAcked(prepareCreate("test").setSettings(Settings.builder()
                .put("index.number_of_shards", 1)
                .put("index.number_of_replicas", 0)
                .put("index.refresh_interval", "-1")
                .put(MockEngineSupport.DISABLE_FLUSH_ON_CLOSE.getKey(), true)         ));

                int numDocs = scaledRandomIntBetween(100, 1000);
        IndexRequestBuilder[] builders = new IndexRequestBuilder[numDocs];
        for (int i = 0; i < builders.length; i++) {
            builders[i] = client().prepareIndex("test", "type").setSource("foo", "bar");
        }
        disableTranslogFlush("test");
        indexRandom(false, false, false, Arrays.asList(builders));  
                corruptRandomTranslogFiles();

                internalCluster().fullRestart();
        client().admin().cluster().prepareHealth().setWaitForYellowStatus().setTimeout(new TimeValue(1000, TimeUnit.MILLISECONDS)).setWaitForEvents(Priority.LANGUID).get();

        try {
            client().prepareSearch("test").setQuery(matchAllQuery()).get();
            fail("all shards should be failed due to a corrupted translog");
        } catch (SearchPhaseExecutionException e) {
                                }
    }


    private void corruptRandomTranslogFiles() throws IOException {
        ClusterState state = client().admin().cluster().prepareState().get().getState();
        GroupShardsIterator shardIterators = state.getRoutingTable().activePrimaryShardsGrouped(new String[]{"test"}, false);
        final Index test = state.metaData().index("test").getIndex();
        List<ShardIterator> iterators = iterableAsArrayList(shardIterators);
        ShardIterator shardIterator = RandomPicks.randomFrom(random(), iterators);
        ShardRouting shardRouting = shardIterator.nextOrNull();
        assertNotNull(shardRouting);
        assertTrue(shardRouting.primary());
        assertTrue(shardRouting.assignedToNode());
        String nodeId = shardRouting.currentNodeId();
        NodesStatsResponse nodeStatses = client().admin().cluster().prepareNodesStats(nodeId).setFs(true).get();
        Set<Path> translogDirs = new HashSet<>();
        for (FsInfo.Path fsPath : nodeStatses.getNodes().get(0).getFs()) {
            String path = fsPath.getPath();
            String relativeDataLocationPath = "indices/" + test.getUUID() + "/" + Integer.toString(shardRouting.getId()) + "/translog";
            Path translogDir = PathUtils.get(path).resolve(relativeDataLocationPath);
            translogDirs.add(translogDir);
        }
        TestTranslog.corruptTranslogFiles(logger, random(), translogDirs);
    }

    
    private static void disableTranslogFlush(String index) {
        Settings settings = Settings.builder().put(IndexSettings.INDEX_TRANSLOG_FLUSH_THRESHOLD_SIZE_SETTING.getKey(), new ByteSizeValue(1, ByteSizeUnit.PB)).build();
        client().admin().indices().prepareUpdateSettings(index).setSettings(settings).get();
    }

    
    private static void enableTranslogFlush(String index) {
        Settings settings = Settings.builder().put(IndexSettings.INDEX_TRANSLOG_FLUSH_THRESHOLD_SIZE_SETTING.getKey(), new ByteSizeValue(512, ByteSizeUnit.MB)).build();
        client().admin().indices().prepareUpdateSettings(index).setSettings(settings).get();
    }
}
