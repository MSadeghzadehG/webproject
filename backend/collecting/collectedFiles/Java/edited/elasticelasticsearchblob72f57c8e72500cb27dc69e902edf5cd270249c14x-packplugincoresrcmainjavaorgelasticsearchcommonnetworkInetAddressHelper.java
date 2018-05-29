
package org.elasticsearch.common.network;

import java.net.InetAddress;
import java.net.SocketException;


public class InetAddressHelper {

    private InetAddressHelper() {}

    public static InetAddress[] getAllAddresses() throws SocketException {
        return NetworkUtils.getAllAddresses();
    }
}
