

package org.elasticsearch.common.inject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

public class ModulesBuilder implements Iterable<Module> {

    private final List<Module> modules = new ArrayList<>();

    public ModulesBuilder add(Module... newModules) {
        Collections.addAll(modules, newModules);
        return this;
    }

    @Override
    public Iterator<Module> iterator() {
        return modules.iterator();
    }

    public Injector createInjector() {
        Injector injector = Guice.createInjector(modules);
        ((InjectorImpl) injector).clearCache();
                        ((InjectorImpl) injector).readOnlyAllSingletons();
        return injector;
    }
}
