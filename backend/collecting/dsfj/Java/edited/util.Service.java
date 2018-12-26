
package hudson.util;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.Collection;
import java.util.Enumeration;
import java.util.List;
import java.util.ArrayList;
import java.util.ServiceLoader;
import java.util.logging.Level;
import java.util.logging.Logger;
import static java.util.logging.Level.WARNING;


@Deprecated
public class Service {
    public static <T> List<T> loadInstances(ClassLoader classLoader, Class<T> type) throws IOException {
        List<T> result = new ArrayList<T>();

        final Enumeration<URL> e = classLoader.getResources("META-INF/services/"+type.getName());
        while (e.hasMoreElements()) {
            URL url = e.nextElement();
            try (BufferedReader configFile = new BufferedReader(new InputStreamReader(url.openStream(), "UTF-8"))) {
                String line;
                while ((line = configFile.readLine()) != null) {
                    line = line.trim();
                    if (line.startsWith("#") || line.length() == 0) continue;

                    try {
                        Class<?> t = classLoader.loadClass(line);
                        if (!type.isAssignableFrom(t)) continue;      
                        result.add(type.cast(t.newInstance()));
                    } catch (ClassNotFoundException x) {
                        LOGGER.log(WARNING, "Failed to load " + line, x);
                    } catch (InstantiationException x) {
                        LOGGER.log(WARNING, "Failed to load " + line, x);
                    } catch (IllegalAccessException x) {
                        LOGGER.log(WARNING, "Failed to load " + line, x);
                    }
                }
            }
        }

        return result;
    }

    
    public static <T> void load(Class<T> spi, ClassLoader cl, Collection<Class<? extends T>> result) {
        try {
            Enumeration<URL> e = cl.getResources("META-INF/services/" + spi.getName());
            while(e.hasMoreElements()) {
                final URL url = e.nextElement();
                try (BufferedReader r = new BufferedReader(new InputStreamReader(url.openStream(), "UTF-8"))) {
                    String line;
                    while ((line = r.readLine()) != null) {
                        if (line.startsWith("#"))
                            continue;                           line = line.trim();
                        if (line.length() == 0)
                            continue;   
                        try {
                            result.add(cl.loadClass(line).asSubclass(spi));
                        } catch (ClassNotFoundException x) {
                            LOGGER.log(Level.WARNING, "Failed to load " + line, x);
                        }
                    }
                } catch (IOException x) {
                    LOGGER.log(Level.WARNING, "Failed to load " + url, x);
                }
            }
        } catch (IOException x) {
            LOGGER.log(Level.WARNING, "Failed to look up service providers for "+spi, x);
        }
    }

    private static final Logger LOGGER = Logger.getLogger(Service.class.getName());
}
