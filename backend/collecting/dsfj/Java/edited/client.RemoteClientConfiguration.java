

package org.springframework.boot.devtools.remote.client;

import java.net.InetSocketAddress;
import java.net.Proxy.Type;
import java.net.URL;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.boot.devtools.autoconfigure.DevToolsProperties;
import org.springframework.boot.devtools.autoconfigure.DevToolsProperties.Restart;
import org.springframework.boot.devtools.autoconfigure.OptionalLiveReloadServer;
import org.springframework.boot.devtools.autoconfigure.RemoteDevToolsProperties;
import org.springframework.boot.devtools.autoconfigure.RemoteDevToolsProperties.Proxy;
import org.springframework.boot.devtools.autoconfigure.TriggerFileFilter;
import org.springframework.boot.devtools.classpath.ClassPathChangedEvent;
import org.springframework.boot.devtools.classpath.ClassPathFileSystemWatcher;
import org.springframework.boot.devtools.classpath.ClassPathRestartStrategy;
import org.springframework.boot.devtools.classpath.PatternClassPathRestartStrategy;
import org.springframework.boot.devtools.filewatch.FileSystemWatcher;
import org.springframework.boot.devtools.filewatch.FileSystemWatcherFactory;
import org.springframework.boot.devtools.livereload.LiveReloadServer;
import org.springframework.boot.devtools.restart.DefaultRestartInitializer;
import org.springframework.boot.devtools.restart.RestartScope;
import org.springframework.boot.devtools.restart.Restarter;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.event.EventListener;
import org.springframework.context.support.PropertySourcesPlaceholderConfigurer;
import org.springframework.http.client.ClientHttpRequestFactory;
import org.springframework.http.client.ClientHttpRequestInterceptor;
import org.springframework.http.client.InterceptingClientHttpRequestFactory;
import org.springframework.http.client.SimpleClientHttpRequestFactory;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


@Configuration
@EnableConfigurationProperties(DevToolsProperties.class)
public class RemoteClientConfiguration implements InitializingBean {

	private static final Log logger = LogFactory.getLog(RemoteClientConfiguration.class);

	private final DevToolsProperties properties;

	@Value("${remoteUrl}")
	private String remoteUrl;

	public RemoteClientConfiguration(DevToolsProperties properties) {
		this.properties = properties;
	}

	@Bean
	public static PropertySourcesPlaceholderConfigurer propertySourcesPlaceholderConfigurer() {
		return new PropertySourcesPlaceholderConfigurer();
	}

	@Bean
	public ClientHttpRequestFactory clientHttpRequestFactory() {
		List<ClientHttpRequestInterceptor> interceptors = Arrays
				.asList(getSecurityInterceptor());
		SimpleClientHttpRequestFactory requestFactory = new SimpleClientHttpRequestFactory();
		Proxy proxy = this.properties.getRemote().getProxy();
		if (proxy.getHost() != null && proxy.getPort() != null) {
			requestFactory.setProxy(new java.net.Proxy(Type.HTTP,
					new InetSocketAddress(proxy.getHost(), proxy.getPort())));
		}
		return new InterceptingClientHttpRequestFactory(requestFactory, interceptors);
	}

	private ClientHttpRequestInterceptor getSecurityInterceptor() {
		RemoteDevToolsProperties remoteProperties = this.properties.getRemote();
		String secretHeaderName = remoteProperties.getSecretHeaderName();
		String secret = remoteProperties.getSecret();
		Assert.state(secret != null,
				"The environment value 'spring.devtools.remote.secret' "
						+ "is required to secure your connection.");
		return new HttpHeaderInterceptor(secretHeaderName, secret);
	}

	@Override
	public void afterPropertiesSet() {
		logWarnings();
	}

	private void logWarnings() {
		RemoteDevToolsProperties remoteProperties = this.properties.getRemote();
		if (!remoteProperties.getRestart().isEnabled()) {
			logger.warn("Remote restart is disabled.");
		}
		if (!this.remoteUrl.startsWith("https:			logger.warn("The connection to " + this.remoteUrl
					+ " is insecure. You should use a URL starting with 'https:		}
	}

	
	@ConditionalOnProperty(prefix = "spring.devtools.livereload", name = "enabled", matchIfMissing = true)
	static class LiveReloadConfiguration {

		@Autowired
		private DevToolsProperties properties;

		@Autowired(required = false)
		private LiveReloadServer liveReloadServer;

		@Autowired
		private ClientHttpRequestFactory clientHttpRequestFactory;

		@Value("${remoteUrl}")
		private String remoteUrl;

		private ExecutorService executor = Executors.newSingleThreadExecutor();

		@Bean
		@RestartScope
		@ConditionalOnMissingBean
		public LiveReloadServer liveReloadServer() {
			return new LiveReloadServer(this.properties.getLivereload().getPort(),
					Restarter.getInstance().getThreadFactory());
		}

		@EventListener
		public void onClassPathChanged(ClassPathChangedEvent event) {
			String url = this.remoteUrl + this.properties.getRemote().getContextPath();
			this.executor.execute(new DelayedLiveReloadTrigger(optionalLiveReloadServer(),
					this.clientHttpRequestFactory, url));
		}

		@Bean
		public OptionalLiveReloadServer optionalLiveReloadServer() {
			return new OptionalLiveReloadServer(this.liveReloadServer);
		}

		final ExecutorService getExecutor() {
			return this.executor;
		}

	}

	
	@ConditionalOnProperty(prefix = "spring.devtools.remote.restart", name = "enabled", matchIfMissing = true)
	static class RemoteRestartClientConfiguration {

		@Autowired
		private DevToolsProperties properties;

		@Value("${remoteUrl}")
		private String remoteUrl;

		@Bean
		public ClassPathFileSystemWatcher classPathFileSystemWatcher() {
			DefaultRestartInitializer restartInitializer = new DefaultRestartInitializer();
			URL[] urls = restartInitializer.getInitialUrls(Thread.currentThread());
			if (urls == null) {
				urls = new URL[0];
			}
			return new ClassPathFileSystemWatcher(getFileSystemWatcherFactory(),
					classPathRestartStrategy(), urls);
		}

		@Bean
		public FileSystemWatcherFactory getFileSystemWatcherFactory() {
			return this::newFileSystemWatcher;
		}

		private FileSystemWatcher newFileSystemWatcher() {
			Restart restartProperties = this.properties.getRestart();
			FileSystemWatcher watcher = new FileSystemWatcher(true,
					restartProperties.getPollInterval(),
					restartProperties.getQuietPeriod());
			String triggerFile = restartProperties.getTriggerFile();
			if (StringUtils.hasLength(triggerFile)) {
				watcher.setTriggerFilter(new TriggerFileFilter(triggerFile));
			}
			return watcher;
		}

		@Bean
		public ClassPathRestartStrategy classPathRestartStrategy() {
			return new PatternClassPathRestartStrategy(
					this.properties.getRestart().getAllExclude());
		}

		@Bean
		public ClassPathChangeUploader classPathChangeUploader(
				ClientHttpRequestFactory requestFactory) {
			String url = this.remoteUrl + this.properties.getRemote().getContextPath()
					+ "/restart";
			return new ClassPathChangeUploader(url, requestFactory);
		}

	}

}
