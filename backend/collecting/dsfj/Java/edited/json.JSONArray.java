
package com.alibaba.dubbo.common.json;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;


@Deprecated
public class JSONArray implements JSONNode {
    private List<Object> mArray = new ArrayList<Object>();

    
    public Object get(int index) {
        return mArray.get(index);
    }

    
    public boolean getBoolean(int index, boolean def) {
        Object tmp = mArray.get(index);
        return tmp != null && tmp instanceof Boolean ? ((Boolean) tmp).booleanValue() : def;
    }

    
    public int getInt(int index, int def) {
        Object tmp = mArray.get(index);
        return tmp != null && tmp instanceof Number ? ((Number) tmp).intValue() : def;
    }

    
    public long getLong(int index, long def) {
        Object tmp = mArray.get(index);
        return tmp != null && tmp instanceof Number ? ((Number) tmp).longValue() : def;
    }

    
    public float getFloat(int index, float def) {
        Object tmp = mArray.get(index);
        return tmp != null && tmp instanceof Number ? ((Number) tmp).floatValue() : def;
    }

    
    public double getDouble(int index, double def) {
        Object tmp = mArray.get(index);
        return tmp != null && tmp instanceof Number ? ((Number) tmp).doubleValue() : def;
    }

    
    public String getString(int index) {
        Object tmp = mArray.get(index);
        return tmp == null ? null : tmp.toString();
    }

    
    public JSONArray getArray(int index) {
        Object tmp = mArray.get(index);
        return tmp == null ? null : tmp instanceof JSONArray ? (JSONArray) tmp : null;
    }

    
    public JSONObject getObject(int index) {
        Object tmp = mArray.get(index);
        return tmp == null ? null : tmp instanceof JSONObject ? (JSONObject) tmp : null;
    }

    
    public int length() {
        return mArray.size();
    }

    
    public void add(Object ele) {
        mArray.add(ele);
    }

    
    public void addAll(Object[] eles) {
        for (Object ele : eles)
            mArray.add(ele);
    }

    
    public void addAll(Collection<?> c) {
        mArray.addAll(c);
    }

    
    public void writeJSON(JSONConverter jc, JSONWriter jb, boolean writeClass) throws IOException {
        jb.arrayBegin();
        for (Object item : mArray) {
            if (item == null)
                jb.valueNull();
            else
                jc.writeValue(item, jb, writeClass);
        }
        jb.arrayEnd();
    }
}