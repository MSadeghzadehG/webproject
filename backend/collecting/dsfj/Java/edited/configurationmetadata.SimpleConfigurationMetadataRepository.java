

package org.springframework.boot.configurationmetadata;

import java.io.Serializable;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("serial")
public class SimpleConfigurationMetadataRepository
		implements ConfigurationMetadataRepository, Serializable {

	private final Map<String, ConfigurationMetadataGroup> allGroups = new HashMap<>();

	@Override
	public Map<String, ConfigurationMetadataGroup> getAllGroups() {
		return Collections.unmodifiableMap(this.allGroups);
	}

	@Override
	public Map<String, ConfigurationMetadataProperty> getAllProperties() {
		Map<String, ConfigurationMetadataProperty> properties = new HashMap<>();
		for (ConfigurationMetadataGroup group : this.allGroups.values()) {
			properties.putAll(group.getProperties());
		}
		return properties;
	}

	
	public void add(Collection<ConfigurationMetadataSource> sources) {
		for (ConfigurationMetadataSource source : sources) {
			String groupId = source.getGroupId();
			ConfigurationMetadataGroup group = this.allGroups.get(groupId);
			if (group == null) {
				group = new ConfigurationMetadataGroup(groupId);
				this.allGroups.put(groupId, group);
			}
			String sourceType = source.getType();
			if (sourceType != null) {
				putIfAbsent(group.getSources(), sourceType, source);
			}
		}
	}

	
	public void add(ConfigurationMetadataProperty property,
			ConfigurationMetadataSource source) {
		if (source != null) {
			putIfAbsent(source.getProperties(), property.getId(), property);
		}
		putIfAbsent(getGroup(source).getProperties(), property.getId(), property);
	}

	
	public void include(ConfigurationMetadataRepository repository) {
		for (ConfigurationMetadataGroup group : repository.getAllGroups().values()) {
			ConfigurationMetadataGroup existingGroup = this.allGroups.get(group.getId());
			if (existingGroup == null) {
				this.allGroups.put(group.getId(), group);
			}
			else {
								for (Map.Entry<String, ConfigurationMetadataProperty> entry : group
						.getProperties().entrySet()) {
					putIfAbsent(existingGroup.getProperties(), entry.getKey(),
							entry.getValue());
				}
								for (Map.Entry<String, ConfigurationMetadataSource> entry : group
						.getSources().entrySet()) {
					putIfAbsent(existingGroup.getSources(), entry.getKey(),
							entry.getValue());
				}
			}
		}

	}

	private ConfigurationMetadataGroup getGroup(ConfigurationMetadataSource source) {
		if (source == null) {
			ConfigurationMetadataGroup rootGroup = this.allGroups.get(ROOT_GROUP);
			if (rootGroup == null) {
				rootGroup = new ConfigurationMetadataGroup(ROOT_GROUP);
				this.allGroups.put(ROOT_GROUP, rootGroup);
			}
			return rootGroup;
		}
		return this.allGroups.get(source.getGroupId());
	}

	private <V> void putIfAbsent(Map<String, V> map, String key, V value) {
		if (!map.containsKey(key)) {
			map.put(key, value);
		}
	}

}
