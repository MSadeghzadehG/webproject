package hudson.util;

import hudson.Functions;
import hudson.Util;
import hudson.os.PosixAPI;
import hudson.os.PosixException;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import org.apache.commons.io.LineIterator;

import java.io.*;
import java.util.Collection;
import java.util.List;
import java.util.regex.Pattern;

import static hudson.Util.fileToPath;


public class IOUtils {
    
    public static void drain(InputStream in) throws IOException {
        org.apache.commons.io.IOUtils.copy(in, new NullStream());
        in.close();
    }

    public static void copy(File src, OutputStream out) throws IOException {
        try (InputStream in = Files.newInputStream(src.toPath())) {
            org.apache.commons.io.IOUtils.copy(in, out);
        } catch (InvalidPathException e) {
            throw new IOException(e);
        }
    }

    public static void copy(InputStream in, File out) throws IOException {
        try (OutputStream fos = Files.newOutputStream(out.toPath())) {
            org.apache.commons.io.IOUtils.copy(in, fos);
        } catch (InvalidPathException e) {
            throw new IOException(e);
        }
    }

    
    public static File mkdirs(File dir) throws IOException {
        try {
            return Files.createDirectories(fileToPath(dir)).toFile();
        } catch (UnsupportedOperationException e) {
            throw new IOException(e);
        }
    }

    
    public static InputStream skip(InputStream in, long size) throws IOException {
        DataInputStream di = new DataInputStream(in);

        while (size>0) {
            int chunk = (int)Math.min(SKIP_BUFFER.length,size);
            di.readFully(SKIP_BUFFER,0,chunk);
            size -= chunk;
        }

        return in;
    }

    
    public static File absolutize(File base, String path) {
        if (isAbsolute(path))
            return new File(path);
        return new File(base, path);
    }

    
    public static boolean isAbsolute(String path) {
        Pattern DRIVE_PATTERN = Pattern.compile("[A-Za-z]:[\\\\/].*");
        return path.startsWith("/") || DRIVE_PATTERN.matcher(path).matches();
    }


    
    public static int mode(File f) throws PosixException {
        if(Functions.isWindows())   return -1;
        try {
            if (Util.NATIVE_CHMOD_MODE) {
                return PosixAPI.jnr().stat(f.getPath()).mode();
            } else {
                return Util.permissionsToMode(Files.getPosixFilePermissions(fileToPath(f)));
            }
        } catch (IOException cause) {
            PosixException e = new PosixException("Unable to get file permissions", null);
            e.initCause(cause);
            throw e;
        }
    }

    
    public static String readFirstLine(InputStream is, String encoding) throws IOException {
        try (BufferedReader reader = new BufferedReader(
                encoding == null ? new InputStreamReader(is) : new InputStreamReader(is, encoding))) {
            return reader.readLine();
        }
    }


    
    @Deprecated
    public static final char DIR_SEPARATOR_UNIX       = org.apache.commons.io.IOUtils.DIR_SEPARATOR_UNIX;

    
    @Deprecated
    public static final char DIR_SEPARATOR_WINDOWS    = org.apache.commons.io.IOUtils.DIR_SEPARATOR_WINDOWS;

    
    @Deprecated
    public static final char DIR_SEPARATOR            = org.apache.commons.io.IOUtils.DIR_SEPARATOR;

    
    @Deprecated
    public static final String LINE_SEPARATOR_UNIX    = org.apache.commons.io.IOUtils.LINE_SEPARATOR_UNIX;

    
    @Deprecated
    public static final String LINE_SEPARATOR_WINDOWS = org.apache.commons.io.IOUtils.LINE_SEPARATOR_WINDOWS;

    
    @Deprecated
    public static final String LINE_SEPARATOR;

