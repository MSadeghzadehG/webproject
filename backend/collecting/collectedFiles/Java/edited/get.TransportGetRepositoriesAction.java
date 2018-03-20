

package org.elasticsearch.action.admin.cluster.repositories.get;

import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.support.ActionFilters;
import org.elasticsearch.action.support.master.TransportMasterNodeReadAction;
import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.block.ClusterBlockException;
import org.elasticsearch.cluster.block.ClusterBlockLevel;
import org.elasticsearch.cluster.metadata.IndexNameExpressionResolver;
import org.elasticsearch.cluster.metadata.MetaData;
import org.elasticsearch.cluster.metadata.RepositoriesMetaData;
import org.elasticsearch.cluster.metadata.RepositoryMetaData;
import org.elasticsearch.cluster.service.ClusterService;
import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.regex.Regex;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.repositories.RepositoryMissingException;
import org.elasticsearch.threadpool.ThreadPool;
import org.elasticsearch.transport.TransportService;

import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;


public class TransportGetRepositoriesAction extends TransportMasterNodeReadAction<GetRepositoriesRequest, GetRepositoriesResponse> {

    @Inject
    public TransportGetRepositoriesAction(Settings settings, TransportService transportService, ClusterService clusterService,
                                          ThreadPool threadPool, ActionFilters actionFilters, IndexNameExpressionResolver indexNameExpressionResolver) {
        super(settings, GetRepositoriesAction.NAME, transportService, clusterService, threadPool, actionFilters, GetRepositoriesRequest::new, indexNameExpressionResolver);
    }

    @Override
    protected String executor() {
        return ThreadPool.Names.MANAGEMENT;
    }

    @Override
    protected GetRepositoriesResponse newResponse() {
        return new GetRepositoriesResponse();
    }

    @Override
    protected ClusterBlockException checkBlock(GetRepositoriesRequest request, ClusterState state) {
        return state.blocks().globalBlockedException(ClusterBlockLevel.METADATA_READ);
    }

    @Override
    protected void masterOperation(final GetRepositoriesRequest request, ClusterState state, final ActionListener<GetRepositoriesResponse> listener) {
        MetaData metaData = state.metaData();
        RepositoriesMetaData repositories = metaData.custom(RepositoriesMetaData.TYPE);
        if (request.repositories().length == 0 || (request.repositories().length == 1 && "_all".equals(request.repositories()[0]))) {
            if (repositories != null) {
                listener.onResponse(new GetRepositoriesResponse(repositories.repositories()));
            } else {
                listener.onResponse(new GetRepositoriesResponse(Collections.<RepositoryMetaData>emptyList()));
            }
        } else {
            if (repositories != null) {
                Set<String> repositoriesToGet = new LinkedHashSet<>();                 for (String repositoryOrPattern : request.repositories()) {
                    if (Regex.isSimpleMatchPattern(repositoryOrPattern) == false) {
                        repositoriesToGet.add(repositoryOrPattern);
                    } else {
                        for (RepositoryMetaData repository : repositories.repositories()) {
                            if (Regex.simpleMatch(repositoryOrPattern, repository.name())) {
                                repositoriesToGet.add(repository.name());
                            }
                        }
                    }
                }
                List<RepositoryMetaData> repositoryListBuilder = new ArrayList<>();
                for (String repository : repositoriesToGet) {
                    RepositoryMetaData repositoryMetaData = repositories.repository(repository);
                    if (repositoryMetaData == null) {
                        listener.onFailure(new RepositoryMissingException(repository));
                        return;
                    }
                    repositoryListBuilder.add(repositoryMetaData);
                }
                listener.onResponse(new GetRepositoriesResponse(Collections.unmodifiableList(repositoryListBuilder)));
            } else {
                listener.onFailure(new RepositoryMissingException(request.repositories()[0]));
            }
        }
    }
}