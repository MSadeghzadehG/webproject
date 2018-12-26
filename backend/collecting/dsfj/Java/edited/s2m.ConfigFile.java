package jenkins.security.s2m;

import hudson.CopyOnWrite;
import hudson.util.TextFile;
import jenkins.model.Jenkins;
import jenkins.util.io.LinesStream;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.StringReader;
import java.util.Collection;


abstract class ConfigFile<T,COL extends Collection<T>> extends TextFile {
    @CopyOnWrite
    protected volatile COL parsed;

    public ConfigFile(File file) {
        super(file);
    }

    protected abstract COL create();
    protected abstract COL readOnly(COL base);

    
    @Deprecated
    public void load() {
        try {
            load2();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    
    public synchronized void load2() throws IOException {
        COL result = create();

        if (exists()) {
            try (LinesStream stream = linesStream()) {
                for (String line : stream) {
                    if (line.startsWith("#")) continue;                       T r = parse(line);
                    if (r != null)
                        result.add(r);
                }
            }
        }

        parsed = readOnly(result);
    }

    
    public void parseTest(String candidate) {
        try {
            BufferedReader r = new BufferedReader(new StringReader(candidate));
            String line;
            while ((line=r.readLine())!=null) {
                if (line.startsWith("#")) continue;                   parse(line);
            }
        } catch (IOException e) {
            throw new IllegalArgumentException(e);          }
    }

    protected abstract T parse(String line);

    public synchronized void set(String newContent) throws IOException {
        Jenkins.getInstance().checkPermission(Jenkins.ADMINISTER);

        write(newContent);
        load2();
    }

    public synchronized void append(String additional) throws IOException {
        String s = read();
        if (!s.endsWith("\n"))
            s += "\n";
        s+= additional;

        set(s);
    }

    public COL get() {
                if (parsed==null) {
            synchronized (this) {
                if (parsed==null) {
                    load();
                }
            }
        }
        return parsed;
    }


}
