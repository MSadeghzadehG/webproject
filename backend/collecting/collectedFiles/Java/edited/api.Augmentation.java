

package org.elasticsearch.painless.api;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Base64;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.function.BiConsumer;
import java.util.function.BiFunction;
import java.util.function.BiPredicate;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.ObjIntConsumer;
import java.util.function.Predicate;
import java.util.function.ToDoubleFunction;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class Augmentation {
    
        private Augmentation() {}

    
    public static <T> int getLength(List<T> receiver) {
        return receiver.size();
    }

    
    public static String namedGroup(Matcher receiver, String name) {
        return receiver.group(name);
    }
    
            
    
    public static <T> boolean any(Iterable<T> receiver, Predicate<T> predicate) {
        for (T t : receiver) {
            if (predicate.test(t)) {
                return true;
            }
        }
        return false;
    }
    
    
    public static <T> Collection<T> asCollection(Iterable<T> receiver) {
        if (receiver instanceof Collection) {
            return (Collection<T>)receiver;
        }
        List<T> list = new ArrayList<>();
        for (T t : receiver) {
            list.add(t);
        }
        return list;
    }
    
    
    public static <T> List<T> asList(Iterable<T> receiver) {
        if (receiver instanceof List) {
            return (List<T>)receiver;
        }
        List<T> list = new ArrayList<>();
        for (T t : receiver) {
            list.add(t);
        }
        return list;
    }
    
     
    public static <T> int count(Iterable<T> receiver, Predicate<T> predicate) {
        int count = 0;
        for (T t : receiver) {
            if (predicate.test(t)) {
                count++;
            }
        }
        return count;
    }
    
        
    
    public static <T> Object each(Iterable<T> receiver, Consumer<T> consumer) {
        receiver.forEach(consumer);
        return receiver;
    }
    
    
    public static <T> Object eachWithIndex(Iterable<T> receiver, ObjIntConsumer<T> consumer) {
        int count = 0;
        for (T t : receiver) {
            consumer.accept(t, count++);
        }
        return receiver;
    }
    
    
    public static <T> boolean every(Iterable<T> receiver, Predicate<T> predicate) {
        for (T t : receiver) {
            if (predicate.test(t) == false) {
                return false;
            }
        }
        return true;
    }
    
    
    public static <T,U> List<U> findResults(Iterable<T> receiver, Function<T,U> filter) {
        List<U> list = new ArrayList<>();
        for (T t: receiver) {
           U result = filter.apply(t);
           if (result != null) {
               list.add(result);
           }
        }
        return list;
    }
    
    
    public static <T,U> Map<U,List<T>> groupBy(Iterable<T> receiver, Function<T,U> mapper) {
        Map<U,List<T>> map = new LinkedHashMap<>();
        for (T t : receiver) {
            U mapped = mapper.apply(t);
            List<T> results = map.get(mapped);
            if (results == null) {
                results = new ArrayList<>();
                map.put(mapped, results);
            }
            results.add(t);
        }
        return map;
    }
    
    
    public static <T> String join(Iterable<T> receiver, String separator) {
        StringBuilder sb = new StringBuilder();
        for (T t : receiver) {
            if (sb.length() > 0) {
                sb.append(separator);
            }
            sb.append(t);
        }
        return sb.toString();
    }
    
    
    public static <T extends Number> double sum(Iterable<T> receiver) {
        double sum = 0;
        for (T t : receiver) {
            sum += t.doubleValue();
        }
        return sum;
    }
    
    
    public static <T> double sum(Iterable<T> receiver, ToDoubleFunction<T> function) {
        double sum = 0;
        for (T t : receiver) {
            sum += function.applyAsDouble(t);
        }
        return sum;
    }
    
            
    
    public static <T,U> List<U> collect(Collection<T> receiver, Function<T,U> function) {
        List<U> list = new ArrayList<>();
        for (T t : receiver) {
            list.add(function.apply(t));
        }
        return list;
    }
    
    
    public static <T,U> Object collect(Collection<T> receiver, Collection<U> collection, Function<T,U> function) {
        for (T t : receiver) {
            collection.add(function.apply(t));
        }
        return collection;
    }
    
    
    public static <T> T find(Collection<T> receiver, Predicate<T> predicate) {
        for (T t : receiver) {
            if (predicate.test(t)) {
                return t;
            }
        }
        return null;
    }
    
    
    public static <T> List<T> findAll(Collection<T> receiver, Predicate<T> predicate) {
        List<T> list = new ArrayList<>();
        for (T t : receiver) {
            if (predicate.test(t)) {
                list.add(t);
            }
        }
        return list;
    }
    
    
    public static <T,U> Object findResult(Collection<T> receiver, Function<T,U> function) {
        return findResult(receiver, null, function);
    }
    
    
    public static <T,U> Object findResult(Collection<T> receiver, Object defaultResult, Function<T,U> function) {
        for (T t : receiver) {
            U value = function.apply(t);
            if (value != null) {
                return value;
            }
        }
        return defaultResult;
    }
    
    
    public static <T> List<List<T>> split(Collection<T> receiver, Predicate<T> predicate) {
        List<T> matched = new ArrayList<>();
        List<T> unmatched = new ArrayList<>();
        List<List<T>> result = new ArrayList<>(2);
        result.add(matched);
        result.add(unmatched);
        for (T t : receiver) {
            if (predicate.test(t)) {
                matched.add(t);
            } else {
                unmatched.add(t);
            }
        }
        return result;
    }
    
            
    
    public static <K,V,T> List<T> collect(Map<K,V> receiver, BiFunction<K,V,T> function) {
        List<T> list = new ArrayList<>();
        for (Map.Entry<K,V> kvPair : receiver.entrySet()) {
            list.add(function.apply(kvPair.getKey(), kvPair.getValue()));
        }
        return list;
    }
    
    
    public static <K,V,T> Object collect(Map<K,V> receiver, Collection<T> collection, BiFunction<K,V,T> function) {
        for (Map.Entry<K,V> kvPair : receiver.entrySet()) {
            collection.add(function.apply(kvPair.getKey(), kvPair.getValue()));
        }
        return collection;
    }
    
     
    public static <K,V> int count(Map<K,V> receiver, BiPredicate<K,V> predicate) {
        int count = 0;
        for (Map.Entry<K,V> kvPair : receiver.entrySet()) {
            if (predicate.test(kvPair.getKey(), kvPair.getValue())) {
                count++;
            }
        }
        return count;
    }
    
    
    public static <K,V> Object each(Map<K,V> receiver, BiConsumer<K,V> consumer) {
        receiver.forEach(consumer);
        return receiver;
    }
    
    
    public static <K,V> boolean every(Map<K,V> receiver, BiPredicate<K,V> predicate) {
        for (Map.Entry<K,V> kvPair : receiver.entrySet()) {
            if (predicate.test(kvPair.getKey(), kvPair.getValue()) == false) {
                return false;
            }
        }
        return true;
    }
    
    
    public static <K,V> Map.Entry<K,V> find(Map<K,V> receiver, BiPredicate<K,V> predicate) {
        for (Map.Entry<K,V> kvPair : receiver.entrySet()) {
            if (predicate.test(kvPair.getKey(), kvPair.getValue())) {
                return kvPair;
            }
        }
        return null;
    }
    
    
    public static <K,V> Map<K,V> findAll(Map<K,V> receiver, BiPredicate<K,V> predicate) {
                final Map<K,V> map;
        if (receiver instanceof TreeMap) {
            map = new TreeMap<>();
        } else {
            map = new LinkedHashMap<>();
        }
        for (Map.Entry<K,V> kvPair : receiver.entrySet()) {
            if (predicate.test(kvPair.getKey(), kvPair.getValue())) {
                map.put(kvPair.getKey(), kvPair.getValue());
            }
        }
        return map;
    }
    
    
    public static <K,V,T> Object findResult(Map<K,V> receiver, BiFunction<K,V,T> function) {
        return findResult(receiver, null, function);
    }
    
    
    public static <K,V,T> Object findResult(Map<K,V> receiver, Object defaultResult, BiFunction<K,V,T> function) {
        for (Map.Entry<K,V> kvPair : receiver.entrySet()) {
            T value = function.apply(kvPair.getKey(), kvPair.getValue());
            if (value != null) {
                return value;
            }
        }
        return defaultResult;
    }
    
    
    public static <K,V,T> List<T> findResults(Map<K,V> receiver, BiFunction<K,V,T> filter) {
        List<T> list = new ArrayList<>();
        for (Map.Entry<K,V> kvPair : receiver.entrySet()) {
           T result = filter.apply(kvPair.getKey(), kvPair.getValue());
           if (result != null) {
               list.add(result);
           }
        }
        return list;
    }
    
    
    public static <K,V,T> Map<T,Map<K,V>> groupBy(Map<K,V> receiver, BiFunction<K,V,T> mapper) {
        Map<T,Map<K,V>> map = new LinkedHashMap<>();
        for (Map.Entry<K,V> kvPair : receiver.entrySet()) {
            T mapped = mapper.apply(kvPair.getKey(), kvPair.getValue());
            Map<K,V> results = map.get(mapped);
            if (results == null) {
                                if (receiver instanceof TreeMap) {
                    results = new TreeMap<>();
                } else {
                    results = new LinkedHashMap<>();
                }
                map.put(mapped, results);
            }
            results.put(kvPair.getKey(), kvPair.getValue());
        }
        return map;
    }

        
    public static String replaceAll(CharSequence receiver, Pattern pattern, Function<Matcher, String> replacementBuilder) {
        Matcher m = pattern.matcher(receiver);
        if (false == m.find()) {
                        return receiver.toString();
        }
        StringBuffer result = new StringBuffer(initialBufferForReplaceWith(receiver));
        do {
            m.appendReplacement(result, Matcher.quoteReplacement(replacementBuilder.apply(m)));
        } while (m.find());
        m.appendTail(result);
        return result.toString();
    }

    
    public static String replaceFirst(CharSequence receiver, Pattern pattern, Function<Matcher, String> replacementBuilder) {
        Matcher m = pattern.matcher(receiver);
        if (false == m.find()) {
                        return receiver.toString();
        }
        StringBuffer result = new StringBuffer(initialBufferForReplaceWith(receiver));
        m.appendReplacement(result, Matcher.quoteReplacement(replacementBuilder.apply(m)));
        m.appendTail(result);
        return result.toString();
    }

    
    private static int initialBufferForReplaceWith(CharSequence seq) {
        return seq.length() + 16;
    }

    
    public static String encodeBase64(String receiver) {
        return Base64.getEncoder().encodeToString(receiver.getBytes(StandardCharsets.UTF_8));
    }

    
    public static String decodeBase64(String receiver) {
        return new String(Base64.getDecoder().decode(receiver.getBytes(StandardCharsets.UTF_8)), StandardCharsets.UTF_8);
    }
}
