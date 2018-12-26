

package org.elasticsearch.plugin.repository.url;

import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.env.Environment;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.RepositoryPlugin;
import org.elasticsearch.repositories.Repository;
import org.elasticsearch.repositories.url.URLRepository;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

public class URLRepositoryPlugin extends Plugin implements RepositoryPlugin {

    @Override
    public List<Setting<?>> getSettings() {
        return Arrays.asList(
            URLRepository.ALLOWED_URLS_SETTING,
            URLRepository.REPOSITORIES_URL_SETTING,
            URLRepository.SUPPORTED_PROTOCOLS_SETTING
        );
    }

    @Override
    public Map<String, Repository.Factory> getRepositories(Environment env, NamedXContentRegistry namedXContentRegistry) {
        return Collections.singletonMap(URLRepository.TYPE, metadata -> new URLRepository(metadata, env, namedXContentRegistry));
    }
}
