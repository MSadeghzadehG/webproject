

package com.badlogic.gdx.backends.iosrobovm;

import java.io.File;

import com.badlogic.gdx.ApplicationLogger;
import org.robovm.apple.coregraphics.CGRect;
import org.robovm.apple.foundation.Foundation;
import org.robovm.apple.foundation.NSMutableDictionary;
import org.robovm.apple.foundation.NSObject;
import org.robovm.apple.foundation.NSString;
import org.robovm.apple.foundation.NSThread;
import org.robovm.apple.uikit.UIApplication;
import org.robovm.apple.uikit.UIApplicationDelegateAdapter;
import org.robovm.apple.uikit.UIApplicationLaunchOptions;
import org.robovm.apple.uikit.UIDevice;
import org.robovm.apple.uikit.UIInterfaceOrientation;
import org.robovm.apple.uikit.UIPasteboard;
import org.robovm.apple.uikit.UIScreen;
import org.robovm.apple.uikit.UIUserInterfaceIdiom;
import org.robovm.apple.uikit.UIViewController;
import org.robovm.apple.uikit.UIWindow;
import org.robovm.rt.bro.Bro;

import com.badlogic.gdx.Application;
import com.badlogic.gdx.ApplicationListener;
import com.badlogic.gdx.Audio;
import com.badlogic.gdx.Files;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.Graphics;
import com.badlogic.gdx.Input;
import com.badlogic.gdx.LifecycleListener;
import com.badlogic.gdx.Net;
import com.badlogic.gdx.Preferences;
import com.badlogic.gdx.backends.iosrobovm.objectal.OALAudioSession;
import com.badlogic.gdx.backends.iosrobovm.objectal.OALSimpleAudio;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.Clipboard;

public class IOSApplication implements Application {

	public static abstract class Delegate extends UIApplicationDelegateAdapter {
		private IOSApplication app;

		protected abstract IOSApplication createApplication ();

		@Override
		public boolean didFinishLaunching (UIApplication application, UIApplicationLaunchOptions launchOptions) {
			application.addStrongRef(this); 			this.app = createApplication();
			return app.didFinishLaunching(application, launchOptions);
		}

		@Override
		public void didBecomeActive (UIApplication application) {
			app.didBecomeActive(application);
		}

		@Override
		public void willEnterForeground (UIApplication application) {
			app.willEnterForeground(application);
		}

		@Override
		public void willResignActive (UIApplication application) {
			app.willResignActive(application);
		}

		@Override
		public void willTerminate (UIApplication application) {
			app.willTerminate(application);
		}
	}

	UIApplication uiApp;
	UIWindow uiWindow;
	ApplicationListener listener;
	IOSViewControllerListener viewControllerListener;
	IOSApplicationConfiguration config;
	IOSGraphics graphics;
	IOSAudio audio;
	IOSFiles files;
	IOSInput input;
	IOSNet net;
	int logLevel = Application.LOG_DEBUG;
	ApplicationLogger applicationLogger;

	
	float displayScaleFactor;

	private CGRect lastScreenBounds = null;

	Array<Runnable> runnables = new Array<Runnable>();
	Array<Runnable> executedRunnables = new Array<Runnable>();
	Array<LifecycleListener> lifecycleListeners = new Array<LifecycleListener>();

	public IOSApplication (ApplicationListener listener, IOSApplicationConfiguration config) {
		this.listener = listener;
		this.config = config;
	}

