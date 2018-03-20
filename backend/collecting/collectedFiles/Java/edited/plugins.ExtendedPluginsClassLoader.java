

package org.elasticsearch.plugins;

import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Collections;
import java.util.List;


public class ExtendedPluginsClassLoader extends ClassLoader {

    
    private final List<ClassLoader> extendedLoaders;

    private ExtendedPluginsClassLoader(ClassLoader parent, List<ClassLoader> extendedLoaders) {
        super(parent);
        this.extendedLoaders = Collections.unmodifiableList(extendedLoaders);
    }

    @Override
    protected Class<?> findClass(String name) throws ClassNotFoundException {
        for (ClassLoader loader : extendedLoaders) {
            try {
                return loader.loadClass(name);
            } catch (ClassNotFoundException e) {
                            }
        }
        throw new ClassNotFoundException(name);
    }

    
    public static ExtendedPluginsClassLoader create(ClassLoader parent, List<ClassLoader> extendedLoaders) {
        return AccessController.doPrivileged((PrivilegedAction<ExtendedPluginsClassLoader>)
            () -> new ExtendedPluginsClassLoader(parent, extendedLoaders));
    }
}
