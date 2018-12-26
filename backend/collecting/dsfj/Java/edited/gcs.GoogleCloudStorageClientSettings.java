
package org.elasticsearch.repositories.gcs;

import com.google.api.client.googleapis.auth.oauth2.GoogleCredential;
import com.google.api.services.storage.StorageScopes;
import org.elasticsearch.common.settings.SecureSetting;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.TimeValue;

import java.io.IOException;
import java.io.InputStream;
import java.io.UncheckedIOException;
import java.util.Collections;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import static org.elasticsearch.common.settings.Setting.timeSetting;


public class GoogleCloudStorageClientSettings {

    private static final String PREFIX = "gcs.client.";

    
    static final Setting.AffixSetting<InputStream> CREDENTIALS_FILE_SETTING = Setting.affixKeySetting(PREFIX, "credentials_file",
        key -> SecureSetting.secureFile(key, null));

    
    static final Setting.AffixSetting<String> ENDPOINT_SETTING = Setting.affixKeySetting(PREFIX, "endpoint",
        key -> new Setting<>(key, "", s -> s, Setting.Property.NodeScope));

    
    static final Setting.AffixSetting<TimeValue> CONNECT_TIMEOUT_SETTING = Setting.affixKeySetting(PREFIX, "connect_timeout",
        key -> timeSetting(key, TimeValue.ZERO, TimeValue.MINUS_ONE, Setting.Property.NodeScope));

    
    static final Setting.AffixSetting<TimeValue> READ_TIMEOUT_SETTING = Setting.affixKeySetting(PREFIX, "read_timeout",
        key -> timeSetting(key, TimeValue.ZERO, TimeValue.MINUS_ONE, Setting.Property.NodeScope));

    
    static final Setting.AffixSetting<String> APPLICATION_NAME_SETTING = Setting.affixKeySetting(PREFIX, "application_name",
        key -> new Setting<>(key, "repository-gcs", s -> s, Setting.Property.NodeScope));

    
    private final GoogleCredential credential;

    
    private final String endpoint;

    
    private final TimeValue connectTimeout;

    
    private final TimeValue readTimeout;

    
    private final String applicationName;

    GoogleCloudStorageClientSettings(final GoogleCredential credential,
                                     final String endpoint,
                                     final TimeValue connectTimeout,
                                     final TimeValue readTimeout,
                                     final String applicationName) {
        this.credential = credential;
        this.endpoint = endpoint;
        this.connectTimeout = connectTimeout;
        this.readTimeout = readTimeout;
        this.applicationName = applicationName;
    }

    public GoogleCredential getCredential() {
        return credential;
    }

    public String getEndpoint() {
        return endpoint;
    }

    public TimeValue getConnectTimeout() {
        return connectTimeout;
    }

    public TimeValue getReadTimeout() {
        return readTimeout;
    }

    public String getApplicationName() {
        return applicationName;
    }

    public static Map<String, GoogleCloudStorageClientSettings> load(final Settings settings) {
        final Map<String, GoogleCloudStorageClientSettings> clients = new HashMap<>();
        for (String clientName: settings.getGroups(PREFIX).keySet()) {
            clients.put(clientName, getClientSettings(settings, clientName));
        }
        if (clients.containsKey("default") == false) {
                                    clients.put("default", getClientSettings(settings, "default"));
        }
        return Collections.unmodifiableMap(clients);
    }

    static GoogleCloudStorageClientSettings getClientSettings(final Settings settings, final String clientName) {
        return new GoogleCloudStorageClientSettings(
            loadCredential(settings, clientName),
            getConfigValue(settings, clientName, ENDPOINT_SETTING),
            getConfigValue(settings, clientName, CONNECT_TIMEOUT_SETTING),
            getConfigValue(settings, clientName, READ_TIMEOUT_SETTING),
            getConfigValue(settings, clientName, APPLICATION_NAME_SETTING)
        );
    }

    
    static GoogleCredential loadCredential(final Settings settings, final String clientName) {
        try {
            if (CREDENTIALS_FILE_SETTING.getConcreteSettingForNamespace(clientName).exists(settings) == false) {
                                                return null;
            }
            try (InputStream credStream = CREDENTIALS_FILE_SETTING.getConcreteSettingForNamespace(clientName).get(settings)) {
                GoogleCredential credential = GoogleCredential.fromStream(credStream);
                if (credential.createScopedRequired()) {
                    credential = credential.createScoped(Collections.singleton(StorageScopes.DEVSTORAGE_FULL_CONTROL));
                }
                return credential;
            }
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private static <T> T getConfigValue(final Settings settings, final String clientName, final Setting.AffixSetting<T> clientSetting) {
        Setting<T> concreteSetting = clientSetting.getConcreteSettingForNamespace(clientName);
        return concreteSetting.get(settings);
    }
}
