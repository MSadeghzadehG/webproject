package hudson.cli;

import org.apache.commons.codec.binary.Base64;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.concurrent.ExecutorService;


public class CLIConnectionFactory {
    URL jenkins;
    ExecutorService exec;
    String httpsProxyTunnel;
    String authorization;

    
    public CLIConnectionFactory url(URL jenkins) {
        this.jenkins = jenkins;
        return this;
    }

    public CLIConnectionFactory url(String jenkins) throws MalformedURLException {
        return url(new URL(jenkins));
    }
    
    
    public CLIConnectionFactory executorService(ExecutorService es) {
        this.exec = es;
        return this;
    }

    
    public CLIConnectionFactory httpsProxyTunnel(String value) {
        this.httpsProxyTunnel = value;
        return this;
    }

    
    public CLIConnectionFactory authorization(String value) {
        this.authorization = value;
        return this;
    }

    
    public CLIConnectionFactory basicAuth(String username, String password) {
        return basicAuth(username+':'+password);
    }

    
    public CLIConnectionFactory basicAuth(String userInfo) {
        return authorization("Basic " + new String(Base64.encodeBase64((userInfo).getBytes())));
    }

    
    @Deprecated
    public CLI connect() throws IOException, InterruptedException {
        return new CLI(this);
    }
}
