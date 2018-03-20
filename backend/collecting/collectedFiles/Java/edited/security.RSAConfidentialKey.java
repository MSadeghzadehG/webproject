
package jenkins.security;

import org.apache.commons.codec.binary.Base64;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.SecureRandom;
import java.security.interfaces.RSAPrivateCrtKey;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.RSAPublicKeySpec;


public abstract class RSAConfidentialKey extends ConfidentialKey {
    private RSAPrivateKey priv;
    private RSAPublicKey pub;

    public RSAConfidentialKey(String id) {
        super(id);
    }

    public RSAConfidentialKey(Class owner, String shortName) {
        this(owner.getName() + '.' + shortName);
    }

    
    protected synchronized RSAPrivateKey getPrivateKey() {
        try {
            if (priv == null) {
                byte[] payload = load();
                if (payload == null) {
                    KeyPairGenerator gen = KeyPairGenerator.getInstance("RSA");
                    gen.initialize(2048, new SecureRandom());                     KeyPair keys = gen.generateKeyPair();
                    priv = (RSAPrivateKey) keys.getPrivate();
                    pub = (RSAPublicKey) keys.getPublic();
                    store(priv.getEncoded());
                } else {
                    KeyFactory keyFactory = KeyFactory.getInstance("RSA");
                    priv = (RSAPrivateKey) keyFactory.generatePrivate(new PKCS8EncodedKeySpec(payload));

                    RSAPrivateCrtKey pks = (RSAPrivateCrtKey) priv;
                    pub = (RSAPublicKey) keyFactory.generatePublic(
                            new RSAPublicKeySpec(pks.getModulus(), pks.getPublicExponent()));
                }
            }
            return priv;
        } catch (IOException e) {
            throw new Error("Failed to load the key: " + getId(), e);
        } catch (GeneralSecurityException e) {
            throw new Error("Failed to load the key: " + getId(), e);
        }
    }

    public RSAPublicKey getPublicKey() {
        getPrivateKey();
        return pub;
    }

    
    public String getEncodedPublicKey() {
        return new String(Base64.encodeBase64(getPublicKey().getEncoded()));
    }
}
