

package org.elasticsearch.action.bulk;

import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.admin.indices.create.CreateIndexResponse;
import org.elasticsearch.action.delete.DeleteRequest;
import org.elasticsearch.action.index.IndexRequest;
import org.elasticsearch.action.support.ActionFilters;
import org.elasticsearch.action.update.UpdateRequest;
import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.service.ClusterService;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.common.util.concurrent.AtomicArray;
import org.elasticsearch.index.IndexNotFoundException;
import org.elasticsearch.index.VersionType;
import org.elasticsearch.tasks.Task;
import org.elasticsearch.test.ESTestCase;
import org.elasticsearch.transport.TransportService;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.function.Function;

import static java.util.Collections.emptySet;
import static java.util.Collections.singleton;
import static org.mockito.Mockito.mock;

public class TransportBulkActionIndicesThatCannotBeCreatedTests extends ESTestCase {
    public void testNonExceptional() {
        BulkRequest bulkRequest = new BulkRequest();
        bulkRequest.add(new IndexRequest(randomAlphaOfLength(5)));
        bulkRequest.add(new IndexRequest(randomAlphaOfLength(5)));
        bulkRequest.add(new DeleteRequest(randomAlphaOfLength(5)));
        bulkRequest.add(new UpdateRequest(randomAlphaOfLength(5), randomAlphaOfLength(5), randomAlphaOfLength(5)));
                indicesThatCannotBeCreatedTestCase(emptySet(), bulkRequest, null);
                indicesThatCannotBeCreatedTestCase(emptySet(), bulkRequest, index -> true);
                indicesThatCannotBeCreatedTestCase(emptySet(), bulkRequest, index -> false);
                indicesThatCannotBeCreatedTestCase(emptySet(), bulkRequest, index -> randomBoolean());
    }

    public void testAllFail() {
        BulkRequest bulkRequest = new BulkRequest();
        bulkRequest.add(new IndexRequest("no"));
        bulkRequest.add(new IndexRequest("can't"));
        bulkRequest.add(new DeleteRequest("do").version(0).versionType(VersionType.EXTERNAL));
        bulkRequest.add(new UpdateRequest("nothin", randomAlphaOfLength(5), randomAlphaOfLength(5)));
        indicesThatCannotBeCreatedTestCase(new HashSet<>(Arrays.asList("no", "can't", "do", "nothin")), bulkRequest, index -> {
            throw new IndexNotFoundException("Can't make it because I say so");
        });
    }

    public void testSomeFail() {
        BulkRequest bulkRequest = new BulkRequest();
        bulkRequest.add(new IndexRequest("ok"));
        bulkRequest.add(new IndexRequest("bad"));
                indicesThatCannotBeCreatedTestCase(singleton("bad"), bulkRequest, index -> {
            if (index.equals("bad")) {
                throw new IndexNotFoundException("Can't make it because I say so");
            }
            return true;
        });
                indicesThatCannotBeCreatedTestCase(singleton("bad"), bulkRequest, index -> {
            if (index.equals("bad")) {
                throw new IndexNotFoundException("Can't make it because I say so");
            }
            return false;
        });
    }


    private void indicesThatCannotBeCreatedTestCase(Set<String> expected,
            BulkRequest bulkRequest, Function<String, Boolean> shouldAutoCreate) {
        TransportBulkAction action = new TransportBulkAction(Settings.EMPTY, null, mock(TransportService.class), mock(ClusterService.class),
                null, null, null, mock(ActionFilters.class), null, null) {
            @Override
            void executeBulk(Task task, BulkRequest bulkRequest, long startTimeNanos, ActionListener<BulkResponse> listener,
                    AtomicArray<BulkItemResponse> responses, Map<String, IndexNotFoundException> indicesThatCannotBeCreated) {
                assertEquals(expected, indicesThatCannotBeCreated.keySet());
            }

            @Override
            boolean needToCheck() {
                return null != shouldAutoCreate;             }

            @Override
            boolean shouldAutoCreate(String index, ClusterState state) {
                return shouldAutoCreate.apply(index);
            }

            @Override
            void createIndex(String index, TimeValue timeout, ActionListener<CreateIndexResponse> listener) {
                                listener.onResponse(new CreateIndexResponse(true, true, index) {});
            }
        };
        action.doExecute(null, bulkRequest, null);
    }
}
