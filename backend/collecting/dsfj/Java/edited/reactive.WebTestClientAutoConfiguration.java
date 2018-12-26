

package org.springframework.boot.test.autoconfigure.web.reactive;

import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.springframework.beans.factory.ObjectProvider;
import org.springframework.boot.autoconfigure.AutoConfigureAfter;
import org.springframework.boot.autoconfigure.condition.ConditionalOnClass;
import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.boot.autoconfigure.http.codec.CodecsAutoConfiguration;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.boot.web.codec.CodecCustomizer;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.test.web.reactive.server.WebTestClient;
import org.springframework.web.reactive.function.client.WebClient;


@Configuration
@ConditionalOnClass({ WebClient.class, WebTestClient.class })
@AutoConfigureAfter(CodecsAutoConfiguration.class)
@EnableConfigurationProperties
public class WebTestClientAutoConfiguration {

	@Bean
	@ConditionalOnMissingBean
	public WebTestClient webTestClient(ApplicationContext applicationContext,
			List<WebTestClientBuilderCustomizer> customizers) {
		WebTestClient.Builder builder = WebTestClient
				.bindToApplicationContext(applicationContext).configureClient();
		for (WebTestClientBuilderCustomizer customizer : customizers) {
			customizer.customize(builder);
		}
		return builder.build();
	}

	@Bean
	@ConfigurationProperties(prefix = "spring.test.webtestclient")
	public SpringBootWebTestClientBuilderCustomizer springBootWebTestClientBuilderCustomizer(
			ObjectProvider<Collection<CodecCustomizer>> codecCustomizers) {
		return new SpringBootWebTestClientBuilderCustomizer(
				codecCustomizers.getIfAvailable(Collections::emptyList));
	}

}
