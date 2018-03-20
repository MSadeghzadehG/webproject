
package hudson.model;

import hudson.util.StreamTaskListener;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.nio.charset.Charset;


public class StreamBuildListener extends StreamTaskListener implements BuildListener {
    public StreamBuildListener(OutputStream out, Charset charset) {
        super(out, charset);
    }

    public StreamBuildListener(File out, Charset charset) throws IOException {
        super(out, charset);
    }

    public StreamBuildListener(OutputStream w) {
        super(w);
    }

    
    @Deprecated
    public StreamBuildListener(PrintStream w) {
        super(w);
    }

    public StreamBuildListener(PrintStream w, Charset charset) {
        super(w,charset);
    }

    private static final long serialVersionUID = 1L;
}
