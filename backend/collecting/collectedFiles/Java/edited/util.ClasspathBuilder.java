package hudson.util;

import hudson.FilePath;
import hudson.Util;
import hudson.remoting.Which;

import java.io.Serializable;
import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;


public class ClasspathBuilder implements Serializable {
    private final List<String> args = new ArrayList<String>();

    
    public ClasspathBuilder add(File f) {
        return add(f.getAbsolutePath());
    }

    
    public ClasspathBuilder add(FilePath f) {
        return add(f.getRemote());
    }

    
    public ClasspathBuilder add(String path) {
        args.add(path);
        return this;
    }

    
    public ClasspathBuilder addJarOf(Class c) throws IOException {
        return add(Which.jarFile(c));
    }

    
    public ClasspathBuilder addAll(FilePath base, String glob) throws IOException, InterruptedException {
        for(FilePath item : base.list(glob))
            add(item);
        return this;
    }

    
    @Override
    public String toString() {
        return Util.join(args,File.pathSeparator);
    }
}
