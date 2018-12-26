

package org.elasticsearch.percolator;

import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.index.mapper.Mapper;
import org.elasticsearch.plugins.MapperPlugin;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.SearchPlugin;
import org.elasticsearch.search.fetch.FetchSubPhase;

import java.util.Arrays;
import java.util.List;
import java.util.Map;

import static java.util.Collections.singletonList;
import static java.util.Collections.singletonMap;

public class PercolatorPlugin extends Plugin implements MapperPlugin, SearchPlugin {

    private final Settings settings;

    public PercolatorPlugin(Settings settings) {
        this.settings = settings;
    }

    @Override
    public List<QuerySpec<?>> getQueries() {
        return singletonList(new QuerySpec<>(PercolateQueryBuilder.NAME, PercolateQueryBuilder::new, PercolateQueryBuilder::fromXContent));
    }

    @Override
    public List<FetchSubPhase> getFetchSubPhases(FetchPhaseConstructionContext context) {
        return Arrays.asList(
            new PercolatorMatchedSlotSubFetchPhase(),
            new PercolatorHighlightSubFetchPhase(settings, context.getHighlighters())
        );
    }

    @Override
    public List<Setting<?>> getSettings() {
        return Arrays.asList(PercolatorFieldMapper.INDEX_MAP_UNMAPPED_FIELDS_AS_TEXT_SETTING);
    }

    @Override
    public Map<String, Mapper.TypeParser> getMappers() {
        return singletonMap(PercolatorFieldMapper.CONTENT_TYPE, new PercolatorFieldMapper.TypeParser());
    }

}
