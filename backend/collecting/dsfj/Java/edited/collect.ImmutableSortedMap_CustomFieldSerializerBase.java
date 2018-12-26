

package com.google.common.collect;

import com.google.gwt.user.client.rpc.SerializationException;
import com.google.gwt.user.client.rpc.SerializationStreamReader;
import com.google.gwt.user.client.rpc.SerializationStreamWriter;
import com.google.gwt.user.client.rpc.core.java.util.Map_CustomFieldSerializerBase;
import java.util.Comparator;
import java.util.SortedMap;
import java.util.TreeMap;


final class ImmutableSortedMap_CustomFieldSerializerBase {
  static ImmutableSortedMap<Object, Object> instantiate(SerializationStreamReader reader)
      throws SerializationException {
    
    @SuppressWarnings("unchecked")
    Comparator<Object> comparator = (Comparator<Object>) reader.readObject();

    SortedMap<Object, Object> entries = new TreeMap<>(comparator);
    Map_CustomFieldSerializerBase.deserialize(reader, entries);

    return ImmutableSortedMap.orderedBy(comparator).putAll(entries).build();
  }

  static void serialize(SerializationStreamWriter writer, ImmutableSortedMap<?, ?> instance)
      throws SerializationException {
    writer.writeObject(instance.comparator());

    Map_CustomFieldSerializerBase.serialize(writer, instance);
  }

  private ImmutableSortedMap_CustomFieldSerializerBase() {}
}
