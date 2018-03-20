


import java.text.DecimalFormat;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

public class Newspaper {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		int numberOfTestCases = input.nextInt();
		while (numberOfTestCases != 0) {
			Map<String, Integer> values = new HashMap<String, Integer>();
			int numberOfValuableCharacters = input.nextInt();
			while (numberOfValuableCharacters != 0) {
				values.put(input.next(), input.nextInt());
				numberOfValuableCharacters--;
			}
			int numberOfLines = input.nextInt();
			input.nextLine();
			double sum = 0;
			while (numberOfLines != 0) {
				String textAsString = input.nextLine();
				for (int i = 0; i < textAsString.length(); i++) {
					String c = textAsString.charAt(i) + "";
					if (values.containsKey(c)) {
						sum = sum + values.get(c);
					}
				}
				numberOfLines--;
			}
			sum = sum / 100;
			DecimalFormat formatter = new DecimalFormat("0.00");
			String sumFormatted = formatter.format(sum);
			System.out.println(sumFormatted + "$");
			numberOfTestCases--;
		}
	}

}