

package org.elasticsearch.common.io;

import org.elasticsearch.common.SuppressForbidden;

import java.io.EOFException;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.GatheringByteChannel;
import java.nio.channels.WritableByteChannel;

@SuppressForbidden(reason = "Channel#read")
public final class Channels {

    private Channels() {
    }

    
    private static final int READ_CHUNK_SIZE = 16384;
    
    private static final int WRITE_CHUNK_SIZE = 8192;

    
    public static byte[] readFromFileChannel(FileChannel channel, long position, int length) throws IOException {
        byte[] res = new byte[length];
        readFromFileChannelWithEofException(channel, position, res, 0, length);
        return res;

    }

    
    public static void readFromFileChannelWithEofException(FileChannel channel, long channelPosition, byte[] dest, int destOffset, int length) throws IOException {
        int read = readFromFileChannel(channel, channelPosition, dest, destOffset, length);
        if (read < 0) {
            throw new EOFException("read past EOF. pos [" + channelPosition + "] length: [" + length + "] end: [" + channel.size() + "]");
        }
    }

    
    public static int readFromFileChannel(FileChannel channel, long channelPosition, byte[] dest, int destOffset, int length) throws IOException {
        ByteBuffer buffer = ByteBuffer.wrap(dest, destOffset, length);
        return readFromFileChannel(channel, channelPosition, buffer);
    }


    
    public static void readFromFileChannelWithEofException(FileChannel channel, long channelPosition, ByteBuffer dest) throws IOException {
        int read = readFromFileChannel(channel, channelPosition, dest);
        if (read < 0) {
            throw new EOFException("read past EOF. pos [" + channelPosition + "] length: [" + dest.limit() + "] end: [" + channel.size() + "]");
        }
    }

    
    public static int readFromFileChannel(FileChannel channel, long channelPosition, ByteBuffer dest) throws IOException {
        if (dest.isDirect() || (dest.remaining() < READ_CHUNK_SIZE)) {
            return readSingleChunk(channel, channelPosition, dest);
        } else {
            int bytesRead = 0;
            int bytesToRead = dest.remaining();

                        ByteBuffer tmpBuffer = dest.duplicate();
            try {
                while (dest.hasRemaining()) {
                    tmpBuffer.limit(Math.min(dest.limit(), tmpBuffer.position() + READ_CHUNK_SIZE));
                    int read = readSingleChunk(channel, channelPosition, tmpBuffer);
                    if (read < 0) {
                        return read;
                    }
                    bytesRead += read;
                    channelPosition += read;
                    dest.position(tmpBuffer.position());
                }
            } finally {
                                dest.position(tmpBuffer.position());
            }

            assert bytesRead == bytesToRead : "failed to read an entire buffer but also didn't get an EOF (read [" + bytesRead + "] needed [" + bytesToRead + "]";
            return bytesRead;
        }
    }


    private static int readSingleChunk(FileChannel channel, long channelPosition, ByteBuffer dest) throws IOException {
        int bytesRead = 0;
        while (dest.hasRemaining()) {
            int read = channel.read(dest, channelPosition);
            if (read < 0) {
                return read;
            }

            assert read > 0 : "FileChannel.read with non zero-length bb.remaining() must always read at least one byte (FileChannel is in blocking mode, see spec of ReadableByteChannel)";

            bytesRead += read;
            channelPosition += read;
        }
        return bytesRead;
    }

    
    public static void writeToChannel(byte[] source, WritableByteChannel channel) throws IOException {
        writeToChannel(source, 0, source.length, channel);
    }


    
    public static void writeToChannel(byte[] source, int offset, int length, WritableByteChannel channel) throws IOException {
        int toWrite = Math.min(length, WRITE_CHUNK_SIZE);
        ByteBuffer buffer = ByteBuffer.wrap(source, offset, toWrite);
        int written = channel.write(buffer);
        length -= written;
        while (length > 0) {
            toWrite = Math.min(length, WRITE_CHUNK_SIZE);
            buffer.limit(buffer.position() + toWrite);
            written = channel.write(buffer);
            length -= written;
        }
        assert length == 0 : "wrote more then expected bytes (length=" + length + ")";
    }

    
    public static void writeToChannel(ByteBuffer byteBuffer, WritableByteChannel channel) throws IOException {
        if (byteBuffer.isDirect() || (byteBuffer.remaining() <= WRITE_CHUNK_SIZE)) {
            while (byteBuffer.hasRemaining()) {
                channel.write(byteBuffer);
            }
        } else {
                        ByteBuffer tmpBuffer = byteBuffer.duplicate();
            try {
                while (byteBuffer.hasRemaining()) {
                    tmpBuffer.limit(Math.min(byteBuffer.limit(), tmpBuffer.position() + WRITE_CHUNK_SIZE));
                    while (tmpBuffer.hasRemaining()) {
                        channel.write(tmpBuffer);
                    }
                    byteBuffer.position(tmpBuffer.position());
                }
            } finally {
                                byteBuffer.position(tmpBuffer.position());
            }
        }
    }
}
