

package org.elasticsearch.action.admin.indices.upgrade.post;

import org.elasticsearch.Version;
import org.elasticsearch.action.ActionRequestValidationException;
import org.elasticsearch.action.support.master.AcknowledgedRequest;
import org.elasticsearch.common.collect.Tuple;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import static org.elasticsearch.action.ValidateActions.addValidationError;


public class UpgradeSettingsRequest extends AcknowledgedRequest<UpgradeSettingsRequest> {

    private Map<String, Tuple<Version, String>> versions;

    public UpgradeSettingsRequest() {
    }

    
    public UpgradeSettingsRequest(Map<String, Tuple<Version, String>> versions) {
        this.versions = versions;
    }


    @Override
    public ActionRequestValidationException validate() {
        ActionRequestValidationException validationException = null;
        if (versions.isEmpty()) {
            validationException = addValidationError("no indices to update", validationException);
        }
        return validationException;
    }


    Map<String, Tuple<Version, String>> versions() {
        return versions;
    }

    
    public UpgradeSettingsRequest versions(Map<String, Tuple<Version, String>> versions) {
        this.versions = versions;
        return this;
    }


    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        int size = in.readVInt();
        versions = new HashMap<>();
        for (int i=0; i<size; i++) {
            String index = in.readString();
            Version upgradeVersion = Version.readVersion(in);
            String oldestLuceneSegment = in.readString();
            versions.put(index, new Tuple<>(upgradeVersion, oldestLuceneSegment));
        }
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeVInt(versions.size());
        for(Map.Entry<String, Tuple<Version, String>> entry : versions.entrySet()) {
            out.writeString(entry.getKey());
            Version.writeVersion(entry.getValue().v1(), out);
            out.writeString(entry.getValue().v2());
        }
    }
}
