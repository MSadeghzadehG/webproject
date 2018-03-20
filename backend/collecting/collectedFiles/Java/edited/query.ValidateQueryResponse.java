

package org.elasticsearch.action.admin.indices.validate.query;

import org.elasticsearch.action.support.DefaultShardOperationFailedException;
import org.elasticsearch.action.support.broadcast.BroadcastResponse;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import static org.elasticsearch.action.admin.indices.validate.query.QueryExplanation.readQueryExplanation;


public class ValidateQueryResponse extends BroadcastResponse {

    private boolean valid;
    
    private List<QueryExplanation> queryExplanations;

    ValidateQueryResponse() {

    }

    ValidateQueryResponse(boolean valid, List<QueryExplanation> queryExplanations, int totalShards, int successfulShards, int failedShards,
                          List<DefaultShardOperationFailedException> shardFailures) {
        super(totalShards, successfulShards, failedShards, shardFailures);
        this.valid = valid;
        this.queryExplanations = queryExplanations;
        if (queryExplanations == null) {
            this.queryExplanations = Collections.emptyList();
        }
    }

    
    public boolean isValid() {
        return valid;
    }

    
    public List<? extends QueryExplanation> getQueryExplanation() {
        if (queryExplanations == null) {
            return Collections.emptyList();
        }
        return queryExplanations;
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        valid = in.readBoolean();
        int size = in.readVInt();
        if (size > 0) {
            queryExplanations = new ArrayList<>(size);
            for (int i = 0; i < size; i++) {
                queryExplanations.add(readQueryExplanation(in));
            }
        }
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeBoolean(valid);
        out.writeVInt(queryExplanations.size());
        for (QueryExplanation exp : queryExplanations) {
            exp.writeTo(out);
        }

    }
}
