

package org.elasticsearch.painless.spi;

import java.util.Collections;
import java.util.List;
import java.util.Objects;


public final class Whitelist {

    private static final String[] BASE_WHITELIST_FILES = new String[] {
        "org.elasticsearch.txt",
        "java.lang.txt",
        "java.math.txt",
        "java.text.txt",
        "java.time.txt",
        "java.time.chrono.txt",
        "java.time.format.txt",
        "java.time.temporal.txt",
        "java.time.zone.txt",
        "java.util.txt",
        "java.util.function.txt",
        "java.util.regex.txt",
        "java.util.stream.txt",
        "joda.time.txt"
    };

    public static final List<Whitelist> BASE_WHITELISTS =
        Collections.singletonList(WhitelistLoader.loadFromResourceFiles(Whitelist.class, BASE_WHITELIST_FILES));

    
    public static final class Struct {

        
        public final String origin;

        
        public final String javaClassName;

        
        public final boolean onlyFQNJavaClassName;

        
        public final List<Constructor> whitelistConstructors;

        
        public final List<Method> whitelistMethods;

        
        public final List<Field> whitelistFields;

        
        public Struct(String origin, String javaClassName, boolean onlyFQNJavaClassName,
                      List<Constructor> whitelistConstructors, List<Method> whitelistMethods, List<Field> whitelistFields) {
            this.origin = Objects.requireNonNull(origin);
            this.javaClassName = Objects.requireNonNull(javaClassName);
            this.onlyFQNJavaClassName = onlyFQNJavaClassName;

            this.whitelistConstructors = Collections.unmodifiableList(Objects.requireNonNull(whitelistConstructors));
            this.whitelistMethods = Collections.unmodifiableList(Objects.requireNonNull(whitelistMethods));
            this.whitelistFields = Collections.unmodifiableList(Objects.requireNonNull(whitelistFields));
        }
    }

    
    public static final class Constructor {

        
        public final String origin;

        
        public final List<String> painlessParameterTypeNames;

        
        public Constructor(String origin, List<String> painlessParameterTypeNames) {
            this.origin = Objects.requireNonNull(origin);
            this.painlessParameterTypeNames = Collections.unmodifiableList(Objects.requireNonNull(painlessParameterTypeNames));
        }
    }

    
    public static class Method {

        
        public final String origin;

        
        public final String javaAugmentedClassName;

        
        public final String javaMethodName;

        
        public final String painlessReturnTypeName;

        
        public final List<String> painlessParameterTypeNames;

        
        public Method(String origin, String javaAugmentedClassName, String javaMethodName,
                      String painlessReturnTypeName, List<String> painlessParameterTypeNames) {
            this.origin = Objects.requireNonNull(origin);
            this.javaAugmentedClassName = javaAugmentedClassName;
            this.javaMethodName = javaMethodName;
            this.painlessReturnTypeName = Objects.requireNonNull(painlessReturnTypeName);
            this.painlessParameterTypeNames = Collections.unmodifiableList(Objects.requireNonNull(painlessParameterTypeNames));
        }
    }

    
    public static class Field {

        
        public final String origin;

        
        public final String javaFieldName;

        
        public final String painlessFieldTypeName;

        
        public Field(String origin, String javaFieldName, String painlessFieldTypeName) {
            this.origin = Objects.requireNonNull(origin);
            this.javaFieldName = Objects.requireNonNull(javaFieldName);
            this.painlessFieldTypeName = Objects.requireNonNull(painlessFieldTypeName);
        }
    }

    
    public final ClassLoader javaClassLoader;

    
    public final List<Struct> whitelistStructs;

    
    public Whitelist(ClassLoader javaClassLoader, List<Struct> whitelistStructs) {
        this.javaClassLoader = Objects.requireNonNull(javaClassLoader);
        this.whitelistStructs = Collections.unmodifiableList(Objects.requireNonNull(whitelistStructs));
    }
}
