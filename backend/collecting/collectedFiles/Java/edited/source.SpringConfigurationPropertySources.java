

package org.springframework.boot.context.properties.source;

import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Iterator;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.function.Function;

import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.core.env.MutablePropertySources;
import org.springframework.core.env.PropertySource;
import org.springframework.core.env.PropertySource.StubPropertySource;
import org.springframework.util.Assert;
import org.springframework.util.ConcurrentReferenceHashMap;
import org.springframework.util.ConcurrentReferenceHashMap.ReferenceType;


class SpringConfigurationPropertySources
		implements Iterable<ConfigurationPropertySource> {

	private final Iterable<PropertySource<?>> sources;

	private final Map<PropertySource<?>, ConfigurationPropertySource> cache = new ConcurrentReferenceHashMap<>(
			16, ReferenceType.SOFT);

	SpringConfigurationPropertySources(Iterable<PropertySource<?>> sources) {
		Assert.notNull(sources, "Sources must not be null");
		this.sources = sources;
	}

	@Override
	public Iterator<ConfigurationPropertySource> iterator() {
		return new SourcesIterator(this.sources.iterator(), this::adapt);
	}

	private ConfigurationPropertySource adapt(PropertySource<?> source) {
		ConfigurationPropertySource result = this.cache.get(source);
						if (result != null && result.getUnderlyingSource() == source) {
			return result;
		}
		result = SpringConfigurationPropertySource.from(source);
		this.cache.put(source, result);
		return result;
	}

	private static class SourcesIterator
			implements Iterator<ConfigurationPropertySource> {

		private Deque<Iterator<PropertySource<?>>> iterators;

		private ConfigurationPropertySource next;

		private final Function<PropertySource<?>, ConfigurationPropertySource> adapter;

		SourcesIterator(Iterator<PropertySource<?>> iterator,
				Function<PropertySource<?>, ConfigurationPropertySource> adapter) {
			this.iterators = new ArrayDeque<>(4);
			this.iterators.push(iterator);
			this.adapter = adapter;
		}

		@Override
		public boolean hasNext() {
			return fetchNext() != null;
		}

		@Override
		public ConfigurationPropertySource next() {
			ConfigurationPropertySource next = fetchNext();
			this.next = null;
			if (next == null) {
				throw new NoSuchElementException();
			}
			return next;
		}

		private ConfigurationPropertySource fetchNext() {
			if (this.next == null) {
				if (this.iterators.isEmpty()) {
					return null;
				}
				if (!this.iterators.peek().hasNext()) {
					this.iterators.pop();
					return fetchNext();
				}
				PropertySource<?> candidate = this.iterators.peek().next();
				if (candidate.getSource() instanceof ConfigurableEnvironment) {
					push((ConfigurableEnvironment) candidate.getSource());
					return fetchNext();
				}
				if (isIgnored(candidate)) {
					return fetchNext();
				}
				this.next = this.adapter.apply(candidate);
			}
			return this.next;
		}

		private void push(ConfigurableEnvironment environment) {
			this.iterators.push(environment.getPropertySources().iterator());
		}

		private boolean isIgnored(PropertySource<?> candidate) {
			return (candidate instanceof StubPropertySource
					|| candidate instanceof ConfigurationPropertySourcesPropertySource);
		}

	}

}
