

package org.elasticsearch.repositories.s3;

import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import com.amazonaws.util.json.Jackson;
import org.elasticsearch.SpecialPermission;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.env.Environment;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.RepositoryPlugin;
import org.elasticsearch.repositories.Repository;


public class S3RepositoryPlugin extends Plugin implements RepositoryPlugin {

    static {
        SpecialPermission.check();
        AccessController.doPrivileged((PrivilegedAction<Void>) () -> {
            try {
                                Jackson.jsonNodeOf("{}");
                                                Class.forName("com.amazonaws.ClientConfiguration");
            } catch (ClassNotFoundException e) {
                throw new RuntimeException(e);
            }
            return null;
        });
    }

    private final Map<String, S3ClientSettings> clientsSettings;

    public S3RepositoryPlugin(Settings settings) {
                clientsSettings = S3ClientSettings.load(settings);
        assert clientsSettings.isEmpty() == false : "always at least have 'default'";
    }

        protected AwsS3Service createStorageService(Settings settings) {
        return new InternalAwsS3Service(settings, clientsSettings);
    }

    @Override
    public Map<String, Repository.Factory> getRepositories(Environment env, NamedXContentRegistry namedXContentRegistry) {
        return Collections.singletonMap(S3Repository.TYPE,
            (metadata) -> new S3Repository(metadata, env.settings(), namedXContentRegistry, createStorageService(env.settings())));
    }

    @Override
    public List<Setting<?>> getSettings() {
        return Arrays.asList(
                        S3ClientSettings.ACCESS_KEY_SETTING,
            S3ClientSettings.SECRET_KEY_SETTING,
            S3ClientSettings.ENDPOINT_SETTING,
            S3ClientSettings.PROTOCOL_SETTING,
            S3ClientSettings.PROXY_HOST_SETTING,
            S3ClientSettings.PROXY_PORT_SETTING,
            S3ClientSettings.PROXY_USERNAME_SETTING,
            S3ClientSettings.PROXY_PASSWORD_SETTING,
            S3ClientSettings.READ_TIMEOUT_SETTING,
            S3ClientSettings.MAX_RETRIES_SETTING,
            S3ClientSettings.USE_THROTTLE_RETRIES_SETTING);
    }
}
