

package com.google.common.io;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.io.FileWriteMode.APPEND;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.base.Joiner;
import com.google.common.base.Optional;
import com.google.common.base.Predicate;
import com.google.common.base.Splitter;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Lists;
import com.google.common.collect.TreeTraverser;
import com.google.common.graph.SuccessorsFunction;
import com.google.common.graph.Traverser;
import com.google.common.hash.HashCode;
import com.google.common.hash.HashFunction;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.RandomAccessFile;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileChannel.MapMode;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;


@Beta
@GwtIncompatible
public final class Files {

  
  private static final int TEMP_DIR_ATTEMPTS = 10000;

  private Files() {}

  
  public static BufferedReader newReader(File file, Charset charset) throws FileNotFoundException {
    checkNotNull(file);
    checkNotNull(charset);
    return new BufferedReader(new InputStreamReader(new FileInputStream(file), charset));
  }

  
  public static BufferedWriter newWriter(File file, Charset charset) throws FileNotFoundException {
    checkNotNull(file);
    checkNotNull(charset);
    return new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), charset));
  }

  
  public static ByteSource asByteSource(File file) {
    return new FileByteSource(file);
  }

  private static final class FileByteSource extends ByteSource {

    private final File file;

    private FileByteSource(File file) {
      this.file = checkNotNull(file);
    }

    @Override
    public FileInputStream openStream() throws IOException {
      return new FileInputStream(file);
    }

    @Override
    public Optional<Long> sizeIfKnown() {
      if (file.isFile()) {
        return Optional.of(file.length());
      } else {
        return Optional.absent();
      }
    }

    @Override
    public long size() throws IOException {
      if (!file.isFile()) {
        throw new FileNotFoundException(file.toString());
      }
      return file.length();
    }

    @Override
    public byte[] read() throws IOException {
      Closer closer = Closer.create();
      try {
        FileInputStream in = closer.register(openStream());
        return readFile(in, in.getChannel().size());
      } catch (Throwable e) {
        throw closer.rethrow(e);
      } finally {
        closer.close();
      }
    }

    @Override
    public String toString() {
      return "Files.asByteSource(" + file + ")";
    }
  }

  
  static byte[] readFile(InputStream in, long expectedSize) throws IOException {
    if (expectedSize > Integer.MAX_VALUE) {
      throw new OutOfMemoryError(
          "file is too large to fit in a byte array: " + expectedSize + " bytes");
    }

                                        return ByteStreams.toByteArray(in, expectedSize == 0 ? 4096 : (int) expectedSize);
  }

  
  public static ByteSink asByteSink(File file, FileWriteMode... modes) {
    return new FileByteSink(file, modes);
  }

  private static final class FileByteSink extends ByteSink {

    private final File file;
    private final ImmutableSet<FileWriteMode> modes;

    private FileByteSink(File file, FileWriteMode... modes) {
      this.file = checkNotNull(file);
      this.modes = ImmutableSet.copyOf(modes);
    }

    @Override
    public FileOutputStream openStream() throws IOException {
      return new FileOutputStream(file, modes.contains(APPEND));
    }

    @Override
    public String toString() {
      return "Files.asByteSink(" + file + ", " + modes + ")";
    }
  }

  
  public static CharSource asCharSource(File file, Charset charset) {
    return asByteSource(file).asCharSource(charset);
  }

  
  public static CharSink asCharSink(File file, Charset charset, FileWriteMode... modes) {
    return asByteSink(file, modes).asCharSink(charset);
  }

  
  public static byte[] toByteArray(File file) throws IOException {
    return asByteSource(file).read();
  }

  
  @Deprecated
  public static String toString(File file, Charset charset) throws IOException {
    return asCharSource(file, charset).read();
  }

  
  public static void write(byte[] from, File to) throws IOException {
    asByteSink(to).write(from);
  }

  
  @Deprecated
  public static void write(CharSequence from, File to, Charset charset) throws IOException {
    asCharSink(to, charset).write(from);
  }

  
  public static void copy(File from, OutputStream to) throws IOException {
    asByteSource(from).copyTo(to);
  }

  
  public static void copy(File from, File to) throws IOException {
    checkArgument(!from.equals(to), "Source %s and destination %s must be different", from, to);
    asByteSource(from).copyTo(asByteSink(to));
  }

  
  @Deprecated
  public static void copy(File from, Charset charset, Appendable to) throws IOException {
    asCharSource(from, charset).copyTo(to);
  }

  
  @Deprecated
  public static void append(CharSequence from, File to, Charset charset) throws IOException {
    asCharSink(to, charset, FileWriteMode.APPEND).write(from);
  }

  
  public static boolean equal(File file1, File file2) throws IOException {
    checkNotNull(file1);
    checkNotNull(file2);
    if (file1 == file2 || file1.equals(file2)) {
      return true;
    }

    
    long len1 = file1.length();
    long len2 = file2.length();
    if (len1 != 0 && len2 != 0 && len1 != len2) {
      return false;
    }
    return asByteSource(file1).contentEquals(asByteSource(file2));
  }

  
  public static File createTempDir() {
    File baseDir = new File(System.getProperty("java.io.tmpdir"));
    String baseName = System.currentTimeMillis() + "-";

    for (int counter = 0; counter < TEMP_DIR_ATTEMPTS; counter++) {
      File tempDir = new File(baseDir, baseName + counter);
      if (tempDir.mkdir()) {
        return tempDir;
      }
    }
    throw new IllegalStateException(
        "Failed to create directory within "
            + TEMP_DIR_ATTEMPTS
            + " attempts (tried "
            + baseName
            + "0 to "
            + baseName
            + (TEMP_DIR_ATTEMPTS - 1)
            + ')');
  }

  
  public static void touch(File file) throws IOException {
    checkNotNull(file);
    if (!file.createNewFile() && !file.setLastModified(System.currentTimeMillis())) {
      throw new IOException("Unable to update modification time of " + file);
    }
  }

  
  public static void createParentDirs(File file) throws IOException {
    checkNotNull(file);
    File parent = file.getCanonicalFile().getParentFile();
    if (parent == null) {
      
      return;
    }
    parent.mkdirs();
    if (!parent.isDirectory()) {
      throw new IOException("Unable to create parent directories of " + file);
    }
  }

  
  public static void move(File from, File to) throws IOException {
    checkNotNull(from);
    checkNotNull(to);
    checkArgument(!from.equals(to), "Source %s and destination %s must be different", from, to);

    if (!from.renameTo(to)) {
      copy(from, to);
      if (!from.delete()) {
        if (!to.delete()) {
          throw new IOException("Unable to delete " + to);
        }
        throw new IOException("Unable to delete " + from);
      }
    }
  }

  
  @Deprecated
  public static String readFirstLine(File file, Charset charset) throws IOException {
    return asCharSource(file, charset).readFirstLine();
  }

  
  public static List<String> readLines(File file, Charset charset) throws IOException {
            return asCharSource(file, charset)
        .readLines(
            new LineProcessor<List<String>>() {
              final List<String> result = Lists.newArrayList();

              @Override
              public boolean processLine(String line) {
                result.add(line);
                return true;
              }

              @Override
              public List<String> getResult() {
                return result;
              }
            });
  }

  
  @Deprecated
  @CanIgnoreReturnValue   public static <T> T readLines(File file, Charset charset, LineProcessor<T> callback)
      throws IOException {
    return asCharSource(file, charset).readLines(callback);
  }

  
  @Deprecated
  @CanIgnoreReturnValue   public static <T> T readBytes(File file, ByteProcessor<T> processor) throws IOException {
    return asByteSource(file).read(processor);
  }

  
  @Deprecated
  public static HashCode hash(File file, HashFunction hashFunction) throws IOException {
    return asByteSource(file).hash(hashFunction);
  }

  
  public static MappedByteBuffer map(File file) throws IOException {
    checkNotNull(file);
    return map(file, MapMode.READ_ONLY);
  }

  
  public static MappedByteBuffer map(File file, MapMode mode) throws IOException {
    checkNotNull(file);
    checkNotNull(mode);
    if (!file.exists()) {
      throw new FileNotFoundException(file.toString());
    }
    return map(file, mode, file.length());
  }

  
  public static MappedByteBuffer map(File file, MapMode mode, long size)
      throws FileNotFoundException, IOException {
    checkNotNull(file);
    checkNotNull(mode);

    Closer closer = Closer.create();
    try {
      RandomAccessFile raf =
          closer.register(new RandomAccessFile(file, mode == MapMode.READ_ONLY ? "r" : "rw"));
      return map(raf, mode, size);
    } catch (Throwable e) {
      throw closer.rethrow(e);
    } finally {
      closer.close();
    }
  }

  private static MappedByteBuffer map(RandomAccessFile raf, MapMode mode, long size)
      throws IOException {
    Closer closer = Closer.create();
    try {
      FileChannel channel = closer.register(raf.getChannel());
      return channel.map(mode, 0, size);
    } catch (Throwable e) {
      throw closer.rethrow(e);
    } finally {
      closer.close();
    }
  }

  
  public static String simplifyPath(String pathname) {
    checkNotNull(pathname);
    if (pathname.length() == 0) {
      return ".";
    }

        Iterable<String> components = Splitter.on('/').omitEmptyStrings().split(pathname);
    List<String> path = new ArrayList<>();

        for (String component : components) {
      switch (component) {
        case ".":
          continue;
        case "..":
          if (path.size() > 0 && !path.get(path.size() - 1).equals("..")) {
            path.remove(path.size() - 1);
          } else {
            path.add("..");
          }
          break;
        default:
          path.add(component);
          break;
      }
    }

        String result = Joiner.on('/').join(path);
    if (pathname.charAt(0) == '/') {
      result = "/" + result;
    }

    while (result.startsWith("/../")) {
      result = result.substring(3);
    }
    if (result.equals("/..")) {
      result = "/";
    } else if ("".equals(result)) {
      result = ".";
    }

    return result;
  }

  
  public static String getFileExtension(String fullName) {
    checkNotNull(fullName);
    String fileName = new File(fullName).getName();
    int dotIndex = fileName.lastIndexOf('.');
    return (dotIndex == -1) ? "" : fileName.substring(dotIndex + 1);
  }

  
  public static String getNameWithoutExtension(String file) {
    checkNotNull(file);
    String fileName = new File(file).getName();
    int dotIndex = fileName.lastIndexOf('.');
    return (dotIndex == -1) ? fileName : fileName.substring(0, dotIndex);
  }

  
  @Deprecated
  public static TreeTraverser<File> fileTreeTraverser() {
    return FILE_TREE_TRAVERSER;
  }

  private static final TreeTraverser<File> FILE_TREE_TRAVERSER =
      new TreeTraverser<File>() {
        @Override
        public Iterable<File> children(File file) {
          return fileTreeChildren(file);
        }

        @Override
        public String toString() {
          return "Files.fileTreeTraverser()";
        }
      };

  
  public static Traverser<File> fileTraverser() {
    return Traverser.forTree(FILE_TREE);
  }

  private static final SuccessorsFunction<File> FILE_TREE =
      new SuccessorsFunction<File>() {
        @Override
        public Iterable<File> successors(File file) {
          return fileTreeChildren(file);
        }
      };

  private static Iterable<File> fileTreeChildren(File file) {
        if (file.isDirectory()) {
      File[] files = file.listFiles();
      if (files != null) {
        return Collections.unmodifiableList(Arrays.asList(files));
      }
    }

    return Collections.emptyList();
  }

  
  public static Predicate<File> isDirectory() {
    return FilePredicate.IS_DIRECTORY;
  }

  
  public static Predicate<File> isFile() {
    return FilePredicate.IS_FILE;
  }

  private enum FilePredicate implements Predicate<File> {
    IS_DIRECTORY {
      @Override
      public boolean apply(File file) {
        return file.isDirectory();
      }

      @Override
      public String toString() {
        return "Files.isDirectory()";
      }
    },

    IS_FILE {
      @Override
      public boolean apply(File file) {
        return file.isFile();
      }

      @Override
      public String toString() {
        return "Files.isFile()";
      }
    }
  }
}