	final boolean didFinishLaunching (UIApplication uiApp, UIApplicationLaunchOptions options) {
		setApplicationLogger(new IOSApplicationLogger());
		Gdx.app = this;
		this.uiApp = uiApp;

				UIApplication.getSharedApplication().setIdleTimerDisabled(config.preventScreenDimming);

		Gdx.app.debug("IOSApplication", "iOS version: " + UIDevice.getCurrentDevice().getSystemVersion());
		
		Gdx.app.debug("IOSApplication", "Running in " + (Bro.IS_64BIT ? "64-bit" : "32-bit") + " mode");

		float scale = (float)(getIosVersion() >= 8 ? UIScreen.getMainScreen().getNativeScale() : UIScreen.getMainScreen()
			.getScale());
		if (scale >= 2.0f) {
			Gdx.app.debug("IOSApplication", "scale: " + scale);
			if (UIDevice.getCurrentDevice().getUserInterfaceIdiom() == UIUserInterfaceIdiom.Pad) {
								displayScaleFactor = config.displayScaleLargeScreenIfRetina * scale;
			} else {
								displayScaleFactor = config.displayScaleSmallScreenIfRetina * scale;
			}
		} else {
						if (UIDevice.getCurrentDevice().getUserInterfaceIdiom() == UIUserInterfaceIdiom.Pad) {
								displayScaleFactor = config.displayScaleLargeScreenIfNonRetina;
			} else {
								displayScaleFactor = config.displayScaleSmallScreenIfNonRetina;
			}
		}

				this.input = createInput();
		this.graphics = createGraphics(scale);
		Gdx.gl = Gdx.gl20 = graphics.gl20;
		Gdx.gl30 = graphics.gl30;
		this.files = new IOSFiles();
		this.audio = new IOSAudio(config);
		this.net = new IOSNet(this);

		Gdx.files = this.files;
		Gdx.graphics = this.graphics;
		Gdx.audio = this.audio;
		Gdx.input = this.input;
		Gdx.net = this.net;

		this.input.setupPeripherals();

		this.uiWindow = new UIWindow(UIScreen.getMainScreen().getBounds());
		this.uiWindow.setRootViewController(this.graphics.viewController);
		this.uiWindow.makeKeyAndVisible();
		Gdx.app.debug("IOSApplication", "created");
		return true;
	}

	protected IOSGraphics createGraphics(float scale) {
		 return new IOSGraphics(scale, this, config, input, config.useGL30);
	}

	protected IOSInput createInput() {
		 return new IOSInput(this);
	}

	private int getIosVersion () {
		String systemVersion = UIDevice.getCurrentDevice().getSystemVersion();
		int version = Integer.parseInt(systemVersion.split("\\.")[0]);
		return version;
	}

	
	public UIViewController getUIViewController () {
		return graphics.viewController;
	}

	
	public UIWindow getUIWindow () {
		return uiWindow;
	}

	
	protected CGRect getBounds () {
		final CGRect screenBounds = UIScreen.getMainScreen().getBounds();
		final CGRect statusBarFrame = uiApp.getStatusBarFrame();
		final UIInterfaceOrientation statusBarOrientation = uiApp.getStatusBarOrientation();

		double statusBarHeight = Math.min(statusBarFrame.getWidth(), statusBarFrame.getHeight());

		double screenWidth = screenBounds.getWidth();
		double screenHeight = screenBounds.getHeight();

				switch (statusBarOrientation) {
		case LandscapeLeft:
		case LandscapeRight:
			if (screenHeight > screenWidth) {
				debug("IOSApplication", "Switching reported width and height (w=" + screenWidth + " h=" + screenHeight + ")");
				double tmp = screenHeight;
								screenHeight = screenWidth;
				screenWidth = tmp;
			}
		}

				screenWidth *= displayScaleFactor;
		screenHeight *= displayScaleFactor;

		if (statusBarHeight != 0.0) {
			debug("IOSApplication", "Status bar is visible (height = " + statusBarHeight + ")");
			statusBarHeight *= displayScaleFactor;
			screenHeight -= statusBarHeight;
		} else {
			debug("IOSApplication", "Status bar is not visible");
		}

		debug("IOSApplication", "Total computed bounds are w=" + screenWidth + " h=" + screenHeight);

		return lastScreenBounds = new CGRect(0.0, statusBarHeight, screenWidth, screenHeight);
	}

	protected CGRect getCachedBounds () {
		if (lastScreenBounds == null)
			return getBounds();
		else
			return lastScreenBounds;
	}

	final void didBecomeActive (UIApplication uiApp) {
		Gdx.app.debug("IOSApplication", "resumed");
						OALAudioSession audioSession = OALAudioSession.sharedInstance();
		if (audioSession != null) {
			audioSession.forceEndInterruption();
		}
		if (config.allowIpod) {
			OALSimpleAudio audio = OALSimpleAudio.sharedInstance();
			if (audio != null) {
				audio.setUseHardwareIfAvailable(false);
			}
		}
		graphics.makeCurrent();
		graphics.resume();
	}

	final void willEnterForeground (UIApplication uiApp) {
						OALAudioSession audioSession = OALAudioSession.sharedInstance();
		if (audioSession != null) {
			audioSession.forceEndInterruption();
		}
	}

	final void willResignActive (UIApplication uiApp) {
		Gdx.app.debug("IOSApplication", "paused");
		graphics.makeCurrent();
		graphics.pause();
		Gdx.gl.glFinish();
	}

