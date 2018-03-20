
package hudson.util;

import edu.umd.cs.findbugs.annotations.CreatesObligation;

import hudson.Util;
import jenkins.util.io.LinesStream;

import java.nio.file.Files;
import javax.annotation.Nonnull;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.RandomAccessFile;
import java.io.Reader;
import java.io.StringWriter;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;


public class TextFile {

    public final @Nonnull File file;

    public TextFile(@Nonnull File file) {
        this.file = file;
    }

    public boolean exists() {
        return file.exists();
    }

    public void delete() {
        file.delete();
    }

    
    public String read() throws IOException {
        StringWriter out = new StringWriter();
        PrintWriter w = new PrintWriter(out);
        try (BufferedReader in = Files.newBufferedReader(Util.fileToPath(file), StandardCharsets.UTF_8)) {
            String line;
            while ((line = in.readLine()) != null)
                w.println(line);
        } catch (Exception e) {
            throw new IOException("Failed to fully read " + file, e);
        }
        return out.toString();
    }

    
    @Deprecated
    public @Nonnull Iterable<String> lines() {
        try {
            return linesStream();
        } catch (IOException ex) {
            throw new RuntimeException(ex);
        }
    }

    
    @CreatesObligation
    public @Nonnull LinesStream linesStream() throws IOException {
        return new LinesStream(Util.fileToPath(file));
    }

    
    public void write(String text) throws IOException {
        file.getParentFile().mkdirs();
        AtomicFileWriter w = new AtomicFileWriter(file);
        try {
            w.write(text);
            w.commit();
        } finally {
            w.abort();
        }
    }

    
    public @Nonnull String head(int numChars) throws IOException {
        char[] buf = new char[numChars];
        int read = 0;
        try (Reader r = new FileReader(file)) {
            while (read<numChars) {
                int d = r.read(buf,read,buf.length-read);
                if (d<0)
                    break;
                read += d;
            }

            return new String(buf,0,read);
        }
    }

    
    public @Nonnull String fastTail(int numChars, Charset cs) throws IOException {

        try (RandomAccessFile raf = new RandomAccessFile(file, "r")) {
            long len = raf.length();
                                    long pos = Math.max(0, len - (numChars * 4 + 1024));
            raf.seek(pos);

            byte[] tail = new byte[(int) (len - pos)];
            raf.readFully(tail);

            String tails = cs.decode(java.nio.ByteBuffer.wrap(tail)).toString();

            return new String(tails.substring(Math.max(0, tails.length() - numChars)));         }
    }

    
    public @Nonnull String fastTail(int numChars) throws IOException {
        return fastTail(numChars,Charset.defaultCharset());
    }


    public String readTrim() throws IOException {
        return read().trim();
    }

    @Override
    public String toString() {
        return file.toString();
    }
}
