package hudson;


import hudson.remoting.Channel;
import hudson.remoting.FastPipedInputStream;
import hudson.remoting.FastPipedOutputStream;
import java.io.IOException;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import org.junit.rules.ExternalResource;


public final class ChannelRule extends ExternalResource {
    
    public Channel french;
    public Channel british;
    private ExecutorService executors;

    @Override protected void before() throws Exception {
        executors = Executors.newCachedThreadPool();
        final FastPipedInputStream p1i = new FastPipedInputStream();
        final FastPipedInputStream p2i = new FastPipedInputStream();
        final FastPipedOutputStream p1o = new FastPipedOutputStream(p1i);
        final FastPipedOutputStream p2o = new FastPipedOutputStream(p2i);

        Future<Channel> f1 = executors.submit(new Callable<Channel>() {
            public Channel call() throws Exception {
                return new Channel("This side of the channel", executors, p1i, p2o);
            }
        });
        Future<Channel> f2 = executors.submit(new Callable<Channel>() {
            public Channel call() throws Exception {
                return new Channel("The other side of the channel", executors, p2i, p1o);
            }
        });
        french = f1.get();
        british = f2.get();
    }

    @Override protected void after() {
        try {
            french.close();             french.join();
            british.join();
        } catch (IOException e) {
                        
            e.printStackTrace();
        } catch (InterruptedException x) {
            throw new AssertionError(x);
        }
        executors.shutdownNow();
    }
}
