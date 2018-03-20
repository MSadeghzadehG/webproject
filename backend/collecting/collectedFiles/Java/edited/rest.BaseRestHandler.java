

package org.elasticsearch.rest;

import org.apache.lucene.search.spell.LevensteinDistance;
import org.apache.lucene.util.CollectionUtil;
import org.elasticsearch.client.node.NodeClient;
import org.elasticsearch.common.CheckedConsumer;
import org.elasticsearch.common.collect.Tuple;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Setting.Property;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.plugins.ActionPlugin;
import org.elasticsearch.rest.action.admin.cluster.RestNodesUsageAction;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;
import java.util.concurrent.atomic.LongAdder;
import java.util.stream.Collectors;


public abstract class BaseRestHandler extends AbstractComponent implements RestHandler {

    public static final Setting<Boolean> MULTI_ALLOW_EXPLICIT_INDEX =
        Setting.boolSetting("rest.action.multi.allow_explicit_index", true, Property.NodeScope);

    private final LongAdder usageCount = new LongAdder();

    protected BaseRestHandler(Settings settings) {
        super(settings);
    }

    public final long getUsageCount() {
        return usageCount.sum();
    }

    
    public abstract String getName();

    @Override
    public final void handleRequest(RestRequest request, RestChannel channel, NodeClient client) throws Exception {
                final RestChannelConsumer action = prepareRequest(request, client);

                        final SortedSet<String> unconsumedParams =
            request.unconsumedParams().stream().filter(p -> !responseParams().contains(p)).collect(Collectors.toCollection(TreeSet::new));

                if (!unconsumedParams.isEmpty()) {
            final Set<String> candidateParams = new HashSet<>();
            candidateParams.addAll(request.consumedParams());
            candidateParams.addAll(responseParams());
            throw new IllegalArgumentException(unrecognized(request, unconsumedParams, candidateParams, "parameter"));
        }

        usageCount.increment();
                action.accept(channel);
    }

    protected final String unrecognized(
        final RestRequest request,
        final Set<String> invalids,
        final Set<String> candidates,
        final String detail) {
        StringBuilder message = new StringBuilder(String.format(
            Locale.ROOT,
            "request [%s] contains unrecognized %s%s: ",
            request.path(),
            detail,
            invalids.size() > 1 ? "s" : ""));
        boolean first = true;
        for (final String invalid : invalids) {
            final LevensteinDistance ld = new LevensteinDistance();
            final List<Tuple<Float, String>> scoredParams = new ArrayList<>();
            for (final String candidate : candidates) {
                final float distance = ld.getDistance(invalid, candidate);
                if (distance > 0.5f) {
                    scoredParams.add(new Tuple<>(distance, candidate));
                }
            }
            CollectionUtil.timSort(scoredParams, (a, b) -> {
                                int compare = a.v1().compareTo(b.v1());
                if (compare != 0) return -compare;
                else return a.v2().compareTo(b.v2());
            });
            if (first == false) {
                message.append(", ");
            }
            message.append("[").append(invalid).append("]");
            final List<String> keys = scoredParams.stream().map(Tuple::v2).collect(Collectors.toList());
            if (keys.isEmpty() == false) {
                message.append(" -> did you mean ");
                if (keys.size() == 1) {
                    message.append("[").append(keys.get(0)).append("]");
                } else {
                    message.append("any of ").append(keys.toString());
                }
                message.append("?");
            }
            first = false;
        }

        return message.toString();
    }

    
    @FunctionalInterface
    protected interface RestChannelConsumer extends CheckedConsumer<RestChannel, Exception> {
    }

    
    protected abstract RestChannelConsumer prepareRequest(RestRequest request, NodeClient client) throws IOException;

    
    protected Set<String> responseParams() {
        return Collections.emptySet();
    }

}
