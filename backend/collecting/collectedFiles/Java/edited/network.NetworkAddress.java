

package org.elasticsearch.common.network;

import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.Objects;


public final class NetworkAddress {
    
    private NetworkAddress() {}

    
    public static String format(InetAddress address) {
        return format(address, -1);
    }

    
    public static String format(InetSocketAddress address) {
        return format(address.getAddress(), address.getPort());
    }

        static String format(InetAddress address, int port) {
        Objects.requireNonNull(address);

        StringBuilder builder = new StringBuilder();

        if (port != -1 && address instanceof Inet6Address) {
            builder.append(InetAddresses.toUriString(address));
        } else {
            builder.append(InetAddresses.toAddrString(address));
        }

        if (port != -1) {
            builder.append(':');
            builder.append(port);
        }

        return builder.toString();
    }
}
