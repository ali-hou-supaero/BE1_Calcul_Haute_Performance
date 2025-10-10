#ifndef KMEANS_H
#define KMEANS_H
#include <stdbool.h>
#include <time.h>

/**
 * Compute the time difference (in seconds) between two timespec structures.
 */
double get_delta(struct timespec begin, struct timespec end);

/**
 * Compute the k-means clustering of the given points.
 * @param[in] d The dimension of the vector space
 * @param[in] n The number of points
 * @param[in] k The number of clusters to compute
 * @param[in] points Array of points coordinates
 * @param[out] means Array of found clusters means
 * @param[out] clusters Array of cluster affectation for each points (values range from 0 to k-1)
 * @param[in] max_iter Maximum number of iterations
 * @param[inout] rand_state RNG state for kmeans initialization
 * @return Number of actual iterations
 */
int kmeans(int d, int n, int k, double points[n][d], double means[k][d], int clusters[n], int max_iter, unsigned int * rand_state, int nb_threads);

/**
 * Initialize the cluster means.
 * @param[in] d The dimension of the vector space
 * @param[in] n The number of points
 * @param[in] k The number of clusters to compute
 * @param[in] points Array of points coordinates
 * @param[out] means Array of initial clusters means
 * @param[inout] rand_state RNG state for kmeans initialization
 */
void kmeans_init(int d, int n, int k, double points[n][d], double means[k][d], unsigned int * rand_state);

/**
 * Assign each point to the cluster with the nearest mean.
 * @param[in] d The dimension of the vector space
 * @param[in] n The number of points
 * @param[in] k The number of clusters to compute
 * @param[in] points Array of points coordinates
 * @param[in] means Array of clusters means
 * @param[out] clusters Array of cluster affectation for each points (values range from 0 to k-1)
 * @return True if any assignement has changed.
 */
bool kmeans_assign_clusters(int d, int n, int k, double points[n][d], double means[k][d], int clusters[n], int nb_threads);

/**
 * Compute the mean of each cluster.
 * @param[in] d The dimension of the vector space
 * @param[in] n The number of points
 * @param[in] k The number of clusters to compute
 * @param[in] points Array of points coordinates
 * @param[out] means Array of updated clusters means
 * @param[in] clusters Array of cluster affectation for each points (values range from 0 to k-1)
 */
void kmeans_compute_means(int d, int n, int k, double points[n][d], double means[k][d], int clusters[n], int nb_threads);


#endif
