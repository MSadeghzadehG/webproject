

package org.elasticsearch.action.get;

import com.carrotsearch.hppc.IntArrayList;
import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class MultiGetShardResponse extends ActionResponse {

    IntArrayList locations;
    List<GetResponse> responses;
    List<MultiGetResponse.Failure> failures;

    MultiGetShardResponse() {
        locations = new IntArrayList();
        responses = new ArrayList<>();
        failures = new ArrayList<>();
    }

    public void add(int location, GetResponse response) {
        locations.add(location);
        responses.add(response);
        failures.add(null);
    }

    public void add(int location, MultiGetResponse.Failure failure) {
        locations.add(location);
        responses.add(null);
        failures.add(failure);
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        int size = in.readVInt();
        locations = new IntArrayList(size);
        responses = new ArrayList<>(size);
        failures = new ArrayList<>(size);
        for (int i = 0; i < size; i++) {
            locations.add(in.readVInt());
            if (in.readBoolean()) {
                GetResponse response = new GetResponse();
                response.readFrom(in);
                responses.add(response);
            } else {
                responses.add(null);
            }
            if (in.readBoolean()) {
                failures.add(MultiGetResponse.Failure.readFailure(in));
            } else {
                failures.add(null);
            }
        }
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeVInt(locations.size());
        for (int i = 0; i < locations.size(); i++) {
            out.writeVInt(locations.get(i));
            if (responses.get(i) == null) {
                out.writeBoolean(false);
            } else {
                out.writeBoolean(true);
                responses.get(i).writeTo(out);
            }
            if (failures.get(i) == null) {
                out.writeBoolean(false);
            } else {
                out.writeBoolean(true);
                failures.get(i).writeTo(out);
            }
        }
    }
}