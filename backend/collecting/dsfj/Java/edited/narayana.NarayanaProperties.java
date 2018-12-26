

package org.springframework.boot.jta.narayana;

import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.convert.DurationUnit;


@ConfigurationProperties(prefix = NarayanaProperties.PROPERTIES_PREFIX)
public class NarayanaProperties {

	
	public static final String PROPERTIES_PREFIX = "spring.jta.narayana";

	
	private String logDir;

	
	private String transactionManagerId = "1";

	
	private boolean onePhaseCommit = true;

	
	@DurationUnit(ChronoUnit.SECONDS)
	private Duration defaultTimeout = Duration.ofSeconds(60);

	
	@DurationUnit(ChronoUnit.SECONDS)
	private Duration periodicRecoveryPeriod = Duration.ofSeconds(120);

	
	@DurationUnit(ChronoUnit.SECONDS)
	private Duration recoveryBackoffPeriod = Duration.ofSeconds(10);

	
	private String recoveryDbUser = null;

	
	private String recoveryDbPass = null;

	
	private String recoveryJmsUser = null;

	
	private String recoveryJmsPass = null;

	
	private List<String> xaResourceOrphanFilters = new ArrayList<>(Arrays.asList(
			"com.arjuna.ats.internal.jta.recovery.arjunacore.JTATransactionLogXAResourceOrphanFilter",
			"com.arjuna.ats.internal.jta.recovery.arjunacore.JTANodeNameXAResourceOrphanFilter"));

	
	private List<String> recoveryModules = new ArrayList<>(Arrays.asList(
			"com.arjuna.ats.internal.arjuna.recovery.AtomicActionRecoveryModule",
			"com.arjuna.ats.internal.jta.recovery.arjunacore.XARecoveryModule"));

	
	private List<String> expiryScanners = new ArrayList<>(Collections.singletonList(
			"com.arjuna.ats.internal.arjuna.recovery.ExpiredTransactionStatusManagerScanner"));

	public String getLogDir() {
		return this.logDir;
	}

	public void setLogDir(String logDir) {
		this.logDir = logDir;
	}

	public String getTransactionManagerId() {
		return this.transactionManagerId;
	}

	public void setTransactionManagerId(String transactionManagerId) {
		this.transactionManagerId = transactionManagerId;
	}

	public boolean isOnePhaseCommit() {
		return this.onePhaseCommit;
	}

	public void setOnePhaseCommit(boolean onePhaseCommit) {
		this.onePhaseCommit = onePhaseCommit;
	}

	public Duration getDefaultTimeout() {
		return this.defaultTimeout;
	}

	public void setDefaultTimeout(Duration defaultTimeout) {
		this.defaultTimeout = defaultTimeout;
	}

	public Duration getPeriodicRecoveryPeriod() {
		return this.periodicRecoveryPeriod;
	}

	public void setPeriodicRecoveryPeriod(Duration periodicRecoveryPeriod) {
		this.periodicRecoveryPeriod = periodicRecoveryPeriod;
	}

	public Duration getRecoveryBackoffPeriod() {
		return this.recoveryBackoffPeriod;
	}

	public void setRecoveryBackoffPeriod(Duration recoveryBackoffPeriod) {
		this.recoveryBackoffPeriod = recoveryBackoffPeriod;
	}

	public List<String> getXaResourceOrphanFilters() {
		return this.xaResourceOrphanFilters;
	}

	public void setXaResourceOrphanFilters(List<String> xaResourceOrphanFilters) {
		this.xaResourceOrphanFilters = xaResourceOrphanFilters;
	}

	public List<String> getRecoveryModules() {
		return this.recoveryModules;
	}

	public void setRecoveryModules(List<String> recoveryModules) {
		this.recoveryModules = recoveryModules;
	}

	public List<String> getExpiryScanners() {
		return this.expiryScanners;
	}

	public void setExpiryScanners(List<String> expiryScanners) {
		this.expiryScanners = expiryScanners;
	}

	public String getRecoveryDbUser() {
		return this.recoveryDbUser;
	}

	public void setRecoveryDbUser(String recoveryDbUser) {
		this.recoveryDbUser = recoveryDbUser;
	}

	public String getRecoveryDbPass() {
		return this.recoveryDbPass;
	}

	public void setRecoveryDbPass(String recoveryDbPass) {
		this.recoveryDbPass = recoveryDbPass;
	}

	public String getRecoveryJmsUser() {
		return this.recoveryJmsUser;
	}

	public void setRecoveryJmsUser(String recoveryJmsUser) {
		this.recoveryJmsUser = recoveryJmsUser;
	}

	public String getRecoveryJmsPass() {
		return this.recoveryJmsPass;
	}

	public void setRecoveryJmsPass(String recoveryJmsPass) {
		this.recoveryJmsPass = recoveryJmsPass;
	}

}
