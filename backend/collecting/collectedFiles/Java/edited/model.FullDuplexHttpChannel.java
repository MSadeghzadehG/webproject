
package hudson.model;

import hudson.remoting.Channel;
import hudson.remoting.PingThread;
import hudson.remoting.Channel.Mode;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;
import java.util.logging.Level;
import java.util.logging.Logger;
import jenkins.util.FullDuplexHttpService;


abstract public class FullDuplexHttpChannel extends FullDuplexHttpService {
    private Channel channel;
    private final boolean restricted;

    public FullDuplexHttpChannel(UUID uuid, boolean restricted) throws IOException {
        super(uuid);
        this.restricted = restricted;
    }

    @Override
    protected void run(final InputStream upload, OutputStream download) throws IOException, InterruptedException {
        channel = new Channel("HTTP full-duplex channel " + uuid,
                Computer.threadPoolForRemoting, Mode.BINARY, upload, download, null, restricted);

                PingThread ping = new PingThread(channel) {
            @Override
            protected void onDead(Throwable diagnosis) {
                LOGGER.log(Level.INFO, "Duplex-HTTP session " + uuid + " is terminated", diagnosis);
                                try {
                    upload.close();
                } catch (IOException e) {
                                        throw new AssertionError(e);
                }
            }

            @Override
            protected void onDead() {
                onDead(null);
            }
        };
        ping.start();
        main(channel);
        channel.join();
        ping.interrupt();
    }

    protected abstract void main(Channel channel) throws IOException, InterruptedException;

    public Channel getChannel() {
        return channel;
    }

    private static final Logger LOGGER = Logger.getLogger(FullDuplexHttpChannel.class.getName());

}
