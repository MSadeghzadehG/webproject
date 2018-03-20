

package org.elasticsearch.repositories.azure;

import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.env.Environment;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.RepositoryPlugin;
import org.elasticsearch.repositories.Repository;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;


public class AzureRepositoryPlugin extends Plugin implements RepositoryPlugin {

    private final Map<String, AzureStorageSettings> clientsSettings;

        protected AzureStorageService createStorageService(Settings settings) {
        return new AzureStorageServiceImpl(settings, clientsSettings);
    }

    public AzureRepositoryPlugin(Settings settings) {
                clientsSettings = AzureStorageSettings.load(settings);
    }

    @Override
    public Map<String, Repository.Factory> getRepositories(Environment env, NamedXContentRegistry namedXContentRegistry) {
        return Collections.singletonMap(AzureRepository.TYPE,
            (metadata) -> new AzureRepository(metadata, env, namedXContentRegistry, createStorageService(env.settings())));
    }

    @Override
    public List<Setting<?>> getSettings() {
        return Arrays.asList(
            AzureStorageSettings.ACCOUNT_SETTING,
            AzureStorageSettings.KEY_SETTING,
            AzureStorageSettings.ENDPOINT_SUFFIX_SETTING,
            AzureStorageSettings.TIMEOUT_SETTING,
            AzureStorageSettings.PROXY_TYPE_SETTING,
            AzureStorageSettings.PROXY_HOST_SETTING,
            AzureStorageSettings.PROXY_PORT_SETTING
        );
    }
}
