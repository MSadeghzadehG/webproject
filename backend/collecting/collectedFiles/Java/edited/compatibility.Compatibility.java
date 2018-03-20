


package com.google.gwt.corp.compatibility;

import java.io.UnsupportedEncodingException;

public class Compatibility {
	public interface Impl {
		int floatToIntBits (float f);

		float intBitsToFloat (int i);

		String createString (byte[] b, int ofs, int length);

		String getOriginatingServerAddress ();

		void printStackTrace (Throwable e);

		String createString (byte[] b, String encoding);

		void sleep (int i);
	}

	public static Impl impl;

	public static int floatToIntBits (float f) {
		return impl.floatToIntBits(f);
	}

	public static float intBitsToFloat (int i) {
		return impl.intBitsToFloat(i);
	}

	public static String newString (byte[] b) {
		return impl.createString(b, 0, b.length);
	}

	public static String newString (byte[] b, int s, int l) {
		return impl.createString(b, s, l);
	}

	public static String newString (byte[] b, String encoding) throws UnsupportedEncodingException {
		return impl.createString(b, encoding);
	}

	public static String getOriginatingServerAddress () {
		return impl.getOriginatingServerAddress();
	}

	public static void printStackTrace (Throwable e) {
		impl.printStackTrace(e);
	}

	public static void sleep (int i) {
		impl.sleep(i);
	}

	public static String bytesToString (byte[] data, int len) {
		char[] chars = new char[len];
		for (int i = 0; i < len; i++) {
			chars[i] = (char)data[i];
		}
		return new String(chars);
	}

	public static int stringToBytes (String s, byte[] data) {
		int len = s.length();
		for (int i = 0; i < len; i++) {
			data[i] = (byte)s.charAt(i);
		}
		return len;
	}

	public static String bytesToHex (byte[] data, int len) {
		char[] hex = new char[len * 2];
		for (int i = 0; i < len; i++) {
			int di = data[i];
			hex[i << 1] = Character.forDigit((di >> 4) & 15, 16);
			hex[(i << 1) + 1] = Character.forDigit(di & 15, 16);
		}
		return new String(hex);
	}

	public static int hexToBytes (String hex, byte[] data) {
		int len = hex.length();
		for (int i = 0; i < len; i += 2) {
			data[i >> 1] = (byte)((Character.digit(hex.charAt(i), 16) << 4) | Character.digit(hex.charAt(i + 1), 16));
		}
		return len / 2;
	}
}
