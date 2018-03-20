

package io.reactivex.internal.disposables;

import io.reactivex.disposables.Disposable;


public interface DisposableContainer {

    
    boolean add(Disposable d);

    
    boolean remove(Disposable d);

    
    boolean delete(Disposable d);
}
