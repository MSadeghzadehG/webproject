

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;
import static java.nio.file.LinkOption.NOFOLLOW_LINKS;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.base.Optional;
import com.google.common.base.Predicate;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.TreeTraverser;
import com.google.common.graph.SuccessorsFunction;
import com.google.common.graph.Traverser;
import com.google.common.io.ByteSource.AsCharSource;
import com.google.j2objc.annotations.J2ObjCIncompatible;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.channels.Channels;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.Charset;
import java.nio.file.DirectoryIteratorException;
import java.nio.file.DirectoryStream;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileSystemException;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.NoSuchFileException;
import java.nio.file.NotDirectoryException;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.SecureDirectoryStream;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.BasicFileAttributeView;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.FileTime;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.stream.Stream;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@Beta
@GwtIncompatible
@J2ObjCIncompatible public final class MoreFiles {

  private MoreFiles() {}

  
  public static ByteSource asByteSource(Path path, OpenOption... options) {
    return new PathByteSource(path, options);
  }

  private static final class PathByteSource extends ByteSource {

    private static final LinkOption[] FOLLOW_LINKS = {};

    private final Path path;
    private final OpenOption[] options;
    private final boolean followLinks;

    private PathByteSource(Path path, OpenOption... options) {
      this.path = checkNotNull(path);
      this.options = options.clone();
      this.followLinks = followLinks(this.options);
          }

    private static boolean followLinks(OpenOption[] options) {
      for (OpenOption option : options) {
        if (option == NOFOLLOW_LINKS) {
          return false;
        }
      }
      return true;
    }

    @Override
    public InputStream openStream() throws IOException {
      return Files.newInputStream(path, options);
    }

    private BasicFileAttributes readAttributes() throws IOException {
      return Files.readAttributes(
          path,
          BasicFileAttributes.class,
          followLinks ? FOLLOW_LINKS : new LinkOption[] {NOFOLLOW_LINKS});
    }

    @Override
    public Optional<Long> sizeIfKnown() {
      BasicFileAttributes attrs;
      try {
        attrs = readAttributes();
      } catch (IOException e) {
                return Optional.absent();
      }

                  if (attrs.isDirectory() || attrs.isSymbolicLink()) {
        return Optional.absent();
      }

      return Optional.of(attrs.size());
    }

    @Override
    public long size() throws IOException {
      BasicFileAttributes attrs = readAttributes();

                  if (attrs.isDirectory()) {
        throw new IOException("can't read: is a directory");
      } else if (attrs.isSymbolicLink()) {
        throw new IOException("can't read: is a symbolic link");
      }

      return attrs.size();
    }

    @Override
    public byte[] read() throws IOException {
      try (SeekableByteChannel channel = Files.newByteChannel(path, options)) {
        return com.google.common.io.Files.readFile(
            Channels.newInputStream(channel), channel.size());
      }
    }

    @Override
    public CharSource asCharSource(Charset charset) {
      if (options.length == 0) {
                                        return new AsCharSource(charset) {
          @SuppressWarnings("FilesLinesLeak")           @Override
          public Stream<String> lines() throws IOException {
            return Files.lines(path, charset);
          }
        };
      }

      return super.asCharSource(charset);
    }

    @Override
    public String toString() {
      return "MoreFiles.asByteSource(" + path + ", " + Arrays.toString(options) + ")";
    }
  }

  
  public static ByteSink asByteSink(Path path, OpenOption... options) {
    return new PathByteSink(path, options);
  }

  private static final class PathByteSink extends ByteSink {

    private final Path path;
    private final OpenOption[] options;

    private PathByteSink(Path path, OpenOption... options) {
      this.path = checkNotNull(path);
      this.options = options.clone();
          }

    @Override
    public OutputStream openStream() throws IOException {
      return Files.newOutputStream(path, options);
    }

    @Override
    public String toString() {
      return "MoreFiles.asByteSink(" + path + ", " + Arrays.toString(options) + ")";
    }
  }

  
  public static CharSource asCharSource(Path path, Charset charset, OpenOption... options) {
    return asByteSource(path, options).asCharSource(charset);
  }

  
  public static CharSink asCharSink(Path path, Charset charset, OpenOption... options) {
    return asByteSink(path, options).asCharSink(charset);
  }

  
  public static ImmutableList<Path> listFiles(Path dir) throws IOException {
    try (DirectoryStream<Path> stream = Files.newDirectoryStream(dir)) {
      return ImmutableList.copyOf(stream);
    } catch (DirectoryIteratorException e) {
      throw e.getCause();
    }
  }

  
  @Deprecated
  public static TreeTraverser<Path> directoryTreeTraverser() {
    return DirectoryTreeTraverser.INSTANCE;
  }

  private static final class DirectoryTreeTraverser extends TreeTraverser<Path> {

