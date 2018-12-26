

package org.elasticsearch.action.fieldcaps;

import org.elasticsearch.action.support.ActionFilters;
import org.elasticsearch.action.support.single.shard.TransportSingleShardAction;
import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.block.ClusterBlockException;
import org.elasticsearch.cluster.block.ClusterBlockLevel;
import org.elasticsearch.cluster.metadata.IndexNameExpressionResolver;
import org.elasticsearch.cluster.routing.ShardsIterator;
import org.elasticsearch.cluster.service.ClusterService;
import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.index.mapper.MappedFieldType;
import org.elasticsearch.index.mapper.MapperService;
import org.elasticsearch.index.shard.ShardId;
import org.elasticsearch.indices.IndicesService;
import org.elasticsearch.threadpool.ThreadPool;
import org.elasticsearch.transport.TransportService;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.function.Predicate;

public class TransportFieldCapabilitiesIndexAction extends TransportSingleShardAction<FieldCapabilitiesIndexRequest,
    FieldCapabilitiesIndexResponse> {

    private static final String ACTION_NAME = FieldCapabilitiesAction.NAME + "[index]";

    private final IndicesService indicesService;

    @Inject
    public TransportFieldCapabilitiesIndexAction(Settings settings, ClusterService clusterService, TransportService transportService,
                                                 IndicesService indicesService, ThreadPool threadPool, ActionFilters actionFilters,
                                                 IndexNameExpressionResolver indexNameExpressionResolver) {
        super(settings, ACTION_NAME, threadPool, clusterService, transportService, actionFilters, indexNameExpressionResolver,
            FieldCapabilitiesIndexRequest::new, ThreadPool.Names.MANAGEMENT);
        this.indicesService = indicesService;
    }

    @Override
    protected boolean resolveIndex(FieldCapabilitiesIndexRequest request) {
                return false;
    }

    @Override
    protected ShardsIterator shards(ClusterState state, InternalRequest request) {
                        return state.routingTable().index(request.concreteIndex()).randomAllActiveShardsIt();
    }

    @Override
    protected FieldCapabilitiesIndexResponse shardOperation(final FieldCapabilitiesIndexRequest request, ShardId shardId) {
        MapperService mapperService = indicesService.indexServiceSafe(shardId.getIndex()).mapperService();
        Set<String> fieldNames = new HashSet<>();
        for (String field : request.fields()) {
            fieldNames.addAll(mapperService.simpleMatchToIndexNames(field));
        }
        Predicate<String> fieldPredicate = indicesService.getFieldFilter().apply(shardId.getIndexName());
        Map<String, FieldCapabilities> responseMap = new HashMap<>();
        for (String field : fieldNames) {
            MappedFieldType ft = mapperService.fullName(field);
            if (ft != null) {
                FieldCapabilities fieldCap = new FieldCapabilities(field, ft.typeName(), ft.isSearchable(), ft.isAggregatable());
                if (indicesService.isMetaDataField(field) || fieldPredicate.test(field)) {
                    responseMap.put(field, fieldCap);
                }
            }
        }
        return new FieldCapabilitiesIndexResponse(shardId.getIndexName(), responseMap);
    }

    @Override
    protected FieldCapabilitiesIndexResponse newResponse() {
        return new FieldCapabilitiesIndexResponse();
    }

    @Override
    protected ClusterBlockException checkRequestBlock(ClusterState state, InternalRequest request) {
        return state.blocks().indexBlockedException(ClusterBlockLevel.METADATA_READ, request.concreteIndex());
    }
}
