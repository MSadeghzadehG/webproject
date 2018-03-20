

package org.springframework.boot.autoconfigure;

import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.validation.Configuration;
import javax.validation.Validation;

import org.apache.catalina.mbeans.MBeanFactory;

import org.springframework.boot.context.event.ApplicationFailedEvent;
import org.springframework.boot.context.event.ApplicationReadyEvent;
import org.springframework.boot.context.event.ApplicationStartingEvent;
import org.springframework.boot.context.event.SpringApplicationEvent;
import org.springframework.boot.context.logging.LoggingApplicationListener;
import org.springframework.context.ApplicationListener;
import org.springframework.core.annotation.Order;
import org.springframework.format.support.DefaultFormattingConversionService;
import org.springframework.http.converter.json.Jackson2ObjectMapperBuilder;
import org.springframework.http.converter.support.AllEncompassingFormHttpMessageConverter;


@Order(LoggingApplicationListener.DEFAULT_ORDER + 1)
public class BackgroundPreinitializer
		implements ApplicationListener<SpringApplicationEvent> {

	private static final AtomicBoolean preinitializationStarted = new AtomicBoolean(
			false);

	private static final CountDownLatch preinitializationComplete = new CountDownLatch(1);

	@Override
	public void onApplicationEvent(SpringApplicationEvent event) {
		if (event instanceof ApplicationStartingEvent
				&& preinitializationStarted.compareAndSet(false, true)) {
			performPreinitialization();
		}
		if ((event instanceof ApplicationReadyEvent
				|| event instanceof ApplicationFailedEvent)
				&& preinitializationStarted.get()) {
			try {
				preinitializationComplete.await();
			}
			catch (InterruptedException ex) {
				Thread.currentThread().interrupt();
			}
		}
	}

	private void performPreinitialization() {
		try {
			Thread thread = new Thread(new Runnable() {

				@Override
				public void run() {
					runSafely(new ConversionServiceInitializer());
					runSafely(new ValidationInitializer());
					runSafely(new MessageConverterInitializer());
					runSafely(new MBeanFactoryInitializer());
					runSafely(new JacksonInitializer());
					runSafely(new CharsetInitializer());
					preinitializationComplete.countDown();
				}

				public void runSafely(Runnable runnable) {
					try {
						runnable.run();
					}
					catch (Throwable ex) {
											}
				}

			}, "background-preinit");
			thread.start();
		}
		catch (Exception ex) {
												preinitializationComplete.countDown();
		}
	}

	
	private static class MessageConverterInitializer implements Runnable {

		@Override
		public void run() {
			new AllEncompassingFormHttpMessageConverter();
		}

	}

	
	private static class MBeanFactoryInitializer implements Runnable {

		@Override
		public void run() {
			new MBeanFactory();
		}

	}

	
	private static class ValidationInitializer implements Runnable {

		@Override
		public void run() {
			Configuration<?> configuration = Validation.byDefaultProvider().configure();
			configuration.buildValidatorFactory().getValidator();
		}

	}

	
	private static class JacksonInitializer implements Runnable {

		@Override
		public void run() {
			Jackson2ObjectMapperBuilder.json().build();
		}

	}

	
	private static class ConversionServiceInitializer implements Runnable {

		@Override
		public void run() {
			new DefaultFormattingConversionService();
		}

	}

	private static class CharsetInitializer implements Runnable {

		@Override
		public void run() {
			StandardCharsets.UTF_8.name();
			Charset.availableCharsets();
		}

	}

}
