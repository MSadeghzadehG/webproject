

package org.elasticsearch.plugins;

import org.elasticsearch.action.ActionRequest;
import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.action.GenericAction;
import org.elasticsearch.action.support.ActionFilter;
import org.elasticsearch.action.support.TransportAction;
import org.elasticsearch.action.support.TransportActions;
import org.elasticsearch.cluster.metadata.IndexNameExpressionResolver;
import org.elasticsearch.cluster.node.DiscoveryNodes;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.settings.ClusterSettings;
import org.elasticsearch.common.settings.IndexScopedSettings;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.settings.SettingsFilter;
import org.elasticsearch.common.util.concurrent.ThreadContext;
import org.elasticsearch.rest.RestController;
import org.elasticsearch.rest.RestHandler;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.function.Supplier;
import java.util.function.UnaryOperator;
import java.util.stream.Collectors;


public interface ActionPlugin {
    
    default List<ActionHandler<? extends ActionRequest, ? extends ActionResponse>> getActions() {
        return Collections.emptyList();
    }

    
    default List<GenericAction> getClientActions() {
        return getActions().stream().map(a -> a.action).collect(Collectors.toList());
    }

    
    default List<ActionFilter> getActionFilters() {
        return Collections.emptyList();
    }
    
    default List<RestHandler> getRestHandlers(Settings settings, RestController restController, ClusterSettings clusterSettings,
            IndexScopedSettings indexScopedSettings, SettingsFilter settingsFilter,
            IndexNameExpressionResolver indexNameExpressionResolver, Supplier<DiscoveryNodes> nodesInCluster) {
        return Collections.emptyList();
    }

    
    default Collection<String> getRestHeaders() {
        return Collections.emptyList();
    }

    
    default Collection<String> getTaskHeaders() {
        return Collections.emptyList();
    }

    
    default UnaryOperator<RestHandler> getRestHandlerWrapper(ThreadContext threadContext) {
        return null;
    }

    final class ActionHandler<Request extends ActionRequest, Response extends ActionResponse> {
        private final GenericAction<Request, Response> action;
        private final Class<? extends TransportAction<Request, Response>> transportAction;
        private final Class<?>[] supportTransportActions;

        
        public ActionHandler(GenericAction<Request, Response> action, Class<? extends TransportAction<Request, Response>> transportAction,
                Class<?>... supportTransportActions) {
            this.action = action;
            this.transportAction = transportAction;
            this.supportTransportActions = supportTransportActions;
        }

        public GenericAction<Request, Response> getAction() {
            return action;
        }

        public Class<? extends TransportAction<Request, Response>> getTransportAction() {
            return transportAction;
        }

        public Class<?>[] getSupportTransportActions() {
            return supportTransportActions;
        }

        @Override
        public String toString() {
            StringBuilder b = new StringBuilder().append(action.name()).append(" is handled by ").append(transportAction.getName());
            if (supportTransportActions.length > 0) {
                b.append('[').append(Strings.arrayToCommaDelimitedString(supportTransportActions)).append(']');
            }
            return b.toString();
        }

        @Override
        public boolean equals(Object obj) {
            if (obj == null || obj.getClass() != ActionHandler.class) {
                return false;
            }
            ActionHandler<?, ?> other = (ActionHandler<?, ?>) obj;
            return Objects.equals(action, other.action)
                    && Objects.equals(transportAction, other.transportAction)
                    && Objects.deepEquals(supportTransportActions, other.supportTransportActions);
        }

        @Override
        public int hashCode() {
            return Objects.hash(action, transportAction, supportTransportActions);
        }
    }
}
