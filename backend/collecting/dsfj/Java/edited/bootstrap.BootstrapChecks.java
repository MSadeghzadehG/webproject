

package org.elasticsearch.bootstrap;

import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.message.ParameterizedMessage;
import org.apache.logging.log4j.util.Supplier;
import org.apache.lucene.util.Constants;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.common.logging.Loggers;
import org.elasticsearch.common.transport.BoundTransportAddress;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.discovery.DiscoveryModule;
import org.elasticsearch.monitor.jvm.JvmInfo;
import org.elasticsearch.monitor.process.ProcessProbe;
import org.elasticsearch.node.Node;
import org.elasticsearch.node.NodeValidationException;

import java.io.BufferedReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.AllPermission;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.function.Predicate;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


final class BootstrapChecks {

    private BootstrapChecks() {
    }

    static final String ES_ENFORCE_BOOTSTRAP_CHECKS = "es.enforce.bootstrap.checks";

    
    static void check(final BootstrapContext context, final BoundTransportAddress boundTransportAddress,
                      List<BootstrapCheck> additionalChecks) throws NodeValidationException {
        final List<BootstrapCheck> builtInChecks = checks();
        final List<BootstrapCheck> combinedChecks = new ArrayList<>(builtInChecks);
        combinedChecks.addAll(additionalChecks);
        check(  context,
                enforceLimits(boundTransportAddress, DiscoveryModule.DISCOVERY_TYPE_SETTING.get(context.settings)),
                Collections.unmodifiableList(combinedChecks),
                Node.NODE_NAME_SETTING.get(context.settings));
    }

    
    static void check(
        final BootstrapContext context,
        final boolean enforceLimits,
        final List<BootstrapCheck> checks,
        final String nodeName) throws NodeValidationException {
        check(context, enforceLimits, checks, Loggers.getLogger(BootstrapChecks.class, nodeName));
    }

    
    static void check(
            final BootstrapContext context,
            final boolean enforceLimits,
            final List<BootstrapCheck> checks,
            final Logger logger) throws NodeValidationException {
        final List<String> errors = new ArrayList<>();
        final List<String> ignoredErrors = new ArrayList<>();

        final String esEnforceBootstrapChecks = System.getProperty(ES_ENFORCE_BOOTSTRAP_CHECKS);
        final boolean enforceBootstrapChecks;
        if (esEnforceBootstrapChecks == null) {
            enforceBootstrapChecks = false;
        } else if (Boolean.TRUE.toString().equals(esEnforceBootstrapChecks)) {
            enforceBootstrapChecks = true;
        } else {
            final String message =
                    String.format(
                            Locale.ROOT,
                            "[%s] must be [true] but was [%s]",
                            ES_ENFORCE_BOOTSTRAP_CHECKS,
                            esEnforceBootstrapChecks);
            throw new IllegalArgumentException(message);
        }

        if (enforceLimits) {
            logger.info("bound or publishing to a non-loopback address, enforcing bootstrap checks");
        } else if (enforceBootstrapChecks) {
            logger.info("explicitly enforcing bootstrap checks");
        }

        for (final BootstrapCheck check : checks) {
            final BootstrapCheck.BootstrapCheckResult result = check.check(context);
            if (result.isFailure()) {
                if (!(enforceLimits || enforceBootstrapChecks) && !check.alwaysEnforce()) {
                    ignoredErrors.add(result.getMessage());
                } else {
                    errors.add(result.getMessage());
                }
            }
        }

        if (!ignoredErrors.isEmpty()) {
            ignoredErrors.forEach(error -> log(logger, error));
        }

        if (!errors.isEmpty()) {
            final List<String> messages = new ArrayList<>(1 + errors.size());
            messages.add("[" + errors.size() + "] bootstrap checks failed");
            for (int i = 0; i < errors.size(); i++) {
                messages.add("[" + (i + 1) + "]: " + errors.get(i));
            }
            final NodeValidationException ne = new NodeValidationException(String.join("\n", messages));
            errors.stream().map(IllegalStateException::new).forEach(ne::addSuppressed);
            throw ne;
        }
    }

    static void log(final Logger logger, final String error) {
        logger.warn(error);
    }

    
    static boolean enforceLimits(final BoundTransportAddress boundTransportAddress, final String discoveryType) {
        final Predicate<TransportAddress> isLoopbackAddress = t -> t.address().getAddress().isLoopbackAddress();
        final boolean bound =
                !(Arrays.stream(boundTransportAddress.boundAddresses()).allMatch(isLoopbackAddress) &&
                isLoopbackAddress.test(boundTransportAddress.publishAddress()));
        return bound && !"single-node".equals(discoveryType);
    }

