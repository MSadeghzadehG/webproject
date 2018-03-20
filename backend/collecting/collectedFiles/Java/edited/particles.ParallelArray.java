

package com.badlogic.gdx.graphics.g3d.particles;

import java.util.Arrays;

import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.reflect.ArrayReflection;


public class ParallelArray {

	
	public static class ChannelDescriptor {
		public int id;
		public Class<?> type;
		public int count;

		public ChannelDescriptor (int id, Class<?> type, int count) {
			this.id = id;
			this.type = type;
			this.count = count;
		}
	}

	
	public abstract class Channel {
		public int id;
		public Object data;
		public int strideSize;

		public Channel (int id, Object data, int strideSize) {
			this.id = id;
			this.strideSize = strideSize;
			this.data = data;
		}

		public abstract void add (int index, Object... objects);

		public abstract void swap (int i, int k);

		protected abstract void setCapacity (int requiredCapacity);
	}

	
	public static interface ChannelInitializer<T extends Channel> {
		public void init (T channel);
	}

	public class FloatChannel extends Channel {
		public float[] data;

		public FloatChannel (int id, int strideSize, int size) {
			super(id, new float[size * strideSize], strideSize);
			this.data = (float[])super.data;
		}

		@Override
		public void add (int index, Object... objects) {
			for (int i = strideSize * size, c = i + strideSize, k = 0; i < c; ++i, ++k) {
				data[i] = (Float)objects[k];
			}
		}

		@Override
		public void swap (int i, int k) {
			float t;
			i = strideSize * i;
			k = strideSize * k;
			for (int c = i + strideSize; i < c; ++i, ++k) {
				t = data[i];
				data[i] = data[k];
				data[k] = t;
			}
		}

		@Override
		public void setCapacity (int requiredCapacity) {
			float[] newData = new float[strideSize * requiredCapacity];
			System.arraycopy(data, 0, newData, 0, Math.min(data.length, newData.length));
			super.data = data = newData;
		}
	}

	public class IntChannel extends Channel {
		public int[] data;

		public IntChannel (int id, int strideSize, int size) {
			super(id, new int[size * strideSize], strideSize);
			this.data = (int[])super.data;
		}

		@Override
		public void add (int index, Object... objects) {
			for (int i = strideSize * size, c = i + strideSize, k = 0; i < c; ++i, ++k) {
				data[i] = (Integer)objects[k];
			}
		}

		@Override
		public void swap (int i, int k) {
			int t;
			i = strideSize * i;
			k = strideSize * k;
			for (int c = i + strideSize; i < c; ++i, ++k) {
				t = data[i];
				data[i] = data[k];
				data[k] = t;
			}
		}

		@Override
		public void setCapacity (int requiredCapacity) {
			int[] newData = new int[strideSize * requiredCapacity];
			System.arraycopy(data, 0, newData, 0, Math.min(data.length, newData.length));
			super.data = data = newData;
		}
	}

	@SuppressWarnings("unchecked")
	public class ObjectChannel<T> extends Channel {
		Class<T> componentType;
		public T[] data;

		public ObjectChannel (int id, int strideSize, int size, Class<T> type) {
			super(id, ArrayReflection.newInstance(type, size * strideSize), strideSize);
			componentType = type;
			this.data = (T[])super.data;
		}

		@Override
		public void add (int index, Object... objects) {
			for (int i = strideSize * size, c = i + strideSize, k = 0; i < c; ++i, ++k) {
				this.data[i] = (T)objects[k];
			}
		}

		@Override
		public void swap (int i, int k) {
			T t;
			i = strideSize * i;
			k = strideSize * k;
			for (int c = i + strideSize; i < c; ++i, ++k) {
				t = data[i];
				data[i] = data[k];
				data[k] = t;
			}
		}

		@Override
		public void setCapacity (int requiredCapacity) {
			T[] newData = (T[])ArrayReflection.newInstance(componentType, strideSize * requiredCapacity);
			System.arraycopy(data, 0, newData, 0, Math.min(data.length, newData.length));
			super.data = data = newData;
		}
	}

	
	Array<Channel> arrays;
	
	public int capacity;
	
	public int size;

	public ParallelArray (int capacity) {
		arrays = new Array<Channel>(false, 2, Channel.class);
		this.capacity = capacity;
		size = 0;
	}

	
	public <T extends Channel> T addChannel (ChannelDescriptor channelDescriptor) {
		return addChannel(channelDescriptor, null);
	}

	
	public <T extends Channel> T addChannel (ChannelDescriptor channelDescriptor, ChannelInitializer<T> initializer) {
		T channel = getChannel(channelDescriptor);
		if (channel == null) {
			channel = allocateChannel(channelDescriptor);
			if (initializer != null) initializer.init(channel);
			arrays.add(channel);
		}
		return channel;
	}

	@SuppressWarnings({"unchecked", "rawtypes"})
	private <T extends Channel> T allocateChannel (ChannelDescriptor channelDescriptor) {
		if (channelDescriptor.type == float.class) {
			return (T)new FloatChannel(channelDescriptor.id, channelDescriptor.count, capacity);
		} else if (channelDescriptor.type == int.class) {
			return (T)new IntChannel(channelDescriptor.id, channelDescriptor.count, capacity);
		} else {
			return (T)new ObjectChannel(channelDescriptor.id, channelDescriptor.count, capacity, channelDescriptor.type);
		}
	}

	
	public <T> void removeArray (int id) {
		arrays.removeIndex(findIndex(id));
	}

	private int findIndex (int id) {
		for (int i = 0; i < arrays.size; ++i) {
			Channel array = arrays.items[i];
			if (array.id == id) return i;
		}
		return -1;
	}

	
	public void addElement (Object... values) {
		
		if (size == capacity) throw new GdxRuntimeException("Capacity reached, cannot add other elements");

		int k = 0;
		for (Channel strideArray : arrays) {
			strideArray.add(k, values);
			k += strideArray.strideSize;
		}
		++size;
	}

	
	public void removeElement (int index) {
		int last = size - 1;
				for (Channel strideArray : arrays) {
			strideArray.swap(index, last);
		}
		size = last;
	}

	
	@SuppressWarnings("unchecked")
	public <T extends Channel> T getChannel (ChannelDescriptor descriptor) {
		for (Channel array : arrays) {
			if (array.id == descriptor.id) return (T)array;
		}
		return null;
	}

	
	public void clear () {
		arrays.clear();
		size = 0;
	}

	
	public void setCapacity (int requiredCapacity) {
		if (capacity != requiredCapacity) {
			for (Channel channel : arrays) {
				channel.setCapacity(requiredCapacity);
			}
			capacity = requiredCapacity;
		}
	}

}
