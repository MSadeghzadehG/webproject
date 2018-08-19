

package org.elasticsearch.bootstrap;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

public class JavaVersion implements Comparable<JavaVersion> {

    private final List<Integer> version;

    public List<Integer> getVersion() {
        return version;
    }

    private JavaVersion(List<Integer> version) {
        if (version.size() >= 2 && version.get(0) == 1 && version.get(1) == 8) {
                                    version = new ArrayList<>(version.subList(1, version.size()));
        }
        this.version = Collections.unmodifiableList(version);
    }

    public static JavaVersion parse(String value) {
        Objects.requireNonNull(value);
        if (!isValid(value)) {
            throw new IllegalArgumentException("value");
        }

        List<Integer> version = new ArrayList<>();
        String[] components = value.split("\\.");
        for (String component : components) {
            version.add(Integer.valueOf(component));
        }

        return new JavaVersion(version);
    }

    public static boolean isValid(String value) {
        return value.matches("^0*[0-9]+(\\.[0-9]+)*$");
    }

    private static final JavaVersion CURRENT = parse(System.getProperty("java.specification.version"));

    public static JavaVersion current() {
        return CURRENT;
    }

    @Override
    public int compareTo(JavaVersion o) {
        int len = Math.max(version.size(), o.version.size());
        for (int i = 0; i < len; i++) {
            int d = (i < version.size() ? version.get(i) : 0);
            int s = (i < o.version.size() ? o.version.get(i) : 0);
            if (s < d)
                return 1;
            if (s > d)
                return -1;
        }
        return 0;
    }

    @Override
    public boolean equals(Object o) {
        if (o == null || o.getClass() != getClass()) {
            return false;
        }
        return compareTo((JavaVersion) o) == 0;
    }

    @Override
    public int hashCode() {
        return version.hashCode();
    }

    @Override
    public String toString() {
        return version.stream().map(v -> Integer.toString(v)).collect(Collectors.joining("."));
    }
}
