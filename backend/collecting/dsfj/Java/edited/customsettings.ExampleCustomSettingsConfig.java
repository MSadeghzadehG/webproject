
package org.elasticsearch.example.customsettings;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.settings.SecureSetting;
import org.elasticsearch.common.settings.SecureString;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Setting.Property;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;

import java.io.IOException;
import java.nio.file.Path;
import java.util.Collections;
import java.util.List;


public class ExampleCustomSettingsConfig {

    
    static final Setting<String> SIMPLE_SETTING = Setting.simpleString("custom.simple", Property.NodeScope);

    
    static final Setting<Boolean> BOOLEAN_SETTING = Setting.boolSetting("custom.bool", false, Property.NodeScope, Property.Dynamic);

    
    static final Setting<String> VALIDATED_SETTING = Setting.simpleString("custom.validated", (value, settings) -> {
        if (value != null && value.contains("forbidden")) {
            throw new IllegalArgumentException("Setting must not contain [forbidden]");
        }
    }, Property.NodeScope, Property.Dynamic);

    
    static final Setting<String> FILTERED_SETTING = Setting.simpleString("custom.filtered", Property.NodeScope, Property.Filtered);

    
    static final Setting<SecureString> SECURED_SETTING = SecureSetting.secureString("custom.secured", null);

    
    static final Setting<List<Integer>> LIST_SETTING =
        Setting.listSetting("custom.list", Collections.emptyList(), Integer::valueOf, Property.NodeScope);


    private final String simple;
    private final String validated;
    private final Boolean bool;
    private final List<Integer> list;
    private final String filtered;

    public ExampleCustomSettingsConfig(final Environment environment) {
                final Path configDir = environment.configFile();

                final Path customSettingsYamlFile = configDir.resolve("custom-settings/custom.yml");

                final Settings customSettings;
        try {
            customSettings = Settings.builder().loadFromPath(customSettingsYamlFile).build();
            assert customSettings != null;
        } catch (IOException e) {
            throw new ElasticsearchException("Failed to load settings", e);
        }

        this.simple = SIMPLE_SETTING.get(customSettings);
        this.bool = BOOLEAN_SETTING.get(customSettings);
        this.validated = VALIDATED_SETTING.get(customSettings);
        this.filtered = FILTERED_SETTING.get(customSettings);
        this.list = LIST_SETTING.get(customSettings);

                final SecureString secured = SECURED_SETTING.get(environment.settings());
        assert secured != null;
    }

    public String getSimple() {
        return simple;
    }

    public Boolean getBool() {
        return bool;
    }

    public String getValidated() {
        return validated;
    }

    public String getFiltered() {
        return filtered;
    }

    public List<Integer> getList() {
        return list;
    }

}
