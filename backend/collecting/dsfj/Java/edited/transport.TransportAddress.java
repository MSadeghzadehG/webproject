

package org.elasticsearch.common.transport;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Writeable;
import org.elasticsearch.common.network.NetworkAddress;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;


public final class TransportAddress implements Writeable {

    
    public static final InetAddress META_ADDRESS;

    static {
        try {
            META_ADDRESS = InetAddress.getByName("0.0.0.0");
        } catch (UnknownHostException e) {
            throw new AssertionError(e);
        }
    }

    private final InetSocketAddress address;

    public TransportAddress(InetAddress address, int port) {
        this(new InetSocketAddress(address, port));
    }

    public TransportAddress(InetSocketAddress address) {
        if (address == null) {
            throw new IllegalArgumentException("InetSocketAddress must not be null");
        }
        if (address.getAddress() == null) {
            throw new IllegalArgumentException("Address must be resolved but wasn't - InetSocketAddress#getAddress() returned null");
        }
        this.address = address;
    }

    
    public TransportAddress(StreamInput in) throws IOException {
        final int len = in.readByte();
        final byte[] a = new byte[len];         in.readFully(a);
        String host = in.readString();         final InetAddress inetAddress = InetAddress.getByAddress(host, a);
        int port = in.readInt();
        this.address = new InetSocketAddress(inetAddress, port);
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        byte[] bytes = address.getAddress().getAddress();          out.writeByte((byte) bytes.length);         out.write(bytes, 0, bytes.length);
        out.writeString(address.getHostString());
                                out.writeInt(address.getPort());
    }

    
    public String getAddress() {
        return NetworkAddress.format(address.getAddress());
    }

    
    public int getPort() {
        return address.getPort();
    }

    
    public InetSocketAddress address() {
        return this.address;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        TransportAddress address1 = (TransportAddress) o;
        return address.equals(address1.address);
    }

    @Override
    public int hashCode() {
        return address != null ? address.hashCode() : 0;
    }

    @Override
    public String toString() {
        return NetworkAddress.format(address);
    }
}
