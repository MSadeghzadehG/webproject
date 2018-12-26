package hudson.init;

import org.kohsuke.MetaInfServices;
import org.jvnet.hudson.reactor.Task;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.Collection;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;

import hudson.PluginManager;
import jenkins.util.SystemProperties;
import hudson.util.DirScanner;
import hudson.util.FileVisitor;
import java.util.Iterator;
import java.util.ServiceLoader;


public class InitStrategy {
    
    public List<File> listPluginArchives(PluginManager pm) throws IOException {
        List<File> r = new ArrayList<File>();

                                getBundledPluginsFromProperty(r);

                listPluginFiles(pm, ".jpl", r);         listPluginFiles(pm, ".hpl", r);         listPluginFiles(pm, ".jpi", r);         listPluginFiles(pm, ".hpi", r); 
        return r;
    }
    
    private void listPluginFiles(PluginManager pm, String extension, Collection<File> all) throws IOException {
        File[] files = pm.rootDir.listFiles(new FilterByExtension(extension));
        if (files==null)
            throw new IOException("Jenkins is unable to create " + pm.rootDir + "\nPerhaps its security privilege is insufficient");

        all.addAll(Arrays.asList(files));
    }

    
    protected void getBundledPluginsFromProperty(final List<File> r) {
        String hplProperty = SystemProperties.getString("hudson.bundled.plugins");
        if (hplProperty != null) {
            for (String hplLocation : hplProperty.split(",")) {
                File hpl = new File(hplLocation.trim());
                if (hpl.exists()) {
                    r.add(hpl);
                } else if (hpl.getName().contains("*")) {
                    try {
                        new DirScanner.Glob(hpl.getName(), null).scan(hpl.getParentFile(), new FileVisitor() {
                            @Override public void visit(File f, String relativePath) throws IOException {
                                r.add(f);
                            }
                        });
                    } catch (IOException x) {
                        LOGGER.log(Level.WARNING, "could not expand " + hplLocation, x);
                    }
                } else {
                    LOGGER.warning("bundled plugin " + hplLocation + " does not exist");
                }
            }
        }
    }

    
    public boolean skipInitTask(Task task) {
        return false;
    }


    
    public static InitStrategy get(ClassLoader cl) throws IOException {
        Iterator<InitStrategy> it = ServiceLoader.load(InitStrategy.class, cl).iterator();
        if (!it.hasNext()) {
            return new InitStrategy();         }
        InitStrategy s = it.next();
        LOGGER.log(Level.FINE, "Using {0} as InitStrategy", s);
        return s;
    }

    private static final Logger LOGGER = Logger.getLogger(InitStrategy.class.getName());

    private static class FilterByExtension implements FilenameFilter {
        private final List<String> extensions;

        public FilterByExtension(String... extensions) {
            this.extensions = Arrays.asList(extensions);
        }

        public boolean accept(File dir, String name) {
            for (String extension : extensions) {
                if (name.endsWith(extension))
                    return true;
            }
            return false;
        }
    }
}
