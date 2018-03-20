

package org.elasticsearch.common.settings;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.AccessDeniedException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.GeneralSecurityException;
import java.security.KeyStore;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.regex.Pattern;

import org.apache.lucene.codecs.CodecUtil;
import org.apache.lucene.store.BufferedChecksumIndexInput;
import org.apache.lucene.store.ChecksumIndexInput;
import org.apache.lucene.store.IOContext;
import org.apache.lucene.store.IndexInput;
import org.apache.lucene.store.IndexOutput;
import org.apache.lucene.store.SimpleFSDirectory;
import org.apache.lucene.util.SetOnce;
import org.elasticsearch.cli.ExitCodes;
import org.elasticsearch.cli.UserException;
import org.elasticsearch.common.Randomness;


public class KeyStoreWrapper implements SecureSettings {

    
    private enum EntryType {
        STRING,
        FILE
    }

    
    private static class Entry {
        final EntryType type;
        final byte[] bytes;

        Entry(EntryType type, byte[] bytes) {
            this.type = type;
            this.bytes = bytes;
        }
    }

    
    private static final Pattern ALLOWED_SETTING_NAME = Pattern.compile("[a-z0-9_\\-.]+");

    public static final Setting<SecureString> SEED_SETTING = SecureSetting.secureString("keystore.seed", null);

    
    private static final char[] SEED_CHARS = ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" +
        "~!@#$%^&*-_=+?").toCharArray();

    
    private static final String KEYSTORE_FILENAME = "elasticsearch.keystore";

    
    private static final int FORMAT_VERSION = 3;

    
    private static final int MIN_FORMAT_VERSION = 1;

    
    private static final String KDF_ALGO = "PBKDF2WithHmacSHA512";

    
    private static final int KDF_ITERS = 10000;

    
    private static final int CIPHER_KEY_BITS = 128;

    
    private static final int GCM_TAG_BITS = 128;

    
    private static final String CIPHER_ALGO = "AES";

    
    private static final String CIPHER_MODE = "GCM";

    
    private static final String CIPHER_PADDING = "NoPadding";

                
    
    private final int formatVersion;

    
    private final boolean hasPassword;

    
    private final byte[] dataBytes;

    
    private final SetOnce<Map<String, Entry>> entries = new SetOnce<>();

    private KeyStoreWrapper(int formatVersion, boolean hasPassword, byte[] dataBytes) {
        this.formatVersion = formatVersion;
        this.hasPassword = hasPassword;
        this.dataBytes = dataBytes;
    }

    
    public static Path keystorePath(Path configDir) {
        return configDir.resolve(KEYSTORE_FILENAME);
    }

    
    public static KeyStoreWrapper create() {
        KeyStoreWrapper wrapper = new KeyStoreWrapper(FORMAT_VERSION, false, null);
        wrapper.entries.set(new HashMap<>());
        addBootstrapSeed(wrapper);
        return wrapper;
    }

    
    public static void addBootstrapSeed(KeyStoreWrapper wrapper) {
        assert wrapper.getSettingNames().contains(SEED_SETTING.getKey()) == false;
        SecureRandom random = Randomness.createSecure();
        int passwordLength = 20;         char[] characters = new char[passwordLength];
        for (int i = 0; i < passwordLength; ++i) {
            characters[i] = SEED_CHARS[random.nextInt(SEED_CHARS.length)];
        }
        wrapper.setString(SEED_SETTING.getKey(), characters);
        Arrays.fill(characters, (char)0);
    }

    
    public static KeyStoreWrapper load(Path configDir) throws IOException {
        Path keystoreFile = keystorePath(configDir);
        if (Files.exists(keystoreFile) == false) {
            return null;
        }

        SimpleFSDirectory directory = new SimpleFSDirectory(configDir);
        try (IndexInput indexInput = directory.openInput(KEYSTORE_FILENAME, IOContext.READONCE)) {
            ChecksumIndexInput input = new BufferedChecksumIndexInput(indexInput);
            int formatVersion = CodecUtil.checkHeader(input, KEYSTORE_FILENAME, MIN_FORMAT_VERSION, FORMAT_VERSION);
            byte hasPasswordByte = input.readByte();
            boolean hasPassword = hasPasswordByte == 1;
            if (hasPassword == false && hasPasswordByte != 0) {
                throw new IllegalStateException("hasPassword boolean is corrupt: "
                    + String.format(Locale.ROOT, "%02x", hasPasswordByte));
            }

            if (formatVersion <= 2) {
                String type = input.readString();
                if (type.equals("PKCS12") == false) {
                    throw new IllegalStateException("Corrupted legacy keystore string encryption algorithm");
                }

                final String stringKeyAlgo = input.readString();
                if (stringKeyAlgo.equals("PBE") == false) {
                    throw new IllegalStateException("Corrupted legacy keystore string encryption algorithm");
                }
                if (formatVersion == 2) {
                    final String fileKeyAlgo = input.readString();
                    if (fileKeyAlgo.equals("PBE") == false) {
                        throw new IllegalStateException("Corrupted legacy keystore file encryption algorithm");
                    }
                }
            }

            final byte[] dataBytes;
            if (formatVersion == 2) {
                                                                                Map<String, String> settingTypes = input.readMapOfStrings();
                ByteArrayOutputStream bytes = new ByteArrayOutputStream();
                try (DataOutputStream output = new DataOutputStream(bytes)) {
                    output.writeInt(settingTypes.size());
                    for (Map.Entry<String, String> entry : settingTypes.entrySet()) {
                        output.writeUTF(entry.getKey());
                        output.writeUTF(entry.getValue());
                    }
                    int keystoreLen = input.readInt();
                    byte[] keystoreBytes = new byte[keystoreLen];
                    input.readBytes(keystoreBytes, 0, keystoreLen);
                    output.write(keystoreBytes);
                }
                dataBytes = bytes.toByteArray();
            } else {
                int dataBytesLen = input.readInt();
                dataBytes = new byte[dataBytesLen];
                input.readBytes(dataBytes, 0, dataBytesLen);
            }

            CodecUtil.checkFooter(input);
            return new KeyStoreWrapper(formatVersion, hasPassword, dataBytes);
        }
    }

    
    public static void upgrade(KeyStoreWrapper wrapper, Path configDir, char[] password) throws Exception {
                if (wrapper.getSettingNames().contains(SEED_SETTING.getKey())) {
            return;
        }
        addBootstrapSeed(wrapper);
        wrapper.save(configDir, password);
    }

    @Override
    public boolean isLoaded() {
        return entries.get() != null;
    }

    
    public boolean hasPassword() {
        return hasPassword;
    }

    private Cipher createCipher(int opmode, char[] password, byte[] salt, byte[] iv) throws GeneralSecurityException {
        PBEKeySpec keySpec = new PBEKeySpec(password, salt, KDF_ITERS, CIPHER_KEY_BITS);
        SecretKeyFactory keyFactory = SecretKeyFactory.getInstance(KDF_ALGO);
        SecretKey secretKey = keyFactory.generateSecret(keySpec);
        SecretKeySpec secret = new SecretKeySpec(secretKey.getEncoded(), CIPHER_ALGO);

        GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_BITS, iv);
        Cipher cipher = Cipher.getInstance(CIPHER_ALGO + "/" + CIPHER_MODE + "/" + CIPHER_PADDING);
        cipher.init(opmode, secret, spec);
        cipher.updateAAD(salt);
        return cipher;
    }

    
    public void decrypt(char[] password) throws GeneralSecurityException, IOException {
        if (entries.get() != null) {
            throw new IllegalStateException("Keystore has already been decrypted");
        }
        if (formatVersion <= 2) {
            decryptLegacyEntries();
            assert password.length == 0;
            return;
        }

        final byte[] salt;
        final byte[] iv;
        final byte[] encryptedBytes;
        try (ByteArrayInputStream bytesStream = new ByteArrayInputStream(dataBytes);
             DataInputStream input = new DataInputStream(bytesStream)) {
            int saltLen = input.readInt();
            salt = new byte[saltLen];
            if (input.read(salt) != saltLen) {
                throw new SecurityException("Keystore has been corrupted or tampered with");
            }
            int ivLen = input.readInt();
            iv = new byte[ivLen];
            if (input.read(iv) != ivLen) {
                throw new SecurityException("Keystore has been corrupted or tampered with");
            }
            int encryptedLen = input.readInt();
            encryptedBytes = new byte[encryptedLen];
            if (input.read(encryptedBytes) != encryptedLen) {
                throw new SecurityException("Keystore has been corrupted or tampered with");
            }
        }

        Cipher cipher = createCipher(Cipher.DECRYPT_MODE, password, salt, iv);
        try (ByteArrayInputStream bytesStream = new ByteArrayInputStream(encryptedBytes);
             CipherInputStream cipherStream = new CipherInputStream(bytesStream, cipher);
             DataInputStream input = new DataInputStream(cipherStream)) {

            entries.set(new HashMap<>());
            int numEntries = input.readInt();
            while (numEntries-- > 0) {
                String setting = input.readUTF();
                EntryType entryType = EntryType.valueOf(input.readUTF());
                int entrySize = input.readInt();
                byte[] entryBytes = new byte[entrySize];
                if (input.read(entryBytes) != entrySize) {
                    throw new SecurityException("Keystore has been corrupted or tampered with");
                }
                entries.get().put(setting, new Entry(entryType, entryBytes));
            }
        }
    }

    
    private byte[] encrypt(char[] password, byte[] salt, byte[] iv) throws GeneralSecurityException, IOException {
        assert isLoaded();

        ByteArrayOutputStream bytes = new ByteArrayOutputStream();
        Cipher cipher = createCipher(Cipher.ENCRYPT_MODE, password, salt, iv);
        try (CipherOutputStream cipherStream = new CipherOutputStream(bytes, cipher);
             DataOutputStream output = new DataOutputStream(cipherStream)) {

            output.writeInt(entries.get().size());
            for (Map.Entry<String, Entry> mapEntry : entries.get().entrySet()) {
                output.writeUTF(mapEntry.getKey());
                Entry entry = mapEntry.getValue();
                output.writeUTF(entry.type.name());
                output.writeInt(entry.bytes.length);
                output.write(entry.bytes);
            }
        }

        return bytes.toByteArray();
    }

    private void decryptLegacyEntries() throws GeneralSecurityException, IOException {
                KeyStore keystore = KeyStore.getInstance("PKCS12");
        Map<String, EntryType> settingTypes = new HashMap<>();
        ByteArrayInputStream inputBytes = new ByteArrayInputStream(dataBytes);
        try (DataInputStream input = new DataInputStream(inputBytes)) {
                        if (formatVersion == 2) {
                int numSettings = input.readInt();
                for (int i = 0; i < numSettings; ++i) {
                    String key = input.readUTF();
                    String value = input.readUTF();
                    settingTypes.put(key, EntryType.valueOf(value));
                }
            }
                        keystore.load(input, "".toCharArray());
        }

                Enumeration<String> aliases = keystore.aliases();
        if (formatVersion == 1) {
            while (aliases.hasMoreElements()) {
                settingTypes.put(aliases.nextElement(), EntryType.STRING);
            }
        } else {
                        Set<String> expectedSettings = new HashSet<>(settingTypes.keySet());
            while (aliases.hasMoreElements()) {
                String settingName = aliases.nextElement();
                if (expectedSettings.remove(settingName) == false) {
                    throw new SecurityException("Keystore has been corrupted or tampered with");
                }
            }
            if (expectedSettings.isEmpty() == false) {
                throw new SecurityException("Keystore has been corrupted or tampered with");
            }
        }

                this.entries.set(new HashMap<>());
        SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("PBE");
        KeyStore.PasswordProtection password = new KeyStore.PasswordProtection("".toCharArray());

        for (Map.Entry<String, EntryType> settingEntry : settingTypes.entrySet()) {
            String setting = settingEntry.getKey();
            EntryType settingType = settingEntry.getValue();
            KeyStore.SecretKeyEntry keystoreEntry = (KeyStore.SecretKeyEntry) keystore.getEntry(setting, password);
            PBEKeySpec keySpec = (PBEKeySpec) keyFactory.getKeySpec(keystoreEntry.getSecretKey(), PBEKeySpec.class);
            char[] chars = keySpec.getPassword();
            keySpec.clearPassword();

            final byte[] bytes;
            if (settingType == EntryType.STRING) {
                ByteBuffer byteBuffer = StandardCharsets.UTF_8.encode(CharBuffer.wrap(chars));
                bytes = Arrays.copyOfRange(byteBuffer.array(), byteBuffer.position(), byteBuffer.limit());
                Arrays.fill(byteBuffer.array(), (byte)0);
            } else {
                assert settingType == EntryType.FILE;
                                byte[] tmpBytes = new byte[chars.length];
                for (int i = 0; i < tmpBytes.length; ++i) {
                    tmpBytes[i] = (byte)chars[i];                 }
                bytes = Base64.getDecoder().decode(tmpBytes);
                Arrays.fill(tmpBytes, (byte)0);
            }
            Arrays.fill(chars, '\0');

            entries.get().put(setting, new Entry(settingType, bytes));
        }
    }

    
    public void save(Path configDir, char[] password) throws Exception {
        assert isLoaded();

        SimpleFSDirectory directory = new SimpleFSDirectory(configDir);
                String tmpFile = KEYSTORE_FILENAME + ".tmp";
        try (IndexOutput output = directory.createOutput(tmpFile, IOContext.DEFAULT)) {
            CodecUtil.writeHeader(output, KEYSTORE_FILENAME, FORMAT_VERSION);
            output.writeByte(password.length == 0 ? (byte)0 : (byte)1);

                        SecureRandom random = Randomness.createSecure();
                                    byte[] salt = new byte[64];
            random.nextBytes(salt);
                                    byte[] iv = new byte[12];
            random.nextBytes(iv);
                        byte[] encryptedBytes = encrypt(password, salt, iv);

                        output.writeInt(4 + salt.length + 4 + iv.length + 4 + encryptedBytes.length);

            output.writeInt(salt.length);
            output.writeBytes(salt, salt.length);
            output.writeInt(iv.length);
            output.writeBytes(iv, iv.length);
            output.writeInt(encryptedBytes.length);
            output.writeBytes(encryptedBytes, encryptedBytes.length);

            CodecUtil.writeFooter(output);

        } catch (final AccessDeniedException e) {
            final String message = String.format(
                    Locale.ROOT,
                    "unable to create temporary keystore at [%s], please check filesystem permissions",
                    configDir.resolve(tmpFile));
            throw new UserException(ExitCodes.CONFIG, message, e);
        }

        Path keystoreFile = keystorePath(configDir);
        Files.move(configDir.resolve(tmpFile), keystoreFile, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
        PosixFileAttributeView attrs = Files.getFileAttributeView(keystoreFile, PosixFileAttributeView.class);
        if (attrs != null) {
                        attrs.setPermissions(PosixFilePermissions.fromString("rw-rw----"));
        }
    }

    @Override
    public Set<String> getSettingNames() {
        assert isLoaded();
        return entries.get().keySet();
    }

        @Override
    public SecureString getString(String setting) {
        assert isLoaded();
        Entry entry = entries.get().get(setting);
        if (entry == null || entry.type != EntryType.STRING) {
            throw new IllegalArgumentException("Secret setting " + setting + " is not a string");
        }
        ByteBuffer byteBuffer = ByteBuffer.wrap(entry.bytes);
        CharBuffer charBuffer = StandardCharsets.UTF_8.decode(byteBuffer);
        return new SecureString(charBuffer.array());
    }

    @Override
    public InputStream getFile(String setting) {
        assert isLoaded();
        Entry entry = entries.get().get(setting);
        if (entry == null || entry.type != EntryType.FILE) {
            throw new IllegalArgumentException("Secret setting " + setting + " is not a file");
        }

        return new ByteArrayInputStream(entry.bytes);
    }

    
    public static void validateSettingName(String setting) {
        if (ALLOWED_SETTING_NAME.matcher(setting).matches() == false) {
            throw new IllegalArgumentException("Setting name [" + setting + "] does not match the allowed setting name pattern ["
                + ALLOWED_SETTING_NAME.pattern() + "]");
        }
    }

    
    void setString(String setting, char[] value) {
        assert isLoaded();
        validateSettingName(setting);

        ByteBuffer byteBuffer = StandardCharsets.UTF_8.encode(CharBuffer.wrap(value));
        byte[] bytes = Arrays.copyOfRange(byteBuffer.array(), byteBuffer.position(), byteBuffer.limit());
        Entry oldEntry = entries.get().put(setting, new Entry(EntryType.STRING, bytes));
        if (oldEntry != null) {
            Arrays.fill(oldEntry.bytes, (byte)0);
        }
    }

    
    void setFile(String setting, byte[] bytes) {
        assert isLoaded();
        validateSettingName(setting);

        Entry oldEntry = entries.get().put(setting, new Entry(EntryType.FILE, Arrays.copyOf(bytes, bytes.length)));
        if (oldEntry != null) {
            Arrays.fill(oldEntry.bytes, (byte)0);
        }
    }

    
    void remove(String setting) {
        assert isLoaded();
        Entry oldEntry = entries.get().remove(setting);
        if (oldEntry != null) {
            Arrays.fill(oldEntry.bytes, (byte)0);
        }
    }

    @Override
    public void close() {
        for (Entry entry : entries.get().values()) {
            Arrays.fill(entry.bytes, (byte)0);
        }
    }
}
