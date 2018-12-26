
package org.elasticsearch.plugins;

import org.elasticsearch.client.Client;
import org.elasticsearch.cluster.service.ClusterService;
import org.elasticsearch.persistent.PersistentTasksExecutor;
import org.elasticsearch.threadpool.ThreadPool;

import java.util.Collections;
import java.util.List;


public interface PersistentTaskPlugin {

    
    default List<PersistentTasksExecutor<?>> getPersistentTasksExecutor(ClusterService clusterService,
                                                                        ThreadPool threadPool, Client client) {
        return Collections.emptyList();
    }

}
