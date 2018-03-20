
package org.elasticsearch.index.query;

import org.elasticsearch.action.ActionListener;
import org.elasticsearch.client.Client;
import org.elasticsearch.common.io.stream.NamedWriteableRegistry;
import org.elasticsearch.common.util.concurrent.CountDown;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.common.xcontent.XContentParser;

import java.util.ArrayList;
import java.util.List;
import java.util.function.BiConsumer;
import java.util.function.LongSupplier;


public class QueryRewriteContext {
    private final NamedXContentRegistry xContentRegistry;
    private final NamedWriteableRegistry writeableRegistry;
    protected final Client client;
    protected final LongSupplier nowInMillis;
    private final List<BiConsumer<Client, ActionListener<?>>> asyncActions = new ArrayList<>();

    public QueryRewriteContext(
            NamedXContentRegistry xContentRegistry, NamedWriteableRegistry writeableRegistry,Client client,
            LongSupplier nowInMillis) {

        this.xContentRegistry = xContentRegistry;
        this.writeableRegistry = writeableRegistry;
        this.client = client;
        this.nowInMillis = nowInMillis;
    }

    
    public NamedXContentRegistry getXContentRegistry() {
        return xContentRegistry;
    }

    
    public long nowInMillis() {
        return nowInMillis.getAsLong();
    }

    public NamedWriteableRegistry getWriteableRegistry() {
        return writeableRegistry;
    }

    
    public QueryShardContext convertToShardContext() {
        return null;
    }

    
    public void registerAsyncAction(BiConsumer<Client, ActionListener<?>> asyncAction) {
        asyncActions.add(asyncAction);
    }

    
    public boolean hasAsyncActions() {
        return asyncActions.isEmpty() == false;
    }

    
    public void executeAsyncActions(ActionListener listener) {
        if (asyncActions.isEmpty()) {
            listener.onResponse(null);
        } else {
            CountDown countDown = new CountDown(asyncActions.size());
            ActionListener<?> internalListener = new ActionListener() {
                @Override
                public void onResponse(Object o) {
                    if (countDown.countDown()) {
                        listener.onResponse(null);
                    }
                }

                @Override
                public void onFailure(Exception e) {
                    if (countDown.fastForward()) {
                        listener.onFailure(e);
                    }
                }
            };
                        List<BiConsumer<Client, ActionListener<?>>> biConsumers = new ArrayList<>(asyncActions);
            asyncActions.clear();
            for (BiConsumer<Client, ActionListener<?>> action : biConsumers) {
                action.accept(client, internalListener);
            }
        }
    }

}
