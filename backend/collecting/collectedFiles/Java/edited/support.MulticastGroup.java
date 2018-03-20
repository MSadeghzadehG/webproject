
package com.alibaba.dubbo.remoting.p2p.support;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.StringUtils;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.p2p.Peer;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;


public class MulticastGroup extends AbstractGroup {

    private static final String JOIN = "join";

    private static final String LEAVE = "leave";

    private InetAddress mutilcastAddress;

    private MulticastSocket mutilcastSocket;

    public MulticastGroup(URL url) {
        super(url);
        if (!isMulticastAddress(url.getHost())) {
            throw new IllegalArgumentException("Invalid multicast address " + url.getHost() + ", scope: 224.0.0.0 - 239.255.255.255");
        }
        try {
            mutilcastAddress = InetAddress.getByName(url.getHost());
            mutilcastSocket = new MulticastSocket(url.getPort());
            mutilcastSocket.setLoopbackMode(false);
            mutilcastSocket.joinGroup(mutilcastAddress);
            Thread thread = new Thread(new Runnable() {
                public void run() {
                    byte[] buf = new byte[1024];
                    DatagramPacket recv = new DatagramPacket(buf, buf.length);
                    while (true) {
                        try {
                            mutilcastSocket.receive(recv);
                            MulticastGroup.this.receive(new String(recv.getData()).trim(), (InetSocketAddress) recv.getSocketAddress());
                        } catch (Exception e) {
                            logger.error(e.getMessage(), e);
                        }
                    }
                }
            }, "MulticastGroupReceiver");
            thread.setDaemon(true);
            thread.start();
        } catch (IOException e) {
            throw new IllegalStateException(e.getMessage(), e);
        }
    }

    private static boolean isMulticastAddress(String ip) {
        int i = ip.indexOf('.');
        if (i > 0) {
            String prefix = ip.substring(0, i);
            if (StringUtils.isInteger(prefix)) {
                int p = Integer.parseInt(prefix);
                return p >= 224 && p <= 239;
            }
        }
        return false;
    }

    private void send(String msg) throws RemotingException {
        DatagramPacket hi = new DatagramPacket(msg.getBytes(), msg.length(), mutilcastAddress, mutilcastSocket.getLocalPort());
        try {
            mutilcastSocket.send(hi);
        } catch (IOException e) {
            throw new IllegalStateException(e.getMessage(), e);
        }
    }

    private void receive(String msg, InetSocketAddress remoteAddress) throws RemotingException {
        if (msg.startsWith(JOIN)) {
            String url = msg.substring(JOIN.length()).trim();
            connect(URL.valueOf(url));
        } else if (msg.startsWith(LEAVE)) {
            String url = msg.substring(LEAVE.length()).trim();
            disconnect(URL.valueOf(url));
        }
    }

    @Override
    public Peer join(URL url, ChannelHandler handler) throws RemotingException {
        Peer peer = super.join(url, handler);
        send(JOIN + " " + url.toFullString());
        return peer;
    }

    @Override
    public void leave(URL url) throws RemotingException {
        super.leave(url);
        send(LEAVE + " " + url.toFullString());
    }

}