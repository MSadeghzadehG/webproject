

package org.elasticsearch.action.admin.cluster.repositories.get;

import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.cluster.metadata.RepositoryMetaData;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.settings.Settings;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;


public class GetRepositoriesResponse extends ActionResponse implements Iterable<RepositoryMetaData> {

    private List<RepositoryMetaData> repositories = Collections.emptyList();


    GetRepositoriesResponse() {
    }

    GetRepositoriesResponse(List<RepositoryMetaData> repositories) {
        this.repositories = repositories;
    }

    
    public List<RepositoryMetaData> repositories() {
        return repositories;
    }


    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        int size = in.readVInt();
        List<RepositoryMetaData> repositoryListBuilder = new ArrayList<>(size);
        for (int j = 0; j < size; j++) {
            repositoryListBuilder.add(new RepositoryMetaData(
                    in.readString(),
                    in.readString(),
                    Settings.readSettingsFromStream(in))
            );
        }
        repositories = Collections.unmodifiableList(repositoryListBuilder);
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeVInt(repositories.size());
        for (RepositoryMetaData repository : repositories) {
            out.writeString(repository.name());
            out.writeString(repository.type());
            Settings.writeSettingsToStream(repository.settings(), out);
        }
    }

    
    @Override
    public Iterator<RepositoryMetaData> iterator() {
        return repositories.iterator();
    }
}
