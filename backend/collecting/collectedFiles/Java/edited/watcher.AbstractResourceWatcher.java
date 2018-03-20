
package org.elasticsearch.watcher;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;


public abstract class AbstractResourceWatcher<Listener> implements ResourceWatcher {
    private final List<Listener> listeners = new CopyOnWriteArrayList<>();
    private boolean initialized = false;

    @Override
    public void init() throws IOException {
        if (!initialized) {
            doInit();
            initialized = true;
        }
    }

    @Override
    public void checkAndNotify() throws IOException {
        init();
        doCheckAndNotify();
    }

    
    public void addListener(Listener listener) {
        listeners.add(listener);
    }

    
    public void remove(Listener listener) {
        listeners.remove(listener);
    }

    
    protected List<Listener> listeners() {
        return listeners;
    }

    
    protected abstract void doInit() throws IOException;

    
    protected abstract void doCheckAndNotify() throws IOException;

}
