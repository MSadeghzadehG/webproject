
package com.alibaba.dubbo.config.spring.util;

import org.springframework.core.env.EnumerablePropertySource;
import org.springframework.core.env.PropertySource;
import org.springframework.core.env.PropertySources;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Properties;


public abstract class PropertySourcesUtils {

    
    public static Map<String, String> getSubProperties(PropertySources propertySources, String prefix) {

        Map<String, String> subProperties = new LinkedHashMap<String, String>();

        String normalizedPrefix = prefix.endsWith(".") ? prefix : prefix + ".";

        for (PropertySource<?> source : propertySources) {
            if (source instanceof EnumerablePropertySource) {
                for (String name : ((EnumerablePropertySource<?>) source).getPropertyNames()) {
                    if (name.startsWith(normalizedPrefix)) {
                        String subName = name.substring(normalizedPrefix.length());
                        Object value = source.getProperty(name);
                        subProperties.put(subName, String.valueOf(value));
                    }
                }
            }
        }

        return subProperties;

    }

}
