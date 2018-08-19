
package org.elasticsearch.xpack.core.ssl;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.Nullable;
import org.elasticsearch.env.Environment;
import org.elasticsearch.xpack.core.ssl.cert.CertificateInfo;

import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509ExtendedTrustManager;

import java.io.IOException;
import java.nio.file.Path;
import java.security.GeneralSecurityException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;


class DefaultJDKTrustConfig extends TrustConfig {

    static final DefaultJDKTrustConfig INSTANCE = new DefaultJDKTrustConfig();

    private DefaultJDKTrustConfig() {
    }

    @Override
    X509ExtendedTrustManager createTrustManager(@Nullable Environment environment) {
        try {
            return CertUtils.trustManager(null, TrustManagerFactory.getDefaultAlgorithm());
        } catch (Exception e) {
            throw new ElasticsearchException("failed to initialize a TrustManagerFactory", e);
        }
    }

    @Override
    
    Collection<CertificateInfo> certificates(Environment environment) throws GeneralSecurityException, IOException {
        return Collections.emptyList();
    }

    @Override
    List<Path> filesToMonitor(@Nullable Environment environment) {
        return Collections.emptyList();
    }

    @Override
    public String toString() {
        return "JDK trusted certs";
    }

    @Override
    public boolean equals(Object o) {
        return o == this;
    }

    @Override
    public int hashCode() {
        return System.identityHashCode(this);
    }

    
    static TrustConfig merge(TrustConfig trustConfig) {
        if (trustConfig == null) {
            return INSTANCE;
        } else {
            return new CombiningTrustConfig(Arrays.asList(INSTANCE, trustConfig));
        }
    }
}
