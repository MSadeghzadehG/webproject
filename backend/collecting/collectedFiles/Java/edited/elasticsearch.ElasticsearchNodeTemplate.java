

package org.springframework.boot.autoconfigure.data.elasticsearch;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.function.Consumer;

import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.node.InternalSettingsPreparer;
import org.elasticsearch.node.Node;
import org.elasticsearch.node.NodeValidationException;
import org.elasticsearch.transport.Netty4Plugin;
import org.elasticsearch.transport.Transport;


public class ElasticsearchNodeTemplate {

	public void doWithNode(Consumer<ElasticsearchNode> consumer) {
		System.setProperty("es.set.netty.runtime.available.processors", "false");
		Node node = null;
		try {
			node = startNode();
			consumer.accept(new ElasticsearchNode(node));
		}
		catch (Exception ex) {
			throw new RuntimeException(ex);
		}
		finally {
			if (node != null) {
				try {
					node.close();
				}
				catch (Exception ex) {
									}
			}
			System.clearProperty("es.set.netty.runtime.available.processors");
		}
	}

	private Node startNode() throws NodeValidationException {
		Node node = new NettyTransportNode();
		node.start();
		return node;
	}

	private static final class NettyTransportNode extends Node {

		private NettyTransportNode() {
			super(InternalSettingsPreparer.prepareEnvironment(Settings.builder()
					.put("path.home", "target/es/node").put("transport.type", "netty4")
					.put("http.enabled", true).put("node.portsfile", true)
					.put("http.port", 0).put("transport.tcp.port", 0).build(), null),
					Arrays.asList(Netty4Plugin.class));
			new File("target/es/node/logs").mkdirs();
		}
	}

	public final class ElasticsearchNode {

		private final Node node;

		private ElasticsearchNode(Node node) {
			this.node = node;
		}

		public int getTcpPort() {
			return this.node.injector().getInstance(Transport.class).boundAddress()
					.publishAddress().getPort();
		}

		public int getHttpPort() {
			try {
				for (String line : Files
						.readAllLines(Paths.get("target/es/node/logs/http.ports"))) {
					if (line.startsWith("127.0.0.1")) {
						return Integer.parseInt(line.substring(line.indexOf(":") + 1));
					}
				}
				throw new IllegalStateException("HTTP port not found");
			}
			catch (IOException ex) {
				throw new IllegalStateException("Failed to read HTTP port", ex);
			}
		}

	}

}
