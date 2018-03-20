

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.gwt.user.client.rpc.SerializationException;
import com.google.gwt.user.client.rpc.SerializationStreamReader;
import com.google.gwt.user.client.rpc.SerializationStreamWriter;
import com.google.gwt.user.client.rpc.core.java.util.Collection_CustomFieldSerializerBase;
import java.util.ArrayList;


@GwtCompatible(emulated = true)
public class RegularImmutableAsList_CustomFieldSerializer {

  public static void deserialize(
      SerializationStreamReader reader, RegularImmutableAsList<?> instance) {}

  public static RegularImmutableAsList<Object> instantiate(SerializationStreamReader reader)
      throws SerializationException {
    ArrayList<Object> elements = new ArrayList<>();
    Collection_CustomFieldSerializerBase.deserialize(reader, elements);
    ImmutableList<Object> delegate = ImmutableList.copyOf(elements);
    return new RegularImmutableAsList<>(delegate, delegate);
  }

  public static void serialize(SerializationStreamWriter writer, RegularImmutableAsList<?> instance)
      throws SerializationException {
    Collection_CustomFieldSerializerBase.serialize(writer, instance);
  }
}
