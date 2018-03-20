

package org.elasticsearch.cluster.metadata;

import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.regex.Regex;
import org.elasticsearch.common.settings.Settings;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;


public final class ClusterNameExpressionResolver extends AbstractComponent {

    private final WildcardExpressionResolver wildcardResolver = new WildcardExpressionResolver();

    public ClusterNameExpressionResolver(Settings settings) {
        super(settings);
    }

    
    public List<String> resolveClusterNames(Set<String> remoteClusters, String clusterExpression) {
        if (remoteClusters.contains(clusterExpression)) {
            return Collections.singletonList(clusterExpression);
        } else if (Regex.isSimpleMatchPattern(clusterExpression)) {
            return wildcardResolver.resolve(remoteClusters, clusterExpression);
        } else {
            return Collections.emptyList();
        }
    }

    private static class WildcardExpressionResolver {

        private List<String> resolve(Set<String> remoteClusters, String clusterExpression) {
            if (isTrivialWildcard(clusterExpression)) {
                return resolveTrivialWildcard(remoteClusters);
            }

            Set<String> matches = matches(remoteClusters, clusterExpression);
            if (matches.isEmpty()) {
                return Collections.emptyList();
            } else {
                return new ArrayList<>(matches);
            }
        }

        private boolean isTrivialWildcard(String clusterExpression) {
            return Regex.isMatchAllPattern(clusterExpression);
        }

        private List<String> resolveTrivialWildcard(Set<String> remoteClusters) {
            return new ArrayList<>(remoteClusters);
        }

        private static Set<String> matches(Set<String> remoteClusters, String expression) {
            if (expression.indexOf("*") == expression.length() - 1) {
                return otherWildcard(remoteClusters, expression);
            } else {
                return otherWildcard(remoteClusters, expression);
            }
        }

        private static Set<String> otherWildcard(Set<String> remoteClusters, String expression) {
            final String pattern = expression;
            return remoteClusters.stream()
                .filter(n -> Regex.simpleMatch(pattern, n))
                .collect(Collectors.toSet());
        }
    }
}
