

package sample.integration.producer;

import java.io.File;
import java.io.FileOutputStream;

import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.context.annotation.Configuration;

@Configuration
public class ProducerApplication implements CommandLineRunner {

	@Override
	public void run(String... args) throws Exception {
		new File("target/input").mkdirs();
		if (args.length > 0) {
			FileOutputStream stream = new FileOutputStream(
					"target/input/data" + System.currentTimeMillis() + ".txt");
			for (String arg : args) {
				stream.write(arg.getBytes());
			}
			stream.flush();
			stream.close();
		}
	}

	public static void main(String[] args) {
		SpringApplication.run(ProducerApplication.class, args);
	}

}
