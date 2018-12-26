

package org.elasticsearch.index.store;

import java.io.FilterOutputStream;
import java.io.IOException;
import java.nio.channels.Channels;
import java.nio.file.Files;
import java.nio.file.StandardOpenOption;
import org.apache.lucene.store.FSDirectory;
import org.apache.lucene.store.FilterDirectory;
import org.apache.lucene.store.IOContext;
import org.apache.lucene.store.IndexOutput;
import org.apache.lucene.store.OutputStreamIndexOutput;


public final class SmbDirectoryWrapper extends FilterDirectory {

    private final FSDirectory fsDirectory;

    public SmbDirectoryWrapper(FSDirectory in) {
        super(in);
        fsDirectory = in;
    }

    @Override
    public IndexOutput createOutput(String name, IOContext context) throws IOException {
        this.ensureOpen();
        return new SmbFSIndexOutput(name);
    }

    final class SmbFSIndexOutput extends OutputStreamIndexOutput {
        
        static final int CHUNK_SIZE = 8192;

        SmbFSIndexOutput(String name) throws IOException {
            super("SmbFSIndexOutput(path=\"" + fsDirectory.getDirectory().resolve(name) + "\")", name, new FilterOutputStream(Channels.newOutputStream(Files.newByteChannel(fsDirectory.getDirectory().resolve(name), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.READ, StandardOpenOption.WRITE))) {
                                @Override
                public void write(byte[] b, int offset, int length) throws IOException {
                    while (length > 0) {
                        final int chunk = Math.min(length, CHUNK_SIZE);
                        out.write(b, offset, chunk);
                        length -= chunk;
                        offset += chunk;
                    }
                }
            }, CHUNK_SIZE);
        }
    }
}
