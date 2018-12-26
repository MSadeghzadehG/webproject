

package org.elasticsearch.rest;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.Nullable;
import org.elasticsearch.common.bytes.BytesReference;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

public abstract class RestResponse {

    protected Map<String, List<String>> customHeaders;


    
    public abstract String contentType();

    
    public abstract BytesReference content();

    
    public abstract RestStatus status();

    public void copyHeaders(ElasticsearchException ex) {
        Set<String> headerKeySet = ex.getHeaderKeys();
        if (customHeaders == null) {
            customHeaders = new HashMap<>(headerKeySet.size());
        }
        for (String key : headerKeySet) {
            List<String> values = customHeaders.get(key);
            if (values == null) {
                values = new ArrayList<>();
                customHeaders.put(key, values);
            }
            values.addAll(ex.getHeader(key));
        }
    }

    
    public void addHeader(String name, String value) {
        if (customHeaders == null) {
            customHeaders = new HashMap<>(2);
        }
        List<String> header = customHeaders.get(name);
        if (header == null) {
            header = new ArrayList<>();
            customHeaders.put(name, header);
        }
        header.add(value);
    }

    
    @Nullable
    public Map<String, List<String>> getHeaders() {
        return customHeaders;
    }
}
