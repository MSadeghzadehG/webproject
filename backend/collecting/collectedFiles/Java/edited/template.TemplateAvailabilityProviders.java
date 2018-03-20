

package org.springframework.boot.autoconfigure.template;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import org.springframework.context.ApplicationContext;
import org.springframework.core.env.Environment;
import org.springframework.core.io.ResourceLoader;
import org.springframework.core.io.support.SpringFactoriesLoader;
import org.springframework.util.Assert;


public class TemplateAvailabilityProviders {

	private final List<TemplateAvailabilityProvider> providers;

	private static final int CACHE_LIMIT = 1024;

	private static final TemplateAvailabilityProvider NONE = new NoTemplateAvailabilityProvider();

	
	private final Map<String, TemplateAvailabilityProvider> resolved = new ConcurrentHashMap<>(
			CACHE_LIMIT);

	
	@SuppressWarnings("serial")
	private final Map<String, TemplateAvailabilityProvider> cache = new LinkedHashMap<String, TemplateAvailabilityProvider>(
			CACHE_LIMIT, 0.75f, true) {

		@Override
		protected boolean removeEldestEntry(
				Map.Entry<String, TemplateAvailabilityProvider> eldest) {
			if (size() > CACHE_LIMIT) {
				TemplateAvailabilityProviders.this.resolved.remove(eldest.getKey());
				return true;
			}
			return false;
		}

	};

	
	public TemplateAvailabilityProviders(ApplicationContext applicationContext) {
		this(applicationContext == null ? null : applicationContext.getClassLoader());
	}

	
	public TemplateAvailabilityProviders(ClassLoader classLoader) {
		Assert.notNull(classLoader, "ClassLoader must not be null");
		this.providers = SpringFactoriesLoader
				.loadFactories(TemplateAvailabilityProvider.class, classLoader);
	}

	
	protected TemplateAvailabilityProviders(
			Collection<? extends TemplateAvailabilityProvider> providers) {
		Assert.notNull(providers, "Providers must not be null");
		this.providers = new ArrayList<>(providers);
	}

	
	public List<TemplateAvailabilityProvider> getProviders() {
		return this.providers;
	}

	
	public TemplateAvailabilityProvider getProvider(String view,
			ApplicationContext applicationContext) {
		Assert.notNull(applicationContext, "ApplicationContext must not be null");
		return getProvider(view, applicationContext.getEnvironment(),
				applicationContext.getClassLoader(), applicationContext);
	}

	
	public TemplateAvailabilityProvider getProvider(String view, Environment environment,
			ClassLoader classLoader, ResourceLoader resourceLoader) {
		Assert.notNull(view, "View must not be null");
		Assert.notNull(environment, "Environment must not be null");
		Assert.notNull(classLoader, "ClassLoader must not be null");
		Assert.notNull(resourceLoader, "ResourceLoader must not be null");
		Boolean useCache = environment.getProperty("spring.template.provider.cache",
				Boolean.class, true);
		if (!useCache) {
			return findProvider(view, environment, classLoader, resourceLoader);
		}
		TemplateAvailabilityProvider provider = this.resolved.get(view);
		if (provider == null) {
			synchronized (this.cache) {
				provider = findProvider(view, environment, classLoader, resourceLoader);
				provider = (provider == null ? NONE : provider);
				this.resolved.put(view, provider);
				this.cache.put(view, provider);
			}
		}
		return (provider == NONE ? null : provider);
	}

	private TemplateAvailabilityProvider findProvider(String view,
			Environment environment, ClassLoader classLoader,
			ResourceLoader resourceLoader) {
		for (TemplateAvailabilityProvider candidate : this.providers) {
			if (candidate.isTemplateAvailable(view, environment, classLoader,
					resourceLoader)) {
				return candidate;
			}
		}
		return null;
	}

	private static class NoTemplateAvailabilityProvider
			implements TemplateAvailabilityProvider {

		@Override
		public boolean isTemplateAvailable(String view, Environment environment,
				ClassLoader classLoader, ResourceLoader resourceLoader) {
			return false;
		}

	}

}
