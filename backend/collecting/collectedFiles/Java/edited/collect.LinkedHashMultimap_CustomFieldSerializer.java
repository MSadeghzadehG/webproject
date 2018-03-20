

package com.google.common.collect;

import com.google.gwt.user.client.rpc.SerializationException;
import com.google.gwt.user.client.rpc.SerializationStreamReader;
import com.google.gwt.user.client.rpc.SerializationStreamWriter;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;


public class LinkedHashMultimap_CustomFieldSerializer {

  public static void deserialize(SerializationStreamReader in, LinkedHashMultimap<?, ?> out) {}

  public static LinkedHashMultimap<Object, Object> instantiate(SerializationStreamReader stream)
      throws SerializationException {
    LinkedHashMultimap<Object, Object> multimap = LinkedHashMultimap.create();

    int distinctKeys = stream.readInt();
    Map<Object, Collection<Object>> map = new LinkedHashMap<>();
    for (int i = 0; i < distinctKeys; i++) {
      Object key = stream.readObject();
      map.put(key, multimap.createCollection(key));
    }
    int entries = stream.readInt();
    for (int i = 0; i < entries; i++) {
      Object key = stream.readObject();
      Object value = stream.readObject();
      map.get(key).add(value);
    }
    multimap.setMap(map);

    return multimap;
  }

  public static void serialize(SerializationStreamWriter stream, LinkedHashMultimap<?, ?> multimap)
      throws SerializationException {
    stream.writeInt(multimap.keySet().size());
    for (Object key : multimap.keySet()) {
      stream.writeObject(key);
    }
    stream.writeInt(multimap.size());
    for (Entry<?, ?> entry : multimap.entries()) {
      stream.writeObject(entry.getKey());
      stream.writeObject(entry.getValue());
    }
  }
}
