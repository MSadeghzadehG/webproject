

package com.badlogic.gdx.tools.texturepacker;

import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.badlogic.gdx.tools.FileProcessor;
import com.badlogic.gdx.tools.texturepacker.TexturePacker.Settings;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.Json;
import com.badlogic.gdx.utils.JsonReader;
import com.badlogic.gdx.utils.ObjectMap;


public class TexturePackerFileProcessor extends FileProcessor {
	private final Settings defaultSettings;
	private ObjectMap<File, Settings> dirToSettings = new ObjectMap();
	private Json json = new Json();
	private String packFileName;
	private File root;
	ArrayList<File> ignoreDirs = new ArrayList();

	public TexturePackerFileProcessor () {
		this(new Settings(), "pack.atlas");
	}

	public TexturePackerFileProcessor (Settings defaultSettings, String packFileName) {
		this.defaultSettings = defaultSettings;

		if (packFileName.toLowerCase().endsWith(defaultSettings.atlasExtension.toLowerCase()))
			packFileName = packFileName.substring(0, packFileName.length() - defaultSettings.atlasExtension.length());
		this.packFileName = packFileName;

		setFlattenOutput(true);
		addInputSuffix(".png", ".jpg", ".jpeg");
	}

	public ArrayList<Entry> process (File inputFile, File outputRoot) throws Exception {
		root = inputFile;

				final ArrayList<File> settingsFiles = new ArrayList();
		FileProcessor settingsProcessor = new FileProcessor() {
			protected void processFile (Entry inputFile) throws Exception {
				settingsFiles.add(inputFile.inputFile);
			}
		};
		settingsProcessor.addInputRegex("pack\\.json");
		settingsProcessor.process(inputFile, null);
				Collections.sort(settingsFiles, new Comparator<File>() {
			public int compare (File file1, File file2) {
				return file1.toString().length() - file2.toString().length();
			}
		});
		for (File settingsFile : settingsFiles) {
						Settings settings = null;
			File parent = settingsFile.getParentFile();
			while (true) {
				if (parent.equals(root)) break;
				parent = parent.getParentFile();
				settings = dirToSettings.get(parent);
				if (settings != null) {
					settings = new Settings(settings);
					break;
				}
			}
			if (settings == null) settings = new Settings(defaultSettings);
						merge(settings, settingsFile);
			dirToSettings.put(settingsFile.getParentFile(), settings);
		}

				return super.process(inputFile, outputRoot);
	}

	private void merge (Settings settings, File settingsFile) {
		try {
			json.readFields(settings, new JsonReader().parse(new FileReader(settingsFile)));
		} catch (Exception ex) {
			throw new GdxRuntimeException("Error reading settings file: " + settingsFile, ex);
		}
	}

	public ArrayList<Entry> process (File[] files, File outputRoot) throws Exception {
				if (outputRoot.exists()) {
						File settingsFile = new File(root, "pack.json");
			Settings rootSettings = defaultSettings;
			if (settingsFile.exists()) {
				rootSettings = new Settings(rootSettings);
				merge(rootSettings, settingsFile);
			}

			String atlasExtension = rootSettings.atlasExtension == null ? "" : rootSettings.atlasExtension;
			atlasExtension = Pattern.quote(atlasExtension);

			for (int i = 0, n = rootSettings.scale.length; i < n; i++) {
				FileProcessor deleteProcessor = new FileProcessor() {
					protected void processFile (Entry inputFile) throws Exception {
						inputFile.inputFile.delete();
					}
				};
				deleteProcessor.setRecursive(false);

				File packFile = new File(rootSettings.getScaledPackFileName(packFileName, i));
				String scaledPackFileName = packFile.getName();

				String prefix = packFile.getName();
				int dotIndex = prefix.lastIndexOf('.');
				if (dotIndex != -1) prefix = prefix.substring(0, dotIndex);
				deleteProcessor.addInputRegex("(?i)" + prefix + "\\d*\\.(png|jpg|jpeg)");
				deleteProcessor.addInputRegex("(?i)" + prefix + atlasExtension);

				String dir = packFile.getParent();
				if (dir == null)
					deleteProcessor.process(outputRoot, null);
				else if (new File(outputRoot + "/" + dir).exists()) 					deleteProcessor.process(outputRoot + "/" + dir, null);
			}
		}
		return super.process(files, outputRoot);
	}

	protected void processDir (Entry inputDir, ArrayList<Entry> files) throws Exception {
		if (ignoreDirs.contains(inputDir.inputFile)) return;

				Settings settings = null;
		File parent = inputDir.inputFile;
		while (true) {
			settings = dirToSettings.get(parent);
			if (settings != null) break;
			if (parent == null || parent.equals(root)) break;
			parent = parent.getParentFile();
		}
		if (settings == null) settings = defaultSettings;

		if (settings.ignore) return;

		if (settings.combineSubdirectories) {
						files = new FileProcessor(this) {
				protected void processDir (Entry entryDir, ArrayList<Entry> files) {
					ignoreDirs.add(entryDir.inputFile);
				}

				protected void processFile (Entry entry) {
					addProcessedFile(entry);
				}
			}.process(inputDir.inputFile, null);
		}

		if (files.isEmpty()) return;

				Collections.sort(files, new Comparator<Entry>() {
			final Pattern digitSuffix = Pattern.compile("(.*?)(\\d+)$");

			public int compare (Entry entry1, Entry entry2) {
				String full1 = entry1.inputFile.getName();
				int dotIndex = full1.lastIndexOf('.');
				if (dotIndex != -1) full1 = full1.substring(0, dotIndex);

				String full2 = entry2.inputFile.getName();
				dotIndex = full2.lastIndexOf('.');
				if (dotIndex != -1) full2 = full2.substring(0, dotIndex);

				String name1 = full1, name2 = full2;
				int num1 = 0, num2 = 0;

				Matcher matcher = digitSuffix.matcher(full1);
				if (matcher.matches()) {
					try {
						num1 = Integer.parseInt(matcher.group(2));
						name1 = matcher.group(1);
					} catch (Exception ignored) {
					}
				}
				matcher = digitSuffix.matcher(full2);
				if (matcher.matches()) {
					try {
						num2 = Integer.parseInt(matcher.group(2));
						name2 = matcher.group(1);
					} catch (Exception ignored) {
					}
				}
				int compare = name1.compareTo(name2);
				if (compare != 0 || num1 == num2) return compare;
				return num1 - num2;
			}
		});

				if (!settings.silent) System.out.println(inputDir.inputFile.getName());
		TexturePacker packer = newTexturePacker(root, settings);
		for (Entry file : files)
			packer.addImage(file.inputFile);
		packer.pack(inputDir.outputDir, packFileName);
	}

	protected TexturePacker newTexturePacker (File root, Settings settings) {
		return new TexturePacker(root, settings);
	}
}
