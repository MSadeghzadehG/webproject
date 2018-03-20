

package sample.undertow.web;

import java.util.concurrent.Callable;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class SampleController {

	@GetMapping("/")
	public String helloWorld() {
		return "Hello World";
	}

	@GetMapping("/async")
	public Callable<String> helloWorldAsync() {
		return () -> "async: Hello World";
	}

}
