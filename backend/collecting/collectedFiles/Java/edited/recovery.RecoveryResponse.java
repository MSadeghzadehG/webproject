

package org.elasticsearch.action.admin.indices.recovery;

import org.elasticsearch.action.support.DefaultShardOperationFailedException;
import org.elasticsearch.action.support.broadcast.BroadcastResponse;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.indices.recovery.RecoveryState;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class RecoveryResponse extends BroadcastResponse implements ToXContentFragment {

    private boolean detailed = false;
    private Map<String, List<RecoveryState>> shardRecoveryStates = new HashMap<>();

    public RecoveryResponse() { }

    
    public RecoveryResponse(int totalShards, int successfulShards, int failedShards, boolean detailed,
                            Map<String, List<RecoveryState>> shardRecoveryStates,
                            List<DefaultShardOperationFailedException> shardFailures) {
        super(totalShards, successfulShards, failedShards, shardFailures);
        this.shardRecoveryStates = shardRecoveryStates;
        this.detailed = detailed;
    }

    public boolean hasRecoveries() {
        return shardRecoveryStates.size() > 0;
    }

    public boolean detailed() {
        return detailed;
    }

    public void detailed(boolean detailed) {
        this.detailed = detailed;
    }

    public Map<String, List<RecoveryState>> shardRecoveryStates() {
        return shardRecoveryStates;
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        if (hasRecoveries()) {
            for (String index : shardRecoveryStates.keySet()) {
                List<RecoveryState> recoveryStates = shardRecoveryStates.get(index);
                if (recoveryStates == null || recoveryStates.size() == 0) {
                    continue;
                }
                builder.startObject(index);
                builder.startArray("shards");
                for (RecoveryState recoveryState : recoveryStates) {
                    builder.startObject();
                    recoveryState.toXContent(builder, params);
                    builder.endObject();
                }
                builder.endArray();
                builder.endObject();
            }
        }
        return builder;
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeVInt(shardRecoveryStates.size());
        for (Map.Entry<String, List<RecoveryState>> entry : shardRecoveryStates.entrySet()) {
            out.writeString(entry.getKey());
            out.writeVInt(entry.getValue().size());
            for (RecoveryState recoveryState : entry.getValue()) {
                recoveryState.writeTo(out);
            }
        }
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        int size = in.readVInt();
        for (int i = 0; i < size; i++) {
            String s = in.readString();
            int listSize = in.readVInt();
            List<RecoveryState> list = new ArrayList<>(listSize);
            for (int j = 0; j < listSize; j++) {
                list.add(RecoveryState.readRecoveryState(in));
            }
            shardRecoveryStates.put(s, list);
        }
    }

    @Override
    public String toString() {
        return Strings.toString(this, true, true);
    }
}