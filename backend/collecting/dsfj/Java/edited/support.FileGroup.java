
package com.alibaba.dubbo.remoting.p2p.support;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.IOUtils;
import com.alibaba.dubbo.common.utils.NamedThreadFactory;
import com.alibaba.dubbo.common.utils.NetUtils;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.p2p.Peer;

import java.io.File;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;


public class FileGroup extends AbstractGroup {

    private final File file;
        private final ScheduledExecutorService scheduledExecutorService = Executors.newScheduledThreadPool(3, new NamedThreadFactory("FileGroupModifiedChecker", true));
        private final ScheduledFuture<?> checkModifiedFuture;
    private volatile long last;

    public FileGroup(URL url) {
        super(url);
        String path = url.getAbsolutePath();
        file = new File(path);
        checkModifiedFuture = scheduledExecutorService.scheduleWithFixedDelay(new Runnable() {
            public void run() {
                                try {
                    check();
                } catch (Throwable t) {                     logger.error("Unexpected error occur at reconnect, cause: " + t.getMessage(), t);
                }
            }
        }, 2000, 2000, TimeUnit.MILLISECONDS);
    }

    public void close() {
        super.close();
        try {
            if (!checkModifiedFuture.isCancelled()) {
                checkModifiedFuture.cancel(true);
            }
        } catch (Throwable t) {
            logger.error(t.getMessage(), t);
        }
    }

    private void check() throws RemotingException {
        long modified = file.lastModified();
        if (modified > last) {
            last = modified;
            changed();
        }
    }

    private void changed() throws RemotingException {
        try {
            String[] lines = IOUtils.readLines(file);
            for (String line : lines) {
                connect(URL.valueOf(line));
            }
        } catch (IOException e) {
            throw new RemotingException(new InetSocketAddress(NetUtils.getLocalHost(), 0), getUrl().toInetSocketAddress(), e.getMessage(), e);
        }
    }

    public Peer join(URL url, ChannelHandler handler) throws RemotingException {
        Peer peer = super.join(url, handler);
        try {
            String full = url.toFullString();
            String[] lines = IOUtils.readLines(file);
            for (String line : lines) {
                if (full.equals(line)) {
                    return peer;
                }
            }
            IOUtils.appendLines(file, new String[]{full});
        } catch (IOException e) {
            throw new RemotingException(new InetSocketAddress(NetUtils.getLocalHost(), 0), getUrl().toInetSocketAddress(), e.getMessage(), e);
        }
        return peer;
    }

    @Override
    public void leave(URL url) throws RemotingException {
        super.leave(url);
        try {
            String full = url.toFullString();
            String[] lines = IOUtils.readLines(file);
            List<String> saves = new ArrayList<String>();
            for (String line : lines) {
                if (full.equals(line)) {
                    return;
                }
                saves.add(line);
            }
            IOUtils.appendLines(file, saves.toArray(new String[0]));
        } catch (IOException e) {
            throw new RemotingException(new InetSocketAddress(NetUtils.getLocalHost(), 0), getUrl().toInetSocketAddress(), e.getMessage(), e);
        }
    }

}