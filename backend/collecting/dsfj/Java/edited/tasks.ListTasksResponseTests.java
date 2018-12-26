

package org.elasticsearch.tasks;

import org.elasticsearch.action.admin.cluster.node.tasks.list.ListTasksResponse;
import org.elasticsearch.test.ESTestCase;

import java.util.Collections;

import static java.util.Collections.emptyList;
import static java.util.Collections.singletonList;

public class ListTasksResponseTests extends ESTestCase {

    public void testEmptyToString() {
        assertEquals("{\"tasks\":{}}", new ListTasksResponse().toString());
    }

    public void testNonEmptyToString() {
        TaskInfo info = new TaskInfo(
            new TaskId("node1", 1), "dummy-type", "dummy-action", "dummy-description", null, 0, 1, true, new TaskId("node1", 0),
            Collections.singletonMap("foo", "bar"));
        ListTasksResponse tasksResponse = new ListTasksResponse(singletonList(info), emptyList(), emptyList());
        assertEquals("{\"tasks\":{\"node1:1\":{\"node\":\"node1\",\"id\":1,\"type\":\"dummy-type\",\"action\":\"dummy-action\","
                + "\"description\":\"dummy-description\",\"start_time_in_millis\":0,\"running_time_in_nanos\":1,\"cancellable\":true,"
                + "\"parent_task_id\":\"node1:0\",\"headers\":{\"foo\":\"bar\"}}}}", tasksResponse.toString());
    }
}
