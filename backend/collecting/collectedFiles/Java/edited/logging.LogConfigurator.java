

package org.elasticsearch.common.logging;

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.core.LoggerContext;
import org.apache.logging.log4j.core.config.AbstractConfiguration;
import org.apache.logging.log4j.core.config.Configurator;
import org.apache.logging.log4j.core.config.builder.api.ConfigurationBuilder;
import org.apache.logging.log4j.core.config.builder.api.ConfigurationBuilderFactory;
import org.apache.logging.log4j.core.config.builder.impl.BuiltConfiguration;
import org.apache.logging.log4j.core.config.composite.CompositeConfiguration;
import org.apache.logging.log4j.core.config.properties.PropertiesConfiguration;
import org.apache.logging.log4j.core.config.properties.PropertiesConfigurationFactory;
import org.apache.logging.log4j.status.StatusConsoleListener;
import org.apache.logging.log4j.status.StatusData;
import org.apache.logging.log4j.status.StatusListener;
import org.apache.logging.log4j.status.StatusLogger;
import org.elasticsearch.cli.ExitCodes;
import org.elasticsearch.cli.UserException;
import org.elasticsearch.cluster.ClusterName;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.node.Node;

import java.io.IOException;
import java.nio.file.FileVisitOption;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.stream.StreamSupport;

public class LogConfigurator {

    
    private static final AtomicBoolean error = new AtomicBoolean();
    private static final StatusListener ERROR_LISTENER = new StatusConsoleListener(Level.ERROR) {
        @Override
        public void log(StatusData data) {
            error.set(true);
            super.log(data);
        }
    };

    
    public static void registerErrorListener() {
        error.set(false);
        StatusLogger.getLogger().registerListener(ERROR_LISTENER);
    }

    
    public static void configureWithoutConfig(final Settings settings) {
        Objects.requireNonNull(settings);
                configureStatusLogger();
        configureLoggerLevels(settings);
    }

    
    public static void configure(final Environment environment) throws IOException, UserException {
        Objects.requireNonNull(environment);
        try {
                        checkErrorListener();
        } finally {
                        StatusLogger.getLogger().removeListener(ERROR_LISTENER);
        }
        configure(environment.settings(), environment.configFile(), environment.logsFile());
    }

    private static void checkErrorListener() {
        assert errorListenerIsRegistered() : "expected error listener to be registered";
        if (error.get()) {
            throw new IllegalStateException("status logger logged an error before logging was configured");
        }
    }

    private static boolean errorListenerIsRegistered() {
        return StreamSupport.stream(StatusLogger.getLogger().getListeners().spliterator(), false).anyMatch(l -> l == ERROR_LISTENER);
    }

    private static void configure(final Settings settings, final Path configsPath, final Path logsPath) throws IOException, UserException {
        Objects.requireNonNull(settings);
        Objects.requireNonNull(configsPath);
        Objects.requireNonNull(logsPath);

        setLogConfigurationSystemProperty(logsPath, settings);
                configureStatusLogger();

        final LoggerContext context = (LoggerContext) LogManager.getContext(false);

        final List<AbstractConfiguration> configurations = new ArrayList<>();
        final PropertiesConfigurationFactory factory = new PropertiesConfigurationFactory();
        final Set<FileVisitOption> options = EnumSet.of(FileVisitOption.FOLLOW_LINKS);
        Files.walkFileTree(configsPath, options, Integer.MAX_VALUE, new SimpleFileVisitor<Path>() {
            @Override
            public FileVisitResult visitFile(final Path file, final BasicFileAttributes attrs) throws IOException {
                if (file.getFileName().toString().equals("log4j2.properties")) {
                    configurations.add((PropertiesConfiguration) factory.getConfiguration(context, file.toString(), file.toUri()));
                }
                return FileVisitResult.CONTINUE;
            }
        });

        if (configurations.isEmpty()) {
            throw new UserException(
                    ExitCodes.CONFIG,
                    "no log4j2.properties found; tried [" + configsPath + "] and its subdirectories");
        }

        context.start(new CompositeConfiguration(configurations));

        configureLoggerLevels(settings);
    }

    private static void configureStatusLogger() {
        final ConfigurationBuilder<BuiltConfiguration> builder = ConfigurationBuilderFactory.newConfigurationBuilder();
        builder.setStatusLevel(Level.ERROR);
        Configurator.initialize(builder.build());
    }

    
    private static void configureLoggerLevels(final Settings settings) {
        if (Loggers.LOG_DEFAULT_LEVEL_SETTING.exists(settings)) {
            final Level level = Loggers.LOG_DEFAULT_LEVEL_SETTING.get(settings);
            Loggers.setLevel(ESLoggerFactory.getRootLogger(), level);
        }
        Loggers.LOG_LEVEL_SETTING.getAllConcreteSettings(settings)
                        .filter(s -> s.getKey().equals(Loggers.LOG_DEFAULT_LEVEL_SETTING.getKey()) == false).forEach(s -> {
            final Level level = s.get(settings);
            Loggers.setLevel(ESLoggerFactory.getLogger(s.getKey().substring("logger.".length())), level);
        });
    }

    
    @SuppressForbidden(reason = "sets system property for logging configuration")
    private static void setLogConfigurationSystemProperty(final Path logsPath, final Settings settings) {
        System.setProperty("es.logs.base_path", logsPath.toString());
        System.setProperty("es.logs.cluster_name", ClusterName.CLUSTER_NAME_SETTING.get(settings).value());
        if (Node.NODE_NAME_SETTING.exists(settings)) {
            System.setProperty("es.logs.node_name", Node.NODE_NAME_SETTING.get(settings));
        }
    }

}
