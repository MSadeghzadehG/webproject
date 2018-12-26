

package com.badlogic.gdx.jnigen;

import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;


public class AntPathMatcher {

	public boolean isPattern (String str) {
		return (str.indexOf('*') != -1 || str.indexOf('?') != -1);
	}

	public static String[] tokenizeToStringArray (String str, String delimiters, boolean trimTokens, boolean ignoreEmptyTokens) {
		if (str == null) {
			return null;
		}
		StringTokenizer st = new StringTokenizer(str, delimiters);
		List<String> tokens = new ArrayList<String>();
		while (st.hasMoreTokens()) {
			String token = st.nextToken();
			if (trimTokens) {
				token = token.trim();
			}
			if (!ignoreEmptyTokens || token.length() > 0) {
				tokens.add(token);
			}
		}
		return tokens.toArray(new String[tokens.size()]);
	}

	public boolean match (String file, String[] patterns) {
		if (patterns == null || patterns.length == 0) return true;
		for (String pattern : patterns) {
			if (match(pattern, file)) {
				return true;
			}
		}
		return false;
	}

	public boolean match (String pattern, String str) {
		if (str.startsWith("/") != pattern.startsWith("/")) {
			return false;
		}

		String[] patDirs = tokenizeToStringArray(pattern, "/", true, true);
		String[] strDirs = tokenizeToStringArray(str, "/", true, true);

		int patIdxStart = 0;
		int patIdxEnd = patDirs.length - 1;
		int strIdxStart = 0;
		int strIdxEnd = strDirs.length - 1;

				while (patIdxStart <= patIdxEnd && strIdxStart <= strIdxEnd) {
			String patDir = (String)patDirs[patIdxStart];
			if (patDir.equals("**")) {
				break;
			}
			if (!matchStrings(patDir, (String)strDirs[strIdxStart])) {
				return false;
			}
			patIdxStart++;
			strIdxStart++;
		}

		if (strIdxStart > strIdxEnd) {
						for (int i = patIdxStart; i <= patIdxEnd; i++) {
				if (!patDirs[i].equals("**")) {
					return false;
				}
			}
			return true;
		} else {
			if (patIdxStart > patIdxEnd) {
								return false;
			}
		}

				while (patIdxStart <= patIdxEnd && strIdxStart <= strIdxEnd) {
			String patDir = (String)patDirs[patIdxEnd];
			if (patDir.equals("**")) {
				break;
			}
			if (!matchStrings(patDir, (String)strDirs[strIdxEnd])) {
				return false;
			}
			patIdxEnd--;
			strIdxEnd--;
		}
		if (strIdxStart > strIdxEnd) {
						for (int i = patIdxStart; i <= patIdxEnd; i++) {
				if (!patDirs[i].equals("**")) {
					return false;
				}
			}
			return true;
		}

		while (patIdxStart != patIdxEnd && strIdxStart <= strIdxEnd) {
			int patIdxTmp = -1;
			for (int i = patIdxStart + 1; i <= patIdxEnd; i++) {
				if (patDirs[i].equals("**")) {
					patIdxTmp = i;
					break;
				}
			}
			if (patIdxTmp == patIdxStart + 1) {
								patIdxStart++;
				continue;
			}
									int patLength = (patIdxTmp - patIdxStart - 1);
			int strLength = (strIdxEnd - strIdxStart + 1);
			int foundIdx = -1;
			strLoop:
			for (int i = 0; i <= strLength - patLength; i++) {
				for (int j = 0; j < patLength; j++) {
					String subPat = (String)patDirs[patIdxStart + j + 1];
					String subStr = (String)strDirs[strIdxStart + i + j];
					if (!matchStrings(subPat, subStr)) {
						continue strLoop;
					}
				}

				foundIdx = strIdxStart + i;
				break;
			}

			if (foundIdx == -1) {
				return false;
			}

			patIdxStart = patIdxTmp;
			strIdxStart = foundIdx + patLength;
		}

		for (int i = patIdxStart; i <= patIdxEnd; i++) {
			if (!patDirs[i].equals("**")) {
				return false;
			}
		}

		return true;
	}

	
	private boolean matchStrings (String pattern, String str) {
		char[] patArr = pattern.toCharArray();
		char[] strArr = str.toCharArray();
		int patIdxStart = 0;
		int patIdxEnd = patArr.length - 1;
		int strIdxStart = 0;
		int strIdxEnd = strArr.length - 1;
		char ch;

		boolean containsStar = false;
		for (int i = 0; i < patArr.length; i++) {
			if (patArr[i] == '*') {
				containsStar = true;
				break;
			}
		}

		if (!containsStar) {
						if (patIdxEnd != strIdxEnd) {
				return false; 			}
			for (int i = 0; i <= patIdxEnd; i++) {
				ch = patArr[i];
				if (ch != '?') {
					if (ch != strArr[i]) {
						return false;					}
				}
			}
			return true; 		}

		if (patIdxEnd == 0) {
			return true; 		}

				while ((ch = patArr[patIdxStart]) != '*' && strIdxStart <= strIdxEnd) {
			if (ch != '?') {
				if (ch != strArr[strIdxStart]) {
					return false;				}
			}
			patIdxStart++;
			strIdxStart++;
		}
		if (strIdxStart > strIdxEnd) {
									for (int i = patIdxStart; i <= patIdxEnd; i++) {
				if (patArr[i] != '*') {
					return false;
				}
			}
			return true;
		}

				while ((ch = patArr[patIdxEnd]) != '*' && strIdxStart <= strIdxEnd) {
			if (ch != '?') {
				if (ch != strArr[strIdxEnd]) {
					return false;				}
			}
			patIdxEnd--;
			strIdxEnd--;
		}
		if (strIdxStart > strIdxEnd) {
									for (int i = patIdxStart; i <= patIdxEnd; i++) {
				if (patArr[i] != '*') {
					return false;
				}
			}
			return true;
		}

						while (patIdxStart != patIdxEnd && strIdxStart <= strIdxEnd) {
			int patIdxTmp = -1;
			for (int i = patIdxStart + 1; i <= patIdxEnd; i++) {
				if (patArr[i] == '*') {
					patIdxTmp = i;
					break;
				}
			}
			if (patIdxTmp == patIdxStart + 1) {
								patIdxStart++;
				continue;
			}
									int patLength = (patIdxTmp - patIdxStart - 1);
			int strLength = (strIdxEnd - strIdxStart + 1);
			int foundIdx = -1;
			strLoop:
			for (int i = 0; i <= strLength - patLength; i++) {
				for (int j = 0; j < patLength; j++) {
					ch = patArr[patIdxStart + j + 1];
					if (ch != '?') {
						if (ch != strArr[strIdxStart + i + j]) {
							continue strLoop;
						}
					}
				}

				foundIdx = strIdxStart + i;
				break;
			}

			if (foundIdx == -1) {
				return false;
			}

			patIdxStart = patIdxTmp;
			strIdxStart = foundIdx + patLength;
		}

						for (int i = patIdxStart; i <= patIdxEnd; i++) {
			if (patArr[i] != '*') {
				return false;
			}
		}

		return true;
	}

}
