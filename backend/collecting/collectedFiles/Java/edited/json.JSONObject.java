
package com.alibaba.dubbo.common.json;

import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;


@Deprecated
public class JSONObject implements JSONNode {
    private Map<String, Object> mMap = new HashMap<String, Object>();

    
    public Object get(String key) {
        return mMap.get(key);
    }

    
    public boolean getBoolean(String key, boolean def) {
        Object tmp = mMap.get(key);
        return tmp != null && tmp instanceof Boolean ? (Boolean) tmp : def;
    }

    
    public int getInt(String key, int def) {
        Object tmp = mMap.get(key);
        return tmp != null && tmp instanceof Number ? ((Number) tmp).intValue() : def;
    }

    
    public long getLong(String key, long def) {
        Object tmp = mMap.get(key);
        return tmp != null && tmp instanceof Number ? ((Number) tmp).longValue() : def;
    }

    
    public float getFloat(String key, float def) {
        Object tmp = mMap.get(key);
        return tmp != null && tmp instanceof Number ? ((Number) tmp).floatValue() : def;
    }

    
    public double getDouble(String key, double def) {
        Object tmp = mMap.get(key);
        return tmp != null && tmp instanceof Number ? ((Number) tmp).doubleValue() : def;
    }

    
    public String getString(String key) {
        Object tmp = mMap.get(key);
        return tmp == null ? null : tmp.toString();
    }

    
    public JSONArray getArray(String key) {
        Object tmp = mMap.get(key);
        return tmp == null ? null : tmp instanceof JSONArray ? (JSONArray) tmp : null;
    }

    
    public JSONObject getObject(String key) {
        Object tmp = mMap.get(key);
        return tmp == null ? null : tmp instanceof JSONObject ? (JSONObject) tmp : null;
    }

    
    public Iterator<String> keys() {
        return mMap.keySet().iterator();
    }

    
    public boolean contains(String key) {
        return mMap.containsKey(key);
    }

    
    public void put(String name, Object value) {
        mMap.put(name, value);
    }

    
    public void putAll(String[] names, Object[] values) {
        for (int i = 0, len = Math.min(names.length, values.length); i < len; i++)
            mMap.put(names[i], values[i]);
    }

    
    public void putAll(Map<String, Object> map) {
        for (Map.Entry<String, Object> entry : map.entrySet())
            mMap.put(entry.getKey(), entry.getValue());
    }

    
    public void writeJSON(JSONConverter jc, JSONWriter jb, boolean writeClass) throws IOException {
        String key;
        Object value;
        jb.objectBegin();
        for (Map.Entry<String, Object> entry : mMap.entrySet()) {
            key = entry.getKey();
            jb.objectItem(key);
            value = entry.getValue();
            if (value == null)
                jb.valueNull();
            else
                jc.writeValue(value, jb, writeClass);
        }
        jb.objectEnd();
    }
}