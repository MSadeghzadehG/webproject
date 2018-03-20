

package org.elasticsearch.painless.spi;

import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.nio.charset.StandardCharsets;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


public final class WhitelistLoader {

    
    public static Whitelist loadFromResourceFiles(Class<?> resource, String... filepaths) {
        List<Whitelist.Struct> whitelistStructs = new ArrayList<>();

                        for (String filepath : filepaths) {
            String line;
            int number = -1;

            try (LineNumberReader reader = new LineNumberReader(
                new InputStreamReader(resource.getResourceAsStream(filepath), StandardCharsets.UTF_8))) {

                String whitelistStructOrigin = null;
                String javaClassName = null;
                boolean onlyFQNJavaClassName = false;
                List<Whitelist.Constructor> whitelistConstructors = null;
                List<Whitelist.Method> whitelistMethods = null;
                List<Whitelist.Field> whitelistFields = null;

                while ((line = reader.readLine()) != null) {
                    number = reader.getLineNumber();
                    line = line.trim();

                                        if (line.length() == 0 || line.charAt(0) == '#') {
                        continue;
                    }

                                                            if (line.startsWith("class ")) {
                                                if (line.endsWith("{") == false) {
                            throw new IllegalArgumentException(
                                "invalid struct definition: failed to parse class opening bracket [" + line + "]");
                        }

                                                String[] tokens = line.substring(5, line.length() - 1).trim().split("\\s+");

                                                if (tokens.length == 2 && "only_fqn".equals(tokens[1])) {
                            onlyFQNJavaClassName = true;
                        } else if (tokens.length != 1) {
                            throw new IllegalArgumentException("invalid struct definition: failed to parse class name [" + line + "]");
                        }

                        whitelistStructOrigin = "[" + filepath + "]:[" + number + "]";
                        javaClassName = tokens[0];

                                                whitelistConstructors = new ArrayList<>();
                        whitelistMethods = new ArrayList<>();
                        whitelistFields = new ArrayList<>();

                                                                                } else if (line.equals("}")) {
                        if (javaClassName == null) {
                            throw new IllegalArgumentException("invalid struct definition: extraneous closing bracket");
                        }

                        whitelistStructs.add(new Whitelist.Struct(whitelistStructOrigin, javaClassName, onlyFQNJavaClassName,
                            whitelistConstructors, whitelistMethods, whitelistFields));

                                                whitelistStructOrigin = null;
                        javaClassName = null;
                        onlyFQNJavaClassName = false;
                        whitelistConstructors = null;
                        whitelistMethods = null;
                        whitelistFields = null;

                                        } else {
                                                String origin = "[" + filepath + "]:[" + number + "]";

                                                if (javaClassName == null) {
                            throw new IllegalArgumentException("invalid object definition: expected a class name [" + line + "]");
                        }

                                                                        if (line.startsWith("(")) {
                                                        if (line.endsWith(")") == false) {
                                throw new IllegalArgumentException(
                                    "invalid constructor definition: expected a closing parenthesis [" + line + "]");
                            }

                                                        String[] tokens = line.substring(1, line.length() - 1).replaceAll("\\s+", "").split(",");

                                                        if ("".equals(tokens[0])) {
                                tokens = new String[0];
                            }

                            whitelistConstructors.add(new Whitelist.Constructor(origin, Arrays.asList(tokens)));

                                                                        } else if (line.contains("(")) {
                                                        if (line.endsWith(")") == false) {
                                throw new IllegalArgumentException(
                                    "invalid method definition: expected a closing parenthesis [" + line + "]");
                            }

                                                        int parameterIndex = line.indexOf('(');
                            String[] tokens = line.trim().substring(0, parameterIndex).split("\\s+");

                            String javaMethodName;
                            String javaAugmentedClassName;

                                                        if (tokens.length == 2) {
                                javaMethodName = tokens[1];
                                javaAugmentedClassName = null;
                            } else if (tokens.length == 3) {
                                javaMethodName = tokens[2];
                                javaAugmentedClassName = tokens[1];
                            } else {
                                throw new IllegalArgumentException("invalid method definition: unexpected format [" + line + "]");
                            }

                            String painlessReturnTypeName = tokens[0];

                                                        tokens = line.substring(parameterIndex + 1, line.length() - 1).replaceAll("\\s+", "").split(",");

                                                        if ("".equals(tokens[0])) {
                                tokens = new String[0];
                            }

                            whitelistMethods.add(new Whitelist.Method(origin, javaAugmentedClassName, javaMethodName,
                                painlessReturnTypeName, Arrays.asList(tokens)));

                                                                        } else {
                                                        String[] tokens = line.split("\\s+");

                                                        if (tokens.length != 2) {
                                throw new IllegalArgumentException("invalid field definition: unexpected format [" + line + "]");
                            }

                            whitelistFields.add(new Whitelist.Field(origin, tokens[1], tokens[0]));
                        }
                    }
                }

                                if (javaClassName != null) {
                    throw new IllegalArgumentException("invalid struct definition: expected closing bracket");
                }
            } catch (Exception exception) {
                throw new RuntimeException("error in [" + filepath + "] at line [" + number + "]", exception);
            }
        }
        ClassLoader loader = AccessController.doPrivileged((PrivilegedAction<ClassLoader>)resource::getClassLoader);

        return new Whitelist(loader, whitelistStructs);
    }

    private WhitelistLoader() {}
}
