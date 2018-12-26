
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


@Deprecated public class ReopenableFileOutputStream extends OutputStream {
    protected final File out;

    private OutputStream current;
    private boolean appendOnNextOpen = false;

    public ReopenableFileOutputStream(File out) {
        this.out = out;
    }

    private synchronized OutputStream current() throws IOException {
        if (current==null)
            try {
                current = Files.newOutputStream(out.toPath(), StandardOpenOption.CREATE,
                        appendOnNextOpen ? StandardOpenOption.APPEND : StandardOpenOption.TRUNCATE_EXISTING);
            } catch (FileNotFoundException | NoSuchFileException | InvalidPathException e) {
                throw new IOException("Failed to open "+out,e);
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
        if (current!=null) {
            current.close();
            appendOnNextOpen = true;
            current = null;
        }
    }

    
    public synchronized void rewind() throws IOException {
        close();
        appendOnNextOpen = false;
    }
}
