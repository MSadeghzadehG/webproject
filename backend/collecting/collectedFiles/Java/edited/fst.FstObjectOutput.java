
package com.alibaba.dubbo.common.serialize.fst;

import com.alibaba.dubbo.common.serialize.ObjectOutput;

import org.nustaq.serialization.FSTObjectOutput;

import java.io.IOException;
import java.io.OutputStream;


public class FstObjectOutput implements ObjectOutput {

    private FSTObjectOutput output;

    public FstObjectOutput(OutputStream outputStream) {
        output = FstFactory.getDefaultFactory().getObjectOutput(outputStream);
    }

    public void writeBool(boolean v) throws IOException {
        output.writeBoolean(v);
    }

    public void writeByte(byte v) throws IOException {
        output.writeByte(v);
    }

    public void writeShort(short v) throws IOException {
        output.writeShort(v);
    }

    public void writeInt(int v) throws IOException {
        output.writeInt(v);
    }

    public void writeLong(long v) throws IOException {
        output.writeLong(v);
    }

    public void writeFloat(float v) throws IOException {
        output.writeFloat(v);
    }

    public void writeDouble(double v) throws IOException {
        output.writeDouble(v);
    }

    public void writeBytes(byte[] v) throws IOException {
        if (v == null) {
            output.writeInt(-1);
        } else {
            writeBytes(v, 0, v.length);
        }
    }

    public void writeBytes(byte[] v, int off, int len) throws IOException {
        if (v == null) {
            output.writeInt(-1);
        } else {
            output.writeInt(len);
            output.write(v, off, len);
        }
    }


    public void writeUTF(String v) throws IOException {
        output.writeUTF(v);
    }

    public void writeObject(Object v) throws IOException {
        output.writeObject(v);
    }

    public void flushBuffer() throws IOException {
        output.flush();
    }
}