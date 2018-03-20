

package org.springframework.boot.actuate.liquibase;

import java.time.Instant;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

import javax.sql.DataSource;

import liquibase.changelog.ChangeLogHistoryService;
import liquibase.changelog.ChangeSet.ExecType;
import liquibase.changelog.RanChangeSet;
import liquibase.changelog.StandardChangeLogHistoryService;
import liquibase.database.Database;
import liquibase.database.DatabaseFactory;
import liquibase.database.jvm.JdbcConnection;
import liquibase.integration.spring.SpringLiquibase;

import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.annotation.ReadOperation;
import org.springframework.context.ApplicationContext;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


@Endpoint(id = "liquibase")
public class LiquibaseEndpoint {

	private final ApplicationContext context;

	public LiquibaseEndpoint(ApplicationContext context) {
		Assert.notNull(context, "Context must be specified");
		this.context = context;
	}

	@ReadOperation
	public ApplicationLiquibaseBeans liquibaseBeans() {
		ApplicationContext target = this.context;
		Map<String, ContextLiquibaseBeans> contextBeans = new HashMap<>();
		while (target != null) {
			Map<String, LiquibaseBean> liquibaseBeans = new HashMap<>();
			DatabaseFactory factory = DatabaseFactory.getInstance();
			StandardChangeLogHistoryService service = new StandardChangeLogHistoryService();
			this.context.getBeansOfType(SpringLiquibase.class)
					.forEach((name, liquibase) -> liquibaseBeans.put(name,
							createReport(liquibase, service, factory)));
			ApplicationContext parent = target.getParent();
			contextBeans.put(target.getId(), new ContextLiquibaseBeans(liquibaseBeans,
					parent == null ? null : parent.getId()));
			target = parent;
		}
		return new ApplicationLiquibaseBeans(contextBeans);
	}

	private LiquibaseBean createReport(SpringLiquibase liquibase,
			ChangeLogHistoryService service, DatabaseFactory factory) {
		try {
			DataSource dataSource = liquibase.getDataSource();
			JdbcConnection connection = new JdbcConnection(dataSource.getConnection());
			try {
				Database database = factory.findCorrectDatabaseImplementation(connection);
				String defaultSchema = liquibase.getDefaultSchema();
				if (StringUtils.hasText(defaultSchema)) {
					database.setDefaultSchemaName(defaultSchema);
				}
				service.setDatabase(database);
				return new LiquibaseBean(service.getRanChangeSets().stream()
						.map(ChangeSet::new).collect(Collectors.toList()));
			}
			finally {
				connection.close();
			}
		}
		catch (Exception ex) {
			throw new IllegalStateException("Unable to get Liquibase change sets", ex);
		}
	}

	
	public static final class ApplicationLiquibaseBeans {

		private final Map<String, ContextLiquibaseBeans> contexts;

		private ApplicationLiquibaseBeans(Map<String, ContextLiquibaseBeans> contexts) {
			this.contexts = contexts;
		}

		public Map<String, ContextLiquibaseBeans> getContexts() {
			return this.contexts;
		}

	}

	
	public static final class ContextLiquibaseBeans {

		private final Map<String, LiquibaseBean> liquibaseBeans;

		private final String parentId;

		private ContextLiquibaseBeans(Map<String, LiquibaseBean> liquibaseBeans,
				String parentId) {
			this.liquibaseBeans = liquibaseBeans;
			this.parentId = parentId;
		}

		public Map<String, LiquibaseBean> getLiquibaseBeans() {
			return this.liquibaseBeans;
		}

		public String getParentId() {
			return this.parentId;
		}

	}

	
	public static final class LiquibaseBean {

		private final List<ChangeSet> changeSets;

		public LiquibaseBean(List<ChangeSet> changeSets) {
			this.changeSets = changeSets;
		}

		public List<ChangeSet> getChangeSets() {
			return this.changeSets;
		}

	}

	
	public static class ChangeSet {

		private final String author;

		private final String changeLog;

		private final String comments;

		private final Set<String> contexts;

		private final Instant dateExecuted;

		private final String deploymentId;

		private final String description;

		private final ExecType execType;

		private final String id;

		private final Set<String> labels;

		private final String checksum;

		private final Integer orderExecuted;

		private final String tag;

		public ChangeSet(RanChangeSet ranChangeSet) {
			this.author = ranChangeSet.getAuthor();
			this.changeLog = ranChangeSet.getChangeLog();
			this.comments = ranChangeSet.getComments();
			this.contexts = ranChangeSet.getContextExpression().getContexts();
			this.dateExecuted = Instant
					.ofEpochMilli(ranChangeSet.getDateExecuted().getTime());
			this.deploymentId = ranChangeSet.getDeploymentId();
			this.description = ranChangeSet.getDescription();
			this.execType = ranChangeSet.getExecType();
			this.id = ranChangeSet.getId();
			this.labels = ranChangeSet.getLabels().getLabels();
			this.checksum = ranChangeSet.getLastCheckSum() == null ? null
					: ranChangeSet.getLastCheckSum().toString();
			this.orderExecuted = ranChangeSet.getOrderExecuted();
			this.tag = ranChangeSet.getTag();
		}

		public String getAuthor() {
			return this.author;
		}

		public String getChangeLog() {
			return this.changeLog;
		}

		public String getComments() {
			return this.comments;
		}

		public Set<String> getContexts() {
			return this.contexts;
		}

		public Instant getDateExecuted() {
			return this.dateExecuted;
		}

		public String getDeploymentId() {
			return this.deploymentId;
		}

		public String getDescription() {
			return this.description;
		}

		public ExecType getExecType() {
			return this.execType;
		}

		public String getId() {
			return this.id;
		}

		public Set<String> getLabels() {
			return this.labels;
		}

		public String getChecksum() {
			return this.checksum;
		}

		public Integer getOrderExecuted() {
			return this.orderExecuted;
		}

		public String getTag() {
			return this.tag;
		}

	}

	
	public static class ContextExpression {

		private final Set<String> contexts;

		public ContextExpression(Set<String> contexts) {
			this.contexts = contexts;
		}

		public Set<String> getContexts() {
			return this.contexts;
		}

	}

}
