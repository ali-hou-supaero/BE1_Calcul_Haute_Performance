#include "kmeans.h"
#include "spng.h"

#define MAX_ITER 1000
void make_palette(int n, unsigned char colors[n][3], int k, struct spng_plte * palette, unsigned char assign[n]) {
	palette->n_entries = k;

	double (* dcolors)[3] = malloc(n*3*sizeof(double));
	for (int i = 0; i < n; i++) {
		for (int p = 0; p < 3; p++) {
			dcolors[i][p] = colors[i][p]/256.;
		}
	}
	double means[k][3];
	int * clusters = malloc(n*sizeof(int));
	unsigned int rand_state = 0;
	int nb_threads = 1;
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	int niter = kmeans(3, n, k, dcolors, means, clusters, MAX_ITER, &rand_state, nb_threads);
	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Total time for kmeans: %f seconds\n", get_delta(start, end));
	printf("Niter = %d\n",niter);
	for (int i = 0; i < n; i++) {
		assign[i] = clusters[i];
	}
	for (int i = 0; i < k; i++) {
		palette->entries[i].red = (uint8_t)(means[i][0]*256);
		palette->entries[i].green = (uint8_t)(means[i][1]*256);
		palette->entries[i].blue = (uint8_t)(means[i][2]*256);
		palette->entries[i].alpha = 255;
	}
	free(clusters);
}

int main(int argc, char * argv[]) {
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	if (argc < 4) {
		printf("Usage : %s <input.png> <output.png> <n_colors>\n", argv[0]);
		return -1;
	}
	FILE * input;
	input = fopen(argv[1], "rb");
	if (input == NULL) {
		printf("Error reading file %s.\n",argv[1]);
		return -2;
	}
	spng_ctx *ctx = spng_ctx_new(0);
	spng_set_png_file(ctx, input);

	int fmt = SPNG_FMT_RGB8;
	size_t width, height;

	struct spng_ihdr ihdr;
    spng_get_ihdr(ctx, &ihdr);
	width = ihdr.width;
	height = ihdr.height;
	size_t image_size;
	spng_decoded_image_size(ctx,fmt, &image_size);
	unsigned char (* image)[3] = malloc(image_size);
	spng_decode_image(ctx, image, image_size, fmt, 0);

	spng_ctx_free(ctx);
	fclose(input);

	struct spng_plte palette;
	unsigned char * assign = malloc(width*height);
	int k = atoi(argv[3]);
	struct timespec start_make_palette, end_make_palette;
	clock_gettime(CLOCK_MONOTONIC, &start_make_palette);
	make_palette(width*height, image, k, &palette, assign);
	clock_gettime(CLOCK_MONOTONIC, &end_make_palette);
	printf("Total time for making palette: %f seconds\n", get_delta(start_make_palette, end_make_palette));

	free(image);

	ctx = spng_ctx_new(SPNG_CTX_ENCODER);
	FILE * output;
	output = fopen(argv[2], "wb");
	ihdr.color_type = 3;
	ihdr.bit_depth = 8;
	spng_set_png_file(ctx, output);
	spng_set_ihdr(ctx, &ihdr);
	spng_set_plte(ctx, &palette);

	int ret = spng_encode_image(ctx, assign, width*height, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
	if(ret) {
		printf("encode error (%d): %s\n", ret, spng_strerror(ret));
	}
	spng_ctx_free(ctx);
	fclose(output);
	free(assign);
	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Total time for compress algorithm: %f seconds\n", get_delta(start, end));
	return 0;
}
