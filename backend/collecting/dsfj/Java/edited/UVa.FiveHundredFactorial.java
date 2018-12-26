


import java.math.BigInteger;
import java.util.Scanner;

public class FiveHundredFactorial {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		while (input.hasNext()) {
			int number = input.nextInt();
			BigInteger product = BigInteger.ONE;
			for (int i = 2; i < number + 1; i++) {
				product = product.multiply(BigInteger.valueOf(i));
			}
			System.out.println(number + "!\n" + product);
		}
	}
}
