

package org.elasticsearch.node;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.function.Function;

import org.elasticsearch.Version;
import org.elasticsearch.cluster.ClusterName;
import org.elasticsearch.common.collect.Tuple;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.settings.SettingsException;
import org.elasticsearch.env.Environment;

public class InternalSettingsPreparer {

    private static final String SECRET_PROMPT_VALUE = "${prompt.secret}";
    private static final String TEXT_PROMPT_VALUE = "${prompt.text}";

    
    public static Settings prepareSettings(Settings input) {
        Settings.Builder output = Settings.builder();
        initializeSettings(output, input, Collections.emptyMap());
        finalizeSettings(output);
        return output.build();
    }

    
    public static Environment prepareEnvironment(Settings input) {
        return prepareEnvironment(input, Collections.emptyMap(), null);
    }

    
    public static Environment prepareEnvironment(Settings input, Map<String, String> properties, Path configPath) {
                Settings.Builder output = Settings.builder();
        initializeSettings(output, input, properties);
        Environment environment = new Environment(output.build(), configPath);

        if (Files.exists(environment.configFile().resolve("elasticsearch.yaml"))) {
            throw new SettingsException("elasticsearch.yaml was deprecated in 5.5.0 and must be renamed to elasticsearch.yml");
        }

        if (Files.exists(environment.configFile().resolve("elasticsearch.json"))) {
            throw new SettingsException("elasticsearch.json was deprecated in 5.5.0 and must be converted to elasticsearch.yml");
        }

        output = Settings.builder();         Path path = environment.configFile().resolve("elasticsearch.yml");
        if (Files.exists(path)) {
            try {
                output.loadFromPath(path);
            } catch (IOException e) {
                throw new SettingsException("Failed to load settings from " + path.toString(), e);
            }
        }

                initializeSettings(output, input, properties);
        checkSettingsForTerminalDeprecation(output);
        finalizeSettings(output);

        environment = new Environment(output.build(), configPath);

                output.put(Environment.PATH_LOGS_SETTING.getKey(), environment.logsFile().toAbsolutePath().normalize().toString());
        return new Environment(output.build(), configPath);
    }

    
    static void initializeSettings(final Settings.Builder output, final Settings input, final Map<String, String> esSettings) {
        output.put(input);
        output.putProperties(esSettings, Function.identity());
        output.replacePropertyPlaceholders();
    }

    
    private static void checkSettingsForTerminalDeprecation(final Settings.Builder output) throws SettingsException {
                assert Version.CURRENT.major != 8: "Logic pertaining to config driven prompting should be removed";
        for (String setting : output.keys()) {
            switch (output.get(setting)) {
                case SECRET_PROMPT_VALUE:
                    throw new SettingsException("Config driven secret prompting was deprecated in 6.0.0. Use the keystore" +
                        " for secure settings.");
                case TEXT_PROMPT_VALUE:
                    throw new SettingsException("Config driven text prompting was deprecated in 6.0.0. Use the keystore" +
                        " for secure settings.");
            }
        }
    }

    
    private static void finalizeSettings(Settings.Builder output) {
                List<String> forcedSettings = new ArrayList<>();
        for (String setting : output.keys()) {
            if (setting.startsWith("force.")) {
                forcedSettings.add(setting);
            }
        }
        for (String forcedSetting : forcedSettings) {
            String value = output.remove(forcedSetting);
            output.put(forcedSetting.substring("force.".length()), value);
        }
        output.replacePropertyPlaceholders();

                if (output.get(ClusterName.CLUSTER_NAME_SETTING.getKey()) == null) {
            output.put(ClusterName.CLUSTER_NAME_SETTING.getKey(), ClusterName.CLUSTER_NAME_SETTING.getDefault(Settings.EMPTY).value());
        }
    }
}
