
package org.elasticsearch.xpack.core.ssl;

import org.bouncycastle.operator.OperatorCreationException;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;

import javax.net.ssl.SSLContext;
import javax.security.auth.DestroyFailedException;

import java.io.IOException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;


public class TestsSSLService extends SSLService {

    public TestsSSLService(Settings settings, Environment environment) throws CertificateException, UnrecoverableKeyException,
            NoSuchAlgorithmException, IOException, DestroyFailedException, KeyStoreException, OperatorCreationException {
        super(settings, environment);
    }

    @Override
    public SSLContext sslContext() {
        return super.sslContext();
    }

    
    public SSLContext sslContext(Settings settings) {
        return sslContextHolder(super.sslConfiguration(settings)).sslContext();
    }
}
