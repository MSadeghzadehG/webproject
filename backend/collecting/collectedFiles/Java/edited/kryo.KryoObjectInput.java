
package com.alibaba.dubbo.common.serialize.kryo;

import com.alibaba.dubbo.common.serialize.Cleanable;
import com.alibaba.dubbo.common.serialize.ObjectInput;
import com.alibaba.dubbo.common.serialize.kryo.utils.KryoUtils;

import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryo.KryoException;
import com.esotericsoftware.kryo.io.Input;

import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Type;

public class KryoObjectInput implements ObjectInput, Cleanable {

    private Kryo kryo;
    private Input input;

    public KryoObjectInput(InputStream inputStream) {
        input = new Input(inputStream);
        this.kryo = KryoUtils.get();
    }

    public boolean readBool() throws IOException {
        try {
            return input.readBoolean();
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }

    public byte readByte() throws IOException {
        try {
            return input.readByte();
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }

    public short readShort() throws IOException {
        try {
            return input.readShort();
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }

    public int readInt() throws IOException {
        try {
            return input.readInt();
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }

    public long readLong() throws IOException {
        try {
            return input.readLong();
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }

    public float readFloat() throws IOException {
        try {
            return input.readFloat();
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }

    public double readDouble() throws IOException {
        try {
            return input.readDouble();
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }

    public byte[] readBytes() throws IOException {
        try {
            int len = input.readInt();
            if (len < 0) {
                return null;
            } else if (len == 0) {
                return new byte[]{};
            } else {
                return input.readBytes(len);
            }
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }

    public String readUTF() throws IOException {
        try {
            return input.readString();
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }

    public Object readObject() throws IOException, ClassNotFoundException {
                try {
            return kryo.readClassAndObject(input);
        } catch (KryoException e) {
            throw new IOException(e);
        }
    }


    @SuppressWarnings("unchecked")
    public <T> T readObject(Class<T> clazz) throws IOException, ClassNotFoundException {
                return (T) readObject();
    }

    @SuppressWarnings("unchecked")
    public <T> T readObject(Class<T> clazz, Type type) throws IOException, ClassNotFoundException {
                return readObject(clazz);
    }

    public void cleanup() {
        KryoUtils.release(kryo);
        kryo = null;
    }
}