

package org.springframework.boot.actuate.autoconfigure.mail;

import java.util.Map;

import org.springframework.beans.factory.ObjectProvider;
import org.springframework.boot.actuate.autoconfigure.health.CompositeHealthIndicatorConfiguration;
import org.springframework.boot.actuate.autoconfigure.health.ConditionalOnEnabledHealthIndicator;
import org.springframework.boot.actuate.autoconfigure.health.HealthIndicatorAutoConfiguration;
import org.springframework.boot.actuate.health.HealthIndicator;
import org.springframework.boot.actuate.mail.MailHealthIndicator;
import org.springframework.boot.autoconfigure.AutoConfigureAfter;
import org.springframework.boot.autoconfigure.AutoConfigureBefore;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.autoconfigure.condition.ConditionalOnBean;
import org.springframework.boot.autoconfigure.condition.ConditionalOnClass;
import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.boot.autoconfigure.mail.MailSenderAutoConfiguration;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.mail.javamail.JavaMailSenderImpl;


@Configuration
@ConditionalOnClass(JavaMailSenderImpl.class)
@ConditionalOnBean(JavaMailSenderImpl.class)
@ConditionalOnEnabledHealthIndicator("mail")
@AutoConfigureBefore(HealthIndicatorAutoConfiguration.class)
@AutoConfigureAfter(MailSenderAutoConfiguration.class)
public class MailHealthIndicatorAutoConfiguration extends
		CompositeHealthIndicatorConfiguration<MailHealthIndicator, JavaMailSenderImpl> {

	private final Map<String, JavaMailSenderImpl> mailSenders;

	public MailHealthIndicatorAutoConfiguration(
			ObjectProvider<Map<String, JavaMailSenderImpl>> mailSenders) {
		this.mailSenders = mailSenders.getIfAvailable();
	}

	@Bean
	@ConditionalOnMissingBean(name = "mailHealthIndicator")
	public HealthIndicator mailHealthIndicator() {
		return createHealthIndicator(this.mailSenders);
	}

}
