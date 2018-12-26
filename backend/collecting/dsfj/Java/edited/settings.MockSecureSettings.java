

package org.elasticsearch.common.settings;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicBoolean;


public class MockSecureSettings implements SecureSettings {

    private Map<String, SecureString> secureStrings = new HashMap<>();
    private Map<String, byte[]> files = new HashMap<>();
    private Set<String> settingNames = new HashSet<>();
    private final AtomicBoolean closed = new AtomicBoolean(false);

    public MockSecureSettings() {
    }

    private MockSecureSettings(MockSecureSettings source) {
        secureStrings.putAll(source.secureStrings);
        files.putAll(source.files);
        settingNames.addAll(source.settingNames);
    }

    @Override
    public boolean isLoaded() {
        return true;
    }

    @Override
    public Set<String> getSettingNames() {
        return settingNames;
    }

    @Override
    public SecureString getString(String setting) {
        ensureOpen();
        return secureStrings.get(setting);
    }

    @Override
    public InputStream getFile(String setting) {
        ensureOpen();
        return new ByteArrayInputStream(files.get(setting));
    }

    public void setString(String setting, String value) {
        ensureOpen();
        secureStrings.put(setting, new SecureString(value.toCharArray()));
        settingNames.add(setting);
    }

    public void setFile(String setting, byte[] value) {
        ensureOpen();
        files.put(setting, value);
        settingNames.add(setting);
    }

    
    public void merge(MockSecureSettings secureSettings) {
        for (String setting : secureSettings.getSettingNames()) {
            if (settingNames.contains(setting)) {
                throw new IllegalArgumentException("Cannot overwrite existing secure setting " + setting);
            }
        }
        settingNames.addAll(secureSettings.settingNames);
        secureStrings.putAll(secureSettings.secureStrings);
        files.putAll(secureSettings.files);
    }

    @Override
    public void close() throws IOException {
        closed.set(true);
    }

    private void ensureOpen() {
        if (closed.get()) {
            throw new IllegalStateException("secure settings are already closed");
        }
    }

    public SecureSettings clone() {
        ensureOpen();
        return new MockSecureSettings(this);
    }
}
