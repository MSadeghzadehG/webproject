
package org.elasticsearch.indices.analysis;

import org.apache.logging.log4j.message.ParameterizedMessage;
import org.apache.logging.log4j.util.Supplier;
import org.apache.lucene.analysis.hunspell.Dictionary;
import org.apache.lucene.store.Directory;
import org.apache.lucene.store.SimpleFSDirectory;
import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.io.FileSystemUtils;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Setting.Property;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Function;


public class HunspellService extends AbstractComponent {

    public static final Setting<Boolean> HUNSPELL_LAZY_LOAD =
        Setting.boolSetting("indices.analysis.hunspell.dictionary.lazy", Boolean.FALSE, Property.NodeScope);
    public static final Setting<Boolean> HUNSPELL_IGNORE_CASE =
        Setting.boolSetting("indices.analysis.hunspell.dictionary.ignore_case", Boolean.FALSE, Property.NodeScope);
    public static final Setting<Settings> HUNSPELL_DICTIONARY_OPTIONS =
        Setting.groupSetting("indices.analysis.hunspell.dictionary.", Property.NodeScope);
    private final ConcurrentHashMap<String, Dictionary> dictionaries = new ConcurrentHashMap<>();
    private final Map<String, Dictionary> knownDictionaries;
    private final boolean defaultIgnoreCase;
    private final Path hunspellDir;
    private final Function<String, Dictionary> loadingFunction;

    public HunspellService(final Settings settings, final Environment env, final Map<String, Dictionary> knownDictionaries)
            throws IOException {
        super(settings);
        this.knownDictionaries = Collections.unmodifiableMap(knownDictionaries);
        this.hunspellDir = resolveHunspellDirectory(env);
        this.defaultIgnoreCase = HUNSPELL_IGNORE_CASE.get(settings);
        this.loadingFunction = (locale) -> {
            try {
                return loadDictionary(locale, settings, env);
            } catch (Exception e) {
                throw new IllegalStateException("failed to load hunspell dictionary for locale: " + locale, e);
            }
        };
        if (!HUNSPELL_LAZY_LOAD.get(settings)) {
            scanAndLoadDictionaries();
        }

    }

    
    public Dictionary getDictionary(String locale) {
        Dictionary dictionary = knownDictionaries.get(locale);
        if (dictionary == null) {
            dictionary = dictionaries.computeIfAbsent(locale, loadingFunction);
        }
        return dictionary;
    }

    private Path resolveHunspellDirectory(Environment env) {
        return env.configFile().resolve("hunspell");
    }

    
    private void scanAndLoadDictionaries() throws IOException {
        if (Files.isDirectory(hunspellDir)) {
            try (DirectoryStream<Path> stream = Files.newDirectoryStream(hunspellDir)) {
                for (Path file : stream) {
                    if (Files.isDirectory(file)) {
                        try (DirectoryStream<Path> inner = Files.newDirectoryStream(hunspellDir.resolve(file), "*.dic")) {
                            if (inner.iterator().hasNext()) {                                 try {
                                    getDictionary(file.getFileName().toString());
                                } catch (Exception e) {
                                                                                                            logger.error(
                                        (Supplier<?>) () -> new ParameterizedMessage(
                                            "exception while loading dictionary {}", file.getFileName()), e);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    
    private Dictionary loadDictionary(String locale, Settings nodeSettings, Environment env) throws Exception {
        if (logger.isDebugEnabled()) {
            logger.debug("Loading hunspell dictionary [{}]...", locale);
        }
        Path dicDir = hunspellDir.resolve(locale);
        if (FileSystemUtils.isAccessibleDirectory(dicDir, logger) == false) {
            throw new ElasticsearchException(String.format(Locale.ROOT, "Could not find hunspell dictionary [%s]", locale));
        }

                Settings dictSettings = HUNSPELL_DICTIONARY_OPTIONS.get(nodeSettings);
        nodeSettings = loadDictionarySettings(dicDir, dictSettings.getByPrefix(locale + "."));

        boolean ignoreCase = nodeSettings.getAsBoolean("ignore_case", defaultIgnoreCase);

        Path[] affixFiles = FileSystemUtils.files(dicDir, "*.aff");
        if (affixFiles.length == 0) {
            throw new ElasticsearchException(String.format(Locale.ROOT, "Missing affix file for hunspell dictionary [%s]", locale));
        }
        if (affixFiles.length != 1) {
            throw new ElasticsearchException(String.format(Locale.ROOT, "Too many affix files exist for hunspell dictionary [%s]", locale));
        }
        InputStream affixStream = null;

        Path[] dicFiles = FileSystemUtils.files(dicDir, "*.dic");
        List<InputStream> dicStreams = new ArrayList<>(dicFiles.length);
        try {

            for (int i = 0; i < dicFiles.length; i++) {
                dicStreams.add(Files.newInputStream(dicFiles[i]));
            }

            affixStream = Files.newInputStream(affixFiles[0]);

            try (Directory tmp = new SimpleFSDirectory(env.tmpFile())) {
                return new Dictionary(tmp, "hunspell", affixStream, dicStreams, ignoreCase);
            }

        } catch (Exception e) {
            logger.error((Supplier<?>) () -> new ParameterizedMessage("Could not load hunspell dictionary [{}]", locale), e);
            throw e;
        } finally {
            IOUtils.close(affixStream);
            IOUtils.close(dicStreams);
        }
    }

    
    private static Settings loadDictionarySettings(Path dir, Settings defaults) throws IOException {
        Path file = dir.resolve("settings.yml");
        if (Files.exists(file)) {
            return Settings.builder().loadFromPath(file).put(defaults).build();
        }

        file = dir.resolve("settings.json");
        if (Files.exists(file)) {
            return Settings.builder().loadFromPath(file).put(defaults).build();
        }

        return defaults;
    }
}

