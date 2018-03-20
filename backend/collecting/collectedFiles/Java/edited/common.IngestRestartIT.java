
package org.elasticsearch.ingest.common;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.ExceptionsHelper;
import org.elasticsearch.action.support.WriteRequest;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.bytes.BytesArray;
import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.util.set.Sets;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.script.MockScriptEngine;
import org.elasticsearch.script.MockScriptPlugin;
import org.elasticsearch.test.ESIntegTestCase;
import org.elasticsearch.test.InternalTestCluster;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Map;
import java.util.function.Consumer;
import java.util.function.Function;

import static org.hamcrest.Matchers.equalTo;

@ESIntegTestCase.ClusterScope(numDataNodes = 0, numClientNodes = 0, scope = ESIntegTestCase.Scope.TEST)
public class IngestRestartIT extends ESIntegTestCase {

    @Override
    protected Collection<Class<? extends Plugin>> nodePlugins() {
        return Arrays.asList(IngestCommonPlugin.class, CustomScriptPlugin.class);
    }

    @Override
    protected boolean ignoreExternalCluster() {
        return true;
    }

    public static class CustomScriptPlugin extends MockScriptPlugin {
        @Override
        protected Map<String, Function<Map<String, Object>, Object>> pluginScripts() {
            return Collections.singletonMap("my_script", script -> {
                @SuppressWarnings("unchecked")
                Map<String, Object> ctx = (Map) script.get("ctx");
                ctx.put("z", 0);
                return null;
            });
        }
    }

    public void testScriptDisabled() throws Exception {
        String pipelineIdWithoutScript = randomAlphaOfLengthBetween(5, 10);
        String pipelineIdWithScript = pipelineIdWithoutScript + "_script";
        internalCluster().startNode();

        BytesReference pipelineWithScript = new BytesArray("{\n" +
            "  \"processors\" : [\n" +
            "      {\"script\" : {\"lang\": \"" + MockScriptEngine.NAME + "\", \"source\": \"my_script\"}}\n" +
            "  ]\n" +
            "}");
        BytesReference pipelineWithoutScript = new BytesArray("{\n" +
            "  \"processors\" : [\n" +
            "      {\"set\" : {\"field\": \"y\", \"value\": 0}}\n" +
            "  ]\n" +
            "}");

        Consumer<String> checkPipelineExists = (id) -> assertThat(client().admin().cluster().prepareGetPipeline(id)
                .get().pipelines().get(0).getId(), equalTo(id));

        client().admin().cluster().preparePutPipeline(pipelineIdWithScript, pipelineWithScript, XContentType.JSON).get();
        client().admin().cluster().preparePutPipeline(pipelineIdWithoutScript, pipelineWithoutScript, XContentType.JSON).get();

        checkPipelineExists.accept(pipelineIdWithScript);
        checkPipelineExists.accept(pipelineIdWithoutScript);


        internalCluster().stopCurrentMasterNode();
        internalCluster().startNode(Settings.builder().put("script.allowed_types", "none"));

        checkPipelineExists.accept(pipelineIdWithoutScript);
        checkPipelineExists.accept(pipelineIdWithScript);

        client().prepareIndex("index", "doc", "1")
            .setSource("x", 0)
            .setPipeline(pipelineIdWithoutScript)
            .setRefreshPolicy(WriteRequest.RefreshPolicy.IMMEDIATE)
            .get();

        ElasticsearchException exception = expectThrows(ElasticsearchException.class,
            () -> client().prepareIndex("index", "doc", "2")
                .setSource("x", 0)
                .setPipeline(pipelineIdWithScript)
                .setRefreshPolicy(WriteRequest.RefreshPolicy.IMMEDIATE)
                .get());
        assertThat(exception.getHeaderKeys(), equalTo(Sets.newHashSet("processor_type")));
        assertThat(exception.getHeader("processor_type"), equalTo(Arrays.asList("unknown")));
        assertThat(exception.getRootCause().getMessage(),
            equalTo("pipeline with id [" + pipelineIdWithScript + "] could not be loaded, caused by " +
                "[ElasticsearchParseException[Error updating pipeline with id [" + pipelineIdWithScript + "]]; " +
                "nested: ElasticsearchException[java.lang.IllegalArgumentException: cannot execute [inline] scripts]; " +
                "nested: IllegalArgumentException[cannot execute [inline] scripts];; " +
                "ElasticsearchException[java.lang.IllegalArgumentException: cannot execute [inline] scripts]; " +
                "nested: IllegalArgumentException[cannot execute [inline] scripts];; java.lang.IllegalArgumentException: " +
                "cannot execute [inline] scripts]"));

        Map<String, Object> source = client().prepareGet("index", "doc", "1").get().getSource();
        assertThat(source.get("x"), equalTo(0));
        assertThat(source.get("y"), equalTo(0));
    }

