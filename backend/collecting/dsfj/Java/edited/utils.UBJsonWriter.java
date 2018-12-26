

package com.badlogic.gdx.utils;

import java.io.Closeable;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;


public class UBJsonWriter implements Closeable {

	final DataOutputStream out;

	private JsonObject current;
	private boolean named;
	private final Array<JsonObject> stack = new Array();

	public UBJsonWriter (OutputStream out) {
		if (!(out instanceof DataOutputStream)) out = new DataOutputStream(out);
		this.out = (DataOutputStream)out;
	}

	
	public UBJsonWriter object () throws IOException {
		if (current != null) {
			if (!current.array) {
				if (!named) throw new IllegalStateException("Name must be set.");
				named = false;
			}
		}
		stack.add(current = new JsonObject(false));
		return this;
	}

	
	public UBJsonWriter object (String name) throws IOException {
		name(name).object();
		return this;
	}

	
	public UBJsonWriter array () throws IOException {
		if (current != null) {
			if (!current.array) {
				if (!named) throw new IllegalStateException("Name must be set.");
				named = false;
			}
		}
		stack.add(current = new JsonObject(true));
		return this;
	}

	
	public UBJsonWriter array (String name) throws IOException {
		name(name).array();
		return this;
	}

	
	public UBJsonWriter name (String name) throws IOException {
		if (current == null || current.array) throw new IllegalStateException("Current item must be an object.");
		byte[] bytes = name.getBytes("UTF-8");
		if (bytes.length <= Byte.MAX_VALUE) {
			out.writeByte('i');
			out.writeByte(bytes.length);
		} else if (bytes.length <= Short.MAX_VALUE) {
			out.writeByte('I');
			out.writeShort(bytes.length);
		} else {
			out.writeByte('l');
			out.writeInt(bytes.length);
		}
		out.write(bytes);
		named = true;
		return this;
	}

	
	public UBJsonWriter value (byte value) throws IOException {
		checkName();
		out.writeByte('i');
		out.writeByte(value);
		return this;
	}

	
	public UBJsonWriter value (short value) throws IOException {
		checkName();
		out.writeByte('I');
		out.writeShort(value);
		return this;
	}

	
	public UBJsonWriter value (int value) throws IOException {
		checkName();
		out.writeByte('l');
		out.writeInt(value);
		return this;
	}

	
	public UBJsonWriter value (long value) throws IOException {
		checkName();
		out.writeByte('L');
		out.writeLong(value);
		return this;
	}

	
	public UBJsonWriter value (float value) throws IOException {
		checkName();
		out.writeByte('d');
		out.writeFloat(value);
		return this;
	}

	
	public UBJsonWriter value (double value) throws IOException {
		checkName();
		out.writeByte('D');
		out.writeDouble(value);
		return this;
	}

	
	public UBJsonWriter value (boolean value) throws IOException {
		checkName();
		out.writeByte(value ? 'T' : 'F');
		return this;
	}

	
	public UBJsonWriter value (char value) throws IOException {
		checkName();
		out.writeByte('I');
		out.writeChar(value);
		return this;
	}

	
	public UBJsonWriter value (String value) throws IOException {
		checkName();
		byte[] bytes = value.getBytes("UTF-8");
		out.writeByte('S');
		if (bytes.length <= Byte.MAX_VALUE) {
			out.writeByte('i');
			out.writeByte(bytes.length);
		} else if (bytes.length <= Short.MAX_VALUE) {
			out.writeByte('I');
			out.writeShort(bytes.length);
		} else {
			out.writeByte('l');
			out.writeInt(bytes.length);
		}
		out.write(bytes);
		return this;
	}

	
	public UBJsonWriter value (byte[] values) throws IOException {
		array();
		out.writeByte('$');
		out.writeByte('i');
		out.writeByte('#');
		value(values.length);
		for (int i = 0, n = values.length; i < n; i++) {
			out.writeByte(values[i]);
		}
		pop(true);
		return this;
	}

	
	public UBJsonWriter value (short[] values) throws IOException {
		array();
		out.writeByte('$');
		out.writeByte('I');
		out.writeByte('#');
		value(values.length);
		for (int i = 0, n = values.length; i < n; i++) {
			out.writeShort(values[i]);
		}
		pop(true);
		return this;
	}

	
	public UBJsonWriter value (int[] values) throws IOException {
		array();
		out.writeByte('$');
		out.writeByte('l');
		out.writeByte('#');
		value(values.length);
		for (int i = 0, n = values.length; i < n; i++) {
			out.writeInt(values[i]);
		}
		pop(true);
		return this;
	}

	
	public UBJsonWriter value (long[] values) throws IOException {
		array();
		out.writeByte('$');
		out.writeByte('L');
		out.writeByte('#');
		value(values.length);
		for (int i = 0, n = values.length; i < n; i++) {
			out.writeLong(values[i]);
		}
		pop(true);
		return this;
	}

	
	public UBJsonWriter value (float[] values) throws IOException {
		array();
		out.writeByte('$');
		out.writeByte('d');
		out.writeByte('#');
		value(values.length);
		for (int i = 0, n = values.length; i < n; i++) {
			out.writeFloat(values[i]);
		}
		pop(true);
		return this;
	}

	
	public UBJsonWriter value (double[] values) throws IOException {
		array();
		out.writeByte('$');
		out.writeByte('D');
		out.writeByte('#');
		value(values.length);
		for (int i = 0, n = values.length; i < n; i++) {
			out.writeDouble(values[i]);
		}
		pop(true);
		return this;
	}

	
	public UBJsonWriter value (boolean[] values) throws IOException {
		array();
		for (int i = 0, n = values.length; i < n; i++) {
			out.writeByte(values[i] ? 'T' : 'F');
		}
		pop();
		return this;
	}

	
	public UBJsonWriter value (char[] values) throws IOException {
		array();
		out.writeByte('$');
		out.writeByte('C');
		out.writeByte('#');
		value(values.length);
		for (int i = 0, n = values.length; i < n; i++) {
			out.writeChar(values[i]);
		}
		pop(true);
		return this;
	}

	
	public UBJsonWriter value (String[] values) throws IOException {
		array();
		out.writeByte('$');
		out.writeByte('S');
		out.writeByte('#');
		value(values.length);
		for (int i = 0, n = values.length; i < n; i++) {
			byte[] bytes = values[i].getBytes("UTF-8");
			if (bytes.length <= Byte.MAX_VALUE) {
				out.writeByte('i');
				out.writeByte(bytes.length);
			} else if (bytes.length <= Short.MAX_VALUE) {
				out.writeByte('I');
				out.writeShort(bytes.length);
			} else {
				out.writeByte('l');
				out.writeInt(bytes.length);
			}
			out.write(bytes);
		}
		pop(true);
		return this;
	}

	
	public UBJsonWriter value (JsonValue value) throws IOException {
		if (value.isObject()) {
			if (value.name != null)
				object(value.name);
			else
				object();
			for (JsonValue child = value.child; child != null; child = child.next)
				value(child);
			pop();
		} else if (value.isArray()) {
			if (value.name != null)
				array(value.name);
			else
				array();
			for (JsonValue child = value.child; child != null; child = child.next)
				value(child);
			pop();
		} else if (value.isBoolean()) {
			if (value.name != null) name(value.name);
			value(value.asBoolean());
		} else if (value.isDouble()) {
			if (value.name != null) name(value.name);
			value(value.asDouble());
		} else if (value.isLong()) {
			if (value.name != null) name(value.name);
			value(value.asLong());
		} else if (value.isString()) {
			if (value.name != null) name(value.name);
			value(value.asString());
		} else if (value.isNull()) {
			if (value.name != null) name(value.name);
			value();
		} else {
			throw new IOException("Unhandled JsonValue type");
		}
		return this;
	}

	
	public UBJsonWriter value (Object object) throws IOException {
		if (object == null) {
			return value();
		} else if (object instanceof Number) {
			Number number = (Number)object;
			if (object instanceof Byte) return value(number.byteValue());
			if (object instanceof Short) return value(number.shortValue());
			if (object instanceof Integer) return value(number.intValue());
			if (object instanceof Long) return value(number.longValue());
			if (object instanceof Float) return value(number.floatValue());
			if (object instanceof Double) return value(number.doubleValue());
		} else if (object instanceof Character) {
			return value(((Character)object).charValue());
		} else if (object instanceof CharSequence) {
			return value(object.toString());
		} else
			throw new IOException("Unknown object type.");

		return this;
	}

	
	public UBJsonWriter value () throws IOException {
		checkName();
		out.writeByte('Z');
		return this;
	}

	
	public UBJsonWriter set (String name, byte value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, short value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, int value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, long value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, float value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, double value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, boolean value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, char value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, String value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, byte[] value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, short[] value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, int[] value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, long[] value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, float[] value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, double[] value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, boolean[] value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, char[] value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name, String[] value) throws IOException {
		return name(name).value(value);
	}

	
	public UBJsonWriter set (String name) throws IOException {
		return name(name).value();
	}

	private void checkName () {
		if (current != null) {
			if (!current.array) {
				if (!named) throw new IllegalStateException("Name must be set.");
				named = false;
			}
		}
	}

	
	public UBJsonWriter pop () throws IOException {
		return pop(false);
	}

	protected UBJsonWriter pop (boolean silent) throws IOException {
		if (named) throw new IllegalStateException("Expected an object, array, or value since a name was set.");
		if (silent)
			stack.pop();
		else
			stack.pop().close();
		current = stack.size == 0 ? null : stack.peek();
		return this;
	}

	
	public void flush () throws IOException {
		out.flush();
	}

	
	public void close () throws IOException {
		while (stack.size > 0)
			pop();
		out.close();
	}

	private class JsonObject {
		final boolean array;

		JsonObject (boolean array) throws IOException {
			this.array = array;
			out.writeByte(array ? '[' : '{');
		}

		void close () throws IOException {
			out.writeByte(array ? ']' : '}');
		}
	}

}
