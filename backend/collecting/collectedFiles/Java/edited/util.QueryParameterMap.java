
package hudson.util;

import javax.servlet.http.HttpServletRequest;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class QueryParameterMap {
    private final Map<String,List<String>> store = new HashMap<String, List<String>>();

    
    public QueryParameterMap(String queryString) {
        if (queryString==null || queryString.length()==0)   return;
        try {
            for (String param : queryString.split("&")) {
                String[] kv = param.split("=");
                String key = URLDecoder.decode(kv[0], "UTF-8");
                String value = URLDecoder.decode(kv[1], "UTF-8");
                List<String> values = store.get(key);
                if (values == null)
                    store.put(key, values = new ArrayList<String>());
                values.add(value);
            }
        } catch (UnsupportedEncodingException e) {
            throw new AssertionError(e);
        }
    }

    public QueryParameterMap(HttpServletRequest req) {
        this(req.getQueryString());
    }

    public String get(String name) {
        List<String> v = store.get(name);
        return v!=null?v.get(0):null;
    }

    public List<String> getAll(String name) {
        List<String> v = store.get(name);
        return v!=null? Collections.unmodifiableList(v) : Collections.<String>emptyList();
    }
}
