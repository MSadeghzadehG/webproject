
package jenkins.security;

import java.io.UnsupportedEncodingException;
import java.security.GeneralSecurityException;
import java.security.Signature;
import java.security.interfaces.RSAPrivateKey;


public class RSADigitalSignatureConfidentialKey extends RSAConfidentialKey {
    public RSADigitalSignatureConfidentialKey(String id) {
        super(id);
    }

    public RSADigitalSignatureConfidentialKey(Class owner, String shortName) {
        super(owner, shortName);
    }

    
    public String sign(String msg) {
        try {
            RSAPrivateKey key = getPrivateKey();
            Signature sig = Signature.getInstance(SIGNING_ALGORITHM + "with" + key.getAlgorithm());
            sig.initSign(key);
            sig.update(msg.getBytes("UTF-8"));
            return hudson.remoting.Base64.encode(sig.sign());
        } catch (GeneralSecurityException e) {
            throw new SecurityException(e);
        } catch (UnsupportedEncodingException e) {
            throw new AssertionError(e);            }
    }

    static final String SIGNING_ALGORITHM = "SHA256";
}