        static List<BootstrapCheck> checks() {
        final List<BootstrapCheck> checks = new ArrayList<>();
        checks.add(new HeapSizeCheck());
        final FileDescriptorCheck fileDescriptorCheck
            = Constants.MAC_OS_X ? new OsXFileDescriptorCheck() : new FileDescriptorCheck();
        checks.add(fileDescriptorCheck);
        checks.add(new MlockallCheck());
        if (Constants.LINUX) {
            checks.add(new MaxNumberOfThreadsCheck());
        }
        if (Constants.LINUX || Constants.MAC_OS_X) {
            checks.add(new MaxSizeVirtualMemoryCheck());
        }
        if (Constants.LINUX || Constants.MAC_OS_X) {
            checks.add(new MaxFileSizeCheck());
        }
        if (Constants.LINUX) {
            checks.add(new MaxMapCountCheck());
        }
        checks.add(new ClientJvmCheck());
        checks.add(new UseSerialGCCheck());
        checks.add(new SystemCallFilterCheck());
        checks.add(new OnErrorCheck());
        checks.add(new OnOutOfMemoryErrorCheck());
        checks.add(new EarlyAccessCheck());
        checks.add(new G1GCCheck());
        checks.add(new AllPermissionCheck());
        return Collections.unmodifiableList(checks);
    }

    static class HeapSizeCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            final long initialHeapSize = getInitialHeapSize();
            final long maxHeapSize = getMaxHeapSize();
            if (initialHeapSize != 0 && maxHeapSize != 0 && initialHeapSize != maxHeapSize) {
                final String message = String.format(
                        Locale.ROOT,
                        "initial heap size [%d] not equal to maximum heap size [%d]; " +
                                "this can cause resize pauses and prevents mlockall from locking the entire heap",
                        getInitialHeapSize(),
                        getMaxHeapSize());
                return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

                long getInitialHeapSize() {
            return JvmInfo.jvmInfo().getConfiguredInitialHeapSize();
        }

                long getMaxHeapSize() {
            return JvmInfo.jvmInfo().getConfiguredMaxHeapSize();
        }

    }

    static class OsXFileDescriptorCheck extends FileDescriptorCheck {

        OsXFileDescriptorCheck() {
                                                            super(10240);
        }

    }

    static class FileDescriptorCheck implements BootstrapCheck {

        private final int limit;

        FileDescriptorCheck() {
            this(1 << 16);
        }

        protected FileDescriptorCheck(final int limit) {
            if (limit <= 0) {
                throw new IllegalArgumentException("limit must be positive but was [" + limit + "]");
            }
            this.limit = limit;
        }

        public final BootstrapCheckResult check(BootstrapContext context) {
            final long maxFileDescriptorCount = getMaxFileDescriptorCount();
            if (maxFileDescriptorCount != -1 && maxFileDescriptorCount < limit) {
                final String message = String.format(
                        Locale.ROOT,
                        "max file descriptors [%d] for elasticsearch process is too low, increase to at least [%d]",
                        getMaxFileDescriptorCount(),
                        limit);
                return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

                long getMaxFileDescriptorCount() {
            return ProcessProbe.getInstance().getMaxFileDescriptorCount();
        }

    }

    static class MlockallCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            if (BootstrapSettings.MEMORY_LOCK_SETTING.get(context.settings) && !isMemoryLocked()) {
                return BootstrapCheckResult.failure("memory locking requested for elasticsearch process but memory is not locked");
            } else {
                return BootstrapCheckResult.success();
            }
        }

                boolean isMemoryLocked() {
            return Natives.isMemoryLocked();
        }

    }

    static class MaxNumberOfThreadsCheck implements BootstrapCheck {

                private static final long MAX_NUMBER_OF_THREADS_THRESHOLD = 1 << 12;

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            if (getMaxNumberOfThreads() != -1 && getMaxNumberOfThreads() < MAX_NUMBER_OF_THREADS_THRESHOLD) {
                final String message = String.format(
                        Locale.ROOT,
                        "max number of threads [%d] for user [%s] is too low, increase to at least [%d]",
                        getMaxNumberOfThreads(),
                        BootstrapInfo.getSystemProperties().get("user.name"),
                        MAX_NUMBER_OF_THREADS_THRESHOLD);
                return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

                long getMaxNumberOfThreads() {
            return JNANatives.MAX_NUMBER_OF_THREADS;
        }

    }

    static class MaxSizeVirtualMemoryCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            if (getMaxSizeVirtualMemory() != Long.MIN_VALUE && getMaxSizeVirtualMemory() != getRlimInfinity()) {
                final String message = String.format(
                        Locale.ROOT,
                        "max size virtual memory [%d] for user [%s] is too low, increase to [unlimited]",
                        getMaxSizeVirtualMemory(),
                        BootstrapInfo.getSystemProperties().get("user.name"));
                return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

                long getRlimInfinity() {
            return JNACLibrary.RLIM_INFINITY;
        }

                long getMaxSizeVirtualMemory() {
            return JNANatives.MAX_SIZE_VIRTUAL_MEMORY;
        }

    }

    
    static class MaxFileSizeCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            final long maxFileSize = getMaxFileSize();
            if (maxFileSize != Long.MIN_VALUE && maxFileSize != getRlimInfinity()) {
                final String message = String.format(
                        Locale.ROOT,
                        "max file size [%d] for user [%s] is too low, increase to [unlimited]",
                        getMaxFileSize(),
                        BootstrapInfo.getSystemProperties().get("user.name"));
                return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

        long getRlimInfinity() {
            return JNACLibrary.RLIM_INFINITY;
        }

        long getMaxFileSize() {
            return JNANatives.MAX_FILE_SIZE;
        }

    }

    static class MaxMapCountCheck implements BootstrapCheck {

        private static final long LIMIT = 1 << 18;

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            if (getMaxMapCount() != -1 && getMaxMapCount() < LIMIT) {
               final String message = String.format(
                        Locale.ROOT,
                        "max virtual memory areas vm.max_map_count [%d] is too low, increase to at least [%d]",
                        getMaxMapCount(),
                        LIMIT);
               return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

                long getMaxMapCount() {
            return getMaxMapCount(Loggers.getLogger(BootstrapChecks.class));
        }

                long getMaxMapCount(Logger logger) {
            final Path path = getProcSysVmMaxMapCountPath();
            try (BufferedReader bufferedReader = getBufferedReader(path)) {
                final String rawProcSysVmMaxMapCount = readProcSysVmMaxMapCount(bufferedReader);
                if (rawProcSysVmMaxMapCount != null) {
                    try {
                        return parseProcSysVmMaxMapCount(rawProcSysVmMaxMapCount);
                    } catch (final NumberFormatException e) {
                        logger.warn(
                            (Supplier<?>) () -> new ParameterizedMessage(
                                "unable to parse vm.max_map_count [{}]",
                                rawProcSysVmMaxMapCount),
                            e);
                    }
                }
            } catch (final IOException e) {
                logger.warn((Supplier<?>) () -> new ParameterizedMessage("I/O exception while trying to read [{}]", path), e);
            }
            return -1;
        }

        @SuppressForbidden(reason = "access /proc/sys/vm/max_map_count")
        private Path getProcSysVmMaxMapCountPath() {
            return PathUtils.get("/proc/sys/vm/max_map_count");
        }

                BufferedReader getBufferedReader(final Path path) throws IOException {
            return Files.newBufferedReader(path);
        }

                String readProcSysVmMaxMapCount(final BufferedReader bufferedReader) throws IOException {
            return bufferedReader.readLine();
        }

                long parseProcSysVmMaxMapCount(final String procSysVmMaxMapCount) throws NumberFormatException {
            return Long.parseLong(procSysVmMaxMapCount);
        }

    }

    static class ClientJvmCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            if (getVmName().toLowerCase(Locale.ROOT).contains("client")) {
                final String message = String.format(
                        Locale.ROOT,
                        "JVM is using the client VM [%s] but should be using a server VM for the best performance",
                        getVmName());
                return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

                String getVmName() {
            return JvmInfo.jvmInfo().getVmName();
        }

    }

    
    static class UseSerialGCCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            if (getUseSerialGC().equals("true")) {
                final String message = String.format(
                        Locale.ROOT,
                        "JVM is using the serial collector but should not be for the best performance; " +
                                "either it's the default for the VM [%s] or -XX:+UseSerialGC was explicitly specified",
                        JvmInfo.jvmInfo().getVmName());
                return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

                String getUseSerialGC() {
            return JvmInfo.jvmInfo().useSerialGC();
        }

    }

    
    static class SystemCallFilterCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            if (BootstrapSettings.SYSTEM_CALL_FILTER_SETTING.get(context.settings) && !isSystemCallFilterInstalled()) {
                final String message =  "system call filters failed to install; " +
                        "check the logs and fix your configuration or disable system call filters at your own risk";
                return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

                boolean isSystemCallFilterInstalled() {
            return Natives.isSystemCallFilterInstalled();
        }

    }

