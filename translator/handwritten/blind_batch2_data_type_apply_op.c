#define SIGNED_CASE(D,T,MIN,MAX) case D: {                              \
    T a = *(const T *)arg1, b = *(const T *)arg2, r;                    \
    if (op == '+')                                                     \
        r = b > 0 && a > (T)((MAX) - b) ? (T)(MAX) :                  \
            b < 0 && a < (T)((MIN) - b) ? (T)(MIN) : (T)(a + b);      \
    else                                                              \
        r = b < 0 && a > (T)((MAX) + b) ? (T)(MAX) :                  \
            b > 0 && a < (T)((MIN) + b) ? (T)(MIN) : (T)(a - b);      \
    *(T *)output = r;                                                 \
    break;                                                            \
}
#define UNSIGNED_CASE(D,T,MAX) case D: {                               \
    T a = *(const T *)arg1, b = *(const T *)arg2, r;                   \
    if (op == '+')                                                     \
        r = a > (T)((MAX) - b) ? (T)(MAX) : (T)(a + b);               \
    else                                                              \
        r = a < b ? (T)0 : (T)(a - b);                                \
    *(T *)output = r;                                                 \
    break;                                                            \
}
#define REAL_CASE(D,T) case D: {                                      \
    T a = *(const T *)arg1, b = *(const T *)arg2;                      \
    *(T *)output = op == '+' ? a + b : a - b;                         \
    break;                                                            \
}

switch (data_type)
{
SIGNED_CASE(@DT_S8@, ImS8, @S8_MIN@, @S8_MAX@)
UNSIGNED_CASE(@DT_U8@, ImU8, @U8_MAX@)
SIGNED_CASE(@DT_S16@, ImS16, @S16_MIN@, @S16_MAX@)
UNSIGNED_CASE(@DT_U16@, ImU16, @U16_MAX@)
SIGNED_CASE(@DT_S32@, ImS32, @S32_MIN@, @S32_MAX@)
UNSIGNED_CASE(@DT_U32@, ImU32, @U32_MAX@)
SIGNED_CASE(@DT_S64@, ImS64, @S64_MIN@, @S64_MAX@)
UNSIGNED_CASE(@DT_U64@, ImU64, @U64_MAX@)
REAL_CASE(@DT_FLOAT@, float)
REAL_CASE(@DT_DOUBLE@, double)
}

#undef REAL_CASE
#undef UNSIGNED_CASE
#undef SIGNED_CASE
