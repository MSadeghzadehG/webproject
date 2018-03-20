

package org.elasticsearch.common.io;

import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.common.io.stream.BytesStream;
import org.elasticsearch.common.io.stream.StreamOutput;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.Reader;
import java.io.StringWriter;
import java.io.Writer;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.function.Consumer;


public abstract class Streams {

    public static final int BUFFER_SIZE = 1024 * 8;


            

    public static long copy(InputStream in, OutputStream out) throws IOException {
        return copy(in, out, new byte[BUFFER_SIZE]);
    }

    
    public static long copy(InputStream in, OutputStream out, byte[] buffer) throws IOException {
        Objects.requireNonNull(in, "No InputStream specified");
        Objects.requireNonNull(out, "No OutputStream specified");
        boolean success = false;
        try {
            long byteCount = 0;
            int bytesRead;
            while ((bytesRead = in.read(buffer)) != -1) {
                out.write(buffer, 0, bytesRead);
                byteCount += bytesRead;
            }
            out.flush();
            success = true;
            return byteCount;
        } finally {
            if (success) {
                IOUtils.close(in, out);
            } else {
                IOUtils.closeWhileHandlingException(in, out);
            }
        }
    }

    
    public static void copy(byte[] in, OutputStream out) throws IOException {
        Objects.requireNonNull(in, "No input byte array specified");
        Objects.requireNonNull(out, "No OutputStream specified");
        try {
            out.write(in);
        } finally {
            try {
                out.close();
            } catch (IOException ex) {
                            }
        }
    }


            
    
    public static int copy(Reader in, Writer out) throws IOException {
        Objects.requireNonNull(in, "No Reader specified");
        Objects.requireNonNull(out, "No Writer specified");
        boolean success = false;
        try {
            int byteCount = 0;
            char[] buffer = new char[BUFFER_SIZE];
            int bytesRead;
            while ((bytesRead = in.read(buffer)) != -1) {
                out.write(buffer, 0, bytesRead);
                byteCount += bytesRead;
            }
            out.flush();
            success = true;
            return byteCount;
        } finally {
            if (success) {
                IOUtils.close(in, out);
            } else {
                IOUtils.closeWhileHandlingException(in, out);
            }
        }
    }

    
    public static void copy(String in, Writer out) throws IOException {
        Objects.requireNonNull(in, "No input String specified");
        Objects.requireNonNull(out, "No Writer specified");
        try {
            out.write(in);
        } finally {
            try {
                out.close();
            } catch (IOException ex) {
                            }
        }
    }

    
    public static String copyToString(Reader in) throws IOException {
        StringWriter out = new StringWriter();
        copy(in, out);
        return out.toString();
    }

    public static int readFully(Reader reader, char[] dest) throws IOException {
        return readFully(reader, dest, 0, dest.length);
    }

    public static int readFully(Reader reader, char[] dest, int offset, int len) throws IOException {
        int read = 0;
        while (read < len) {
            final int r = reader.read(dest, offset + read, len - read);
            if (r == -1) {
                break;
            }
            read += r;
        }
        return read;
    }

    public static int readFully(InputStream reader, byte[] dest) throws IOException {
        return readFully(reader, dest, 0, dest.length);
    }

    public static int readFully(InputStream reader, byte[] dest, int offset, int len) throws IOException {
        int read = 0;
        while (read < len) {
            final int r = reader.read(dest, offset + read, len - read);
            if (r == -1) {
                break;
            }
            read += r;
        }
        return read;
    }

    public static List<String> readAllLines(InputStream input) throws IOException {
        final List<String> lines = new ArrayList<>();
        readAllLines(input, lines::add);
        return lines;
    }

    public static void readAllLines(InputStream input, Consumer<String> consumer) throws IOException {
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(input, StandardCharsets.UTF_8))) {
            String line;
            while ((line = reader.readLine()) != null) {
                consumer.accept(line);
            }
        }
    }

    
    public static BytesStream flushOnCloseStream(BytesStream os) {
        return new FlushOnCloseOutputStream(os);
    }

    
    private static class FlushOnCloseOutputStream extends BytesStream {

        private final BytesStream delegate;

        private FlushOnCloseOutputStream(BytesStream bytesStreamOutput) {
            this.delegate = bytesStreamOutput;
        }

        @Override
        public void writeByte(byte b) throws IOException {
            delegate.writeByte(b);
        }

        @Override
        public void writeBytes(byte[] b, int offset, int length) throws IOException {
            delegate.writeBytes(b, offset, length);
        }

        @Override
        public void flush() throws IOException {
            delegate.flush();
        }

        @Override
        public void close() throws IOException {
            flush();
        }

        @Override
        public void reset() throws IOException {
            delegate.reset();
        }

        @Override
        public BytesReference bytes() {
            return delegate.bytes();
        }
    }
}
