

package org.springframework.boot.convert;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.ParseException;
import java.util.Locale;

import org.springframework.format.Formatter;


final class InetAddressFormatter implements Formatter<InetAddress> {

	@Override
	public String print(InetAddress object, Locale locale) {
		return object.getHostAddress();
	}

	@Override
	public InetAddress parse(String text, Locale locale) throws ParseException {
		try {
			return InetAddress.getByName(text);
		}
		catch (UnknownHostException ex) {
			throw new IllegalStateException("Unknown host " + text, ex);
		}
	}

}
