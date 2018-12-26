

package java.util.regex;

import com.google.gwt.regexp.shared.RegExp;
import com.google.gwt.regexp.shared.MatchResult;


public class Matcher {
	private final RegExp regExp;
	private final String input;
	private final MatchResult matchResult;

	Matcher (Pattern pattern, CharSequence input) {
		this.regExp = pattern.regExp;
		this.input = String.valueOf(input);
		matchResult = regExp.exec(this.input);
	}

	public boolean find () {
		return regExp.test(input);
	}

	public boolean matches () {
		return regExp.test(input);
	}

	public String group (int group) {
		return matchResult.getGroup(group);
	}


    public static String quoteReplacement(String s) {
        if ((s.indexOf('\\') == -1) && (s.indexOf('$') == -1))
            return s;
        StringBuilder sb = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '\\' || c == '$') {
                sb.append('\\');
            }
            sb.append(c);
        }
        return sb.toString();
    }
}
