package hudson.cli;

import hudson.Extension;
import jenkins.model.Jenkins;
import org.jenkinsci.Symbol;
import org.jenkinsci.remoting.nio.NioChannelHub;

import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.io.DataOutputStream;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.net.Socket;
import java.security.GeneralSecurityException;
import java.security.PrivateKey;
import java.security.Signature;


@Deprecated
@Extension @Symbol("cli2")
public class CliProtocol2 extends CliProtocol {
    @Override
    public String getName() {
        return jenkins.CLI.get().isEnabled() ? "CLI2-connect" : null;
    }

    
    @Override
    public boolean isOptIn() {
        return false;
    }

    @Override
    public boolean isDeprecated() {
                return true;
    }
    
    
    @Override
    public String getDisplayName() {
        return "Jenkins CLI Protocol/2 (deprecated)";
    }

    @Override
    public void handle(Socket socket) throws IOException, InterruptedException {
        new Handler2(nio.getHub(), socket).run();
    }

    protected static class Handler2 extends Handler {
        
        @Deprecated
        public Handler2(Socket socket) {
            super(socket);
        }

        public Handler2(NioChannelHub hub, Socket socket) {
            super(hub, socket);
        }

        @Override
        public void run() throws IOException, InterruptedException {
            try {
                DataOutputStream out = new DataOutputStream(socket.getOutputStream());
                out.writeUTF("Welcome");

                                Connection c = new Connection(socket);
                byte[] secret = c.diffieHellman(true).generateSecret();
                SecretKey sessionKey = new SecretKeySpec(Connection.fold(secret,128/8),"AES");
                c = c.encryptConnection(sessionKey,"AES/CFB8/NoPadding");

                try {
                                        Class<?> cls = Jenkins.getActiveInstance().pluginManager.uberClassLoader.loadClass("org.jenkinsci.main.modules.instance_identity.InstanceIdentity");
                    Object iid = cls.getDeclaredMethod("get").invoke(null);
                    PrivateKey instanceId = (PrivateKey)cls.getDeclaredMethod("getPrivate").invoke(iid);

                                        Signature signer = Signature.getInstance("SHA1withRSA");
                    signer.initSign(instanceId);
                    signer.update(secret);
                    c.writeByteArray(signer.sign());
                } catch (ClassNotFoundException | IllegalAccessException | InvocationTargetException | NoSuchMethodException e) {
                    throw new Error(e);
                }

                runCli(c);
            } catch (GeneralSecurityException e) {
                throw new IOException("Failed to encrypt the CLI channel",e);
            }
        }
    }
}
