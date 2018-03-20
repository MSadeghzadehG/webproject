

package org.elasticsearch.plugins;

import java.util.Collections;
import java.util.Map;

import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.env.Environment;
import org.elasticsearch.repositories.Repository;


public interface RepositoryPlugin {

    
    default Map<String, Repository.Factory> getRepositories(Environment env, NamedXContentRegistry namedXContentRegistry) {
        return Collections.emptyMap();
    }
}
