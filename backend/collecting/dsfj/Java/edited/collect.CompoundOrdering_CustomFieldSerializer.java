

package com.google.common.collect;

import com.google.gwt.user.client.rpc.SerializationException;
import com.google.gwt.user.client.rpc.SerializationStreamReader;
import com.google.gwt.user.client.rpc.SerializationStreamWriter;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;


public class CompoundOrdering_CustomFieldSerializer {

  public static void deserialize(SerializationStreamReader reader, CompoundOrdering<?> instance) {}

  @SuppressWarnings("unchecked")   public static CompoundOrdering<Object> instantiate(SerializationStreamReader reader)
      throws SerializationException {
    int n = reader.readInt();
    List<Comparator<Object>> comparators = new ArrayList<>(n);
    for (int i = 0; i < n; i++) {
      comparators.add((Comparator<Object>) reader.readObject());
    }
    return new CompoundOrdering<>(comparators);
  }

  public static void serialize(SerializationStreamWriter writer, CompoundOrdering<?> instance)
      throws SerializationException {
    writer.writeInt(instance.comparators.length);
    for (Comparator<?> comparator : instance.comparators) {
      writer.writeObject(comparator);
    }
  }
}
