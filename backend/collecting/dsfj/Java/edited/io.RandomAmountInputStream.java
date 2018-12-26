

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Random;


class RandomAmountInputStream extends FilterInputStream {
  private final Random random;

  public RandomAmountInputStream(InputStream in, Random random) {
    super(checkNotNull(in));
    this.random = checkNotNull(random);
  }

  @Override
  public int read(byte[] b, int off, int len) throws IOException {
    return super.read(b, off, random.nextInt(len) + 1);
  }
}
