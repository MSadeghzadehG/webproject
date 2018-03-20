

package org.elasticsearch.common.inject.internal;

import java.util.HashSet;
import java.util.Set;

import static java.util.Collections.singleton;
import static java.util.Collections.unmodifiableSet;


public class SourceProvider {

    
    public static final Object UNKNOWN_SOURCE = "[unknown source]";

    private final Set<String> classNamesToSkip;

    public SourceProvider() {
        this.classNamesToSkip = singleton(SourceProvider.class.getName());
    }

    public static final SourceProvider DEFAULT_INSTANCE = new SourceProvider();

    @SuppressWarnings("rawtypes")
    private SourceProvider(SourceProvider copy, Class[] moreClassesToSkip) {
        Set<String> classNamesToSkip = new HashSet<>(copy.classNamesToSkip);
        for (Class toSkip : moreClassesToSkip) {
            classNamesToSkip.add(toSkip.getName());
        }
        this.classNamesToSkip = unmodifiableSet(classNamesToSkip);
    }

    
    @SuppressWarnings("rawtypes")
    public SourceProvider plusSkippedClasses(Class... moreClassesToSkip) {
        return new SourceProvider(this, moreClassesToSkip);
    }

    
    public StackTraceElement get() {
        for (final StackTraceElement element : new Throwable().getStackTrace()) {
            String className = element.getClassName();
            if (!classNamesToSkip.contains(className)) {
                return element;
            }
        }
        throw new AssertionError();
    }
}
