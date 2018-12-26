

package org.elasticsearch.common.inject.name;

import org.elasticsearch.common.inject.Binder;
import org.elasticsearch.common.inject.Key;

import java.util.Enumeration;
import java.util.Map;
import java.util.Properties;


public class Names {

    private Names() {
    }

    
    public static Named named(String name) {
        return new NamedImpl(name);
    }

    
    public static void bindProperties(Binder binder, Map<String, String> properties) {
        binder = binder.skipSources(Names.class);
        for (Map.Entry<String, String> entry : properties.entrySet()) {
            String key = entry.getKey();
            String value = entry.getValue();
            binder.bind(Key.get(String.class, new NamedImpl(key))).toInstance(value);
        }
    }

    
    public static void bindProperties(Binder binder, Properties properties) {
        binder = binder.skipSources(Names.class);

                for (Enumeration<?> e = properties.propertyNames(); e.hasMoreElements(); ) {
            String propertyName = (String) e.nextElement();
            String value = properties.getProperty(propertyName);
            binder.bind(Key.get(String.class, new NamedImpl(propertyName))).toInstance(value);
        }
    }
}
