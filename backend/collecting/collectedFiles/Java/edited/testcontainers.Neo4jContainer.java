

package org.springframework.boot.testsupport.testcontainers;

import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;

import org.neo4j.ogm.config.Configuration;
import org.neo4j.ogm.session.SessionFactory;
import org.rnorth.ducttape.TimeoutException;
import org.rnorth.ducttape.unreliables.Unreliables;
import org.testcontainers.containers.GenericContainer;
import org.testcontainers.containers.wait.HostPortWaitStrategy;


public class Neo4jContainer extends Container {

	public Neo4jContainer() {
		super("neo4j:3.3.1", 7687, (container) -> container.waitingFor(new WaitStrategy())
				.withEnv("NEO4J_AUTH", "none"));
	}

	private static class WaitStrategy extends HostPortWaitStrategy {

		@Override
		protected void waitUntilReady() {
			super.waitUntilReady();
			Configuration configuration = new Configuration.Builder()
					.uri("bolt:					.build();
			SessionFactory sessionFactory = new SessionFactory(configuration,
					"org.springframework.boot.test.autoconfigure.data.neo4j");
			try {
				Unreliables.retryUntilTrue((int) this.startupTimeout.getSeconds(),
						TimeUnit.SECONDS, checkConnection(sessionFactory));
			}
			catch (TimeoutException e) {
				throw new IllegalStateException();
			}
		}

		private Callable<Boolean> checkConnection(SessionFactory sessionFactory) {
			return () -> {
				try {
					sessionFactory.openSession().beginTransaction().close();
					return true;
				}
				catch (Exception ex) {
					return false;
				}
			};
		}

	}

}
