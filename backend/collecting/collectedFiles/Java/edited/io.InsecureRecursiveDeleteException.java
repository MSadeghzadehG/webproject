

package com.google.common.io;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.j2objc.annotations.J2ObjCIncompatible;
import java.nio.file.FileSystemException;
import java.nio.file.SecureDirectoryStream;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@Beta
@GwtIncompatible
@J2ObjCIncompatible public final class InsecureRecursiveDeleteException extends FileSystemException {

  public InsecureRecursiveDeleteException(@NullableDecl String file) {
    super(file, null, "unable to guarantee security of recursive delete");
  }
}
