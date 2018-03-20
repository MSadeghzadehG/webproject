

package org.springframework.boot.actuate.mail;

import org.springframework.boot.actuate.health.AbstractHealthIndicator;
import org.springframework.boot.actuate.health.Health.Builder;
import org.springframework.boot.actuate.health.HealthIndicator;
import org.springframework.mail.javamail.JavaMailSenderImpl;


public class MailHealthIndicator extends AbstractHealthIndicator {

	private final JavaMailSenderImpl mailSender;

	public MailHealthIndicator(JavaMailSenderImpl mailSender) {
		super("Mail health check failed");
		this.mailSender = mailSender;
	}

	@Override
	protected void doHealthCheck(Builder builder) throws Exception {
		builder.withDetail("location",
				this.mailSender.getHost() + ":" + this.mailSender.getPort());
		this.mailSender.testConnection();
		builder.up();
	}

}
