package jadx.core.xmlgen;

import java.util.HashMap;
import java.util.Map;

public class ParserConstants {

	protected ParserConstants() {
	}


	protected static final String ANDROID_NS_URL = "http:
	
	protected static final int RES_NULL_TYPE = 0x0000;
	protected static final int RES_STRING_POOL_TYPE = 0x0001;
	protected static final int RES_TABLE_TYPE = 0x0002;

	protected static final int RES_XML_TYPE = 0x0003;
	protected static final int RES_XML_FIRST_CHUNK_TYPE = 0x0100;
	protected static final int RES_XML_START_NAMESPACE_TYPE = 0x0100;
	protected static final int RES_XML_END_NAMESPACE_TYPE = 0x0101;
	protected static final int RES_XML_START_ELEMENT_TYPE = 0x0102;
	protected static final int RES_XML_END_ELEMENT_TYPE = 0x0103;
	protected static final int RES_XML_CDATA_TYPE = 0x0104;
	protected static final int RES_XML_LAST_CHUNK_TYPE = 0x017f;
	protected static final int RES_XML_RESOURCE_MAP_TYPE = 0x0180;

	protected static final int RES_TABLE_PACKAGE_TYPE = 0x0200;
	protected static final int RES_TABLE_TYPE_TYPE = 0x0201;
	protected static final int RES_TABLE_TYPE_SPEC_TYPE = 0x0202;

	
		protected static final int TYPE_NULL = 0x00;
		protected static final int TYPE_REFERENCE = 0x01;
		protected static final int TYPE_ATTRIBUTE = 0x02;
		protected static final int TYPE_STRING = 0x03;
		protected static final int TYPE_FLOAT = 0x04;
		protected static final int TYPE_DIMENSION = 0x05;
		protected static final int TYPE_FRACTION = 0x06;
		protected static final int TYPE_FIRST_INT = 0x10;
		protected static final int TYPE_INT_DEC = 0x10;
		protected static final int TYPE_INT_HEX = 0x11;
		protected static final int TYPE_INT_BOOLEAN = 0x12;
		protected static final int TYPE_FIRST_COLOR_INT = 0x1c;
		protected static final int TYPE_INT_COLOR_ARGB8 = 0x1c;
		protected static final int TYPE_INT_COLOR_RGB8 = 0x1d;
		protected static final int TYPE_INT_COLOR_ARGB4 = 0x1e;
		protected static final int TYPE_INT_COLOR_RGB4 = 0x1f;
		protected static final int TYPE_LAST_COLOR_INT = 0x1f;
		protected static final int TYPE_LAST_INT = 0x1f;

			protected static final int COMPLEX_UNIT_SHIFT = 0;
	protected static final int COMPLEX_UNIT_MASK = 0xf;

		protected static final int COMPLEX_UNIT_PX = 0;
		protected static final int COMPLEX_UNIT_DIP = 1;
		protected static final int COMPLEX_UNIT_SP = 2;
		protected static final int COMPLEX_UNIT_PT = 3;
		protected static final int COMPLEX_UNIT_IN = 4;
		protected static final int COMPLEX_UNIT_MM = 5;

		protected static final int COMPLEX_UNIT_FRACTION = 0;
		protected static final int COMPLEX_UNIT_FRACTION_PARENT = 1;

				protected static final int COMPLEX_RADIX_SHIFT = 4;
	protected static final int COMPLEX_RADIX_MASK = 0x3;

		protected static final int COMPLEX_RADIX_23P0 = 0;
		protected static final int COMPLEX_RADIX_16P7 = 1;
		protected static final int COMPLEX_RADIX_8P15 = 2;
		protected static final int COMPLEX_RADIX_0P23 = 3;

			protected static final int COMPLEX_MANTISSA_SHIFT = 8;
	protected static final int COMPLEX_MANTISSA_MASK = 0xffffff;

	protected static final double MANTISSA_MULT = 1.0f / (1 << COMPLEX_MANTISSA_SHIFT);
	protected static final double[] RADIX_MULTS = new double[]{
			1.0f * MANTISSA_MULT,
			1.0f / (1 << 7) * MANTISSA_MULT,
			1.0f / (1 << 15) * MANTISSA_MULT,
			1.0f / (1 << 23) * MANTISSA_MULT
	};

	
	protected static final int SORTED_FLAG = 1;
	protected static final int UTF8_FLAG = 1 << 8;

	protected static final int NO_ENTRY = 0xFFFFFFFF;

	
			protected static final int FLAG_COMPLEX = 0x0001;
		protected static final int FLAG_PUBLIC = 0x0002;

	
	protected static final int ATTR_TYPE = makeResInternal(0);
		protected static final int ATTR_MIN = makeResInternal(1);
		protected static final int ATTR_MAX = makeResInternal(2);
		protected static final int ATTR_L10N = makeResInternal(3);

		protected static final int ATTR_OTHER = makeResInternal(4);
	protected static final int ATTR_ZERO = makeResInternal(5);
	protected static final int ATTR_ONE = makeResInternal(6);
	protected static final int ATTR_TWO = makeResInternal(7);
	protected static final int ATTR_FEW = makeResInternal(8);
	protected static final int ATTR_MANY = makeResInternal(9);

	protected static final Map<Integer, String> PLURALS_MAP;

	static {
		PLURALS_MAP = new HashMap<>();
		PLURALS_MAP.put(ATTR_OTHER, "other");
		PLURALS_MAP.put(ATTR_ZERO, "zero");
		PLURALS_MAP.put(ATTR_ONE, "one");
		PLURALS_MAP.put(ATTR_TWO, "two");
		PLURALS_MAP.put(ATTR_FEW, "few");
		PLURALS_MAP.put(ATTR_MANY, "many");
	}

	private static int makeResInternal(int entry) {
		return 0x01000000 | entry & 0xFFFF;
	}

	protected static boolean isResInternalId(int resid) {
		return (resid & 0xFFFF0000) != 0 && (resid & 0xFF0000) == 0;
	}

		protected static final int ATTR_TYPE_ANY = 0x0000FFFF;
		protected static final int ATTR_TYPE_REFERENCE = 1;
		protected static final int ATTR_TYPE_STRING = 1 << 1;
			protected static final int ATTR_TYPE_INTEGER = 1 << 2;
		protected static final int ATTR_TYPE_BOOLEAN = 1 << 3;
		protected static final int ATTR_TYPE_COLOR = 1 << 4;
		protected static final int ATTR_TYPE_FLOAT = 1 << 5;
		protected static final int ATTR_TYPE_DIMENSION = 1 << 6;
		protected static final int ATTR_TYPE_FRACTION = 1 << 7;
			protected static final int ATTR_TYPE_ENUM = 1 << 16;
			protected static final int ATTR_TYPE_FLAGS = 1 << 17;

		protected static final int ATTR_L10N_NOT_REQUIRED = 0;
	protected static final int ATTR_L10N_SUGGESTED = 1;
}
