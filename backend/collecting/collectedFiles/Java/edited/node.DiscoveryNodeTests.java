

package org.elasticsearch.cluster.node;

import org.elasticsearch.Version;
import org.elasticsearch.common.io.stream.BytesStreamOutput;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.test.ESTestCase;

import java.net.InetAddress;

import static java.util.Collections.emptyMap;
import static java.util.Collections.emptySet;

public class DiscoveryNodeTests extends ESTestCase {

    public void testDiscoveryNodeIsCreatedWithHostFromInetAddress() throws Exception {
        InetAddress inetAddress = randomBoolean() ? InetAddress.getByName("192.0.2.1") :
            InetAddress.getByAddress("name1", new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 1});
        TransportAddress transportAddress = new TransportAddress(inetAddress, randomIntBetween(0, 65535));
        DiscoveryNode node = new DiscoveryNode("name1", "id1", transportAddress, emptyMap(), emptySet(), Version.CURRENT);
        assertEquals(transportAddress.address().getHostString(), node.getHostName());
        assertEquals(transportAddress.getAddress(), node.getHostAddress());
    }

    public void testDiscoveryNodeSerializationKeepsHost() throws Exception {
        InetAddress inetAddress = InetAddress.getByAddress("name1", new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 1});
        TransportAddress transportAddress = new TransportAddress(inetAddress, randomIntBetween(0, 65535));
        DiscoveryNode node = new DiscoveryNode("name1", "id1", transportAddress, emptyMap(), emptySet(), Version.CURRENT);

        BytesStreamOutput streamOutput = new BytesStreamOutput();
        streamOutput.setVersion(Version.CURRENT);
        node.writeTo(streamOutput);

        StreamInput in = StreamInput.wrap(streamOutput.bytes().toBytesRef().bytes);
        DiscoveryNode serialized = new DiscoveryNode(in);
        assertEquals(transportAddress.address().getHostString(), serialized.getHostName());
        assertEquals(transportAddress.address().getHostString(), serialized.getAddress().address().getHostString());
        assertEquals(transportAddress.getAddress(), serialized.getHostAddress());
        assertEquals(transportAddress.getAddress(), serialized.getAddress().getAddress());
        assertEquals(transportAddress.getPort(), serialized.getAddress().getPort());
    }
}
