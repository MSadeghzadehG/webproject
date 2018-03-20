package jenkins.slaves;

import hudson.Extension;
import hudson.ExtensionList;
import hudson.model.Computer;
import java.io.IOException;
import java.net.Socket;
import java.util.Collections;
import java.util.HashMap;
import javax.annotation.Nonnull;
import javax.inject.Inject;
import jenkins.AgentProtocol;
import org.jenkinsci.Symbol;
import org.jenkinsci.remoting.engine.JnlpClientDatabase;
import org.jenkinsci.remoting.engine.JnlpConnectionState;
import org.jenkinsci.remoting.engine.JnlpProtocol2Handler;


@Extension
@Symbol("jnlp2")
public class JnlpSlaveAgentProtocol2 extends AgentProtocol {
    private NioChannelSelector hub;

    private JnlpProtocol2Handler handler;

    @Inject
    public void setHub(NioChannelSelector hub) {
        this.hub = hub;
        this.handler = new JnlpProtocol2Handler(JnlpAgentReceiver.DATABASE, Computer.threadPoolForRemoting,
                hub.getHub(), true);
    }

    @Override
    public String getName() {
        return handler.isEnabled() ? handler.getName() : null;
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
        return Messages.JnlpSlaveAgentProtocol2_displayName();
    }

    @Override
    public void handle(Socket socket) throws IOException, InterruptedException {
        handler.handle(socket,
                Collections.singletonMap(JnlpConnectionState.COOKIE_KEY, JnlpAgentReceiver.generateCookie()),
                ExtensionList.lookup(JnlpAgentReceiver.class));
    }

}
