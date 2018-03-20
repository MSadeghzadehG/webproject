

package org.elasticsearch.action.support.tasks;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.action.FailedNodeException;
import org.elasticsearch.action.TaskOperationFailure;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.tasks.TaskId;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.stream.Stream;

import static java.util.stream.Collectors.toList;
import static org.elasticsearch.ExceptionsHelper.rethrowAndSuppress;



public class BaseTasksResponse extends ActionResponse {
    private List<TaskOperationFailure> taskFailures;
    private List<FailedNodeException> nodeFailures;

    public BaseTasksResponse(List<TaskOperationFailure> taskFailures, List<? extends FailedNodeException> nodeFailures) {
        this.taskFailures = taskFailures == null ? Collections.emptyList() : Collections.unmodifiableList(new ArrayList<>(taskFailures));
        this.nodeFailures = nodeFailures == null ? Collections.emptyList() : Collections.unmodifiableList(new ArrayList<>(nodeFailures));
    }

    
    public List<TaskOperationFailure> getTaskFailures() {
        return taskFailures;
    }

    
    public List<FailedNodeException> getNodeFailures() {
        return nodeFailures;
    }

    
    public void rethrowFailures(String operationName) {
        rethrowAndSuppress(Stream.concat(
                    getNodeFailures().stream(),
                    getTaskFailures().stream().map(f -> new ElasticsearchException(
                            "{} of [{}] failed", f.getCause(), operationName, new TaskId(f.getNodeId(), f.getTaskId()))))
                .collect(toList()));
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        int size = in.readVInt();
        List<TaskOperationFailure> taskFailures = new ArrayList<>(size);
        for (int i = 0; i < size; i++) {
            taskFailures.add(new TaskOperationFailure(in));
        }
        size = in.readVInt();
        this.taskFailures = Collections.unmodifiableList(taskFailures);
        List<FailedNodeException> nodeFailures = new ArrayList<>(size);
        for (int i = 0; i < size; i++) {
            nodeFailures.add(new FailedNodeException(in));
        }
        this.nodeFailures = Collections.unmodifiableList(nodeFailures);
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeVInt(taskFailures.size());
        for (TaskOperationFailure exp : taskFailures) {
            exp.writeTo(out);
        }
        out.writeVInt(nodeFailures.size());
        for (FailedNodeException exp : nodeFailures) {
            exp.writeTo(out);
        }
    }
}
