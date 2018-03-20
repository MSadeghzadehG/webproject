

package org.springframework.boot.cli.command.encodepassword;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Supplier;

import joptsimple.OptionSet;
import joptsimple.OptionSpec;

import org.springframework.boot.cli.command.Command;
import org.springframework.boot.cli.command.HelpExample;
import org.springframework.boot.cli.command.OptionParsingCommand;
import org.springframework.boot.cli.command.options.OptionHandler;
import org.springframework.boot.cli.command.status.ExitStatus;
import org.springframework.boot.cli.util.Log;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.factory.PasswordEncoderFactories;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.security.crypto.password.Pbkdf2PasswordEncoder;
import org.springframework.util.StringUtils;


public class EncodePasswordCommand extends OptionParsingCommand {

	private static final Map<String, Supplier<PasswordEncoder>> ENCODERS;

	static {
		Map<String, Supplier<PasswordEncoder>> encoders = new LinkedHashMap<>();
		encoders.put("default",
				PasswordEncoderFactories::createDelegatingPasswordEncoder);
		encoders.put("bcrypt", BCryptPasswordEncoder::new);
		encoders.put("pbkdf2", Pbkdf2PasswordEncoder::new);
		ENCODERS = Collections.unmodifiableMap(encoders);
	}

	public EncodePasswordCommand() {
		super("encodepassword", "Encode a password for use with Spring Security",
				new EncodePasswordOptionHandler());
	}

	@Override
	public String getUsageHelp() {
		return "[options] <password to encode>";
	}

	@Override
	public Collection<HelpExample> getExamples() {
		List<HelpExample> examples = new ArrayList<>();
		examples.add(new HelpExample("To encode a password with the default encoder",
				"spring encodepassword mypassword"));
		examples.add(new HelpExample("To encode a password with pbkdf2",
				"spring encodepassword -a pbkdf2 mypassword"));
		return examples;
	}

	private static final class EncodePasswordOptionHandler extends OptionHandler {

		private OptionSpec<String> algorithm;

		@Override
		protected void options() {
			this.algorithm = option(Arrays.asList("algorithm", "a"),
					"The algorithm to use").withRequiredArg().defaultsTo("default");
		}

		@Override
		protected ExitStatus run(OptionSet options) throws Exception {
			if (options.nonOptionArguments().size() != 1) {
				Log.error("A single password option must be provided");
				return ExitStatus.ERROR;
			}
			String algorithm = options.valueOf(this.algorithm);
			String password = (String) options.nonOptionArguments().get(0);
			Supplier<PasswordEncoder> encoder = ENCODERS.get(algorithm);
			if (encoder == null) {
				Log.error("Unknown algorithm, valid options are: " + StringUtils
						.collectionToCommaDelimitedString(ENCODERS.keySet()));
				return ExitStatus.ERROR;
			}
			Log.info(encoder.get().encode(password));
			return ExitStatus.OK;
		}

	}

}