    abstract static class MightForkCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            if (isSystemCallFilterInstalled() && mightFork()) {
                return BootstrapCheckResult.failure(message(context));
            } else {
                return BootstrapCheckResult.success();
            }
        }

        abstract String message(BootstrapContext context);

                boolean isSystemCallFilterInstalled() {
            return Natives.isSystemCallFilterInstalled();
        }

                abstract boolean mightFork();

        @Override
        public final boolean alwaysEnforce() {
            return true;
        }

    }

    static class OnErrorCheck extends MightForkCheck {

        @Override
        boolean mightFork() {
            final String onError = onError();
            return onError != null && !onError.equals("");
        }

                String onError() {
            return JvmInfo.jvmInfo().onError();
        }

        @Override
        String message(BootstrapContext context) {
            return String.format(
                Locale.ROOT,
                "OnError [%s] requires forking but is prevented by system call filters ([%s=true]);" +
                    " upgrade to at least Java 8u92 and use ExitOnOutOfMemoryError",
                onError(),
                BootstrapSettings.SYSTEM_CALL_FILTER_SETTING.getKey());
        }

    }

    static class OnOutOfMemoryErrorCheck extends MightForkCheck {

        @Override
        boolean mightFork() {
            final String onOutOfMemoryError = onOutOfMemoryError();
            return onOutOfMemoryError != null && !onOutOfMemoryError.equals("");
        }

                String onOutOfMemoryError() {
            return JvmInfo.jvmInfo().onOutOfMemoryError();
        }

        String message(BootstrapContext context) {
            return String.format(
                Locale.ROOT,
                "OnOutOfMemoryError [%s] requires forking but is prevented by system call filters ([%s=true]);" +
                    " upgrade to at least Java 8u92 and use ExitOnOutOfMemoryError",
                onOutOfMemoryError(),
                BootstrapSettings.SYSTEM_CALL_FILTER_SETTING.getKey());
        }

    }

    
    static class EarlyAccessCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            final String javaVersion = javaVersion();
            if ("Oracle Corporation".equals(jvmVendor()) && javaVersion.endsWith("-ea")) {
                final String message = String.format(
                        Locale.ROOT,
                        "Java version [%s] is an early-access build, only use release builds",
                        javaVersion);
                return BootstrapCheckResult.failure(message);
            } else {
                return BootstrapCheckResult.success();
            }
        }

        String jvmVendor() {
            return Constants.JVM_VENDOR;
        }

        String javaVersion() {
            return Constants.JAVA_VERSION;
        }

    }

    
    static class G1GCCheck implements BootstrapCheck {

        @Override
        public BootstrapCheckResult check(BootstrapContext context) {
            if ("Oracle Corporation".equals(jvmVendor()) && isJava8() && isG1GCEnabled()) {
                final String jvmVersion = jvmVersion();
                                final Pattern pattern = Pattern.compile("(\\d+)\\.(\\d+)-b\\d+");
                final Matcher matcher = pattern.matcher(jvmVersion);
                final boolean matches = matcher.matches();
                assert matches : jvmVersion;
                final int major = Integer.parseInt(matcher.group(1));
                final int update = Integer.parseInt(matcher.group(2));
                                if (major == 25 && update < 40) {
                    final String message = String.format(
                            Locale.ROOT,
                            "JVM version [%s] can cause data corruption when used with G1GC; upgrade to at least Java 8u40", jvmVersion);
                    return BootstrapCheckResult.failure(message);
                }
            }
            return BootstrapCheckResult.success();
        }

                String jvmVendor() {
            return Constants.JVM_VENDOR;
        }

                boolean isG1GCEnabled() {
            assert "Oracle Corporation".equals(jvmVendor());
            return JvmInfo.jvmInfo().useG1GC().equals("true");
        }

                String jvmVersion() {
            assert "Oracle Corporation".equals(jvmVendor());
            return Constants.JVM_VERSION;
        }

                boolean isJava8() {
            assert "Oracle Corporation".equals(jvmVendor());
            return JavaVersion.current().equals(JavaVersion.parse("1.8"));
        }

    }

    static class AllPermissionCheck implements BootstrapCheck {

        @Override
        public final BootstrapCheckResult check(BootstrapContext context) {
            if (isAllPermissionGranted()) {
                return BootstrapCheck.BootstrapCheckResult.failure("granting the all permission effectively disables security");
            }
            return BootstrapCheckResult.success();
        }

        boolean isAllPermissionGranted() {
            final SecurityManager sm = System.getSecurityManager();
            assert sm != null;
            try {
                sm.checkPermission(new AllPermission());
            } catch (final SecurityException e) {
                return false;
            }
            return true;
        }

    }

}
