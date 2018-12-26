

package org.springframework.boot.actuate.flyway;

import java.time.Instant;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import org.flywaydb.core.Flyway;
import org.flywaydb.core.api.MigrationInfo;
import org.flywaydb.core.api.MigrationState;
import org.flywaydb.core.api.MigrationType;

import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.annotation.ReadOperation;
import org.springframework.context.ApplicationContext;


@Endpoint(id = "flyway")
public class FlywayEndpoint {

	private final ApplicationContext context;

	public FlywayEndpoint(ApplicationContext context) {
		this.context = context;
	}

	@ReadOperation
	public ApplicationFlywayBeans flywayBeans() {
		ApplicationContext target = this.context;
		Map<String, ContextFlywayBeans> contextFlywayBeans = new HashMap<>();
		while (target != null) {
			Map<String, FlywayDescriptor> flywayBeans = new HashMap<>();
			target.getBeansOfType(Flyway.class).forEach((name, flyway) -> flywayBeans
					.put(name, new FlywayDescriptor(flyway.info().all())));
			ApplicationContext parent = target.getParent();
			contextFlywayBeans.put(target.getId(), new ContextFlywayBeans(flywayBeans,
					parent == null ? null : parent.getId()));
			target = parent;
		}
		return new ApplicationFlywayBeans(contextFlywayBeans);
	}

	
	public static final class ApplicationFlywayBeans {

		private final Map<String, ContextFlywayBeans> contexts;

		private ApplicationFlywayBeans(Map<String, ContextFlywayBeans> contexts) {
			this.contexts = contexts;
		}

		public Map<String, ContextFlywayBeans> getContexts() {
			return this.contexts;
		}

	}

	
	public static final class ContextFlywayBeans {

		private final Map<String, FlywayDescriptor> flywayBeans;

		private final String parentId;

		private ContextFlywayBeans(Map<String, FlywayDescriptor> flywayBeans,
				String parentId) {
			this.flywayBeans = flywayBeans;
			this.parentId = parentId;
		}

		public Map<String, FlywayDescriptor> getFlywayBeans() {
			return this.flywayBeans;
		}

		public String getParentId() {
			return this.parentId;
		}

	}

	
	public static class FlywayDescriptor {

		private final List<FlywayMigration> migrations;

		private FlywayDescriptor(MigrationInfo[] migrations) {
			this.migrations = Stream.of(migrations).map(FlywayMigration::new)
					.collect(Collectors.toList());
		}

		public FlywayDescriptor(List<FlywayMigration> migrations) {
			this.migrations = migrations;
		}

		public List<FlywayMigration> getMigrations() {
			return this.migrations;
		}

	}

	
	public static final class FlywayMigration {

		private final MigrationType type;

		private final Integer checksum;

		private final String version;

		private final String description;

		private final String script;

		private final MigrationState state;

		private final String installedBy;

		private final Instant installedOn;

		private final Integer installedRank;

		private final Integer executionTime;

		private FlywayMigration(MigrationInfo info) {
			this.type = info.getType();
			this.checksum = info.getChecksum();
			this.version = nullSafeToString(info.getVersion());
			this.description = info.getDescription();
			this.script = info.getScript();
			this.state = info.getState();
			this.installedBy = info.getInstalledBy();
			this.installedOn = Instant.ofEpochMilli(info.getInstalledOn().getTime());
			this.installedRank = info.getInstalledRank();
			this.executionTime = info.getExecutionTime();
		}

		private String nullSafeToString(Object obj) {
			return (obj == null ? null : obj.toString());
		}

		public MigrationType getType() {
			return this.type;
		}

		public Integer getChecksum() {
			return this.checksum;
		}

		public String getVersion() {
			return this.version;
		}

		public String getDescription() {
			return this.description;
		}

		public String getScript() {
			return this.script;
		}

		public MigrationState getState() {
			return this.state;
		}

		public String getInstalledBy() {
			return this.installedBy;
		}

		public Instant getInstalledOn() {
			return this.installedOn;
		}

		public Integer getInstalledRank() {
			return this.installedRank;
		}

		public Integer getExecutionTime() {
			return this.executionTime;
		}

	}

}
