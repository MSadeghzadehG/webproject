package jenkins.util.io;

import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import jenkins.model.Jenkins;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;


public class FileBoolean {
    private final File file;
    private volatile Boolean state;

    public FileBoolean(File file) {
        this.file = file;
    }

    public FileBoolean(Class owner, String name) {
        this(new File(Jenkins.getInstance().getRootDir(),owner.getName().replace('$','.')+'/'+name));
    }

    
    public boolean get() {
        return state=file.exists();
    }

    
    public boolean fastGet() {
        if (state==null)    return get();
        return state;
    }

    public boolean isOn() { return get(); }
    public boolean isOff() { return !get(); }

    public void set(boolean b) {
        if (b) on(); else off();
    }

    public void on() {
        try {
            file.getParentFile().mkdirs();
            Files.newOutputStream(file.toPath()).close();
            get();          } catch (IOException | InvalidPathException e) {
            LOGGER.log(Level.WARNING, "Failed to touch "+file);
        }
    }

    public void off() {
        file.delete();
        get();      }

    private static final Logger LOGGER = Logger.getLogger(FileBoolean.class.getName());
}