    static {
                StringWriter buf = new StringWriter(4);
        PrintWriter out = new PrintWriter(buf);
        out.println();
        LINE_SEPARATOR = buf.toString();
    }

    
    @Deprecated
    public static void closeQuietly(Reader input) {
        org.apache.commons.io.IOUtils.closeQuietly(input);
    }

    
    @Deprecated
    public static void closeQuietly(Writer output) {
        org.apache.commons.io.IOUtils.closeQuietly(output);
    }

    
    @Deprecated
    public static void closeQuietly(InputStream input) {
        org.apache.commons.io.IOUtils.closeQuietly(input);
    }

    
    @Deprecated
    public static void closeQuietly(OutputStream output) {
        org.apache.commons.io.IOUtils.closeQuietly(output);
    }

    
    @Deprecated
    public static byte[] toByteArray(InputStream input) throws IOException {
        return org.apache.commons.io.IOUtils.toByteArray(input);
    }

    
    @Deprecated
    public static byte[] toByteArray(Reader input) throws IOException {
        return org.apache.commons.io.IOUtils.toByteArray(input);
    }

    
    @Deprecated
    public static byte[] toByteArray(Reader input, String encoding) throws IOException {
        return org.apache.commons.io.IOUtils.toByteArray(input, encoding);
    }

    
    @Deprecated
    public static byte[] toByteArray(String input) throws IOException {
        return org.apache.commons.io.IOUtils.toByteArray(input);
    }

    
    @Deprecated
    public static char[] toCharArray(InputStream is) throws IOException {
        return org.apache.commons.io.IOUtils.toCharArray(is);
    }

    
    @Deprecated
    public static char[] toCharArray(InputStream is, String encoding) throws IOException {
        return org.apache.commons.io.IOUtils.toCharArray(is, encoding);
    }

    
    @Deprecated
    public static char[] toCharArray(Reader input) throws IOException {
        return org.apache.commons.io.IOUtils.toCharArray(input);
    }

    
    @Deprecated
    public static String toString(InputStream input) throws IOException {
        return org.apache.commons.io.IOUtils.toString(input);
    }

    
    @Deprecated
    public static String toString(InputStream input, String encoding) throws IOException {
        return org.apache.commons.io.IOUtils.toString(input, encoding);
    }

    
    @Deprecated
    public static String toString(Reader input) throws IOException {
        return org.apache.commons.io.IOUtils.toString(input);
    }

    
    @Deprecated
    public static String toString(byte[] input) throws IOException {
        return org.apache.commons.io.IOUtils.toString(input);
    }

    
    @Deprecated
    public static String toString(byte[] input, String encoding) throws IOException {
        return org.apache.commons.io.IOUtils.toString(input, encoding);
    }

    
    @Deprecated
    public static List readLines(InputStream input) throws IOException {
        return org.apache.commons.io.IOUtils.readLines(input);
    }

    
    @Deprecated
    public static List readLines(InputStream input, String encoding) throws IOException {
        return org.apache.commons.io.IOUtils.readLines(input, encoding);
    }

    
    @Deprecated
    public static List readLines(Reader input) throws IOException {
        return org.apache.commons.io.IOUtils.readLines(input);
    }

    
    @Deprecated
    public static LineIterator lineIterator(Reader reader) {
        return org.apache.commons.io.IOUtils.lineIterator(reader);
    }

    
    @Deprecated
    public static LineIterator lineIterator(InputStream input, String encoding) throws IOException {
        return org.apache.commons.io.IOUtils.lineIterator(input, encoding);
    }

    
    @Deprecated
    public static InputStream toInputStream(String input) {
        return org.apache.commons.io.IOUtils.toInputStream(input);
    }

    
    @Deprecated
    public static InputStream toInputStream(String input, String encoding) throws IOException {
        return org.apache.commons.io.IOUtils.toInputStream(input, encoding);
    }

    
    @Deprecated
    public static void write(byte[] data, OutputStream output) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output);
    }

    
    @Deprecated
    public static void write(byte[] data, Writer output) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output);
    }

    
    @Deprecated
    public static void write(byte[] data, Writer output, String encoding) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output, encoding);
    }

    
    @Deprecated
    public static void write(char[] data, Writer output) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output);
    }

    
    @Deprecated
    public static void write(char[] data, OutputStream output) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output);
    }

    
    @Deprecated
    public static void write(char[] data, OutputStream output, String encoding) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output, encoding);
    }

    
    @Deprecated
    public static void write(String data, Writer output) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output);
    }

    
    @Deprecated
    public static void write(String data, OutputStream output) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output);
    }

    
    @Deprecated
    public static void write(String data, OutputStream output, String encoding) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output, encoding);
    }

    
    @Deprecated
    public static void write(StringBuffer data, Writer output) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output);
    }

    
    @Deprecated
    public static void write(StringBuffer data, OutputStream output) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output);
    }

    
    @Deprecated
    public static void write(StringBuffer data, OutputStream output, String encoding) throws IOException {
        org.apache.commons.io.IOUtils.write(data, output, encoding);
    }

    
    @Deprecated
    public static void writeLines(Collection lines, String lineEnding, OutputStream output) throws IOException {
        org.apache.commons.io.IOUtils.writeLines(lines, lineEnding, output);
    }

    
    @Deprecated
    public static void writeLines(Collection lines, String lineEnding, OutputStream output, String encoding) throws IOException {
        org.apache.commons.io.IOUtils.writeLines(lines, lineEnding, output, encoding);
    }

    
    @Deprecated
    public static void writeLines(Collection lines, String lineEnding, Writer writer) throws IOException {
        org.apache.commons.io.IOUtils.writeLines(lines, lineEnding, writer);
    }

    
    @Deprecated
    public static int copy(InputStream input, OutputStream output) throws IOException {
        return org.apache.commons.io.IOUtils.copy(input, output);
    }

    
    @Deprecated
    public static long copyLarge(InputStream input, OutputStream output) throws IOException {
        return org.apache.commons.io.IOUtils.copyLarge(input, output);
    }

    
    @Deprecated
    public static void copy(InputStream input, Writer output) throws IOException {
        org.apache.commons.io.IOUtils.copy(input, output);
    }

    
    @Deprecated
    public static void copy(InputStream input, Writer output, String encoding) throws IOException {
        org.apache.commons.io.IOUtils.copy(input, output, encoding);
    }

    
    @Deprecated
    public static int copy(Reader input, Writer output) throws IOException {
        return org.apache.commons.io.IOUtils.copy(input, output);
    }

    
    @Deprecated
    public static long copyLarge(Reader input, Writer output) throws IOException {
        return org.apache.commons.io.IOUtils.copyLarge(input, output);
    }

    
    @Deprecated
    public static void copy(Reader input, OutputStream output) throws IOException {
        org.apache.commons.io.IOUtils.copy(input, output);
    }

    
    @Deprecated
    public static void copy(Reader input, OutputStream output, String encoding) throws IOException {
        org.apache.commons.io.IOUtils.copy(input, output, encoding);
    }

    
    @Deprecated
    public static boolean contentEquals(InputStream input1, InputStream input2) throws IOException {
        return org.apache.commons.io.IOUtils.contentEquals(input1, input2);
    }

    
    @Deprecated
    public static boolean contentEquals(Reader input1, Reader input2) throws IOException {
        return org.apache.commons.io.IOUtils.contentEquals(input1, input2);
    }

    private static final byte[] SKIP_BUFFER = new byte[8192];
}
