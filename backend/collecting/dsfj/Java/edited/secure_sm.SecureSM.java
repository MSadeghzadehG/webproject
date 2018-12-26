

package org.elasticsearch.secure_sm;

import java.security.AccessController;
import java.security.Permission;
import java.security.PrivilegedAction;
import java.util.Objects;


public class SecureSM extends SecurityManager {

    private final String[] classesThatCanExit;

    
    public SecureSM() {
        this(new String[0]);
    }

    
    public SecureSM(final String[] classesThatCanExit) {
        this.classesThatCanExit = classesThatCanExit;
    }

    
    public static SecureSM createTestSecureSM() {
        return new SecureSM(TEST_RUNNER_PACKAGES);
    }

    static final String[] TEST_RUNNER_PACKAGES = new String[] {
                "org\\.apache\\.maven\\.surefire\\.booter\\..*",
                "com\\.carrotsearch\\.ant\\.tasks\\.junit4\\.slave\\..*",
                "org\\.eclipse.jdt\\.internal\\.junit\\.runner\\..*",
                "com\\.intellij\\.rt\\.execution\\.junit\\..*"
    };

        private static final boolean DEBUG = AccessController.doPrivileged(new PrivilegedAction<Boolean>() {
        @Override
        public Boolean run() {
            try {
                String v = System.getProperty("java.security.debug");
                                return v != null && v.length() > 0;
            } catch (SecurityException e) {
                return false;
            }
        }
    });
    
    @Override
    @SuppressForbidden(reason = "java.security.debug messages go to standard error")
    public void checkAccess(Thread t) {
        try {
            checkThreadAccess(t);
        } catch (SecurityException e) {
            if (DEBUG) {
                System.err.println("access: caller thread=" + Thread.currentThread());
                System.err.println("access: target thread=" + t);
                debugThreadGroups(Thread.currentThread().getThreadGroup(), t.getThreadGroup());
            }
            throw e;
        }
    }
    
    @Override
    @SuppressForbidden(reason = "java.security.debug messages go to standard error")
    public void checkAccess(ThreadGroup g) {
        try {
            checkThreadGroupAccess(g);
        } catch (SecurityException e) {
            if (DEBUG) {
                System.err.println("access: caller thread=" + Thread.currentThread());
                debugThreadGroups(Thread.currentThread().getThreadGroup(), g);
            }
            throw e;
        }
    }

    @SuppressForbidden(reason = "java.security.debug messages go to standard error")
    private void debugThreadGroups(final ThreadGroup caller, final ThreadGroup target) {
        System.err.println("access: caller group=" + caller);
        System.err.println("access: target group=" + target);
    }
    
    
    private static final Permission MODIFY_THREAD_PERMISSION = new RuntimePermission("modifyThread");
    private static final Permission MODIFY_ARBITRARY_THREAD_PERMISSION = new ThreadPermission("modifyArbitraryThread");

    protected void checkThreadAccess(Thread t) {
        Objects.requireNonNull(t);

                checkPermission(MODIFY_THREAD_PERMISSION);
        
                final ThreadGroup source = Thread.currentThread().getThreadGroup();
        final ThreadGroup target = t.getThreadGroup();
        
        if (target == null) {
            return;            } else if (source.parentOf(target) == false) {
            checkPermission(MODIFY_ARBITRARY_THREAD_PERMISSION);
        }
    }
    
    private static final Permission MODIFY_THREADGROUP_PERMISSION = new RuntimePermission("modifyThreadGroup");
    private static final Permission MODIFY_ARBITRARY_THREADGROUP_PERMISSION = new ThreadPermission("modifyArbitraryThreadGroup");
    
    protected void checkThreadGroupAccess(ThreadGroup g) {
        Objects.requireNonNull(g);

                checkPermission(MODIFY_THREADGROUP_PERMISSION);
        
                final ThreadGroup source = Thread.currentThread().getThreadGroup();
        final ThreadGroup target = g;
        
        if (source == null) {
            return;         } else if (source.parentOf(target) == false) {
            checkPermission(MODIFY_ARBITRARY_THREADGROUP_PERMISSION);
        }
    }

        @Override
    public void checkExit(int status) {
        innerCheckExit(status);
    }
    
    
    protected void innerCheckExit(final int status) {
        AccessController.doPrivileged(new PrivilegedAction<Void>() {
            @Override
            public Void run() {
                final String systemClassName = System.class.getName(),
                        runtimeClassName = Runtime.class.getName();
                String exitMethodHit = null;
                for (final StackTraceElement se : Thread.currentThread().getStackTrace()) {
                    final String className = se.getClassName(), methodName = se.getMethodName();
                    if (
                        ("exit".equals(methodName) || "halt".equals(methodName)) &&
                        (systemClassName.equals(className) || runtimeClassName.equals(className))
                    ) {
                        exitMethodHit = className + '#' + methodName + '(' + status + ')';
                        continue;
                    }
                    
                    if (exitMethodHit != null) {
                        if (classesThatCanExit == null) {
                            break;
                        }
                        if (classCanExit(className, classesThatCanExit)) {
                                                        return null;
                        }
                                                break;
                    }
                }
                
                if (exitMethodHit == null) {
                                        exitMethodHit = "JVM exit method";
                }
                throw new SecurityException(exitMethodHit + " calls are not allowed");
            }
        });
        
                super.checkExit(status);
    }

    static boolean classCanExit(final String className, final String[] classesThatCanExit) {
        for (final String classThatCanExit : classesThatCanExit) {
            if (className.matches(classThatCanExit)) {
                return true;
            }
        }
        return false;
    }

}
