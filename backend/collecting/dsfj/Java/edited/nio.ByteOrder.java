

package java.nio;



public final class ByteOrder {

	
	public static final ByteOrder BIG_ENDIAN = new ByteOrder("BIG_ENDIAN"); 
	
	public static final ByteOrder LITTLE_ENDIAN = new ByteOrder("LITTLE_ENDIAN"); 
	private static final ByteOrder NATIVE_ORDER;

	static {
		NATIVE_ORDER = LITTLE_ENDIAN;
	}

	
	public static ByteOrder nativeOrder () {
		return NATIVE_ORDER;
	}

	private final String name;

	private ByteOrder (String name) {
		super();
		this.name = name;
	}

	
	public String toString () {
		return name;
	}
}
