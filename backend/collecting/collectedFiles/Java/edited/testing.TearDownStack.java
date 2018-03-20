

package com.google.common.testing;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.Lists;
import com.google.errorprone.annotations.concurrent.GuardedBy;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;


@Beta
@GwtCompatible
public class TearDownStack implements TearDownAccepter {
  private static final Logger logger = Logger.getLogger(TearDownStack.class.getName());

  @GuardedBy("stack")
  final LinkedList<TearDown> stack = new LinkedList<>();

  private final boolean suppressThrows;

  public TearDownStack() {
    this.suppressThrows = false;
  }

  public TearDownStack(boolean suppressThrows) {
    this.suppressThrows = suppressThrows;
  }

  @Override
  public final void addTearDown(TearDown tearDown) {
    synchronized (stack) {
      stack.addFirst(checkNotNull(tearDown));
    }
  }

  
  public final void runTearDown() {
    List<Throwable> exceptions = new ArrayList<>();
    List<TearDown> stackCopy;
    synchronized (stack) {
      stackCopy = Lists.newArrayList(stack);
      stack.clear();
    }
    for (TearDown tearDown : stackCopy) {
      try {
        tearDown.tearDown();
      } catch (Throwable t) {
        if (suppressThrows) {
          logger.log(Level.INFO, "exception thrown during tearDown", t);
        } else {
          exceptions.add(t);
        }
      }
    }
    if ((!suppressThrows) && (exceptions.size() > 0)) {
      throw ClusterException.create(exceptions);
    }
  }
}
