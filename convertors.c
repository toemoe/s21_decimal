#include "s21_decimal.h"

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  if (dst == NULL) return 1;

  null_decimal(dst);

  if (src < 0) {
    set_sign(dst, 1);
    src = -src;
  }
  dst->bits[0] = (unsigned int)src;
  return 0;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  if (!dst) {
    return 1;
  }

  null_decimal(dst);

  int error_flag = 0;

  // Обработка ноль и минус ноль
  if (src == 0.0f) {
    if (signbit(src)) {
      set_sign(dst, 1);
    }
  } else {
    // Проверка на допустимые границы
    if (isnan(src) || isinf(src) || fabsf(src) < 1e-28f ||
        fabsf(src) > 7.9228162514264337593543950335e+28f) {
      error_flag = 1;
    } else {
      bool negative = signbit(src);
      if (negative) src = -src;

      // Преобразуем float в строку с 7 значимыми цифрами
      char buffer[32];
      snprintf(buffer, sizeof(buffer), "%.7g", src);

      int scale = 0;
      char digits_only[32] = {0};
      int di = 0;

      char *dot = strchr(buffer, '.');
      for (int i = 0; buffer[i]; ++i) {
        if (buffer[i] >= '0' && buffer[i] <= '9') {
          digits_only[di++] = buffer[i];
          if (dot && i > (dot - buffer)) scale++;
        }
      }

      digits_only[di] = '\0';

      // Конвертируем строку цифр в целое число
      uint64_t value = strtoull(digits_only, NULL, 10);

      dst->bits[0] = (uint32_t)(value & 0xFFFFFFFF);
      dst->bits[1] = (uint32_t)((value >> 32) & 0xFFFFFFFF);
      dst->bits[2] = 0;

      set_scale(dst, scale);
      if (negative) {
        set_sign(dst, 1);
      }
    }
  }

  return error_flag;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  if (dst == NULL) return 1;

  int sign = get_sign(src);

  int result = src.bits[0];
  result = (sign == 1) ? -result : result;

  *dst = result;
  return 0;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  if (!dst) return 1;

  __uint128_t value = (__uint128_t)src.bits[0] |
                      ((__uint128_t)src.bits[1] << 32) |
                      ((__uint128_t)src.bits[2] << 64);

  int scale = get_scale(src);
  int sign = get_sign(src);

  *dst = (float)value / powf(10, scale);
  if (sign) *dst = -*dst;

  return 0;
}