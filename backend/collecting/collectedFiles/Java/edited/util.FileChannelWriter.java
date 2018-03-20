package hudson.util;

import org.kohsuke.accmod.Restricted;
import org.kohsuke.accmod.restrictions.NoExternalUse;

import java.io.IOException;
import java.io.Writer;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.Charset;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.util.logging.Logger;


@Restricted(NoExternalUse.class)
public class FileChannelWriter extends Writer {

    private static final Logger LOGGER = Logger.getLogger(FileChannelWriter.class.getName());

    private final Charset charset;
    private final FileChannel channel;

    
    private boolean forceOnFlush;

    
    private boolean forceOnClose;

    
    FileChannelWriter(Path filePath, Charset charset, boolean forceOnFlush, boolean forceOnClose, OpenOption... options) throws IOException {
        this.charset = charset;
        this.forceOnFlush = forceOnFlush;
        this.forceOnClose = forceOnClose;
        channel = FileChannel.open(filePath, options);
    }

    @Override
    public void write(char cbuf[], int off, int len) throws IOException {
        final CharBuffer charBuffer = CharBuffer.wrap(cbuf, off, len);
        ByteBuffer byteBuffer = charset.encode(charBuffer);
        channel.write(byteBuffer);
    }

    @Override
    public void flush() throws IOException {
        if (forceOnFlush) {
            LOGGER.finest("Flush is forced");
            channel.force(true);
        } else {
            LOGGER.finest("Force disabled on flush(), no-op");
        }
    }

    @Override
    public void close() throws IOException {
        if(channel.isOpen()) {
            if (forceOnClose) {
                channel.force(true);
            }
            channel.close();
        }
    }
}
