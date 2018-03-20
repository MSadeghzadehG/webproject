
package hudson.cli;

import hudson.remoting.CallableFilter;
import hudson.remoting.Channel;
import hudson.remoting.Pipe;
import org.acegisecurity.Authentication;
import org.acegisecurity.context.SecurityContext;
import org.acegisecurity.context.SecurityContextHolder;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.Serializable;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.Callable;
import java.util.logging.Logger;


@Deprecated
public class CliManagerImpl implements CliEntryPoint, Serializable {
    private transient final Channel channel;
    
    private Authentication transportAuth;

        
    private transient final CallableFilter authenticationFilter = new CallableFilter() {
        public <V> V call(Callable<V> callable) throws Exception {
            SecurityContext context = SecurityContextHolder.getContext();
            Authentication old = context.getAuthentication();
            if (transportAuth!=null)
                context.setAuthentication(transportAuth);
            try {
                return callable.call();
            } finally {
                context.setAuthentication(old);
            }
        }
    };

    public CliManagerImpl(Channel channel) {
        this.channel = channel;
        channel.addLocalExecutionInterceptor(authenticationFilter);
    }

    public int main(List<String> args, Locale locale, InputStream stdin, OutputStream stdout, OutputStream stderr) {
                                Thread.currentThread().setContextClassLoader(getClass().getClassLoader());

        PrintStream out = new PrintStream(stdout);
        PrintStream err = new PrintStream(stderr);

        String subCmd = args.get(0);
        CLICommand cmd = CLICommand.clone(subCmd);
        if(cmd!=null) {
            cmd.channel = Channel.current();
            final CLICommand old = CLICommand.setCurrent(cmd);
            try {
                transportAuth = Channel.current().getProperty(CLICommand.TRANSPORT_AUTHENTICATION);
                cmd.setTransportAuth(transportAuth);
                return cmd.main(args.subList(1,args.size()),locale, stdin, out, err);
            } finally {
                CLICommand.setCurrent(old);
            }
        }

        err.println("No such command: "+subCmd);
        new HelpCommand().main(Collections.<String>emptyList(), locale, stdin, out, err);
        return -1;
    }

    public void authenticate(final String protocol, final Pipe c2s, final Pipe s2c) {
        for (final CliTransportAuthenticator cta : CliTransportAuthenticator.all()) {
            if (cta.supportsProtocol(protocol)) {
                new Thread() {
                    @Override
                    public void run() {
                        cta.authenticate(protocol,channel,new Connection(c2s.getIn(), s2c.getOut()));
                    }
                }.start();
                return;
            }
        }
        throw new UnsupportedOperationException("Unsupported authentication protocol: "+protocol);
    }

    public boolean hasCommand(String name) {
        return CLICommand.clone(name)!=null;
    }

    public int protocolVersion() {
        return VERSION;
    }

    private Object writeReplace() {
        return Channel.current().export(CliEntryPoint.class,this);
    }

    private static final Logger LOGGER = Logger.getLogger(CliManagerImpl.class.getName());
}
