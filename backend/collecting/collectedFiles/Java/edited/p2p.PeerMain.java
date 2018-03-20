
package com.alibaba.dubbo.remoting.p2p;

import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.transport.ChannelHandlerAdapter;

import java.util.Collection;


public class PeerMain {

    public static void main(String[] args) throws Throwable {
        String groupURL = "multicast:        final String peerURL = "dubbo:
                Peer peer = Networkers.join(groupURL, peerURL, new ChannelHandlerAdapter() {
            @Override
            public void received(Channel channel, Object message) throws RemotingException {
                System.out.println("Received: " + message + " in " + peerURL);
            }
        });

                for (int i = 0; i < Integer.MAX_VALUE; i++) {
            Collection<Channel> channels = peer.getChannels();             if (channels != null && channels.size() > 0) {
                for (Channel channel : channels) {
                    channel.send("(" + i + ") " + peerURL);                 }
            }
            Thread.sleep(1000);
        }

                peer.leave();
    }

}