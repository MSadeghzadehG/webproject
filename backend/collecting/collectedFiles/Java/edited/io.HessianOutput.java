

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.io.OutputStream;
import java.util.IdentityHashMap;


public class HessianOutput extends AbstractHessianOutput {
        protected OutputStream os;
        private IdentityHashMap _refs;
    private int _version = 1;

    
    public HessianOutput(OutputStream os) {
        init(os);
    }

    
    public HessianOutput() {
    }

    
    public void init(OutputStream os) {
        this.os = os;

        _refs = null;

        if (_serializerFactory == null)
            _serializerFactory = new SerializerFactory();
    }

    
    public void setVersion(int version) {
        _version = version;
    }

    
    public void call(String method, Object[] args)
            throws IOException {
        int length = args != null ? args.length : 0;

        startCall(method, length);

        for (int i = 0; i < length; i++)
            writeObject(args[i]);

        completeCall();
    }

    
    public void startCall(String method, int length)
            throws IOException {
        os.write('c');
        os.write(_version);
        os.write(0);

        os.write('m');
        int len = method.length();
        os.write(len >> 8);
        os.write(len);
        printString(method, 0, len);
    }

    
    public void startCall()
            throws IOException {
        os.write('c');
        os.write(0);
        os.write(1);
    }

    
    public void writeMethod(String method)
            throws IOException {
        os.write('m');
        int len = method.length();
        os.write(len >> 8);
        os.write(len);
        printString(method, 0, len);
    }

    
    public void completeCall()
            throws IOException {
        os.write('z');
    }

    
    public void startReply()
            throws IOException {
        os.write('r');
        os.write(1);
        os.write(0);
    }

    
    public void completeReply()
            throws IOException {
        os.write('z');
    }

    
    public void writeHeader(String name)
            throws IOException {
        int len = name.length();

        os.write('H');
        os.write(len >> 8);
        os.write(len);

        printString(name);
    }

    
    public void writeFault(String code, String message, Object detail)
            throws IOException {
        os.write('f');
        writeString("code");
        writeString(code);

        writeString("message");
        writeString(message);

        if (detail != null) {
            writeString("detail");
            writeObject(detail);
        }
        os.write('z');
    }

    
    public void writeObject(Object object)
            throws IOException {
        if (object == null) {
            writeNull();
            return;
        }

        Serializer serializer;

        serializer = _serializerFactory.getSerializer(object.getClass());

        serializer.writeObject(object, this);
    }

    
    public boolean writeListBegin(int length, String type)
            throws IOException {
        os.write('V');

        if (type != null) {
            os.write('t');
            printLenString(type);
        }

        if (length >= 0) {
            os.write('l');
            os.write(length >> 24);
            os.write(length >> 16);
            os.write(length >> 8);
            os.write(length);
        }

        return true;
    }

    
    public void writeListEnd()
            throws IOException {
        os.write('z');
    }

    
    public void writeMapBegin(String type)
            throws IOException {
        os.write('M');
        os.write('t');
        printLenString(type);
    }

    
    public void writeMapEnd()
            throws IOException {
        os.write('z');
    }

    
    public void writeRemote(String type, String url)
            throws IOException {
        os.write('r');
        os.write('t');
        printLenString(type);
        os.write('S');
        printLenString(url);
    }

    
    public void writeBoolean(boolean value)
            throws IOException {
        if (value)
            os.write('T');
        else
            os.write('F');
    }

    
    public void writeInt(int value)
            throws IOException {
        os.write('I');
        os.write(value >> 24);
        os.write(value >> 16);
        os.write(value >> 8);
        os.write(value);
    }

    
    public void writeLong(long value)
            throws IOException {
        os.write('L');
        os.write((byte) (value >> 56));
        os.write((byte) (value >> 48));
        os.write((byte) (value >> 40));
        os.write((byte) (value >> 32));
        os.write((byte) (value >> 24));
        os.write((byte) (value >> 16));
        os.write((byte) (value >> 8));
        os.write((byte) (value));
    }

    
    public void writeDouble(double value)
            throws IOException {
        long bits = Double.doubleToLongBits(value);

        os.write('D');
        os.write((byte) (bits >> 56));
        os.write((byte) (bits >> 48));
        os.write((byte) (bits >> 40));
        os.write((byte) (bits >> 32));
        os.write((byte) (bits >> 24));
        os.write((byte) (bits >> 16));
        os.write((byte) (bits >> 8));
        os.write((byte) (bits));
    }

    
    public void writeUTCDate(long time)
            throws IOException {
        os.write('d');
        os.write((byte) (time >> 56));
        os.write((byte) (time >> 48));
        os.write((byte) (time >> 40));
        os.write((byte) (time >> 32));
        os.write((byte) (time >> 24));
        os.write((byte) (time >> 16));
        os.write((byte) (time >> 8));
        os.write((byte) (time));
    }

    
    public void writeNull()
            throws IOException {
        os.write('N');
    }

    
    public void writeString(String value)
            throws IOException {
        if (value == null) {
            os.write('N');
        } else {
            int length = value.length();
            int offset = 0;

            while (length > 0x8000) {
                int sublen = 0x8000;

                                char tail = value.charAt(offset + sublen - 1);

                if (0xd800 <= tail && tail <= 0xdbff)
                    sublen--;

                os.write('s');
                os.write(sublen >> 8);
                os.write(sublen);

                printString(value, offset, sublen);

                length -= sublen;
                offset += sublen;
            }

            os.write('S');
            os.write(length >> 8);
            os.write(length);

            printString(value, offset, length);
        }
    }

    
    public void writeString(char[] buffer, int offset, int length)
            throws IOException {
        if (buffer == null) {
            os.write('N');
        } else {
            while (length > 0x8000) {
                int sublen = 0x8000;

                                char tail = buffer[offset + sublen - 1];

                if (0xd800 <= tail && tail <= 0xdbff)
                    sublen--;

                os.write('s');
                os.write(sublen >> 8);
                os.write(sublen);

                printString(buffer, offset, sublen);

                length -= sublen;
                offset += sublen;
            }

            os.write('S');
            os.write(length >> 8);
            os.write(length);

            printString(buffer, offset, length);
        }
    }

    
    public void writeBytes(byte[] buffer)
            throws IOException {
        if (buffer == null)
            os.write('N');
        else
            writeBytes(buffer, 0, buffer.length);
    }

    
    public void writeBytes(byte[] buffer, int offset, int length)
            throws IOException {
        if (buffer == null) {
            os.write('N');
        } else {
            while (length > 0x8000) {
                int sublen = 0x8000;

                os.write('b');
                os.write(sublen >> 8);
                os.write(sublen);

                os.write(buffer, offset, sublen);

                length -= sublen;
                offset += sublen;
            }

            os.write('B');
            os.write(length >> 8);
            os.write(length);
            os.write(buffer, offset, length);
        }
    }

    
    public void writeByteBufferStart()
            throws IOException {
    }

    
    public void writeByteBufferPart(byte[] buffer, int offset, int length)
            throws IOException {
        while (length > 0) {
            int sublen = length;

            if (0x8000 < sublen)
                sublen = 0x8000;

            os.write('b');
            os.write(sublen >> 8);
            os.write(sublen);

            os.write(buffer, offset, sublen);

            length -= sublen;
            offset += sublen;
        }
    }

    
    public void writeByteBufferEnd(byte[] buffer, int offset, int length)
            throws IOException {
        writeBytes(buffer, offset, length);
    }

    
    public void writeRef(int value)
            throws IOException {
        os.write('R');
        os.write(value >> 24);
        os.write(value >> 16);
        os.write(value >> 8);
        os.write(value);
    }

    
    public void writePlaceholder()
            throws IOException {
        os.write('P');
    }

    
    public boolean addRef(Object object)
            throws IOException {
        if (_refs == null)
            _refs = new IdentityHashMap();

        Integer ref = (Integer) _refs.get(object);

        if (ref != null) {
            int value = ref.intValue();

            writeRef(value);
            return true;
        } else {
            _refs.put(object, new Integer(_refs.size()));

            return false;
        }
    }

    
    public void resetReferences() {
        if (_refs != null)
            _refs.clear();
    }

    
    public boolean removeRef(Object obj)
            throws IOException {
        if (_refs != null) {
            _refs.remove(obj);

            return true;
        } else
            return false;
    }

    
    public boolean replaceRef(Object oldRef, Object newRef)
            throws IOException {
        Integer value = (Integer) _refs.remove(oldRef);

        if (value != null) {
            _refs.put(newRef, value);
            return true;
        } else
            return false;
    }

    
    public void printLenString(String v)
            throws IOException {
        if (v == null) {
            os.write(0);
            os.write(0);
        } else {
            int len = v.length();
            os.write(len >> 8);
            os.write(len);

            printString(v, 0, len);
        }
    }

    
    public void printString(String v)
            throws IOException {
        printString(v, 0, v.length());
    }

    
    public void printString(String v, int offset, int length)
            throws IOException {
        for (int i = 0; i < length; i++) {
            char ch = v.charAt(i + offset);

            if (ch < 0x80)
                os.write(ch);
            else if (ch < 0x800) {
                os.write(0xc0 + ((ch >> 6) & 0x1f));
                os.write(0x80 + (ch & 0x3f));
            } else {
                os.write(0xe0 + ((ch >> 12) & 0xf));
                os.write(0x80 + ((ch >> 6) & 0x3f));
                os.write(0x80 + (ch & 0x3f));
            }
        }
    }

    
    public void printString(char[] v, int offset, int length)
            throws IOException {
        for (int i = 0; i < length; i++) {
            char ch = v[i + offset];

            if (ch < 0x80)
                os.write(ch);
            else if (ch < 0x800) {
                os.write(0xc0 + ((ch >> 6) & 0x1f));
                os.write(0x80 + (ch & 0x3f));
            } else {
                os.write(0xe0 + ((ch >> 12) & 0xf));
                os.write(0x80 + ((ch >> 6) & 0x3f));
                os.write(0x80 + (ch & 0x3f));
            }
        }
    }

    public void flush()
            throws IOException {
        if (this.os != null)
            this.os.flush();
    }

    public void close()
            throws IOException {
        if (this.os != null)
            this.os.flush();
    }
}
