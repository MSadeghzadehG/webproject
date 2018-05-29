
package com.iluwatar.reader.writer.lock;

import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class ReaderWriterLock implements ReadWriteLock {
  
  private static final Logger LOGGER = LoggerFactory.getLogger(ReaderWriterLock.class);


  private Object readerMutex = new Object();

  private int currentReaderCount;

  
  private Set<Object> globalMutex = new HashSet<>();

  private ReadLock readerLock = new ReadLock();
  private WriteLock writerLock = new WriteLock();

  @Override
  public Lock readLock() {
    return readerLock;
  }

  @Override
  public Lock writeLock() {
    return writerLock;
  }

  
  private boolean doesWriterOwnThisLock() {
    return globalMutex.contains(writerLock);
  }

  
  private boolean isLockFree() {
    return globalMutex.isEmpty();
  }

  
  private class ReadLock implements Lock {

    @Override
    public void lock() {
      synchronized (readerMutex) {
        currentReaderCount++;
        if (currentReaderCount == 1) {
          acquireForReaders();
        }
      }
    }

    
    private void acquireForReaders() {
            synchronized (globalMutex) {
                        while (doesWriterOwnThisLock()) {
          try {
            globalMutex.wait();
          } catch (InterruptedException e) {
            LOGGER.info("InterruptedException while waiting for globalMutex in acquireForReaders", e);
            Thread.currentThread().interrupt();
          }
        }
        globalMutex.add(this);
      }
    }

    @Override
    public void unlock() {

      synchronized (readerMutex) {
        currentReaderCount--;
                        if (currentReaderCount == 0) {
          synchronized (globalMutex) {
                        globalMutex.remove(this);
            globalMutex.notifyAll();
          }
        }
      }

    }

    @Override
    public void lockInterruptibly() throws InterruptedException {
      throw new UnsupportedOperationException();
    }

    @Override
    public boolean tryLock() {
      throw new UnsupportedOperationException();
    }

    @Override
    public boolean tryLock(long time, TimeUnit unit) throws InterruptedException {
      throw new UnsupportedOperationException();
    }

    @Override
    public Condition newCondition() {
      throw new UnsupportedOperationException();
    }

  }

  
  private class WriteLock implements Lock {

    @Override
    public void lock() {

      synchronized (globalMutex) {

                while (!isLockFree()) {
          try {
            globalMutex.wait();
          } catch (InterruptedException e) {
            LOGGER.info("InterruptedException while waiting for globalMutex to begin writing", e);
            Thread.currentThread().interrupt();
          }
        }
                globalMutex.add(this);
      }
    }

    @Override
    public void unlock() {

      synchronized (globalMutex) {
        globalMutex.remove(this);
                globalMutex.notifyAll();
      }
    }

    @Override
    public void lockInterruptibly() throws InterruptedException {
      throw new UnsupportedOperationException();
    }

    @Override
    public boolean tryLock() {
      throw new UnsupportedOperationException();
    }

    @Override
    public boolean tryLock(long time, TimeUnit unit) throws InterruptedException {
      throw new UnsupportedOperationException();
    }

    @Override
    public Condition newCondition() {
      throw new UnsupportedOperationException();
    }
  }

}
