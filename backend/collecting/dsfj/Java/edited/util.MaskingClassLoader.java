
package hudson.util;

import java.io.IOException;
import java.net.URL;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Enumeration;
import java.util.Collections;
import java.util.concurrent.CopyOnWriteArrayList;


public class MaskingClassLoader extends ClassLoader {
    
    private final List<String> masksClasses = new CopyOnWriteArrayList<>();

    private final List<String> masksResources = new CopyOnWriteArrayList<>();

    public MaskingClassLoader(ClassLoader parent, String... masks) {
        this(parent, Arrays.asList(masks));
    }

    public MaskingClassLoader(ClassLoader parent, Collection<String> masks) {
        super(parent);
        this.masksClasses.addAll(masks);

        
        for (String mask : masks) {
            masksResources.add(mask.replace(".","/"));
        }
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        for (String mask : masksClasses) {
            if(name.startsWith(mask))
                throw new ClassNotFoundException();
        }

        return super.loadClass(name, resolve);
    }

    @Override
    public URL getResource(String name) {
        if (isMasked(name)) return null;

        return super.getResource(name);
    }

    @Override
    public Enumeration<URL> getResources(String name) throws IOException {
        if (isMasked(name)) return Collections.emptyEnumeration();

        return super.getResources(name);
    }

    public void add(String prefix) {
        masksClasses.add(prefix);
        if(prefix !=null){
            masksResources.add(prefix.replace(".","/"));
        }
    }

    private boolean isMasked(String name) {
        for (String mask : masksResources) {
            if(name.startsWith(mask))
                return true;
        }
        return false;
    }
}
