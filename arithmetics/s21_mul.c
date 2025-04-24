#include "../s21_decimal.h"

int multiply(s21_big_decimal val1, s21_big_decimal val2, s21_big_decimal *res) {
  int flag = 0;
  for (int i = 0; i < 8; i++) res->bits[i] = 0;

  for (int i = 0; i < 224; i++) {
    if (big_get_bit(val2, i)) {
      s21_big_decimal temp = {0};
      for (int j = 0; j < 224; j++) {
        if (big_get_bit(val1, j)) {
          big_set_bit(&temp, j + i, 1);
        }
      }

      unsigned long long carry = 0;
      for (int k = 0; k < 8; k++) {
        unsigned long long sum =
            (unsigned long long)res->bits[k] + temp.bits[k] + carry;
        res->bits[k] = sum & 0xFFFFFFFF;
        carry = sum >> 32;
      }
    }
  }
  return flag;
}

int divide_big_decimal_by_10(s21_big_decimal *value) {
  unsigned long long remainder = 0;
  for (int i = 7; i >= 0; i--) {
    unsigned long long current = (remainder << 32) | value->bits[i];
    value->bits[i] = (unsigned int)(current / 10);
    remainder = current % 10;
  }
  return (remainder == 0) ? 0 : 1;
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int flag = 0;

  if (!result) {
    flag = 1;
  }

  if (!flag) {
    int sign1 = get_sign(value_1);
    int sign2 = get_sign(value_2);
    int scale1 = get_scale(value_1);
    int scale2 = get_scale(value_2);
    int result_sign = sign1 ^ sign2;
    int result_scale = scale1 + scale2;

    s21_big_decimal big_val1 = convert_to_big_decimal(value_1);
    s21_big_decimal big_val2 = convert_to_big_decimal(value_2);
    s21_big_decimal big_result = {0};

    if (multiply(big_val1, big_val2, &big_result)) {
      flag = 1;
    }

    if (!flag) {
      if (scale1 == 28 && scale2 == 28) {
        result_scale = 28;
      } else {
        while (result_scale > 0 && (big_result.bits[0] % 10 == 0)) {
          divide_big_decimal_by_10(&big_result);
          result_scale--;
        }
      }

      if (overflow_check_big_decimal(&big_result)) {
        flag = 1;
      }

      if (!flag && convert_from_big_decimal(big_result, result)) {
        flag = 1;
      }

      if (!flag) {
        set_sign(result, result_sign);
        set_scale(result, result_scale > 28 ? 28 : result_scale);
      }
    }
  }

  return flag;
}