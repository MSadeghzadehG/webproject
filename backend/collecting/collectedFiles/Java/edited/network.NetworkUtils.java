

package org.elasticsearch.common.network;

import org.apache.lucene.util.BytesRef;
import org.apache.lucene.util.Constants;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;


public abstract class NetworkUtils {

    
    private NetworkUtils() {}
    
    
    @Deprecated
    static final boolean PREFER_V6 = Boolean.parseBoolean(System.getProperty("java.net.preferIPv6Addresses", "false"));

    
    @Deprecated
    public static final boolean SUPPORTS_V6;

    static {
        boolean v = false;
        try {
            for (NetworkInterface nic : getInterfaces()) {
                for (InetAddress address : Collections.list(nic.getInetAddresses())) {
                    if (address instanceof Inet6Address) {
                        v = true;
                        break;
                    }
                }
            }
        } catch (SecurityException | SocketException misconfiguration) {
            v = true;         }
        SUPPORTS_V6 = v;
    }
    
    
    static int sortKey(InetAddress address, boolean prefer_v6) {
        int key = address.getAddress().length;
        if (prefer_v6) {
            key = -key;
        }
        
        if (address.isAnyLocalAddress()) {
            key += 5;
        }
        if (address.isMulticastAddress()) {
            key += 4;
        }
        if (address.isLoopbackAddress()) {
            key += 3;
        }
        if (address.isLinkLocalAddress()) {
            key += 2;
        }
        if (address.isSiteLocalAddress()) {
            key += 1;
        }

        return key;
    }

    
    @Deprecated
        public static void sortAddresses(List<InetAddress> list) {
        Collections.sort(list, new Comparator<InetAddress>() {
            @Override
            public int compare(InetAddress left, InetAddress right) {
                int cmp = Integer.compare(sortKey(left, PREFER_V6), sortKey(right, PREFER_V6));
                if (cmp == 0) {
                    cmp = new BytesRef(left.getAddress()).compareTo(new BytesRef(right.getAddress()));
                }
                return cmp;
            }
        });
    }
    
    
    static List<NetworkInterface> getInterfaces() throws SocketException {
        List<NetworkInterface> all = new ArrayList<>();
        addAllInterfaces(all, Collections.list(NetworkInterface.getNetworkInterfaces()));
        Collections.sort(all, new Comparator<NetworkInterface>() {
            @Override
            public int compare(NetworkInterface left, NetworkInterface right) {
                return Integer.compare(left.getIndex(), right.getIndex());
            }
        });
        return all;
    }
    
    
    private static void addAllInterfaces(List<NetworkInterface> target, List<NetworkInterface> level) {
        if (!level.isEmpty()) {
            target.addAll(level);
            for (NetworkInterface intf : level) {
                addAllInterfaces(target, Collections.list(intf.getSubInterfaces()));
            }
        }
    }
    
    
    public static boolean defaultReuseAddress() {
        return Constants.WINDOWS ? false : true;
    }

    
    static InetAddress[] getLoopbackAddresses() throws SocketException {
        List<InetAddress> list = new ArrayList<>();
        for (NetworkInterface intf : getInterfaces()) {
            if (intf.isUp()) {
                for (InetAddress address : Collections.list(intf.getInetAddresses())) {
                    if (address.isLoopbackAddress()) {
                        list.add(address);
                    }
                }
            }
        }
        if (list.isEmpty()) {
            throw new IllegalArgumentException("No up-and-running loopback addresses found, got " + getInterfaces());
        }
        return list.toArray(new InetAddress[list.size()]);
    }
    
    
    static InetAddress[] getSiteLocalAddresses() throws SocketException {
        List<InetAddress> list = new ArrayList<>();
        for (NetworkInterface intf : getInterfaces()) {
            if (intf.isUp()) {
                for (InetAddress address : Collections.list(intf.getInetAddresses())) {
                    if (address.isSiteLocalAddress()) {
                        list.add(address);
                    }
                }
            }
        }
        if (list.isEmpty()) {
            throw new IllegalArgumentException("No up-and-running site-local (private) addresses found, got " + getInterfaces());
        }
        return list.toArray(new InetAddress[list.size()]);
    }
    
    
    static InetAddress[] getGlobalAddresses() throws SocketException {
        List<InetAddress> list = new ArrayList<>();
        for (NetworkInterface intf : getInterfaces()) {
            if (intf.isUp()) {
                for (InetAddress address : Collections.list(intf.getInetAddresses())) {
                    if (address.isLoopbackAddress() == false && 
                        address.isSiteLocalAddress() == false &&
                        address.isLinkLocalAddress() == false) {
                        list.add(address);
                    }
                }
            }
        }
        if (list.isEmpty()) {
            throw new IllegalArgumentException("No up-and-running global-scope (public) addresses found, got " + getInterfaces());
        }
        return list.toArray(new InetAddress[list.size()]);
    }
    
    
    static InetAddress[] getAllAddresses() throws SocketException {
        List<InetAddress> list = new ArrayList<>();
        for (NetworkInterface intf : getInterfaces()) {
            if (intf.isUp()) {
                for (InetAddress address : Collections.list(intf.getInetAddresses())) {
                    list.add(address);
                }
            }
        }
        if (list.isEmpty()) {
            throw new IllegalArgumentException("No up-and-running addresses found, got " + getInterfaces());
        }
        return list.toArray(new InetAddress[list.size()]);
    }
    
    
    static InetAddress[] getAddressesForInterface(String name) throws SocketException {
        Optional<NetworkInterface> networkInterface = getInterfaces().stream().filter((netIf) -> name.equals(netIf.getName())).findFirst();

        if (networkInterface.isPresent() == false) {
            throw new IllegalArgumentException("No interface named '" + name + "' found, got " + getInterfaces());
        }
        if (!networkInterface.get().isUp()) {
            throw new IllegalArgumentException("Interface '" + name + "' is not up and running");
        }
        List<InetAddress> list = Collections.list(networkInterface.get().getInetAddresses());
        if (list.isEmpty()) {
            throw new IllegalArgumentException("Interface '" + name + "' has no internet addresses");
        }
        return list.toArray(new InetAddress[list.size()]);
    }
    
    
    static InetAddress[] filterIPV4(InetAddress addresses[]) {
        List<InetAddress> list = new ArrayList<>();
        for (InetAddress address : addresses) {
            if (address instanceof Inet4Address) {
                list.add(address);
            }
        }
        if (list.isEmpty()) {
            throw new IllegalArgumentException("No ipv4 addresses found in " + Arrays.toString(addresses));
        }
        return list.toArray(new InetAddress[list.size()]);
    }
    
    
    static InetAddress[] filterIPV6(InetAddress addresses[]) {
        List<InetAddress> list = new ArrayList<>();
        for (InetAddress address : addresses) {
            if (address instanceof Inet6Address) {
                list.add(address);
            }
        }
        if (list.isEmpty()) {
            throw new IllegalArgumentException("No ipv6 addresses found in " + Arrays.toString(addresses));
        }
        return list.toArray(new InetAddress[list.size()]);
    }
}
