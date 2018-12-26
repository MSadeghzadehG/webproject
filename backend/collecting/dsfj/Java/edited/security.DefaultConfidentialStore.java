package jenkins.security;

import hudson.FilePath;
import hudson.Util;
import hudson.util.Secret;
import hudson.util.TextFile;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import jenkins.model.Jenkins;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.SecretKey;
import java.io.File;
import java.io.IOException;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import javax.crypto.BadPaddingException;
import org.apache.commons.io.IOUtils;


public class DefaultConfidentialStore extends ConfidentialStore {
    private final SecureRandom sr = new SecureRandom();

    
    private final File rootDir;

    
    private final SecretKey masterKey;

    public DefaultConfidentialStore() throws IOException, InterruptedException {
        this(new File(Jenkins.getInstance().getRootDir(),"secrets"));
    }

    public DefaultConfidentialStore(File rootDir) throws IOException, InterruptedException {
        this.rootDir = rootDir;
        if (rootDir.mkdirs()) {
                                    new FilePath(rootDir).chmod(0700);
        }

        TextFile masterSecret = new TextFile(new File(rootDir,"master.key"));
        if (!masterSecret.exists()) {
                                    masterSecret.write(Util.toHexString(randomBytes(128)));
        }
        this.masterKey = Util.toAes128Key(masterSecret.readTrim());
    }

    
    @Override
    protected void store(ConfidentialKey key, byte[] payload) throws IOException {
        try {
            Cipher sym = Secret.getCipher("AES");
            sym.init(Cipher.ENCRYPT_MODE, masterKey);
            try (OutputStream fos = Files.newOutputStream(getFileFor(key).toPath());
                 CipherOutputStream cos = new CipherOutputStream(fos, sym)) {
                cos.write(payload);
                cos.write(MAGIC);
            }
        } catch (GeneralSecurityException e) {
            throw new IOException("Failed to persist the key: "+key.getId(),e);
        } catch (InvalidPathException e) {
            throw new IOException(e);
        }
    }

    
    @Override
    protected byte[] load(ConfidentialKey key) throws IOException {
        try {
            File f = getFileFor(key);
            if (!f.exists())    return null;

            Cipher sym = Secret.getCipher("AES");
            sym.init(Cipher.DECRYPT_MODE, masterKey);
            try (InputStream fis=Files.newInputStream(f.toPath());
                 CipherInputStream cis = new CipherInputStream(fis, sym)) {
                byte[] bytes = IOUtils.toByteArray(cis);
                return verifyMagic(bytes);
            }
        } catch (GeneralSecurityException e) {
            throw new IOException("Failed to load the key: "+key.getId(),e);
        } catch (InvalidPathException e) {
            throw new IOException(e);
        } catch (IOException x) {
            if (x.getCause() instanceof BadPaddingException) {
                return null;             } else {
                throw x;
            }
        }
    }

    
    private byte[] verifyMagic(byte[] payload) {
        int payloadLen = payload.length-MAGIC.length;
        if (payloadLen<0)   return null;    
        for (int i=0; i<MAGIC.length; i++) {
            if (payload[payloadLen+i]!=MAGIC[i])
                return null;            }
        byte[] truncated = new byte[payloadLen];
        System.arraycopy(payload,0,truncated,0,truncated.length);
        return truncated;
    }

    private File getFileFor(ConfidentialKey key) {
        return new File(rootDir, key.getId());
    }

    public byte[] randomBytes(int size) {
        byte[] random = new byte[size];
        sr.nextBytes(random);
        return random;
    }

    private static final byte[] MAGIC = "::::MAGIC::::".getBytes();
}