	final void willTerminate (UIApplication uiApp) {
		Gdx.app.debug("IOSApplication", "disposed");
		graphics.makeCurrent();
		Array<LifecycleListener> listeners = lifecycleListeners;
		synchronized (listeners) {
			for (LifecycleListener listener : listeners) {
				listener.pause();
			}
		}
		listener.dispose();
		Gdx.gl.glFinish();
	}

	@Override
	public ApplicationListener getApplicationListener () {
		return listener;
	}

	@Override
	public Graphics getGraphics () {
		return graphics;
	}

	@Override
	public Audio getAudio () {
		return audio;
	}

	@Override
	public Input getInput () {
		return input;
	}

	@Override
	public Files getFiles () {
		return files;
	}

	@Override
	public Net getNet () {
		return net;
	}

	@Override
	public void debug (String tag, String message) {
		if (logLevel >= LOG_DEBUG) getApplicationLogger().debug(tag, message);
	}

	@Override
	public void debug (String tag, String message, Throwable exception) {
		if (logLevel >= LOG_DEBUG) getApplicationLogger().debug(tag, message, exception);
	}

	@Override
	public void log (String tag, String message) {
		if (logLevel >= LOG_INFO) getApplicationLogger().log(tag, message);
	}

	@Override
	public void log (String tag, String message, Throwable exception) {
		if (logLevel >= LOG_INFO) getApplicationLogger().log(tag, message, exception);
	}

	@Override
	public void error (String tag, String message) {
		if (logLevel >= LOG_ERROR) getApplicationLogger().error(tag, message);
	}

	@Override
	public void error (String tag, String message, Throwable exception) {
		if (logLevel >= LOG_ERROR) getApplicationLogger().error(tag, message, exception);
	}

	@Override
	public void setLogLevel (int logLevel) {
		this.logLevel = logLevel;
	}

	@Override
	public int getLogLevel () {
		return logLevel;
	}

	@Override
	public void setApplicationLogger (ApplicationLogger applicationLogger) {
		this.applicationLogger = applicationLogger;
	}

	@Override
	public ApplicationLogger getApplicationLogger () {
		return applicationLogger;
	}

	@Override
	public ApplicationType getType () {
		return ApplicationType.iOS;
	}

	@Override
	public int getVersion () {
		return Integer.parseInt(UIDevice.getCurrentDevice().getSystemVersion().split("\\.")[0]);
	}

	@Override
	public long getJavaHeap () {
		return Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory();
	}

	@Override
	public long getNativeHeap () {
		return getJavaHeap();
	}

	@Override
	public Preferences getPreferences (String name) {
		File libraryPath = new File(System.getenv("HOME"), "Library");
		File finalPath = new File(libraryPath, name + ".plist");

		@SuppressWarnings("unchecked")
		NSMutableDictionary<NSString, NSObject> nsDictionary = (NSMutableDictionary<NSString, NSObject>)NSMutableDictionary
			.read(finalPath);

				if (nsDictionary == null) {
			nsDictionary = new NSMutableDictionary<NSString, NSObject>();
			nsDictionary.write(finalPath, false);
		}
		return new IOSPreferences(nsDictionary, finalPath.getAbsolutePath());
	}

	@Override
	public void postRunnable (Runnable runnable) {
		synchronized (runnables) {
			runnables.add(runnable);
			Gdx.graphics.requestRendering();
		}
	}

	public void processRunnables () {
		synchronized (runnables) {
			executedRunnables.clear();
			executedRunnables.addAll(runnables);
			runnables.clear();
		}
		for (int i = 0; i < executedRunnables.size; i++) {
			try {
				executedRunnables.get(i).run();
			} catch (Throwable t) {
				t.printStackTrace();
			}
		}
	}

	@Override
	public void exit () {
		NSThread.exit();
	}

	@Override
	public Clipboard getClipboard () {
		return new Clipboard() {
			@Override
			public void setContents (String content) {
				UIPasteboard.getGeneralPasteboard().setString(content);
			}

			@Override
			public String getContents () {
				return UIPasteboard.getGeneralPasteboard().getString();
			}
		};
	}

	@Override
	public void addLifecycleListener (LifecycleListener listener) {
		synchronized (lifecycleListeners) {
			lifecycleListeners.add(listener);
		}
	}

	@Override
	public void removeLifecycleListener (LifecycleListener listener) {
		synchronized (lifecycleListeners) {
			lifecycleListeners.removeValue(listener, true);
		}
	}

	
	public void addViewControllerListener (IOSViewControllerListener listener) {
		viewControllerListener = listener;
	}
}