    private static final DirectoryTreeTraverser INSTANCE = new DirectoryTreeTraverser();

    @Override
    public Iterable<Path> children(Path dir) {
      return fileTreeChildren(dir);
    }
  }

  
  public static Traverser<Path> fileTraverser() {
    return Traverser.forTree(FILE_TREE);
  }

  private static final SuccessorsFunction<Path> FILE_TREE =
      new SuccessorsFunction<Path>() {
        @Override
        public Iterable<Path> successors(Path path) {
          return fileTreeChildren(path);
        }
      };

  private static Iterable<Path> fileTreeChildren(Path dir) {
    if (Files.isDirectory(dir, NOFOLLOW_LINKS)) {
      try {
        return listFiles(dir);
      } catch (IOException e) {
                throw new DirectoryIteratorException(e);
      }
    }
    return ImmutableList.of();
  }

  
  public static Predicate<Path> isDirectory(LinkOption... options) {
    final LinkOption[] optionsCopy = options.clone();
    return new Predicate<Path>() {
      @Override
      public boolean apply(Path input) {
        return Files.isDirectory(input, optionsCopy);
      }

      @Override
      public String toString() {
        return "MoreFiles.isDirectory(" + Arrays.toString(optionsCopy) + ")";
      }
    };
  }

  
  private static boolean isDirectory(
      SecureDirectoryStream<Path> dir, Path name, LinkOption... options) throws IOException {
    return dir.getFileAttributeView(name, BasicFileAttributeView.class, options)
        .readAttributes()
        .isDirectory();
  }

  
  public static Predicate<Path> isRegularFile(LinkOption... options) {
    final LinkOption[] optionsCopy = options.clone();
    return new Predicate<Path>() {
      @Override
      public boolean apply(Path input) {
        return Files.isRegularFile(input, optionsCopy);
      }

      @Override
      public String toString() {
        return "MoreFiles.isRegularFile(" + Arrays.toString(optionsCopy) + ")";
      }
    };
  }

  
  public static boolean equal(Path path1, Path path2) throws IOException {
    checkNotNull(path1);
    checkNotNull(path2);
    if (Files.isSameFile(path1, path2)) {
      return true;
    }

    
    ByteSource source1 = asByteSource(path1);
    ByteSource source2 = asByteSource(path2);
    long len1 = source1.sizeIfKnown().or(0L);
    long len2 = source2.sizeIfKnown().or(0L);
    if (len1 != 0 && len2 != 0 && len1 != len2) {
      return false;
    }
    return source1.contentEquals(source2);
  }

  
  public static void touch(Path path) throws IOException {
    checkNotNull(path);

    try {
      Files.setLastModifiedTime(path, FileTime.fromMillis(System.currentTimeMillis()));
    } catch (NoSuchFileException e) {
      try {
        Files.createFile(path);
      } catch (FileAlreadyExistsException ignore) {
                                                      }
    }
  }

  
  public static void createParentDirectories(Path path, FileAttribute<?>... attrs)
      throws IOException {
                    Path normalizedAbsolutePath = path.toAbsolutePath().normalize();
    Path parent = normalizedAbsolutePath.getParent();
    if (parent == null) {
                              return;
    }

                    if (!Files.isDirectory(parent)) {
      Files.createDirectories(parent, attrs);
      if (!Files.isDirectory(parent)) {
        throw new IOException("Unable to create parent directories of " + path);
      }
    }
  }

  
  public static String getFileExtension(Path path) {
    Path name = path.getFileName();

        if (name == null) {
      return "";
    }

    String fileName = name.toString();
    int dotIndex = fileName.lastIndexOf('.');
    return dotIndex == -1 ? "" : fileName.substring(dotIndex + 1);
  }

  
  public static String getNameWithoutExtension(Path path) {
    Path name = path.getFileName();

        if (name == null) {
      return "";
    }

    String fileName = name.toString();
    int dotIndex = fileName.lastIndexOf('.');
    return dotIndex == -1 ? fileName : fileName.substring(0, dotIndex);
  }

  
  public static void deleteRecursively(Path path, RecursiveDeleteOption... options)
      throws IOException {
    Path parentPath = getParentPath(path);
    if (parentPath == null) {
      throw new FileSystemException(path.toString(), null, "can't delete recursively");
    }

    Collection<IOException> exceptions = null;     try {
      boolean sdsSupported = false;
      try (DirectoryStream<Path> parent = Files.newDirectoryStream(parentPath)) {
        if (parent instanceof SecureDirectoryStream) {
          sdsSupported = true;
          exceptions =
              deleteRecursivelySecure((SecureDirectoryStream<Path>) parent, path.getFileName());
        }
      }

      if (!sdsSupported) {
        checkAllowsInsecure(path, options);
        exceptions = deleteRecursivelyInsecure(path);
      }
    } catch (IOException e) {
      if (exceptions == null) {
        throw e;
      } else {
        exceptions.add(e);
      }
    }

    if (exceptions != null) {
      throwDeleteFailed(path, exceptions);
    }
  }

  
  public static void deleteDirectoryContents(Path path, RecursiveDeleteOption... options)
      throws IOException {
    Collection<IOException> exceptions = null;     try (DirectoryStream<Path> stream = Files.newDirectoryStream(path)) {
      if (stream instanceof SecureDirectoryStream) {
        SecureDirectoryStream<Path> sds = (SecureDirectoryStream<Path>) stream;
        exceptions = deleteDirectoryContentsSecure(sds);
      } else {
        checkAllowsInsecure(path, options);
        exceptions = deleteDirectoryContentsInsecure(stream);
      }
    } catch (IOException e) {
      if (exceptions == null) {
        throw e;
      } else {
        exceptions.add(e);
      }
    }

    if (exceptions != null) {
      throwDeleteFailed(path, exceptions);
    }
  }

  
  @NullableDecl
  private static Collection<IOException> deleteRecursivelySecure(
      SecureDirectoryStream<Path> dir, Path path) {
    Collection<IOException> exceptions = null;
    try {
      if (isDirectory(dir, path, NOFOLLOW_LINKS)) {
        try (SecureDirectoryStream<Path> childDir = dir.newDirectoryStream(path, NOFOLLOW_LINKS)) {
          exceptions = deleteDirectoryContentsSecure(childDir);
        }

                        if (exceptions == null) {
          dir.deleteDirectory(path);
        }
      } else {
        dir.deleteFile(path);
      }

      return exceptions;
    } catch (IOException e) {
      return addException(exceptions, e);
    }
  }

  
  @NullableDecl
  private static Collection<IOException> deleteDirectoryContentsSecure(
      SecureDirectoryStream<Path> dir) {
    Collection<IOException> exceptions = null;
    try {
      for (Path path : dir) {
        exceptions = concat(exceptions, deleteRecursivelySecure(dir, path.getFileName()));
      }

      return exceptions;
    } catch (DirectoryIteratorException e) {
      return addException(exceptions, e.getCause());
    }
  }

  
  @NullableDecl
  private static Collection<IOException> deleteRecursivelyInsecure(Path path) {
    Collection<IOException> exceptions = null;
    try {
      if (Files.isDirectory(path, NOFOLLOW_LINKS)) {
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(path)) {
          exceptions = deleteDirectoryContentsInsecure(stream);
        }
      }

                  if (exceptions == null) {
        Files.delete(path);
      }

      return exceptions;
    } catch (IOException e) {
      return addException(exceptions, e);
    }
  }

  
  @NullableDecl
  private static Collection<IOException> deleteDirectoryContentsInsecure(
      DirectoryStream<Path> dir) {
    Collection<IOException> exceptions = null;
    try {
      for (Path entry : dir) {
        exceptions = concat(exceptions, deleteRecursivelyInsecure(entry));
      }

      return exceptions;
    } catch (DirectoryIteratorException e) {
      return addException(exceptions, e.getCause());
    }
  }

  
  @NullableDecl
  private static Path getParentPath(Path path) {
    Path parent = path.getParent();

        if (parent != null) {
                                    return parent;
    }

        if (path.getNameCount() == 0) {
                                                                        return null;
    } else {
            return path.getFileSystem().getPath(".");
    }
  }

  
  private static void checkAllowsInsecure(Path path, RecursiveDeleteOption[] options)
      throws InsecureRecursiveDeleteException {
    if (!Arrays.asList(options).contains(RecursiveDeleteOption.ALLOW_INSECURE)) {
      throw new InsecureRecursiveDeleteException(path.toString());
    }
  }

  
  private static Collection<IOException> addException(
      @NullableDecl Collection<IOException> exceptions, IOException e) {
    if (exceptions == null) {
      exceptions = new ArrayList<>();     }
    exceptions.add(e);
    return exceptions;
  }

  
  @NullableDecl
  private static Collection<IOException> concat(
      @NullableDecl Collection<IOException> exceptions,
      @NullableDecl Collection<IOException> other) {
    if (exceptions == null) {
      return other;
    } else if (other != null) {
      exceptions.addAll(other);
    }
    return exceptions;
  }

  
  private static void throwDeleteFailed(Path path, Collection<IOException> exceptions)
      throws FileSystemException {
                FileSystemException deleteFailed =
        new FileSystemException(
            path.toString(),
            null,
            "failed to delete one or more files; see suppressed exceptions for details");
    for (IOException e : exceptions) {
      deleteFailed.addSuppressed(e);
    }
    throw deleteFailed;
  }
}
