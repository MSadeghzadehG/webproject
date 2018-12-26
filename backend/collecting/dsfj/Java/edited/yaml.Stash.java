

package org.elasticsearch.test.rest.yaml;

import org.apache.logging.log4j.Logger;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.logging.Loggers;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class Stash implements ToXContentFragment {
    private static final Pattern EXTENDED_KEY = Pattern.compile("\\$\\{([^}]+)\\}");
    private static final Pattern PATH = Pattern.compile("\\$_path");

    private static final Logger logger = Loggers.getLogger(Stash.class);

    public static final Stash EMPTY = new Stash();

    private final Map<String, Object> stash = new HashMap<>();
    private final ObjectPath stashObjectPath = new ObjectPath(stash);

    
    public void stashValue(String key, Object value) {
        logger.trace("stashing [{}]=[{}]", key, value);
        Object old = stash.put(key, value);
        if (old != null && old != value) {
            logger.trace("replaced stashed value [{}] with same key [{}]", old, key);
        }
    }

    
    public void clear() {
        stash.clear();
    }

    
    public boolean containsStashedValue(Object key) {
        if (key == null || false == key instanceof CharSequence) {
            return false;
        }
        String stashKey = key.toString();
        if (false == Strings.hasLength(stashKey)) {
            return false;
        }
        if (stashKey.startsWith("$")) {
            return true;
        }
        return EXTENDED_KEY.matcher(stashKey).find();
    }

    
    public Object getValue(String key) throws IOException {
        if (key.charAt(0) == '$' && key.charAt(1) != '{') {
            return unstash(key.substring(1));
        }
        Matcher matcher = EXTENDED_KEY.matcher(key);
        
        StringBuffer result = new StringBuffer(key.length());
        if (false == matcher.find()) {
            throw new IllegalArgumentException("Doesn't contain any stash keys [" + key + "]");
        }
        do {
            matcher.appendReplacement(result, Matcher.quoteReplacement(unstash(matcher.group(1)).toString()));
        } while (matcher.find());
        matcher.appendTail(result);
        return result.toString();
    }

    private Object unstash(String key) throws IOException {
        Object stashedValue = stashObjectPath.evaluate(key);
        if (stashedValue == null) {
            throw new IllegalArgumentException("stashed value not found for key [" + key + "]");
        }
        return stashedValue;
    }

    
    @SuppressWarnings("unchecked")     public Map<String, Object> replaceStashedValues(Map<String, Object> map) throws IOException {
        return (Map<String, Object>) unstashObject(new ArrayList<>(), map);
    }

    private Object unstashObject(List<Object> path, Object obj) throws IOException {
        if (obj instanceof List) {
            List<?> list = (List<?>) obj;
            List<Object> result = new ArrayList<>();
            int index = 0;
            for (Object o : list) {
                path.add(index++);
                if (containsStashedValue(o)) {
                    result.add(getValue(path, o.toString()));
                } else {
                    result.add(unstashObject(path, o));
                }
                path.remove(path.size() - 1);
            }
            return result;
        }
        if (obj instanceof Map) {
            Map<?, ?> map = (Map<?, ?>) obj;
            Map<String, Object> result = new HashMap<>();
            for (Map.Entry<?, ?> entry : map.entrySet()) {
                String key = (String) entry.getKey();
                Object value = entry.getValue();
                if (containsStashedValue(key)) {
                    key = getValue(key).toString();
                }
                path.add(key);
                if (containsStashedValue(value)) {
                    value = getValue(path, value.toString());
                } else {
                    value = unstashObject(path, value);
                }
                path.remove(path.size() - 1);
                if (null != result.putIfAbsent(key, value)) {
                    throw new IllegalArgumentException("Unstashing has caused a key conflict! The map is [" + result + "] and the key is ["
                            + entry.getKey() + "] which unstashes to [" + key + "]");
                }
            }
            return result;
        }
        return obj;
    }

    
    private Object getValue(List<Object> path, String key) throws IOException {
        Matcher matcher = PATH.matcher(key);
        if (false == matcher.find()) {
            return getValue(key);
        }
        StringBuilder pathBuilder = new StringBuilder();
        Iterator<Object> element = path.iterator();
        if (element.hasNext()) {
            pathBuilder.append(element.next());
            while (element.hasNext()) {
                pathBuilder.append('.');
                pathBuilder.append(element.next());
            }
        }
        String builtPath = Matcher.quoteReplacement(pathBuilder.toString());
        StringBuffer newKey = new StringBuffer(key.length());
        do {
            matcher.appendReplacement(newKey, builtPath);
        } while (matcher.find());
        matcher.appendTail(newKey);
        return getValue(newKey.toString());
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.field("stash", stash);
        return builder;
    }
}
