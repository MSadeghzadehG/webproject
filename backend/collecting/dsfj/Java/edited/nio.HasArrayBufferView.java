

package java.nio;

import com.google.gwt.typedarrays.shared.ArrayBufferView;

public interface HasArrayBufferView {

	public ArrayBufferView getTypedArray ();

	public int getElementSize ();
}
