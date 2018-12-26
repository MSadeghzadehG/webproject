
package hudson.util.io;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.NoSuchFileException;
import java.nio.file.StandardOpenOption;
import org.apache.commons.io.FileUtils;


public class RewindableFileOutputStream extends OutputStream {
    protected final File out;
    private boolean closed;

    private OutputStream current;

    public RewindableFileOutputStream(File out) {
        this.out = out;
    }

    private synchronized OutputStream current() throws IOException {
        if (current == null) {
            if (!closed) {
                FileUtils.forceMkdir(out.getParentFile());
                try {
                    current = Files.newOutputStream(out.toPath(), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
                } catch (FileNotFoundException | NoSuchFileException | InvalidPathException e) {
                    throw new IOException("Failed to open "+out,e);
                }
            }
            else {
                throw new IOException(out.getName()+" stream is closed");
            }
        }
        return current;
    }

    @Override
    public void write(int b) throws IOException {
        current().write(b);
    }

    @Override
    public void write(byte[] b) throws IOException {
        current().write(b);
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
        current().write(b, off, len);
    }

    @Override
    public void flush() throws IOException {
        current().flush();
    }

    @Override
    public synchronized void close() throws IOException {
        closeCurrent();
        closed = true;
    }

    
    public synchronized void rewind() throws IOException {
        closeCurrent();
    }

    private void closeCurrent() throws IOException {
        if (current != null) {
            current.close();
            current = null;
        }
    }
}
