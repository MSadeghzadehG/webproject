

package sample.jndi;

import org.springframework.jms.annotation.JmsListener;
import org.springframework.stereotype.Component;

@Component
public class Messages {

	@JmsListener(destination = "java:/jms/queue/bootdemo")
	public void onMessage(String content) {
		System.out.println("----> " + content);
	}

}
