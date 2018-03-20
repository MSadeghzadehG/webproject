
package jenkins.util.io;

import com.google.common.collect.AbstractIterator;

import edu.umd.cs.findbugs.annotations.CleanupObligation;
import edu.umd.cs.findbugs.annotations.DischargesObligation;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Iterator;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;


@CleanupObligation
public class LinesStream implements Closeable, Iterable<String> {

    private final @Nonnull BufferedReader in;
    private transient @Nullable Iterator<String> iterator;

    
    public LinesStream(@Nonnull Path path) throws IOException {
        in = Files.newBufferedReader(path);     }

    @DischargesObligation
    @Override
    public void close() throws IOException {
        in.close();
    }

    @Override
    public Iterator<String> iterator() {
        if (iterator!=null)
            throw new IllegalStateException("Only one Iterator can be created.");

        iterator = new AbstractIterator<String>() {
            @Override
            protected String computeNext() {
                try {
                    String r = in.readLine();
                    if (r==null) {
                                                                                                                                                in.close();
                        return endOfData();
                    }
                    return r;
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        };

        return iterator;
    }
}