    public void testPipelineWithScriptProcessorThatHasStoredScript() throws Exception {
        internalCluster().startNode();

        client().admin().cluster().preparePutStoredScript()
                .setId("1")
                .setContent(new BytesArray("{\"script\": {\"lang\": \"" + MockScriptEngine.NAME +
                        "\", \"source\": \"my_script\"} }"), XContentType.JSON)
                .get();
        BytesReference pipeline = new BytesArray("{\n" +
                "  \"processors\" : [\n" +
                "      {\"set\" : {\"field\": \"y\", \"value\": 0}},\n" +
                "      {\"script\" : {\"id\": \"1\"}}\n" +
                "  ]\n" +
                "}");
        client().admin().cluster().preparePutPipeline("_id", pipeline, XContentType.JSON).get();

        client().prepareIndex("index", "doc", "1")
                .setSource("x", 0)
                .setPipeline("_id")
                .setRefreshPolicy(WriteRequest.RefreshPolicy.IMMEDIATE)
                .get();

        Map<String, Object> source = client().prepareGet("index", "doc", "1").get().getSource();
        assertThat(source.get("x"), equalTo(0));
        assertThat(source.get("y"), equalTo(0));
        assertThat(source.get("z"), equalTo(0));

                                        internalCluster().fullRestart();
        ensureYellow("index");

        client().prepareIndex("index", "doc", "2")
                .setSource("x", 0)
                .setPipeline("_id")
                .setRefreshPolicy(WriteRequest.RefreshPolicy.IMMEDIATE)
                .get();

        source = client().prepareGet("index", "doc", "2").get().getSource();
        assertThat(source.get("x"), equalTo(0));
        assertThat(source.get("y"), equalTo(0));
        assertThat(source.get("z"), equalTo(0));
    }

    public void testWithDedicatedIngestNode() throws Exception {
        String node = internalCluster().startNode();
        String ingestNode = internalCluster().startNode(Settings.builder()
                .put("node.master", false)
                .put("node.data", false)
        );

        BytesReference pipeline = new BytesArray("{\n" +
                "  \"processors\" : [\n" +
                "      {\"set\" : {\"field\": \"y\", \"value\": 0}}\n" +
                "  ]\n" +
                "}");
        client().admin().cluster().preparePutPipeline("_id", pipeline, XContentType.JSON).get();

        client().prepareIndex("index", "doc", "1")
                .setSource("x", 0)
                .setPipeline("_id")
                .setRefreshPolicy(WriteRequest.RefreshPolicy.IMMEDIATE)
                .get();

        Map<String, Object> source = client().prepareGet("index", "doc", "1").get().getSource();
        assertThat(source.get("x"), equalTo(0));
        assertThat(source.get("y"), equalTo(0));

        logger.info("Stopping");
        internalCluster().restartNode(node, new InternalTestCluster.RestartCallback());

        client(ingestNode).prepareIndex("index", "doc", "2")
                .setSource("x", 0)
                .setPipeline("_id")
                .setRefreshPolicy(WriteRequest.RefreshPolicy.IMMEDIATE)
                .get();

        source = client(ingestNode).prepareGet("index", "doc", "2").get().getSource();
        assertThat(source.get("x"), equalTo(0));
        assertThat(source.get("y"), equalTo(0));
    }

}
