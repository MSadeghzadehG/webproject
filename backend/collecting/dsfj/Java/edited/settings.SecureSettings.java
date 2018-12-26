

package org.elasticsearch.common.settings;

import java.io.Closeable;
import java.io.IOException;
import java.io.InputStream;
import java.security.GeneralSecurityException;
import java.util.Set;


public interface SecureSettings extends Closeable {

    
    boolean isLoaded();

    
    Set<String> getSettingNames();

    
    SecureString getString(String setting) throws GeneralSecurityException;

    
    InputStream getFile(String setting) throws GeneralSecurityException;

    @Override
    void close() throws IOException;
}
