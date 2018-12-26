

package com.badlogic.gdx.utils;

import java.util.Arrays;


public class StringBuilder implements Appendable, CharSequence {
	static final int INITIAL_CAPACITY = 16;

	public char[] chars;
	public int length;

	private static final char[] digits = new char[] {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

	
	public static int numChars (int value, int radix) {
		int result = (value < 0) ? 2 : 1;
		while ((value /= radix) != 0)
			++result;
		return result;
	}

	
	public static int numChars (long value, int radix) {
		int result = (value < 0) ? 2 : 1;
		while ((value /= radix) != 0)
			++result;
		return result;
	}

	
	final char[] getValue () {
		return chars;
	}

	
	public StringBuilder () {
		chars = new char[INITIAL_CAPACITY];
	}

	
	public StringBuilder (int capacity) {
		if (capacity < 0) {
			throw new NegativeArraySizeException();
		}
		chars = new char[capacity];
	}

	
	public StringBuilder (CharSequence seq) {
		this(seq.toString());
	}

	public StringBuilder (StringBuilder builder) {
		length = builder.length;
		chars = new char[length + INITIAL_CAPACITY];
		System.arraycopy(builder.chars, 0, chars, 0, length);
	}

	
	public StringBuilder (String string) {
		length = string.length();
		chars = new char[length + INITIAL_CAPACITY];
		string.getChars(0, length, chars, 0);
	}

	private void enlargeBuffer (int min) {
		int newSize = (chars.length >> 1) + chars.length + 2;
		char[] newData = new char[min > newSize ? min : newSize];
		System.arraycopy(chars, 0, newData, 0, length);
		chars = newData;
	}

	final void appendNull () {
		int newSize = length + 4;
		if (newSize > chars.length) {
			enlargeBuffer(newSize);
		}
		chars[length++] = 'n';
		chars[length++] = 'u';
		chars[length++] = 'l';
		chars[length++] = 'l';
	}

	final void append0 (char[] value) {
		int newSize = length + value.length;
		if (newSize > chars.length) {
			enlargeBuffer(newSize);
		}
		System.arraycopy(value, 0, chars, length, value.length);
		length = newSize;
	}

	final void append0 (char[] value, int offset, int length) {
				if (offset > value.length || offset < 0) {
			throw new ArrayIndexOutOfBoundsException("Offset out of bounds: " + offset);
		}
		if (length < 0 || value.length - offset < length) {
			throw new ArrayIndexOutOfBoundsException("Length out of bounds: " + length);
		}

		int newSize = this.length + length;
		if (newSize > chars.length) {
			enlargeBuffer(newSize);
		}
		System.arraycopy(value, offset, chars, this.length, length);
		this.length = newSize;
	}

	final void append0 (char ch) {
		if (length == chars.length) {
			enlargeBuffer(length + 1);
		}
		chars[length++] = ch;
	}

	final void append0 (String string) {
		if (string == null) {
			appendNull();
			return;
		}
		int adding = string.length();
		int newSize = length + adding;
		if (newSize > chars.length) {
			enlargeBuffer(newSize);
		}
		string.getChars(0, adding, chars, length);
		length = newSize;
	}

	final void append0 (CharSequence s, int start, int end) {
		if (s == null) {
			s = "null";
		}
		if (start < 0 || end < 0 || start > end || end > s.length()) {
			throw new IndexOutOfBoundsException();
		}

		append0(s.subSequence(start, end).toString());
	}

	
	public int capacity () {
		return chars.length;
	}

	
	public char charAt (int index) {
		if (index < 0 || index >= length) {
			throw new StringIndexOutOfBoundsException(index);
		}
		return chars[index];
	}

	final void delete0 (int start, int end) {
		if (start >= 0) {
			if (end > length) {
				end = length;
			}
			if (end == start) {
				return;
			}
			if (end > start) {
				int count = length - end;
				if (count >= 0) System.arraycopy(chars, end, chars, start, count);
				length -= end - start;
				return;
			}
		}
		throw new StringIndexOutOfBoundsException();
	}

	final void deleteCharAt0 (int location) {
		if (0 > location || location >= length) {
			throw new StringIndexOutOfBoundsException(location);
		}
		int count = length - location - 1;
		if (count > 0) {
			System.arraycopy(chars, location + 1, chars, location, count);
		}
		length--;
	}

	
	public void ensureCapacity (int min) {
		if (min > chars.length) {
			int twice = (chars.length << 1) + 2;
			enlargeBuffer(twice > min ? twice : min);
		}
	}

	
	public void getChars (int start, int end, char[] dest, int destStart) {
		if (start > length || end > length || start > end) {
			throw new StringIndexOutOfBoundsException();
		}
		System.arraycopy(chars, start, dest, destStart, end - start);
	}

	final void insert0 (int index, char[] value) {
		if (0 > index || index > length) {
			throw new StringIndexOutOfBoundsException(index);
		}
		if (value.length != 0) {
			move(value.length, index);
			System.arraycopy(value, 0, value, index, value.length);
			length += value.length;
		}
	}

	final void insert0 (int index, char[] value, int start, int length) {
		if (0 <= index && index <= length) {
						if (start >= 0 && 0 <= length && length <= value.length - start) {
				if (length != 0) {
					move(length, index);
					System.arraycopy(value, start, chars, index, length);
					this.length += length;
				}
				return;
			}
			throw new StringIndexOutOfBoundsException("offset " + start + ", length " + length + ", char[].length " + value.length);
		}
		throw new StringIndexOutOfBoundsException(index);
	}

	final void insert0 (int index, char ch) {
		if (0 > index || index > length) {
						throw new ArrayIndexOutOfBoundsException(index);
		}
		move(1, index);
		chars[index] = ch;
		length++;
	}

	final void insert0 (int index, String string) {
		if (0 <= index && index <= length) {
			if (string == null) {
				string = "null";
			}
			int min = string.length();
			if (min != 0) {
				move(min, index);
				string.getChars(0, min, chars, index);
				length += min;
			}
		} else {
			throw new StringIndexOutOfBoundsException(index);
		}
	}

	final void insert0 (int index, CharSequence s, int start, int end) {
		if (s == null) {
			s = "null";
		}
		if (index < 0 || index > length || start < 0 || end < 0 || start > end || end > s.length()) {
			throw new IndexOutOfBoundsException();
		}
		insert0(index, s.subSequence(start, end).toString());
	}

	
	public int length () {
		return length;
	}

	private void move (int size, int index) {
		if (chars.length - length >= size) {
			System.arraycopy(chars, index, chars, index + size, length - index); 			return;
		}
		int a = length + size, b = (chars.length << 1) + 2;
		int newSize = a > b ? a : b;
		char[] newData = new char[newSize];
		System.arraycopy(chars, 0, newData, 0, index);
				System.arraycopy(chars, index, newData, index + size, length - index);
		chars = newData;
	}

	final void replace0 (int start, int end, String string) {
		if (start >= 0) {
			if (end > length) {
				end = length;
			}
			if (end > start) {
				int stringLength = string.length();
				int diff = end - start - stringLength;
				if (diff > 0) { 										System.arraycopy(chars, end, chars, start + stringLength, length - end);
				} else if (diff < 0) {
										move(-diff, end);
				}
				string.getChars(0, stringLength, chars, start);
				length -= diff;
				return;
			}
			if (start == end) {
				if (string == null) {
					throw new NullPointerException();
				}
				insert0(start, string);
				return;
			}
		}
		throw new StringIndexOutOfBoundsException();
	}

	final void reverse0 () {
		if (length < 2) {
			return;
		}
		int end = length - 1;
		char frontHigh = chars[0];
		char endLow = chars[end];
		boolean allowFrontSur = true, allowEndSur = true;
		for (int i = 0, mid = length / 2; i < mid; i++, --end) {
			char frontLow = chars[i + 1];
			char endHigh = chars[end - 1];
			boolean surAtFront = allowFrontSur && frontLow >= 0xdc00 && frontLow <= 0xdfff && frontHigh >= 0xd800
				&& frontHigh <= 0xdbff;
			if (surAtFront && length < 3) {
				return;
			}
			boolean surAtEnd = allowEndSur && endHigh >= 0xd800 && endHigh <= 0xdbff && endLow >= 0xdc00 && endLow <= 0xdfff;
			allowFrontSur = allowEndSur = true;
			if (surAtFront == surAtEnd) {
				if (surAtFront) {
										chars[end] = frontLow;
					chars[end - 1] = frontHigh;
					chars[i] = endHigh;
					chars[i + 1] = endLow;
					frontHigh = chars[i + 2];
					endLow = chars[end - 2];
					i++;
					end--;
				} else {
										chars[end] = frontHigh;
					chars[i] = endLow;
					frontHigh = frontLow;
					endLow = endHigh;
				}
			} else {
				if (surAtFront) {
										chars[end] = frontLow;
					chars[i] = endLow;
					endLow = endHigh;
					allowFrontSur = false;
				} else {
										chars[end] = frontHigh;
					chars[i] = endHigh;
					frontHigh = frontLow;
					allowEndSur = false;
				}
			}
		}
		if ((length & 1) == 1 && (!allowFrontSur || !allowEndSur)) {
			chars[end] = allowFrontSur ? endLow : frontHigh;
		}
	}

	
	public void setCharAt (int index, char ch) {
		if (0 > index || index >= length) {
			throw new StringIndexOutOfBoundsException(index);
		}
		chars[index] = ch;
	}

	
	public void setLength (int newLength) {
		if (newLength < 0) {
			throw new StringIndexOutOfBoundsException(newLength);
		}
		if (newLength > chars.length) {
			enlargeBuffer(newLength);
		} else {
			if (length < newLength) {
				Arrays.fill(chars, length, newLength, (char)0);
			}
		}
		length = newLength;
	}

	
	public String substring (int start) {
		if (0 <= start && start <= length) {
			if (start == length) {
				return "";
			}

						return new String(chars, start, length - start);
		}
		throw new StringIndexOutOfBoundsException(start);
	}

	
	public String substring (int start, int end) {
		if (0 <= start && start <= end && end <= length) {
			if (start == end) {
				return "";
			}

						return new String(chars, start, end - start);
		}
		throw new StringIndexOutOfBoundsException();
	}

	
	@Override
	public String toString () {
		if (length == 0) return "";
		return new String(chars, 0, length);
	}

	
	public CharSequence subSequence (int start, int end) {
		return substring(start, end);
	}

	
	public int indexOf (String string) {
		return indexOf(string, 0);
	}

	
	public int indexOf (String subString, int start) {
		if (start < 0) {
			start = 0;
		}
		int subCount = subString.length();
		if (subCount > 0) {
			if (subCount + start > length) {
				return -1;
			}
			char firstChar = subString.charAt(0);
			while (true) {
				int i = start;
				boolean found = false;
				for (; i < length; i++) {
					if (chars[i] == firstChar) {
						found = true;
						break;
					}
				}
				if (!found || subCount + i > length) {
					return -1; 				}
				int o1 = i, o2 = 0;
				while (++o2 < subCount && chars[++o1] == subString.charAt(o2)) {
									}
				if (o2 == subCount) {
					return i;
				}
				start = i + 1;
			}
		}
		return start < length || start == 0 ? start : length;
	}

	
	public int lastIndexOf (String string) {
		return lastIndexOf(string, length);
	}

	
	public int lastIndexOf (String subString, int start) {
		int subCount = subString.length();
		if (subCount <= length && start >= 0) {
			if (subCount > 0) {
				if (start > length - subCount) {
					start = length - subCount; 				}
								char firstChar = subString.charAt(0);
				while (true) {
					int i = start;
					boolean found = false;
					for (; i >= 0; --i) {
						if (chars[i] == firstChar) {
							found = true;
							break;
						}
					}
					if (!found) {
						return -1;
					}
					int o1 = i, o2 = 0;
					while (++o2 < subCount && chars[++o1] == subString.charAt(o2)) {
											}
					if (o2 == subCount) {
						return i;
					}
					start = i - 1;
				}
			}
			return start < length ? start : length;
		}
		return -1;
	}

	
	public void trimToSize () {
		if (length < chars.length) {
			char[] newValue = new char[length];
			System.arraycopy(chars, 0, newValue, 0, length);
			chars = newValue;
		}
	}

	
	public int codePointAt (int index) {
		if (index < 0 || index >= length) {
			throw new StringIndexOutOfBoundsException(index);
		}
		return Character.codePointAt(chars, index, length);
	}

	
	public int codePointBefore (int index) {
		if (index < 1 || index > length) {
			throw new StringIndexOutOfBoundsException(index);
		}
		return Character.codePointBefore(chars, index);
	}

	
	public int codePointCount (int beginIndex, int endIndex) {
		if (beginIndex < 0 || endIndex > length || beginIndex > endIndex) {
			throw new StringIndexOutOfBoundsException();
		}
		return Character.codePointCount(chars, beginIndex, endIndex - beginIndex);
	}

	
	public int offsetByCodePoints (int index, int codePointOffset) {
		return Character.offsetByCodePoints(chars, 0, length, index, codePointOffset);
	}

	
	public StringBuilder append (boolean b) {
		append0(b ? "true" : "false"); 		return this;
	}

	
	public StringBuilder append (char c) {
		append0(c);
		return this;
	}

	
	public StringBuilder append (int value) {
		return append(value, 0);
	}

	
	public StringBuilder append (int value, int minLength) {
		return append(value, minLength, '0');
	}

	
	public StringBuilder append (int value, final int minLength, final char prefix) {
		if (value == Integer.MIN_VALUE) {
			append0("-2147483648");
			return this;
		}
		if (value < 0) {
			append0('-');
			value = -value;
		}
		if (minLength > 1) {
			for (int j = minLength - numChars(value, 10); j > 0; --j)
				append(prefix);
		}
		if (value >= 10000) {
			if (value >= 1000000000) append0(digits[(int)((long)value % 10000000000L / 1000000000L)]);
			if (value >= 100000000) append0(digits[value % 1000000000 / 100000000]);
			if (value >= 10000000) append0(digits[value % 100000000 / 10000000]);
			if (value >= 1000000) append0(digits[value % 10000000 / 1000000]);
			if (value >= 100000) append0(digits[value % 1000000 / 100000]);
			append0(digits[value % 100000 / 10000]);
		}
		if (value >= 1000) append0(digits[value % 10000 / 1000]);
		if (value >= 100) append0(digits[value % 1000 / 100]);
		if (value >= 10) append0(digits[value % 100 / 10]);
		append0(digits[value % 10]);
		return this;
	}

	
	public StringBuilder append (long value) {
		return append(value, 0);
	}

	
	public StringBuilder append (long value, int minLength) {
		return append(value, minLength, '0');
	}

	
	public StringBuilder append (long value, int minLength, char prefix) {
		if (value == Long.MIN_VALUE) {
			append0("-9223372036854775808");
			return this;
		}
		if (value < 0L) {
			append0('-');
			value = -value;
		}
		if (minLength > 1) {
			for (int j = minLength - numChars(value, 10); j > 0; --j)
				append(prefix);
		}
		if (value >= 10000) {
			if (value >= 1000000000000000000L) append0(digits[(int)(value % 10000000000000000000D / 1000000000000000000L)]);
			if (value >= 100000000000000000L) append0(digits[(int)(value % 1000000000000000000L / 100000000000000000L)]);
			if (value >= 10000000000000000L) append0(digits[(int)(value % 100000000000000000L / 10000000000000000L)]);
			if (value >= 1000000000000000L) append0(digits[(int)(value % 10000000000000000L / 1000000000000000L)]);
			if (value >= 100000000000000L) append0(digits[(int)(value % 1000000000000000L / 100000000000000L)]);
			if (value >= 10000000000000L) append0(digits[(int)(value % 100000000000000L / 10000000000000L)]);
			if (value >= 1000000000000L) append0(digits[(int)(value % 10000000000000L / 1000000000000L)]);
			if (value >= 100000000000L) append0(digits[(int)(value % 1000000000000L / 100000000000L)]);
			if (value >= 10000000000L) append0(digits[(int)(value % 100000000000L / 10000000000L)]);
			if (value >= 1000000000L) append0(digits[(int)(value % 10000000000L / 1000000000L)]);
			if (value >= 100000000L) append0(digits[(int)(value % 1000000000L / 100000000L)]);
			if (value >= 10000000L) append0(digits[(int)(value % 100000000L / 10000000L)]);
			if (value >= 1000000L) append0(digits[(int)(value % 10000000L / 1000000L)]);
			if (value >= 100000L) append0(digits[(int)(value % 1000000L / 100000L)]);
			append0(digits[(int)(value % 100000L / 10000L)]);
		}
		if (value >= 1000L) append0(digits[(int)(value % 10000L / 1000L)]);
		if (value >= 100L) append0(digits[(int)(value % 1000L / 100L)]);
		if (value >= 10L) append0(digits[(int)(value % 100L / 10L)]);
		append0(digits[(int)(value % 10L)]);
		return this;
	}

	
	public StringBuilder append (float f) {
		append0(Float.toString(f));
		return this;
	}

	
	public StringBuilder append (double d) {
		append0(Double.toString(d));
		return this;
	}

	
	public StringBuilder append (Object obj) {
		if (obj == null) {
			appendNull();
		} else {
			append0(obj.toString());
		}
		return this;
	}

	
	public StringBuilder append (String str) {
		append0(str);
		return this;
	}

	
	public StringBuilder appendLine (String str) {
		append0(str);
		append0('\n');
		return this;
	}

	
	public StringBuilder append (char[] ch) {
		append0(ch);
		return this;
	}

	
	public StringBuilder append (char[] str, int offset, int len) {
		append0(str, offset, len);
		return this;
	}

	
	public StringBuilder append (CharSequence csq) {
		if (csq == null) {
			appendNull();
		} else if (csq instanceof StringBuilder) {
			StringBuilder builder = (StringBuilder)csq;
			append0(builder.chars, 0, builder.length);
		} else {
			append0(csq.toString());
		}
		return this;
	}

	public StringBuilder append (StringBuilder builder) {
		if (builder == null)
			appendNull();
		else
			append0(builder.chars, 0, builder.length);
		return this;
	}

	
	public StringBuilder append (CharSequence csq, int start, int end) {
		append0(csq, start, end);
		return this;
	}

	public StringBuilder append (StringBuilder builder, int start, int end) {
		if (builder == null)
			appendNull();
		else
			append0(builder.chars, start, end);
		return this;
	}

	
	public StringBuilder appendCodePoint (int codePoint) {
		append0(Character.toChars(codePoint));
		return this;
	}

	
	public StringBuilder delete (int start, int end) {
		delete0(start, end);
		return this;
	}

	
	public StringBuilder deleteCharAt (int index) {
		deleteCharAt0(index);
		return this;
	}

	
	public StringBuilder insert (int offset, boolean b) {
		insert0(offset, b ? "true" : "false"); 		return this;
	}

	
	public StringBuilder insert (int offset, char c) {
		insert0(offset, c);
		return this;
	}

	
	public StringBuilder insert (int offset, int i) {
		insert0(offset, Integer.toString(i));
		return this;
	}

	
	public StringBuilder insert (int offset, long l) {
		insert0(offset, Long.toString(l));
		return this;
	}

	
	public StringBuilder insert (int offset, float f) {
		insert0(offset, Float.toString(f));
		return this;
	}

	
	public StringBuilder insert (int offset, double d) {
		insert0(offset, Double.toString(d));
		return this;
	}

	
	public StringBuilder insert (int offset, Object obj) {
		insert0(offset, obj == null ? "null" : obj.toString()); 		return this;
	}

	
	public StringBuilder insert (int offset, String str) {
		insert0(offset, str);
		return this;
	}

	
	public StringBuilder insert (int offset, char[] ch) {
		insert0(offset, ch);
		return this;
	}

	
	public StringBuilder insert (int offset, char[] str, int strOffset, int strLen) {
		insert0(offset, str, strOffset, strLen);
		return this;
	}

	
	public StringBuilder insert (int offset, CharSequence s) {
		insert0(offset, s == null ? "null" : s.toString()); 		return this;
	}

	
	public StringBuilder insert (int offset, CharSequence s, int start, int end) {
		insert0(offset, s, start, end);
		return this;
	}

	
	public StringBuilder replace (int start, int end, String str) {
		replace0(start, end, str);
		return this;
	}

	
	public StringBuilder replace (String find, String replace) {
		int findLength = find.length(), replaceLength = replace.length();
		int index = 0;
		while (true) {
			index = indexOf(find, index);
			if (index == -1) break;
			replace0(index, index + findLength, replace);
			index += replaceLength;
		}
		return this;
	}

	
	public StringBuilder replace (char find, String replace) {
		int replaceLength = replace.length();
		int index = 0;
		while (true) {
			while (true) {
				if (index == length) return this;
				if (chars[index] == find) break;
				index++;
			}
			replace0(index, index + 1, replace);
			index += replaceLength;
		}
	}

	
	public StringBuilder reverse () {
		reverse0();
		return this;
	}

	public int hashCode () {
		final int prime = 31;
		int result = 1;
		result = prime + length;
		result = prime * result + Arrays.hashCode(chars);
		return result;
	}

	public boolean equals (Object obj) {
		if (this == obj) return true;
		if (obj == null) return false;
		if (getClass() != obj.getClass()) return false;
		StringBuilder other = (StringBuilder)obj;
		int length = this.length;
		if (length != other.length) return false;
		char[] chars = this.chars;
		char[] chars2 = other.chars;
		if (chars == chars2) return true;
		if (chars == null || chars2 == null) return false;
		for (int i = 0; i < length; i++)
			if (chars[i] != chars2[i]) return false;
		return true;
	}
}
