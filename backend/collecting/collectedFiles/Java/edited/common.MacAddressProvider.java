

package org.elasticsearch.common;

import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

public class MacAddressProvider {

    private static byte[] getMacAddress() throws SocketException {
        Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces();
        if (en != null) {
            while (en.hasMoreElements()) {
                NetworkInterface nint = en.nextElement();
                if (!nint.isLoopback()) {
                                        byte[] address = nint.getHardwareAddress();
                    if (isValidAddress(address)) {
                        return address;
                    }
                }
            }
        }
                return null;
    }

    private static boolean isValidAddress(byte[] address) {
        if (address == null || address.length != 6) {
            return false;
        }
        for (byte b : address) {
            if (b != 0x00) {
                return true;             }
        }
        return false;
    }

    public static byte[] getSecureMungedAddress() {
        byte[] address = null;
        try {
            address = getMacAddress();
        } catch (SocketException e) {
                    }

        if (!isValidAddress(address)) {
            address = constructDummyMulticastAddress();
        }

        byte[] mungedBytes = new byte[6];
        SecureRandomHolder.INSTANCE.nextBytes(mungedBytes);
        for (int i = 0; i < 6; ++i) {
            mungedBytes[i] ^= address[i];
        }

        return mungedBytes;
    }

    private static byte[] constructDummyMulticastAddress() {
        byte[] dummy = new byte[6];
        SecureRandomHolder.INSTANCE.nextBytes(dummy);
        
        dummy[0] |= (byte) 0x01;
        return dummy;
    }


}
