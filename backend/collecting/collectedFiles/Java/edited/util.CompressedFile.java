
package hudson.util;

import com.jcraft.jzlib.GZIPInputStream;
import com.jcraft.jzlib.GZIPOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.Reader;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;


public class CompressedFile {
    
    private final File file;

    
    private final File gz;

    public CompressedFile(File file) {
        this.file = file;
        this.gz = new File(file.getParentFile(),file.getName()+".gz");
    }

    
    public OutputStream write() throws IOException {
        if(gz.exists())
            gz.delete();
        try {
            return Files.newOutputStream(file.toPath());
        } catch (InvalidPathException e) {
            throw new IOException(e);
        }
    }

    
    public InputStream read() throws IOException {
        if(file.exists())
            try {
                return Files.newInputStream(file.toPath());
            } catch (InvalidPathException e) {
                throw new IOException(e);
            }

                if(gz.exists())
            try {
                return new GZIPInputStream(Files.newInputStream(gz.toPath()));
            } catch (InvalidPathException e) {
                throw new IOException(e);
            }

                throw new FileNotFoundException(file.getName());
    }

    
    public String loadAsString() throws IOException {
        long sizeGuess;
        if(file.exists())
            sizeGuess = file.length();
        else
        if(gz.exists())
            sizeGuess = gz.length()*2;
        else
            return "";

        StringBuilder str = new StringBuilder((int)sizeGuess);

        try (InputStream is = read();
             Reader r = new InputStreamReader(is)) {
            char[] buf = new char[8192];
            int len;
            while((len=r.read(buf,0,buf.length))>0)
                str.append(buf,0,len);
        }

        return str.toString();
    }

    
    public void compress() {
        compressionThread.submit(new Runnable() {
            public void run() {
                try {
                    try (InputStream in = read();
                         OutputStream os = Files.newOutputStream(gz.toPath());
                         OutputStream out = new GZIPOutputStream(os)) {
                        org.apache.commons.io.IOUtils.copy(in, out);
                    }
                                        file.delete();
                } catch (IOException e) {
                    LOGGER.log(Level.WARNING, "Failed to compress "+file,e);
                    gz.delete();                 }
            }
        });
    }

    
    private static final ExecutorService compressionThread = new ThreadPoolExecutor(
        0, 1, 5L, TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>(),
        new ExceptionCatchingThreadFactory(new NamingThreadFactory(new DaemonThreadFactory(), "CompressedFile")));

    private static final Logger LOGGER = Logger.getLogger(CompressedFile.class.getName());
}
