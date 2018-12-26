
package com.alibaba.dubbo.rpc.protocol.dubbo.status;

import com.alibaba.dubbo.common.extension.Activate;
import com.alibaba.dubbo.common.status.Status;
import com.alibaba.dubbo.common.status.StatusChecker;
import com.alibaba.dubbo.remoting.exchange.ExchangeServer;
import com.alibaba.dubbo.rpc.protocol.dubbo.DubboProtocol;

import java.util.Collection;


@Activate
public class ServerStatusChecker implements StatusChecker {

    public Status check() {
        Collection<ExchangeServer> servers = DubboProtocol.getDubboProtocol().getServers();
        if (servers == null || servers.isEmpty()) {
            return new Status(Status.Level.UNKNOWN);
        }
        Status.Level level = Status.Level.OK;
        StringBuilder buf = new StringBuilder();
        for (ExchangeServer server : servers) {
            if (!server.isBound()) {
                level = Status.Level.ERROR;
                buf.setLength(0);
                buf.append(server.getLocalAddress());
                break;
            }
            if (buf.length() > 0) {
                buf.append(",");
            }
            buf.append(server.getLocalAddress());
            buf.append("(clients:");
            buf.append(server.getChannels().size());
            buf.append(")");
        }
        return new Status(level, buf.toString());
    }

}