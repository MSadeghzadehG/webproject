

package com.alibaba.com.caucho.hessian.security;

import com.alibaba.com.caucho.hessian.io.Hessian2Input;
import com.alibaba.com.caucho.hessian.io.Hessian2Output;
import com.alibaba.com.caucho.hessian.io.HessianEnvelope;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.Key;
import java.security.MessageDigest;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;

public class X509Encryption extends HessianEnvelope {
    private String _algorithm = "AES";

        private X509Certificate _cert;

        private PrivateKey _privateKey;

    private SecureRandom _secureRandom;

    public X509Encryption() {
    }

    
    public String getAlgorithm() {
        return _algorithm;
    }

    
    public void setAlgorithm(String algorithm) {
        if (algorithm == null)
            throw new NullPointerException();

        _algorithm = algorithm;
    }

    
    public X509Certificate getCertificate() {
        return _cert;
    }

    
    public void setCertificate(X509Certificate cert) {
        _cert = cert;
    }

    
    public PrivateKey getPrivateKey() {
        return _privateKey;
    }

    
    public void setPrivateKey(PrivateKey privateKey) {
        _privateKey = privateKey;
    }

    
    public SecureRandom getSecureRandom() {
        return _secureRandom;
    }

    
    public void setSecureRandom(SecureRandom random) {
        _secureRandom = random;
    }

    public Hessian2Output wrap(Hessian2Output out)
            throws IOException {
        if (_cert == null)
            throw new IOException("X509Encryption.wrap requires a certificate");

        OutputStream os = new EncryptOutputStream(out);

        Hessian2Output filterOut = new Hessian2Output(os);

        filterOut.setCloseStreamOnClose(true);

        return filterOut;
    }

    public Hessian2Input unwrap(Hessian2Input in)
            throws IOException {
        if (_privateKey == null)
            throw new IOException("X509Encryption.unwrap requires a private key");

        if (_cert == null)
            throw new IOException("X509Encryption.unwrap requires a certificate");

        int version = in.readEnvelope();

        String method = in.readMethod();

        if (!method.equals(getClass().getName()))
            throw new IOException("expected hessian Envelope method '" +
                    getClass().getName() + "' at '" + method + "'");

        return unwrapHeaders(in);
    }

    public Hessian2Input unwrapHeaders(Hessian2Input in)
            throws IOException {
        if (_privateKey == null)
            throw new IOException("X509Encryption.unwrap requires a private key");

        if (_cert == null)
            throw new IOException("X509Encryption.unwrap requires a certificate");

        InputStream is = new EncryptInputStream(in);

        Hessian2Input filter = new Hessian2Input(is);

        filter.setCloseStreamOnClose(true);

        return filter;
    }

    class EncryptOutputStream extends OutputStream {
        private Hessian2Output _out;

        private Cipher _cipher;
        private OutputStream _bodyOut;
        private CipherOutputStream _cipherOut;

        EncryptOutputStream(Hessian2Output out)
                throws IOException {
            try {
                _out = out;

                KeyGenerator keyGen = KeyGenerator.getInstance(_algorithm);

                if (_secureRandom != null)
                    keyGen.init(_secureRandom);

                SecretKey sharedKey = keyGen.generateKey();

                _out = out;

                _out.startEnvelope(X509Encryption.class.getName());

                PublicKey publicKey = _cert.getPublicKey();

                byte[] encoded = publicKey.getEncoded();
                MessageDigest md = MessageDigest.getInstance("SHA1");
                md.update(encoded);
                byte[] fingerprint = md.digest();

                String keyAlgorithm = publicKey.getAlgorithm();
                Cipher keyCipher = Cipher.getInstance(keyAlgorithm);
                if (_secureRandom != null)
                    keyCipher.init(Cipher.WRAP_MODE, _cert, _secureRandom);
                else
                    keyCipher.init(Cipher.WRAP_MODE, _cert);

                byte[] encKey = keyCipher.wrap(sharedKey);

                _out.writeInt(4);

                _out.writeString("algorithm");
                _out.writeString(_algorithm);
                _out.writeString("fingerprint");
                _out.writeBytes(fingerprint);
                _out.writeString("key-algorithm");
                _out.writeString(keyAlgorithm);
                _out.writeString("key");
                _out.writeBytes(encKey);

                _bodyOut = _out.getBytesOutputStream();

                _cipher = Cipher.getInstance(_algorithm);
                if (_secureRandom != null)
                    _cipher.init(Cipher.ENCRYPT_MODE, sharedKey, _secureRandom);
                else
                    _cipher.init(Cipher.ENCRYPT_MODE, sharedKey);

                _cipherOut = new CipherOutputStream(_bodyOut, _cipher);
            } catch (RuntimeException e) {
                throw e;
            } catch (IOException e) {
                throw e;
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        public void write(int ch)
                throws IOException {
            _cipherOut.write(ch);
        }

        public void write(byte[] buffer, int offset, int length)
                throws IOException {
            _cipherOut.write(buffer, offset, length);
        }

        public void close()
                throws IOException {
            Hessian2Output out = _out;
            _out = null;

            if (out != null) {
                _cipherOut.close();
                _bodyOut.close();

                out.writeInt(0);
                out.completeEnvelope();
                out.close();
            }
        }
    }

    class EncryptInputStream extends InputStream {
        private Hessian2Input _in;

        private Cipher _cipher;
        private InputStream _bodyIn;
        private CipherInputStream _cipherIn;

        EncryptInputStream(Hessian2Input in)
                throws IOException {
            try {
                _in = in;

                byte[] fingerprint = null;
                String keyAlgorithm = null;
                String algorithm = null;
                byte[] encKey = null;

                int len = in.readInt();

                for (int i = 0; i < len; i++) {
                    String header = in.readString();

                    if ("fingerprint".equals(header))
                        fingerprint = in.readBytes();
                    else if ("key-algorithm".equals(header))
                        keyAlgorithm = in.readString();
                    else if ("algorithm".equals(header))
                        algorithm = in.readString();
                    else if ("key".equals(header))
                        encKey = in.readBytes();
                    else
                        throw new IOException("'" + header + "' is an unexpected header");
                }

                Cipher keyCipher = Cipher.getInstance(keyAlgorithm);
                keyCipher.init(Cipher.UNWRAP_MODE, _privateKey);

                Key key = keyCipher.unwrap(encKey, algorithm, Cipher.SECRET_KEY);
                _bodyIn = _in.readInputStream();

                _cipher = Cipher.getInstance(algorithm);
                _cipher.init(Cipher.DECRYPT_MODE, key);

                _cipherIn = new CipherInputStream(_bodyIn, _cipher);
            } catch (RuntimeException e) {
                throw e;
            } catch (IOException e) {
                throw e;
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        public int read()
                throws IOException {
            return _cipherIn.read();
        }

        public int read(byte[] buffer, int offset, int length)
                throws IOException {
            return _cipherIn.read(buffer, offset, length);
        }

        public void close()
                throws IOException {
            Hessian2Input in = _in;
            _in = null;

            if (in != null) {
                _cipherIn.close();
                _bodyIn.close();

                int len = in.readInt();

                if (len != 0)
                    throw new IOException("Unexpected footer");

                in.completeEnvelope();

                in.close();
            }
        }
    }
}
