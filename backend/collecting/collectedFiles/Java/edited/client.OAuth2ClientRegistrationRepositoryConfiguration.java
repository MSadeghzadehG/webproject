

package org.springframework.boot.autoconfigure.security.oauth2.client;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import org.springframework.boot.autoconfigure.condition.ConditionMessage;
import org.springframework.boot.autoconfigure.condition.ConditionOutcome;
import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.boot.autoconfigure.condition.SpringBootCondition;
import org.springframework.boot.autoconfigure.security.oauth2.client.OAuth2ClientProperties.Registration;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.boot.context.properties.bind.Bindable;
import org.springframework.boot.context.properties.bind.Binder;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.ConditionContext;
import org.springframework.context.annotation.Conditional;
import org.springframework.context.annotation.Configuration;
import org.springframework.core.env.Environment;
import org.springframework.core.type.AnnotatedTypeMetadata;
import org.springframework.security.oauth2.client.registration.ClientRegistration;
import org.springframework.security.oauth2.client.registration.ClientRegistrationRepository;
import org.springframework.security.oauth2.client.registration.InMemoryClientRegistrationRepository;


@Configuration
@EnableConfigurationProperties(OAuth2ClientProperties.class)
@Conditional(OAuth2ClientRegistrationRepositoryConfiguration.ClientsConfiguredCondition.class)
class OAuth2ClientRegistrationRepositoryConfiguration {

	private final OAuth2ClientProperties properties;

	OAuth2ClientRegistrationRepositoryConfiguration(OAuth2ClientProperties properties) {
		this.properties = properties;
	}

	@Bean
	@ConditionalOnMissingBean(ClientRegistrationRepository.class)
	public InMemoryClientRegistrationRepository clientRegistrationRepository() {
		List<ClientRegistration> registrations = new ArrayList<>(
				OAuth2ClientPropertiesRegistrationAdapter
						.getClientRegistrations(this.properties).values());
		return new InMemoryClientRegistrationRepository(registrations);
	}

	
	static class ClientsConfiguredCondition extends SpringBootCondition {

		private static final Bindable<Map<String, Registration>> BINDABLE_REGISTRATION = Bindable
				.mapOf(String.class, OAuth2ClientProperties.Registration.class);

		@Override
		public ConditionOutcome getMatchOutcome(ConditionContext context,
				AnnotatedTypeMetadata metadata) {
			ConditionMessage.Builder message = ConditionMessage
					.forCondition("OAuth2 Clients Configured Condition");
			Map<String, Registration> registrations = this
					.getRegistrations(context.getEnvironment());
			if (!registrations.isEmpty()) {
				return ConditionOutcome.match(message.foundExactly(
						"registered clients " + registrations.values().stream()
								.map(OAuth2ClientProperties.Registration::getClientId)
								.collect(Collectors.joining(", "))));
			}
			return ConditionOutcome.noMatch(message.notAvailable("registered clients"));
		}

		private Map<String, Registration> getRegistrations(Environment environment) {
			return Binder.get(environment)
					.bind("spring.security.oauth2.client.registration",
							BINDABLE_REGISTRATION)
					.orElse(Collections.emptyMap());
		}

	}

}
