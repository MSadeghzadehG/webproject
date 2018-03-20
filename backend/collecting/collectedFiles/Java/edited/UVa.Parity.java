


import java.util.Scanner;

public class Parity {

	public static void main(String[] args) {
		while (true) {
			Scanner input = new Scanner(System.in);
			int number = input.nextInt();
			if (number == 0) {
				break;
			}
			String binaryInString = convertToBinary(number);
			int count = 0;
			for (int i = 0; i < binaryInString.length(); i++) {
				if ("1".equals(binaryInString.charAt(i) + "")) {
					count++;
				}
			}
			System.out.println("The parity of " + binaryInString + " is "
					+ count + " (mod 2).");

		}
	}

	private static String convertToBinary(int number) {
		StringBuilder s = new StringBuilder("");
		while (number != 0) {
			s = s.append(number % 2);
			number = number / 2;
		}
		return s.reverse().toString();
	}
}
