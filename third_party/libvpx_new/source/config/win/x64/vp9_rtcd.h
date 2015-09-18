#ifndef VP9_RTCD_H_
#define VP9_RTCD_H_

#ifdef RTCD_C
#define RTCD_EXTERN
#else
#define RTCD_EXTERN extern
#endif

/*
 * VP9
 */

#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_enums.h"

struct macroblockd;

/* Encoder forward decls */
struct macroblock;
struct vp9_variance_vtable;
struct search_site_config;
struct mv;
union int_mv;
struct yv12_buffer_config;

#ifdef __cplusplus
extern "C" {
#endif

unsigned int vp9_avg_4x4_c(const uint8_t *, int p);
unsigned int vp9_avg_4x4_sse2(const uint8_t *, int p);
#define vp9_avg_4x4 vp9_avg_4x4_sse2

unsigned int vp9_avg_8x8_c(const uint8_t *, int p);
unsigned int vp9_avg_8x8_sse2(const uint8_t *, int p);
#define vp9_avg_8x8 vp9_avg_8x8_sse2

int64_t vp9_block_error_c(const tran_low_t *coeff, const tran_low_t *dqcoeff, intptr_t block_size, int64_t *ssz);
int64_t vp9_block_error_sse2(const tran_low_t *coeff, const tran_low_t *dqcoeff, intptr_t block_size, int64_t *ssz);
int64_t vp9_block_error_avx2(const tran_low_t *coeff, const tran_low_t *dqcoeff, intptr_t block_size, int64_t *ssz);
RTCD_EXTERN int64_t (*vp9_block_error)(const tran_low_t *coeff, const tran_low_t *dqcoeff, intptr_t block_size, int64_t *ssz);

int64_t vp9_block_error_fp_c(const int16_t *coeff, const int16_t *dqcoeff, int block_size);
int64_t vp9_block_error_fp_sse2(const int16_t *coeff, const int16_t *dqcoeff, int block_size);
#define vp9_block_error_fp vp9_block_error_fp_sse2

int vp9_denoiser_filter_c(const uint8_t *sig, int sig_stride, const uint8_t *mc_avg, int mc_avg_stride, uint8_t *avg, int avg_stride, int increase_denoising, BLOCK_SIZE bs, int motion_magnitude);
int vp9_denoiser_filter_sse2(const uint8_t *sig, int sig_stride, const uint8_t *mc_avg, int mc_avg_stride, uint8_t *avg, int avg_stride, int increase_denoising, BLOCK_SIZE bs, int motion_magnitude);
#define vp9_denoiser_filter vp9_denoiser_filter_sse2

int vp9_diamond_search_sad_c(const struct macroblock *x, const struct search_site_config *cfg,  struct mv *ref_mv, struct mv *best_mv, int search_param, int sad_per_bit, int *num00, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv);
#define vp9_diamond_search_sad vp9_diamond_search_sad_c

void vp9_fdct8x8_quant_c(const int16_t *input, int stride, tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);
void vp9_fdct8x8_quant_sse2(const int16_t *input, int stride, tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);
void vp9_fdct8x8_quant_ssse3(const int16_t *input, int stride, tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);
RTCD_EXTERN void (*vp9_fdct8x8_quant)(const int16_t *input, int stride, tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);

void vp9_fht16x16_c(const int16_t *input, tran_low_t *output, int stride, int tx_type);
void vp9_fht16x16_sse2(const int16_t *input, tran_low_t *output, int stride, int tx_type);
#define vp9_fht16x16 vp9_fht16x16_sse2

void vp9_fht4x4_c(const int16_t *input, tran_low_t *output, int stride, int tx_type);
void vp9_fht4x4_sse2(const int16_t *input, tran_low_t *output, int stride, int tx_type);
#define vp9_fht4x4 vp9_fht4x4_sse2

void vp9_fht8x8_c(const int16_t *input, tran_low_t *output, int stride, int tx_type);
void vp9_fht8x8_sse2(const int16_t *input, tran_low_t *output, int stride, int tx_type);
#define vp9_fht8x8 vp9_fht8x8_sse2

void vp9_filter_by_weight16x16_c(const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int src_weight);
void vp9_filter_by_weight16x16_sse2(const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int src_weight);
#define vp9_filter_by_weight16x16 vp9_filter_by_weight16x16_sse2

void vp9_filter_by_weight8x8_c(const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int src_weight);
void vp9_filter_by_weight8x8_sse2(const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int src_weight);
#define vp9_filter_by_weight8x8 vp9_filter_by_weight8x8_sse2

int vp9_full_range_search_c(const struct macroblock *x, const struct search_site_config *cfg, struct mv *ref_mv, struct mv *best_mv, int search_param, int sad_per_bit, int *num00, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv);
#define vp9_full_range_search vp9_full_range_search_c

int vp9_full_search_sad_c(const struct macroblock *x, const struct mv *ref_mv, int sad_per_bit, int distance, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv, struct mv *best_mv);
int vp9_full_search_sadx3(const struct macroblock *x, const struct mv *ref_mv, int sad_per_bit, int distance, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv, struct mv *best_mv);
int vp9_full_search_sadx8(const struct macroblock *x, const struct mv *ref_mv, int sad_per_bit, int distance, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv, struct mv *best_mv);
RTCD_EXTERN int (*vp9_full_search_sad)(const struct macroblock *x, const struct mv *ref_mv, int sad_per_bit, int distance, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv, struct mv *best_mv);

void vp9_fwht4x4_c(const int16_t *input, tran_low_t *output, int stride);
void vp9_fwht4x4_mmx(const int16_t *input, tran_low_t *output, int stride);
#define vp9_fwht4x4 vp9_fwht4x4_mmx

void vp9_hadamard_16x16_c(int16_t const *src_diff, int src_stride, int16_t *coeff);
void vp9_hadamard_16x16_sse2(int16_t const *src_diff, int src_stride, int16_t *coeff);
#define vp9_hadamard_16x16 vp9_hadamard_16x16_sse2

void vp9_hadamard_8x8_c(int16_t const *src_diff, int src_stride, int16_t *coeff);
void vp9_hadamard_8x8_sse2(int16_t const *src_diff, int src_stride, int16_t *coeff);
void vp9_hadamard_8x8_ssse3(int16_t const *src_diff, int src_stride, int16_t *coeff);
RTCD_EXTERN void (*vp9_hadamard_8x8)(int16_t const *src_diff, int src_stride, int16_t *coeff);

void vp9_iht16x16_256_add_c(const tran_low_t *input, uint8_t *output, int pitch, int tx_type);
void vp9_iht16x16_256_add_sse2(const tran_low_t *input, uint8_t *output, int pitch, int tx_type);
#define vp9_iht16x16_256_add vp9_iht16x16_256_add_sse2

void vp9_iht4x4_16_add_c(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);
void vp9_iht4x4_16_add_sse2(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);
#define vp9_iht4x4_16_add vp9_iht4x4_16_add_sse2

void vp9_iht8x8_64_add_c(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);
void vp9_iht8x8_64_add_sse2(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);
#define vp9_iht8x8_64_add vp9_iht8x8_64_add_sse2

int16_t vp9_int_pro_col_c(uint8_t const *ref, const int width);
int16_t vp9_int_pro_col_sse2(uint8_t const *ref, const int width);
#define vp9_int_pro_col vp9_int_pro_col_sse2

void vp9_int_pro_row_c(int16_t *hbuf, uint8_t const *ref, const int ref_stride, const int height);
void vp9_int_pro_row_sse2(int16_t *hbuf, uint8_t const *ref, const int ref_stride, const int height);
#define vp9_int_pro_row vp9_int_pro_row_sse2

void vp9_mbpost_proc_across_ip_c(uint8_t *src, int pitch, int rows, int cols, int flimit);
void vp9_mbpost_proc_across_ip_xmm(uint8_t *src, int pitch, int rows, int cols, int flimit);
#define vp9_mbpost_proc_across_ip vp9_mbpost_proc_across_ip_xmm

void vp9_mbpost_proc_down_c(uint8_t *dst, int pitch, int rows, int cols, int flimit);
void vp9_mbpost_proc_down_xmm(uint8_t *dst, int pitch, int rows, int cols, int flimit);
#define vp9_mbpost_proc_down vp9_mbpost_proc_down_xmm

void vp9_minmax_8x8_c(const uint8_t *s, int p, const uint8_t *d, int dp, int *min, int *max);
void vp9_minmax_8x8_sse2(const uint8_t *s, int p, const uint8_t *d, int dp, int *min, int *max);
#define vp9_minmax_8x8 vp9_minmax_8x8_sse2

void vp9_plane_add_noise_c(uint8_t *Start, char *noise, char blackclamp[16], char whiteclamp[16], char bothclamp[16], unsigned int Width, unsigned int Height, int Pitch);
void vp9_plane_add_noise_wmt(uint8_t *Start, char *noise, char blackclamp[16], char whiteclamp[16], char bothclamp[16], unsigned int Width, unsigned int Height, int Pitch);
#define vp9_plane_add_noise vp9_plane_add_noise_wmt

void vp9_post_proc_down_and_across_c(const uint8_t *src_ptr, uint8_t *dst_ptr, int src_pixels_per_line, int dst_pixels_per_line, int rows, int cols, int flimit);
void vp9_post_proc_down_and_across_xmm(const uint8_t *src_ptr, uint8_t *dst_ptr, int src_pixels_per_line, int dst_pixels_per_line, int rows, int cols, int flimit);
#define vp9_post_proc_down_and_across vp9_post_proc_down_and_across_xmm

void vp9_quantize_fp_c(const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);
void vp9_quantize_fp_sse2(const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);
void vp9_quantize_fp_ssse3(const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);
RTCD_EXTERN void (*vp9_quantize_fp)(const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);

void vp9_quantize_fp_32x32_c(const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);
void vp9_quantize_fp_32x32_ssse3(const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);
RTCD_EXTERN void (*vp9_quantize_fp_32x32)(const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan);

int16_t vp9_satd_c(const int16_t *coeff, int length);
int16_t vp9_satd_sse2(const int16_t *coeff, int length);
#define vp9_satd vp9_satd_sse2

void vp9_temporal_filter_apply_c(uint8_t *frame1, unsigned int stride, uint8_t *frame2, unsigned int block_width, unsigned int block_height, int strength, int filter_weight, unsigned int *accumulator, uint16_t *count);
void vp9_temporal_filter_apply_sse2(uint8_t *frame1, unsigned int stride, uint8_t *frame2, unsigned int block_width, unsigned int block_height, int strength, int filter_weight, unsigned int *accumulator, uint16_t *count);
#define vp9_temporal_filter_apply vp9_temporal_filter_apply_sse2

int vp9_vector_var_c(int16_t const *ref, int16_t const *src, const int bwl);
int vp9_vector_var_sse2(int16_t const *ref, int16_t const *src, const int bwl);
#define vp9_vector_var vp9_vector_var_sse2

void vp9_rtcd(void);

#ifdef RTCD_C
#include "vpx_ports/x86.h"
static void setup_rtcd_internal(void)
{
    int flags = x86_simd_caps();

    (void)flags;

    vp9_block_error = vp9_block_error_sse2;
    if (flags & HAS_AVX2) vp9_block_error = vp9_block_error_avx2;
    vp9_fdct8x8_quant = vp9_fdct8x8_quant_sse2;
    if (flags & HAS_SSSE3) vp9_fdct8x8_quant = vp9_fdct8x8_quant_ssse3;
    vp9_full_search_sad = vp9_full_search_sad_c;
    if (flags & HAS_SSE3) vp9_full_search_sad = vp9_full_search_sadx3;
    if (flags & HAS_SSE4_1) vp9_full_search_sad = vp9_full_search_sadx8;
    vp9_hadamard_8x8 = vp9_hadamard_8x8_sse2;
    if (flags & HAS_SSSE3) vp9_hadamard_8x8 = vp9_hadamard_8x8_ssse3;
    vp9_quantize_fp = vp9_quantize_fp_sse2;
    if (flags & HAS_SSSE3) vp9_quantize_fp = vp9_quantize_fp_ssse3;
    vp9_quantize_fp_32x32 = vp9_quantize_fp_32x32_c;
    if (flags & HAS_SSSE3) vp9_quantize_fp_32x32 = vp9_quantize_fp_32x32_ssse3;
}
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
