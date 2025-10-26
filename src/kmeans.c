#include "kmeans.h"
#include <assert.h>
#include <stdlib.h>
#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

/** Retourne la différence (en secondes) entre deux timespec */
double get_delta(struct timespec begin, struct timespec end) {
	return end.tv_sec - begin.tv_sec + (end.tv_nsec - begin.tv_nsec) * 1e-9;
}

double kmeans_dist(int d, double p1[d], double p2[d]) {
	double dist = 0;
	for (int i = 0; i < d; i++) {
		double delta = p1[i] - p2[i];
		dist += delta * delta;
	}
	return dist;
}

int kmeans(int d, int n, int k, double points[n][d], double means[k][d], int clusters[n], int max_iter, unsigned int * rand_state, int nb_threads) {
	assert(k <= n);

	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	kmeans_init(d, n, k, points, means, rand_state);
	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Total time for initialising clusters: %f seconds\n", get_delta(start, end));

	int i;
	double time_sum_assign_clusters = 0;
	double time_sum_compute_means = 0;

	for (i = 0; i < max_iter; i++) {
		struct timespec start, end;
		clock_gettime(CLOCK_MONOTONIC, &start);
		bool changed = kmeans_assign_clusters(d, n, k, points, means, clusters, nb_threads);
		clock_gettime(CLOCK_MONOTONIC, &end);

		time_sum_assign_clusters += get_delta(start, end);
		if (!changed) {
			break;
		}

		clock_gettime(CLOCK_MONOTONIC, &start);
		kmeans_compute_means(d, n, k, points, means, clusters, nb_threads);
		clock_gettime(CLOCK_MONOTONIC, &end);
		time_sum_compute_means += get_delta(start, end);
	}
	printf("Total time for assigning clusters: %f seconds\n", time_sum_assign_clusters);
	printf("Total time for computing means: %f seconds\n", time_sum_compute_means);
	return i;
}

void kmeans_init(int d, int n, int k, double points[n][d], double means[k][d], unsigned int * rand_state) {
	int * indices = (int *) malloc(n*sizeof(int));
	for (int i = 0; i < n; i++) {
		indices[i] = i;
	}
	for (int i = n-1; i > 0; i--) {
		int j = rand_r(rand_state) % (i+1);
		int temp = indices[i];
		indices[i] = indices[j];
		indices[j] = temp;
	}
	for (int i = 0; i < k; i++) {
		for (int x = 0; x < d; x++) {
			means[i][x] = points[indices[i]][x]; 
		}
	}
	free(indices);
}

bool kmeans_assign_clusters(int d, int n, int k, double points[n][d], double means[k][d], int clusters[n], int nb_threads) {
	bool changed = false;
	omp_set_num_threads(nb_threads);
	#pragma omp parallel 
	{
		// #pragma omp single
		// {
		// 	printf("Number of threads: %d\n", omp_get_num_threads());
		// }
		#pragma omp for reduction(||:changed)
		for (int i = 0; i < n; i++) {
		int min_index = 0;
		double min_dist = kmeans_dist(d, points[i], means[0]);
			for (int j = 1; j < k; j++) {
				double dist = kmeans_dist(d, points[i], means[j]);
				if (dist < min_dist) {
					min_dist = dist;
					min_index = j;
				}
			}
			if (clusters[i] != min_index) {
				clusters[i] = min_index;
				changed = true;
			}
		}
	}	
	return changed;
}

void kmeans_compute_means(int d, int n, int k, double points[n][d], double means[k][d], int clusters[n], int nb_threads) {
	// int nb_points[k];

	// // Si k est assez grand, alors on peut paralléliser cette boucle
	// #pragma omp parallel for
	// for (int j = 0; j < k; j++) {
	// 	for (int x = 0; x < d; x++) {
	// 		means[j][x] = 0;
	// 	}
	// 	nb_points[j] = 0;
	// }

	// On ne peut pas faire une réduction sur le tableau nb_points
	// Il faut faire une réduction déguisée en initialisant des tableau pour chaque threads
	// Après etre sorti de la région parallèle, on fait la somme des tableaux
	// omp_set_num_threads(nb_threads);
	// #pragma omp parallel for reduction(+:nb_points)
	// for (int i = 0; i < n; i++) {
	// 	int j = clusters[i];
	// 	// int id = omp_get_thread_num();
	// 	for (int x = 0; x < d; x++) {
	// 		means[j][x] += points[i][x];
	// 	}
	// 	nb_points[j]++;
	// }

	// for (int j = 0; j < k; j++) {
	// 	for (int x = 0; x < d; x++) {
	// 		means[j][x] /= nb_points[j];
	// 	}
	// }

	int nb_points[k];
	// Boucle parallélisable si k petit
	for (int j = 0; j < k; j++) {
		for (int x = 0; x < d; x++) {
			means[j][x] = 0.0;
		}
		nb_points[j] = 0;
	}

	// Création des tableaux locaux (par thread) avec un padding pour éviter le false sharing
	int padding = 8;
	double local_means[nb_threads][k + padding][d];
	int local_counts[nb_threads][k + padding];

	// Initialisation
	for (int t = 0; t < nb_threads; t++) {
		for (int j = 0; j < k; j++) {
			local_counts[t][j] = 0;
			for (int x = 0; x < d; x++) {
				local_means[t][j][x] = 0.0;
			}
		}
	}

	// Boucle originale parallélisée
	omp_set_num_threads(nb_threads);
	#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();
		#pragma omp for schedule(static)
		for (int i = 0; i < n; i++) {
			int j = clusters[i];
			for (int x = 0; x < d; x++) {
				local_means[thread_id][j][x] += points[i][x];
			}
			local_counts[thread_id][j]++;
		}
	}

	// Fusion des résultats
	for (int j = 0; j < k; j++) {
		nb_points[j] = 0;
		for (int x = 0; x < d; x++) {
			means[j][x] = 0.0;
		}
		for (int t = 0; t < nb_threads; t++) {
			nb_points[j] += local_counts[t][j];
			for (int x = 0; x < d; x++) {
				means[j][x] += local_means[t][j][x];
			}
		}
		if (nb_points[j] > 0) {
			for (int x = 0; x < d; x++) {
				means[j][x] /= nb_points[j];
			}
		}
	}
}