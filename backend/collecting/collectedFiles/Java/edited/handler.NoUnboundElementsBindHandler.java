

package org.springframework.boot.context.properties.bind.handler;

import java.util.HashSet;
import java.util.Set;
import java.util.TreeSet;
import java.util.function.Function;

import org.springframework.boot.context.properties.bind.AbstractBindHandler;
import org.springframework.boot.context.properties.bind.BindContext;
import org.springframework.boot.context.properties.bind.BindHandler;
import org.springframework.boot.context.properties.bind.Bindable;
import org.springframework.boot.context.properties.bind.UnboundConfigurationPropertiesException;
import org.springframework.boot.context.properties.source.ConfigurationProperty;
import org.springframework.boot.context.properties.source.ConfigurationPropertyName;
import org.springframework.boot.context.properties.source.ConfigurationPropertySource;
import org.springframework.boot.context.properties.source.IterableConfigurationPropertySource;


public class NoUnboundElementsBindHandler extends AbstractBindHandler {

	private final Set<ConfigurationPropertyName> boundNames = new HashSet<>();

	private final Function<ConfigurationPropertySource, Boolean> filter;

	NoUnboundElementsBindHandler() {
		this(BindHandler.DEFAULT, (configurationPropertySource) -> true);
	}

	public NoUnboundElementsBindHandler(BindHandler parent) {
		this(parent, (configurationPropertySource) -> true);
	}

	public NoUnboundElementsBindHandler(BindHandler parent,
			Function<ConfigurationPropertySource, Boolean> filter) {
		super(parent);
		this.filter = filter;
	}

	@Override
	public Object onSuccess(ConfigurationPropertyName name, Bindable<?> target,
			BindContext context, Object result) {
		this.boundNames.add(name);
		return super.onSuccess(name, target, context, result);
	}

	@Override
	public void onFinish(ConfigurationPropertyName name, Bindable<?> target,
			BindContext context, Object result) throws Exception {
		if (context.getDepth() == 0) {
			checkNoUnboundElements(name, context);
		}
	}

	private void checkNoUnboundElements(ConfigurationPropertyName name,
			BindContext context) {
		Set<ConfigurationProperty> unbound = new TreeSet<>();
		for (ConfigurationPropertySource source : context.getSources()) {
			if (source instanceof IterableConfigurationPropertySource
					&& this.filter.apply(source)) {
				collectUnbound(name, unbound,
						(IterableConfigurationPropertySource) source);
			}
		}
		if (!unbound.isEmpty()) {
			throw new UnboundConfigurationPropertiesException(unbound);
		}
	}

	private void collectUnbound(ConfigurationPropertyName name,
			Set<ConfigurationProperty> unbound,
			IterableConfigurationPropertySource source) {
		IterableConfigurationPropertySource filtered = source
				.filter((candidate) -> isUnbound(name, candidate));
		for (ConfigurationPropertyName unboundName : filtered) {
			try {
				unbound.add(source.filter((candidate) -> isUnbound(name, candidate))
						.getConfigurationProperty(unboundName));
			}
			catch (Exception ex) {
			}
		}
	}

	private boolean isUnbound(ConfigurationPropertyName name,
			ConfigurationPropertyName candidate) {
		return name.isAncestorOf(candidate) && !this.boundNames.contains(candidate);
	}

}
