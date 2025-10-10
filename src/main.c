#include "kmeans.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>


void generate_points(int n, double mean[2], double sigma, double points[n][2], unsigned int * seed) {
	double epsilon = DBL_MIN;
	double two_pi = 2.0 * M_PI;
	for (int i = 0; i < n; i++) {
		double u1;
		do {
			u1 = (double)rand_r(seed) / RAND_MAX;
		} while (u1 <= epsilon);
		double u2 = (double)rand_r(seed) / RAND_MAX;
		double mag = sigma * sqrt(-2.0 * log(u1));
		points[i][0] = mag * cos(two_pi * u2) + mean[0];
		points[i][1] = mag * sin(two_pi * u2) + mean[1];
	}
}

int main(int argc, char * argv[]) {
	unsigned int rand_state = 0xCAFE;
	int n_centers = 5;
	double centers[5][2] = {
		{0., 0.},
		{0., 10.},
		{10., 0.},
		{10., 10.},
		{5., 5.},
	};
	double sigma = 3.0;
	int n_per_center = 300000;
	int n = n_per_center * n_centers;
	
	double (* points)[2] = malloc(n*2*sizeof(double));
	for (int i_center = 0; i_center < n_centers; i_center++) {
		generate_points(n_per_center, centers[i_center], sigma, points + (i_center*n_per_center), &rand_state);
	}

	int k = n_centers;
	double means[k][2];
	int clusters[n];
	for (int nb_threads = 1; nb_threads <= 8; nb_threads++) {
		printf("Number of threads: %d\n", nb_threads);
		struct timespec start, end;
		clock_gettime(CLOCK_MONOTONIC, &start);
		int nb_iter = kmeans(2, n, k, points, means, clusters, 100, &rand_state, nb_threads);
		clock_gettime(CLOCK_MONOTONIC, &end);
		printf("NB iter: %d\n", nb_iter);
		printf("Time taken for kmeans: %f seconds\n", get_delta(start, end));

		for (int i = 0; i < k; i++) {
			printf("Cluster %d : (", i);
			for (int x=0;x < 2; x++) {
				printf("%f,",means[i][x]);
			}
			printf(")\n");
		}
		printf("\n");
	}
	return 0;
}
