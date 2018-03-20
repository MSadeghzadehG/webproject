
package hudson.cli;

import hudson.remoting.ClassFilter;
import hudson.remoting.ObjectInputStreamEx;
import hudson.remoting.SocketChannelStream;
import org.apache.commons.codec.binary.Base64;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.KeyAgreement;
import javax.crypto.SecretKey;
import javax.crypto.interfaces.DHPublicKey;
import javax.crypto.spec.DHParameterSpec;
import javax.crypto.spec.IvParameterSpec;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.security.AlgorithmParameterGenerator;
import java.security.GeneralSecurityException;
import java.security.Key;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PublicKey;
import java.security.Signature;
import java.security.interfaces.DSAPublicKey;
import java.security.interfaces.RSAPublicKey;
import java.security.spec.X509EncodedKeySpec;
import org.jenkinsci.remoting.util.AnonymousClassWarnings;


public class Connection {
    public final InputStream in;
    public final OutputStream out;

    public final DataInputStream din;
    public final DataOutputStream dout;

    public Connection(Socket socket) throws IOException {
        this(SocketChannelStream.in(socket),SocketChannelStream.out(socket));
    }

    public Connection(InputStream in, OutputStream out) {
        this.in = in;
        this.out = out;
        this.din = new DataInputStream(in);
        this.dout = new DataOutputStream(out);
    }

    public void writeUTF(String msg) throws IOException {
        dout.writeUTF(msg);
    }

    public String readUTF() throws IOException {
        return din.readUTF();
    }

    public void writeBoolean(boolean b) throws IOException {
        dout.writeBoolean(b);
    }

    public boolean readBoolean() throws IOException {
        return din.readBoolean();
    }

    
    public void writeObject(Object o) throws IOException {
        ObjectOutputStream oos = AnonymousClassWarnings.checkingObjectOutputStream(out);
        oos.writeObject(o);
                    }

    
    public <T> T readObject() throws IOException, ClassNotFoundException {
        ObjectInputStream ois = new ObjectInputStreamEx(in,
                getClass().getClassLoader(), ClassFilter.DEFAULT);
        return (T)ois.readObject();
    }

    public void writeKey(Key key) throws IOException {
        writeUTF(new String(Base64.encodeBase64(key.getEncoded())));
    }

    public X509EncodedKeySpec readKey() throws IOException {
        byte[] otherHalf = Base64.decodeBase64(readUTF());         return new X509EncodedKeySpec(otherHalf);
    }

    public void writeByteArray(byte[] data) throws IOException {
        dout.writeInt(data.length);
        dout.write(data);
    }

    public byte[] readByteArray() throws IOException {
        int bufSize = din.readInt();
        if (bufSize < 0) {
            throw new IOException("DataInputStream unexpectedly returned negative integer");
        }
        byte[] buf = new byte[bufSize];
        din.readFully(buf);
        return buf;
    }

    
    public KeyAgreement diffieHellman(boolean side) throws IOException, GeneralSecurityException {
        return diffieHellman(side,512);
    }
    public KeyAgreement diffieHellman(boolean side, int keySize) throws IOException, GeneralSecurityException {
        KeyPair keyPair;
        PublicKey otherHalf;

        if (side) {
            AlgorithmParameterGenerator paramGen = AlgorithmParameterGenerator.getInstance("DH");
            paramGen.init(keySize);

            KeyPairGenerator dh = KeyPairGenerator.getInstance("DH");
            dh.initialize(paramGen.generateParameters().getParameterSpec(DHParameterSpec.class));
            keyPair = dh.generateKeyPair();

                        writeKey(keyPair.getPublic());
            otherHalf = KeyFactory.getInstance("DH").generatePublic(readKey());
        } else {
            otherHalf = KeyFactory.getInstance("DH").generatePublic(readKey());

            KeyPairGenerator keyPairGen = KeyPairGenerator.getInstance("DH");
            keyPairGen.initialize(((DHPublicKey) otherHalf).getParams());
            keyPair = keyPairGen.generateKeyPair();

                        writeKey(keyPair.getPublic());
        }

        KeyAgreement ka = KeyAgreement.getInstance("DH");
        ka.init(keyPair.getPrivate());
        ka.doPhase(otherHalf, true);

        return ka;
    }

    
    public Connection encryptConnection(SecretKey sessionKey, String algorithm) throws IOException, GeneralSecurityException {
        Cipher cout = Cipher.getInstance(algorithm);
        cout.init(Cipher.ENCRYPT_MODE, sessionKey, new IvParameterSpec(sessionKey.getEncoded()));
        CipherOutputStream o = new CipherOutputStream(out, cout);

        Cipher cin = Cipher.getInstance(algorithm);
        cin.init(Cipher.DECRYPT_MODE, sessionKey, new IvParameterSpec(sessionKey.getEncoded()));
        CipherInputStream i = new CipherInputStream(in, cin);

        return new Connection(i,o);
    }

    
    public static byte[] fold(byte[] bytes, int size) {
        byte[] r = new byte[size];
        for (int i=Math.max(bytes.length,size)-1; i>=0; i-- ) {
            r[i%r.length] ^= bytes[i%bytes.length];
        }
        return r;
    }

    private String detectKeyAlgorithm(KeyPair kp) {
        return detectKeyAlgorithm(kp.getPublic());
    }

    private String detectKeyAlgorithm(PublicKey kp) {
        if (kp instanceof RSAPublicKey)     return "RSA";
        if (kp instanceof DSAPublicKey)     return "DSA";
        throw new IllegalArgumentException("Unknown public key type: "+kp);
    }

    
    public void proveIdentity(byte[] sharedSecret, KeyPair key) throws IOException, GeneralSecurityException {
        String algorithm = detectKeyAlgorithm(key);
        writeUTF(algorithm);
        writeKey(key.getPublic());

        Signature sig = Signature.getInstance("SHA1with"+algorithm);
        sig.initSign(key.getPrivate());
        sig.update(key.getPublic().getEncoded());
        sig.update(sharedSecret);
        writeObject(sig.sign());
    }

    
    public PublicKey verifyIdentity(byte[] sharedSecret) throws IOException, GeneralSecurityException {
        try {
            String serverKeyAlgorithm = readUTF();
            PublicKey spk = KeyFactory.getInstance(serverKeyAlgorithm).generatePublic(readKey());

                        Signature sig = Signature.getInstance("SHA1with"+serverKeyAlgorithm);
            sig.initVerify(spk);
            sig.update(spk.getEncoded());
            sig.update(sharedSecret);
            sig.verify((byte[]) readObject());

            return spk;
        } catch (ClassNotFoundException e) {
            throw new Error(e);         }
    }

    public void close() throws IOException {
        in.close();
        out.close();
    }
}
