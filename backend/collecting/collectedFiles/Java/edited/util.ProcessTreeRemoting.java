package hudson.util;

import hudson.EnvVars;
import hudson.util.ProcessTree.ProcessCallable;

import java.io.IOException;
import java.lang.reflect.Proxy;
import java.util.List;
import java.util.Map;


public class ProcessTreeRemoting {
    public interface IProcessTree {
        void killAll(Map<String, String> modelEnvVars) throws InterruptedException;
    }

    public interface IOSProcess {
        int getPid();
        IOSProcess getParent();
        void kill() throws InterruptedException;
        void killRecursively() throws InterruptedException;
        List<String> getArguments();
        EnvVars getEnvironmentVariables();
        <T> T act(ProcessCallable<T> callable) throws IOException, InterruptedException;
    }
}