

import java.util.Scanner;

public class GoldbachConjecture {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		boolean[] isPrime = sieveOfEratosthenes(1000000);
		int number = input.nextInt();
		while (number != 0) {
			boolean found = false;
			for (int i = 3; i < number && !found; i++) {
				if (isPrime[i]) {
					int currentPrime = i;
					int j = number - currentPrime;
					if (isPrime[j]) {
						System.out.println(number + " = " + currentPrime
								+ " + " + j);
						found = true;
					}
				}
			}
			if (!found) {
				System.out.println("Goldbach's conjecture is wrong.");
			}
			number = input.nextInt();
		}
	}

	private static boolean[] sieveOfEratosthenes(int number) {
		boolean[] isPrime = new boolean[number + 1];
		for (int i = 2; i < number + 1; i++) {
			isPrime[i] = true;
		}

		for (int factor = 2; factor * factor <= number; factor++) {
			if (isPrime[factor]) {
				for (int j = factor; factor * j <= number; j++) {
					isPrime[factor * j] = false;
				}
			}
		}
		return isPrime;
	}
}
