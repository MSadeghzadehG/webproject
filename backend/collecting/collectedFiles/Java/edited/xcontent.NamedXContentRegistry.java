

package org.elasticsearch.common.xcontent;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.CheckedFunction;
import org.elasticsearch.common.ParseField;
import org.elasticsearch.common.ParsingException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import static java.util.Collections.emptyList;
import static java.util.Collections.emptyMap;
import static java.util.Collections.unmodifiableMap;

public class NamedXContentRegistry {
    
    public static final NamedXContentRegistry EMPTY = new NamedXContentRegistry(emptyList());

    
    public static class Entry {
        
        public final Class<?> categoryClass;

        
        public final ParseField name;

        
        private final ContextParser<Object, ?> parser;

        
        public <T> Entry(Class<T> categoryClass, ParseField name, CheckedFunction<XContentParser, ? extends T, IOException> parser) {
            this.categoryClass = Objects.requireNonNull(categoryClass);
            this.name = Objects.requireNonNull(name);
            this.parser = Objects.requireNonNull((p, c) -> parser.apply(p));
        }
        
        public <T> Entry(Class<T> categoryClass, ParseField name, ContextParser<Object, ? extends T> parser) {
            this.categoryClass = Objects.requireNonNull(categoryClass);
            this.name = Objects.requireNonNull(name);
            this.parser = Objects.requireNonNull(parser);
        }
    }

    private final Map<Class<?>, Map<String, Entry>> registry;

    public NamedXContentRegistry(List<Entry> entries) {
        if (entries.isEmpty()) {
            registry = emptyMap();
            return;
        }
        entries = new ArrayList<>(entries);
        entries.sort((e1, e2) -> e1.categoryClass.getName().compareTo(e2.categoryClass.getName()));

        Map<Class<?>, Map<String, Entry>> registry = new HashMap<>();
        Map<String, Entry> parsers = null;
        Class<?> currentCategory = null;
        for (Entry entry : entries) {
            if (currentCategory != entry.categoryClass) {
                if (currentCategory != null) {
                                        registry.put(currentCategory, unmodifiableMap(parsers));
                }
                parsers = new HashMap<>();
                currentCategory = entry.categoryClass;
            }

            for (String name : entry.name.getAllNamesIncludedDeprecated()) {
                Object old = parsers.put(name, entry);
                if (old != null) {
                    throw new IllegalArgumentException("NamedXContent [" + currentCategory.getName() + "][" + entry.name + "]" +
                        " is already registered for [" + old.getClass().getName() + "]," +
                        " cannot register [" + entry.parser.getClass().getName() + "]");
                }
            }
        }
                registry.put(currentCategory, unmodifiableMap(parsers));

        this.registry = unmodifiableMap(registry);
    }

    
    public <T, C> T parseNamedObject(Class<T> categoryClass, String name, XContentParser parser, C context) throws IOException {
        Map<String, Entry> parsers = registry.get(categoryClass);
        if (parsers == null) {
            if (registry.isEmpty()) {
                                throw new ElasticsearchException("namedObject is not supported for this parser");
            }
            throw new ElasticsearchException("Unknown namedObject category [" + categoryClass.getName() + "]");
        }
        Entry entry = parsers.get(name);
        if (entry == null) {
            throw new UnknownNamedObjectException(parser.getTokenLocation(), categoryClass, name);
        }
        if (false == entry.name.match(name, parser.getDeprecationHandler())) {
            
            throw new ParsingException(parser.getTokenLocation(),
                    "Unknown " + categoryClass.getSimpleName() + " [" + name + "]: Parser didn't match");
        }
        return categoryClass.cast(entry.parser.parse(parser, context));
    }

}
