

package com.google.common.collect;

import com.google.gwt.user.client.rpc.SerializationException;
import com.google.gwt.user.client.rpc.SerializationStreamReader;
import com.google.gwt.user.client.rpc.SerializationStreamWriter;
import com.google.gwt.user.client.rpc.core.java.util.Map_CustomFieldSerializerBase;
import java.util.Map;


public class ImmutableEnumMap_CustomFieldSerializer {

  public static void deserialize(
      SerializationStreamReader reader, ImmutableEnumMap<?, ?> instance) {}

  public static <K extends Enum<K>, V> ImmutableEnumMap<?, ?> instantiate(
      SerializationStreamReader reader) throws SerializationException {
    Map<K, V> deserialized = Maps.newHashMap();
    Map_CustomFieldSerializerBase.deserialize(reader, deserialized);
    
    return (ImmutableEnumMap<?, ?>) Maps.immutableEnumMap(deserialized);
  }

  public static void serialize(SerializationStreamWriter writer, ImmutableEnumMap<?, ?> instance)
      throws SerializationException {
    Map_CustomFieldSerializerBase.serialize(writer, instance);
  }
}
