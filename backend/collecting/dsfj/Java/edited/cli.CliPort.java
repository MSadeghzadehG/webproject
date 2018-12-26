package hudson.cli;

import org.apache.commons.codec.binary.Base64;

import java.net.InetSocketAddress;
import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.spec.X509EncodedKeySpec;

    
public final class CliPort {
    
    final InetSocketAddress endpoint;

    
    final int version;

    
    final String identity;

    public CliPort(InetSocketAddress endpoint, String identity, int version) {
        this.endpoint = endpoint;
        this.identity = identity;
        this.version = version;
    }

    
    public PublicKey getIdentity() throws GeneralSecurityException {
        if (identity==null) return null;
        byte[] image = Base64.decodeBase64(identity);
        return KeyFactory.getInstance("RSA").generatePublic(new X509EncodedKeySpec(image));
    }
}
