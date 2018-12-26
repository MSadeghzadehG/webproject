

package org.elasticsearch.action.support.replication;

import org.elasticsearch.common.Strings;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.tasks.Task;
import org.elasticsearch.tasks.TaskId;

import java.io.IOException;
import java.util.Map;

import static java.util.Objects.requireNonNull;


public class ReplicationTask extends Task {
    private volatile String phase = "starting";

    public ReplicationTask(long id, String type, String action, String description, TaskId parentTaskId, Map<String, String> headers) {
        super(id, type, action, description, parentTaskId, headers);
    }

    
    public void setPhase(String phase) {
        this.phase = phase;
    }

    
    public String getPhase() {
        return phase;
    }

    @Override
    public Status getStatus() {
        return new Status(phase);
    }

    public static class Status implements Task.Status {
        public static final String NAME = "replication";

        private final String phase;

        public Status(String phase) {
            this.phase = requireNonNull(phase, "Phase cannot be null");
        }

        public Status(StreamInput in) throws IOException {
            phase = in.readString();
        }

        @Override
        public String getWriteableName() {
            return NAME;
        }

        @Override
        public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
            builder.startObject();
            builder.field("phase", phase);
            builder.endObject();
            return builder;
        }

        @Override
        public void writeTo(StreamOutput out) throws IOException {
            out.writeString(phase);
        }

        @Override
        public String toString() {
            return Strings.toString(this);
        }

                @Override
        public boolean equals(Object obj) {
            if (obj == null || obj.getClass() != ReplicationTask.Status.class) {
                return false;
            }
            ReplicationTask.Status other = (Status) obj;
            return phase.equals(other.phase);
        }

        @Override
        public int hashCode() {
            return phase.hashCode();
        }
    }
}
