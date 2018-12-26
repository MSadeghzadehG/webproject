

package org.springframework.boot.devtools;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.Collections;
import java.util.Map;

import org.springframework.boot.context.event.ApplicationEnvironmentPreparedEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.core.Ordered;
import org.springframework.core.env.CommandLinePropertySource;
import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.core.env.MapPropertySource;
import org.springframework.core.env.PropertySource;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


class RemoteUrlPropertyExtractor
		implements ApplicationListener<ApplicationEnvironmentPreparedEvent>, Ordered {

	private static final String NON_OPTION_ARGS = CommandLinePropertySource.DEFAULT_NON_OPTION_ARGS_PROPERTY_NAME;

	@Override
	public void onApplicationEvent(ApplicationEnvironmentPreparedEvent event) {
		ConfigurableEnvironment environment = event.getEnvironment();
		String url = cleanRemoteUrl(environment.getProperty(NON_OPTION_ARGS));
		Assert.state(StringUtils.hasLength(url), "No remote URL specified");
		Assert.state(url.indexOf(',') == -1, "Multiple URLs specified");
		try {
			new URI(url);
		}
		catch (URISyntaxException ex) {
			throw new IllegalStateException("Malformed URL '" + url + "'");
		}
		Map<String, Object> source = Collections.singletonMap("remoteUrl", (Object) url);
		PropertySource<?> propertySource = new MapPropertySource("remoteUrl", source);
		environment.getPropertySources().addLast(propertySource);
	}

	private String cleanRemoteUrl(String url) {
		if (StringUtils.hasText(url) && url.endsWith("/")) {
			return url.substring(0, url.length() - 1);
		}
		return url;
	}

	@Override
	public int getOrder() {
		return Ordered.HIGHEST_PRECEDENCE;
	}

}
