/*****************************************************************************
 *   (c) 2017-2020 Cedric Mesnil <cslashm@gmail.com>, Ledger SAS.
 *   (c) 2020 Ledger SAS.
 *   (c) 2020 The TurtleCoin Developers
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include "hw_crypto.h"

#define BUFFER G_io_apdu_buffer
#define BUFFER_SIZE KEY_SIZE + SIG_SET_SIZE

static const uint32_t HARDENED_OFFSET = 0x80000000;

static const uint32_t derivePath[BIP32_PATH] =
    {44 | HARDENED_OFFSET, 2147485632 | HARDENED_OFFSET, 0 | HARDENED_OFFSET, 0 | HARDENED_OFFSET, 0 | HARDENED_OFFSET};

static const unsigned char WIDE C_ED25519_G[65] = {
    0x04, 0x21, 0x69, 0x36, 0xd3, 0xcd, 0x6e, 0x53, 0xfe, 0xc0, 0xa4, 0xe2, 0x31, 0xfd, 0xd6, 0xdc, 0x5c,
    0x69, 0x2c, 0xc7, 0x60, 0x95, 0x25, 0xa7, 0xb2, 0xc9, 0x56, 0x2d, 0x60, 0x8f, 0x25, 0xd5, 0x1a, 0x66,
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x58};

// q
static const unsigned char C_ED25519_ORDER[32] = {0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xDE, 0xF9, 0xDE, 0xA2, 0xF7,
                                                  0x9C, 0xD6, 0x58, 0x12, 0x63, 0x1A, 0x5C, 0xF5, 0xD3, 0xED};

static const unsigned char C_ED25519_FIELD[32] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xed};

static const unsigned char C_fe_fffb1[32] = {0x7e, 0x71, 0xfb, 0xef, 0xda, 0xd6, 0x1b, 0x17, 0x20, 0xa9, 0xc5,
                                             0x37, 0x41, 0xfb, 0x19, 0xe3, 0xd1, 0x94, 0x04, 0xa8, 0xb9, 0x2a,
                                             0x73, 0x8d, 0x22, 0xa7, 0x69, 0x75, 0x32, 0x1c, 0x41, 0xee};

static const unsigned char C_fe_fffb2[32] = {0x4d, 0x06, 0x1e, 0x0a, 0x04, 0x5a, 0x2c, 0xf6, 0x91, 0xd4, 0x51,
                                             0xb7, 0xc0, 0x16, 0x5f, 0xbe, 0x51, 0xde, 0x03, 0x46, 0x04, 0x56,
                                             0xf7, 0xdf, 0xd2, 0xde, 0x64, 0x83, 0x60, 0x7c, 0x9a, 0xe0};

static const unsigned char C_fe_fffb3[32] = {0x67, 0x4a, 0x11, 0x0d, 0x14, 0xc2, 0x08, 0xef, 0xb8, 0x95, 0x46,
                                             0x40, 0x3f, 0x0d, 0xa2, 0xed, 0x40, 0x24, 0xff, 0x4e, 0xa5, 0x96,
                                             0x42, 0x29, 0x58, 0x1b, 0x7d, 0x87, 0x17, 0x30, 0x2c, 0x66};

static const unsigned char C_fe_fffb4[32] = {0x1a, 0x43, 0xf3, 0x03, 0x10, 0x67, 0xdb, 0xf9, 0x26, 0xc0, 0xf4,
                                             0x88, 0x7e, 0xf7, 0x43, 0x2e, 0xee, 0x46, 0xfc, 0x08, 0xa1, 0x3f,
                                             0x4a, 0x49, 0x85, 0x3d, 0x19, 0x03, 0xb6, 0xb3, 0x91, 0x86};

static const unsigned char C_fe_ma[32] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x92, 0xe7};

static const unsigned char C_fe_ma2[32] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                           0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                           0xff, 0xff, 0xff, 0xff, 0xff, 0xc8, 0xdb, 0x3d, 0xe3, 0xc9};

static const unsigned char C_fe_qm5div8[32] = {0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd};

static const unsigned char C_fe_sqrtm1[32] = {0x2b, 0x83, 0x24, 0x80, 0x4f, 0xc1, 0xdf, 0x0b, 0x2b, 0x4d, 0x00,
                                              0x99, 0x3d, 0xfb, 0xd7, 0xa7, 0x2f, 0x43, 0x18, 0x06, 0xad, 0x2f,
                                              0xe4, 0x78, 0xc4, 0xee, 0x1b, 0x27, 0x4a, 0x0e, 0xa0, 0xb0};

/**
 * Loads a scalar to BE format so that the cx math methods can handle
 * any math functions correctly
 * @param result the result
 * @param source the value to reverse
 */
static void reverse32(unsigned char *result, const unsigned char *source)
{
    unsigned char x;

    unsigned int i;

    for (i = 0; i < 16; i++)
    {
        x = source[i];

        result[i] = source[31 - i];

        result[31 - i] = x;
    }
}

/**
 * Loads the public key into a raw point for further cx manipulation
 * @param point the raw point
 * @param public the public key to load
 * @return whether the operation succeeded
 */
static uint16_t hw_ge_frombytes_vartime(unsigned char *point, const unsigned char *public)
{
    BEGIN_TRY
    {
        TRY
        {
            unsigned char aB[SIG_STR_SIZE] = {0};

            // load the public key
            {
                aB[0] = 0x02;

                os_memmove(&aB[1], public, KEY_SIZE);

                cx_edward_decompress_point(CX_CURVE_Ed25519, aB, SIG_STR_SIZE);
            }

            os_memmove(point, aB, sizeof(aB));

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return ERR_GE_FROMBYTES_VARTIME;
        }
        FINALLY {}
    }
    END_TRY;
}

/**
 * Unloads the raw point into a public key
 * @param public the public key to unload the point to
 * @param point the point to unload
 */
static void hw_ge_tobytes(unsigned char *public, const unsigned char *point)
{
    unsigned char aB[SIG_STR_SIZE] = {0};

    os_memmove(aB, point, SIG_STR_SIZE);

    cx_edward_compress_point(CX_CURVE_Ed25519, aB, SIG_STR_SIZE);

    os_memmove(public, &aB[1], KEY_SIZE);
}

/**
 * Creates a private key point within the correct curve order
 * r = s mod q
 * @param result the resulting scalar
 * @param source the value to reduce
 */
static void hw_sc_reduce32(unsigned char *r, const unsigned char *s)
{
    unsigned char _s[KEY_SIZE];

    // Load the data for reduction
    reverse32(_s, s);

    // Put it on the curve in the proper order
    cx_math_modm(_s, KEY_SIZE, (unsigned char *)C_ED25519_ORDER, KEY_SIZE);

    // Unload the resulting scalar
    reverse32(r, _s);
}

/**
 * Turns a derivation into a scalar such that
 * r = H(d || n) mod q
 * @param r the resulting scalar
 * @param d the key derivation
 * @param n the output index
 */
static uint16_t hw_derivation_to_scalar(unsigned char *r, const unsigned char *d, const size_t n)
{
    unsigned char buffer[KEY_SIZE + 8] = {0};

    unsigned int pos = 0;

    // copy the derivation to the start of the buffer
    os_memmove(buffer, d, KEY_SIZE);

    // bump our current position ahead
    pos += KEY_SIZE;

    // encode n as a varint and add it to the end of the buffer
    pos += encode_varint(buffer + pos, n, 8);

    // hash the buffer
    const uint16_t status = hw_keccak(buffer, pos, buffer);

    if (status != OP_OK)
    {
        return status;
    }

    // reduce the buffer to a scalar
    hw_sc_reduce32(r, buffer);

    return OP_OK;
}

/**
 * Performs scalar addition such that
 * r = (a + b) mod q
 * @param r the result
 * @param a the first scalar
 * @param b the second scalar
 */
static void hw_sc_add(unsigned char *r, const unsigned char *a, const unsigned char *b)
{
    unsigned char _a[KEY_SIZE];

    unsigned char _b[KEY_SIZE];

    // Load scalar a
    reverse32(_a, a);

    // Load scalar b
    reverse32(_b, b);

    // Add the scalars together
    cx_math_addm(r, _a, _b, (unsigned char *)C_ED25519_ORDER, KEY_SIZE);

    // Unload the resulting scalar
    reverse32(r, r);
}

/**
 * Performs scalar subtraction such that
 * r = (a - b) mod q
 * @param r the result
 * @param a the first scalar
 * @param b the second scalar
 */
static void hw_sc_sub(unsigned char *r, const unsigned char *a, const unsigned char *b)
{
    unsigned char _a[KEY_SIZE];

    unsigned char _b[KEY_SIZE];

    // Load scalar a
    reverse32(_a, a);

    // Load scalar b
    reverse32(_b, b);

    // (a - b) mod q
    cx_math_subm(r, _a, _b, (unsigned char *)C_ED25519_ORDER, KEY_SIZE);

    // Unload the resulting scalar
    reverse32(r, r);
}

/**
 * Performs scalar multiplication such that
 * r = (a * b) mod q
 * @param r the result
 * @param a the first scalar
 * @param b the second scalar
 */
static void hw_sc_mul(unsigned char *r, const unsigned char *a, const unsigned char *b)
{
    unsigned char _a[KEY_SIZE];

    unsigned char _b[KEY_SIZE];

    reverse32(_a, a);

    reverse32(_b, b);

    cx_math_multm(r, _a, _b, (unsigned char *)C_ED25519_ORDER, KEY_SIZE);

    reverse32(r, r);
}

/**
 * Performs scalar subtraction and multiplication such that
 * r = (c - (a * b)) mod q
 * @param r the result
 * @param a the first scalar
 * @param b the second scalar
 * @param c the third scalar
 */
static void hw_sc_mulsub(unsigned char *r, const unsigned char *a, const unsigned char *B, const unsigned char *c)
{
    unsigned char aB[KEY_SIZE] = {0};

    // a_b = (a * B) mod q
    hw_sc_mul(aB, a, B);

    // r = (c - (a * B) mod q
    hw_sc_sub(r, c, aB);
}

/**
 * Adds two points together
 * r = p + q
 * @param r the result
 * @param p the first point
 * @param q the second point
 */
static uint16_t hw_ge_add(unsigned char *r, const unsigned char *p, const unsigned char *q)
{
    unsigned char pxy[SIG_STR_SIZE];

    unsigned char qxy[SIG_STR_SIZE];

    // load point #1
    uint16_t status = hw_ge_frombytes_vartime(pxy, p);

    if (status != OP_OK)
    {
        return status;
    }

    // load point #2
    status = hw_ge_frombytes_vartime(qxy, q);

    if (status != OP_OK)
    {
        return status;
    }

    // add them together
    cx_ecfp_add_point(CX_CURVE_Ed25519, pxy, pxy, qxy, SIG_STR_SIZE);

    // compress the point back to bytes
    hw_ge_tobytes(r, pxy);

    return OP_OK;
}

/**
 * Calculates a public key from a private key such that
 * A = a * G
 * @param A the public key
 * @param a the private key
 */
static void hw_ge_scalarmult_base(unsigned char *A, const unsigned char *a)
{
    unsigned char aG[SIG_STR_SIZE];

    unsigned char _a[KEY_SIZE];

    // Load the private key
    reverse32(_a, a);

    // Load G as a point
    os_memmove(aG, C_ED25519_G, SIG_STR_SIZE);

    // Multiply the private key by G
    cx_ecfp_scalar_mult(CX_CURVE_Ed25519, aG, SIG_STR_SIZE, _a, KEY_SIZE);

    // compress the point back to bytes
    hw_ge_tobytes(A, aG);
}

/**
 * Multiplies the the public and private keys together and returns
 * the resulting point such that
 * r = a * B
 * @param r the result
 * @param B the public key
 * @param a the private key
 */
static int hw_ge_scalarmult(unsigned char *r, const unsigned char *B, const unsigned char *a)
{
    unsigned char aB[SIG_STR_SIZE] = {0};

    unsigned char _a[KEY_SIZE] = {0};

    // Load the private key
    reverse32(_a, a);

    // Load the public key
    const uint16_t status = hw_ge_frombytes_vartime(aB, B);

    if (status != OP_OK)
    {
        return status;
    }

    // multiply them together
    cx_ecfp_scalar_mult(CX_CURVE_Ed25519, aB, SIG_STR_SIZE, _a, KEY_SIZE);

    // compress the point back to bytes
    hw_ge_tobytes(r, aB);

    return OP_OK;
}

/**
 * Calculates the result of the point on the ED25519 by 8 such that
 * r = 8 * A
 * @param r the result
 * @param A the point
 */
static int hw_ge_mul8(unsigned char *r, const unsigned char *A)
{
    unsigned char Pxy[SIG_STR_SIZE];

    // Load the point (public key)
    const uint16_t status = hw_ge_frombytes_vartime(Pxy, A);

    if (status != OP_OK)
    {
        return status;
    }

    // Add the point to itself x3
    cx_ecfp_add_point(CX_CURVE_Ed25519, Pxy, Pxy, Pxy, sizeof(Pxy));

    cx_ecfp_add_point(CX_CURVE_Ed25519, Pxy, Pxy, Pxy, sizeof(Pxy));

    cx_ecfp_add_point(CX_CURVE_Ed25519, Pxy, Pxy, Pxy, sizeof(Pxy));

    // compress the point back to bytes
    hw_ge_tobytes(r, Pxy);

    return OP_OK;
}

#define MOD (unsigned char *)C_ED25519_FIELD, KEY_SIZE
#define fe_isnegative(f) (f[31] & 1)

/**
 * Loads the input bytes into a point on the ED25519 curve
 * Thanks to knacc and moneromoo help on IRC #monero-research-lab via
 * https://github.com/LedgerHQ/app-monero/blob/master/src/monero_crypto.c
 * @param ge the resulting point on the ED25519 curve
 * @param bytes the bytes to load
 */
static void hw_ge_fromfe_frombytes_vartime(unsigned char *ge, const unsigned char *bytes)
{
    unsigned char u[KEY_SIZE] = {0};

    unsigned char v[KEY_SIZE] = {0};

    unsigned char w[KEY_SIZE] = {0};

    unsigned char x[KEY_SIZE] = {0};

    unsigned char y[KEY_SIZE] = {0};

    unsigned char z[KEY_SIZE] = {0};

    unsigned char rX[KEY_SIZE] = {0};

    unsigned char rY[KEY_SIZE] = {0};

    unsigned char rZ[KEY_SIZE] = {0};

    union
    {
        struct
        {
            unsigned char _uv7[KEY_SIZE];

            unsigned char _v3[KEY_SIZE];
        };

        unsigned char _Pxy[SIG_STR_SIZE];
    } uv;

    unsigned char sign;

    // ledger cx calls work in BE
    reverse32(u, bytes);

    cx_math_modm(u, KEY_SIZE, (unsigned char *)C_ED25519_FIELD, KEY_SIZE);

    cx_math_multm(v, u, u, MOD); // (2 * u^2)

    cx_math_addm(v, v, v, MOD);

    explicit_bzero(w, KEY_SIZE);

    w[31] = 1; // w = 1

    cx_math_addm(w, v, w, MOD); // w = (2 * u^2 + 1)

    cx_math_multm(x, w, w, MOD); // w^2

    cx_math_multm(y, (unsigned char *)C_fe_ma2, v, MOD); // -2 * A^2 * u^2

    cx_math_addm(x, x, y, MOD); // x = w^2 - 2 * A^2 * u^2

    // inline fe_divpowm1(r->X, w, x);     // (w / x)^(m + 1) =>
    // fe_divpowm1(r,u,v)
    {
        cx_math_multm(uv._v3, x, x, MOD);

        cx_math_multm(uv._v3, uv._v3, x, MOD); // v3 = v^3

        cx_math_multm(uv._uv7, uv._v3, uv._v3, MOD);

        cx_math_multm(uv._uv7, uv._uv7, x, MOD);

        cx_math_multm(uv._uv7, uv._uv7, w, MOD); // uv7 = uv^7

        cx_math_powm(uv._uv7, uv._uv7, (unsigned char *)C_fe_qm5div8, KEY_SIZE,
                     MOD); // (uv^7)^((q-5)/8)

        cx_math_multm(uv._uv7, uv._uv7, uv._v3, MOD);

        cx_math_multm(rX, uv._uv7, w, MOD); // u^(m+1)v^(-(m+1))
    }

    cx_math_multm(y, rX, rX, MOD);

    cx_math_multm(x, y, x, MOD);

    cx_math_subm(y, w, x, MOD);

    os_memmove(z, C_fe_ma, KEY_SIZE);

    if (!cx_math_is_zero(y, KEY_SIZE))
    {
        cx_math_addm(y, w, x, MOD);

        if (!cx_math_is_zero(y, KEY_SIZE))
        {
            goto negative;
        }
        else
        {
            cx_math_multm(rX, rX, (unsigned char *)C_fe_fffb1, MOD);
        }
    }
    else
    {
        cx_math_multm(rX, rX, (unsigned char *)C_fe_fffb2, MOD);
    }

    cx_math_multm(rX, rX, u, MOD); // u * sqrt(2 * A * (A + 2) * w / x)

    cx_math_multm(z, z, v, MOD); // -2 * A * u^2

    sign = 0;

    goto setsign;

// clang-format off
    negative:
    {
        cx_math_multm(x, x, (unsigned char *)C_fe_sqrtm1, MOD);

        cx_math_subm(y, w, x, MOD);

        if (!cx_math_is_zero(y, KEY_SIZE))
        {
            cx_math_addm(y, w, x, MOD);

            cx_math_multm(rX, rX, (unsigned char *)C_fe_fffb3, MOD);
        }
        else
        {
            cx_math_multm(rX, rX, (unsigned char *)C_fe_fffb4, MOD);
        }

        sign = 1;
    }

    setsign:
    {
        if (fe_isnegative(rX) != sign)
        {
            cx_math_sub(rX, (unsigned char *)C_ED25519_FIELD, rX, KEY_SIZE);
        }

        cx_math_addm(rZ, z, w, MOD);

        cx_math_subm(rY, z, w, MOD);

        cx_math_multm(rX, rX, rZ, MOD);

        cx_math_invprimem(u, rZ, MOD);

        uv._Pxy[0] = 0x04;

        cx_math_multm(&uv._Pxy[1], rX, u, MOD);

        cx_math_multm(&uv._Pxy[1 + KEY_SIZE], rY, u, MOD);

        hw_ge_tobytes(ge, uv._Pxy);
    }
    // clang-format on
}

/**
 * Hashes the given key, then loads the result as an elliptic curve point
 * and then multiplying it by 8
 * @param ec the resulting elliptic curve point
 * @param A the key to perform the operation on
 */
static int hw_hash_to_ec(unsigned char *ec, const unsigned char *A)
{
    const uint16_t status = hw_keccak(A, KEY_SIZE, ec);

    if (status != OP_OK)
    {
        return status;
    }

    hw_ge_fromfe_frombytes_vartime(ec, ec);

    return hw_ge_mul8(ec, ec);
}

/**
 * r = (a * P) + (b * G)
 * @param r the result
 * @param a the first private key
 * @param B the public key
 * @param b the second private key
 */
static int hw_ge_double_scalarmult_base(
    unsigned char *r,
    const unsigned char *a,
    const unsigned char *P,
    const unsigned char *b)
{
    unsigned char aP[KEY_SIZE];

    unsigned char bG[KEY_SIZE];

    // multiply a * P
    uint16_t status = hw_ge_scalarmult(aP, P, a);

    if (status != OP_OK)
    {
        return status;
    }

    // multiply b * G
    hw_ge_scalarmult_base(bG, b);

    // add the two points together
    status = hw_ge_add(r, aP, bG);

    if (status != OP_OK)
    {
        return status;
    }

    return OP_OK;
}

/**
 * r = (a * I) + (b * Hp(P))
 * @param r the result
 * @param a the first private key
 * @param I the first public key (key image)
 * @param b the second private key
 * @param P the second public key (point)
 */
static int hw_ge_double_scalarmult(
    unsigned char *r,
    const unsigned char *a,
    const unsigned char *I,
    const unsigned char *b,
    const unsigned char *P)
{
    unsigned char aI[KEY_SIZE];

    unsigned char bP[KEY_SIZE];

    // multiply a * I
    uint16_t status = hw_ge_scalarmult(aI, I, a);

    if (status != OP_OK)
    {
        return status;
    }

    // mutliply b * P
    status = hw_ge_scalarmult(bP, P, b);

    if (status != OP_OK)
    {
        return status;
    }

    // add the two points together
    status = hw_ge_add(r, aI, bP);

    if (status != OP_OK)
    {
        return status;
    }

    return OP_OK;
}

/**
 * Generates a random scalar
 * @param private the resulting scalar
 */
static void hw_random_scalar(unsigned char *private)
{
    unsigned char random[KEY_SIZE + 8];

    cx_rng(random, KEY_SIZE + 8);

    cx_math_modm(random, KEY_SIZE + 8, (unsigned char *)C_ED25519_ORDER, KEY_SIZE);

    reverse32(private, random + 8);
}

static int hw_hash_to_scalar(unsigned char *out, const unsigned char *in, size_t length)
{
    unsigned char hash[KEY_SIZE] = {0};

    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = hw_keccak(in, length, hash);

            if (status != OP_OK)
            {
                THROW(status);
            }

            hw_sc_reduce32(out, hash);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            explicit_bzero(hash, sizeof(hash));
        }
    }
    END_TRY;
}

/**
 * Completes a ring signature set
 * @param s the incomplete real output signature
 * @param k the scalar provided by external signature preparation
 * @param x the private ephemeral for the input being spent
 */
static void hw__complete_ring_signature(
    unsigned char *signatures,
    const size_t real_output_index,
    const unsigned char *k,
    const unsigned char *x)
{
    hw_sc_mulsub(
        signatures + (real_output_index * SIG_SIZE) + KEY_SIZE, signatures + (real_output_index * SIG_SIZE), x, k);
}

static uint16_t hw__prepare_ring_signatures(
    unsigned char *signatures,
    unsigned char *k,
    const unsigned char *tx_prefix_hash,
    const unsigned char *key_image,
    const unsigned char *public_keys,
    const size_t real_output_index)
{
    BEGIN_TRY
    {
#define REAL_SIG_POSITION signatures + (real_output_index * SIG_SIZE)
        TRY
        {
            // clear the buffer to make sure that it's not polluted
            explicit_bzero(BUFFER, BUFFER_SIZE);

            // copy the transaction prefix hash into the buffer
            os_memmove(BUFFER, tx_prefix_hash, KEY_SIZE);

            // generate a random scalar
            hw_random_scalar(k);

            unsigned char sum[KEY_SIZE] = {0};

            size_t i;

            /**
             * Loop through the inputs up through the number of ring
             * participants and perform the necessary calculations to
             * compute the ring signatures
             */
            for (i = 0; i < RING_PARTICIPANTS; i++)
            {
#define L signatures + (i * SIG_SIZE)
#define BL BUFFER + KEY_SIZE + (i * SIG_SIZE)
#define R signatures + (i * SIG_SIZE) + KEY_SIZE
#define BR BL + KEY_SIZE
#define PUBLIC_KEY public_keys + (i * KEY_SIZE)
#define SIGNATURE signatures + (i * SIG_SIZE)

                /**
                 * If the current input is the "real" input being spent then
                 * we do things a little bit differently
                 */
                if (i == real_output_index)
                {
                    // L = k * G
                    hw_ge_scalarmult_base(BL, k);

                    // Hp(P)
                    uint16_t status = hw_hash_to_ec(BR, PUBLIC_KEY);

                    if (status != OP_OK)
                    {
                        THROW(status);
                    }

                    // R = k * Hp(P)
                    status = hw_ge_scalarmult(BR, BR, k);

                    if (status != OP_OK)
                    {
                        THROW(status);
                    }
                }
                else
                {
                    /**
                     * If the current input is not the "real" input (a mixin)
                     * then we do some the regular signature math
                     */
                    // generate a new random scalar
                    hw_random_scalar(L);

                    // generate a new random scalar
                    hw_random_scalar(R);

                    // L = (k1 * P) + (k2 * G)
                    uint16_t status = hw_ge_double_scalarmult_base(BL, L, PUBLIC_KEY, R);

                    if (status != OP_OK)
                    {
                        THROW(status);
                    }

                    // Hp(P)
                    status = hw_hash_to_ec(BR, PUBLIC_KEY);

                    if (status != OP_OK)
                    {
                        THROW(status);
                    }

                    // R = (k1 * I) + (k2 * Hp(P))
                    status = hw_ge_double_scalarmult(BR, R, BR, L, key_image);

                    if (status != OP_OK)
                    {
                        THROW(status);
                    }

                    // add L to the current sum
                    hw_sc_add(sum, sum, L);
                }
#undef SIGNATURE
#undef PUBLIC_KEY
#undef BR
#undef R
#undef BL
#undef L
            }

            unsigned char hash[32] = {0};

            // Hs(prefix + L's + R's)
            const uint16_t status = hw_hash_to_scalar(hash, BUFFER, BUFFER_SIZE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // L'r = Hs(prefix + L's + R's) - sum
            hw_sc_sub(REAL_SIG_POSITION, hash, sum);

            // R'r = {0}
            explicit_bzero(REAL_SIG_POSITION + KEY_SIZE, KEY_SIZE);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            // wipe the buffer as we don't want that leaking
            explicit_bzero(BUFFER, BUFFER_SIZE);
#undef REAL_SIG_POSITION
        }
    }
    END_TRY;
}

/**
 * END OF STATIC METHODS
 */

uint16_t hw_check_signature(
    const unsigned char *message_digest,
    const unsigned char *public_key,
    const unsigned char *signature)
{
#define S_COMM_SIZE KEY_SIZE + KEY_SIZE + KEY_SIZE
#define MESSAGE_DIGEST BUFFER
#define KEY MESSAGE_DIGEST + KEY_SIZE
#define COMM KEY + KEY_SIZE
#define SCALAR COMM + KEY_SIZE
    BEGIN_TRY
    {
        TRY
        {
            os_memmove(MESSAGE_DIGEST, message_digest, KEY_SIZE);

            os_memmove(KEY, public_key, KEY_SIZE);

            uint16_t status = hw_ge_double_scalarmult_base(COMM, signature, public_key, signature + 32);

            if (status != OP_OK)
            {
                THROW(status);
            }

            status = hw_hash_to_scalar(SCALAR, BUFFER, S_COMM_SIZE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            hw_sc_sub(SCALAR, SCALAR, signature);

            reverse32(SCALAR, SCALAR);

            CLOSE_TRY;

            return cx_math_is_zero(SCALAR, KEY_SIZE);
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            // wipe the buffer as we don't want that leaking
            explicit_bzero(BUFFER, BUFFER_SIZE);
#undef SCALAR
#undef COMM
#undef KEY
#undef MESSAGE_DIGEST
#undef S_COMM_SIZE
        }
    }
    END_TRY;
}

uint16_t hw_check_ring_signatures(
    const unsigned char *tx_prefix_hash,
    const unsigned char *key_image,
    const unsigned char *public_keys,
    const unsigned char *signatures)
{
    BEGIN_TRY
    {
        TRY
        {
            // clear the buffer to make sure that it's not polluted
            explicit_bzero(BUFFER, BUFFER_SIZE);

            // copy the transaction prefix hash into the buffer
            os_memmove(BUFFER, tx_prefix_hash, KEY_SIZE);

            unsigned char sum[KEY_SIZE] = {0};

            size_t i;

            for (i = 0; i < RING_PARTICIPANTS; i++)
            {
#define BL BUFFER + KEY_SIZE + (i * SIG_SIZE)
#define BR BL + KEY_SIZE
#define PUBLIC_KEY public_keys + (i * KEY_SIZE)
#define SIGNATURE signatures + (i * SIG_SIZE)
#define L SIGNATURE
#define R SIGNATURE + KEY_SIZE
                // L = (k1 * P) + (k2 * G)
                uint16_t status = hw_ge_double_scalarmult_base(BL, L, PUBLIC_KEY, R);

                if (status != OP_OK)
                {
                    THROW(status);
                }

                // Hp(P)
                status = hw_hash_to_ec(BR, PUBLIC_KEY);

                if (status != OP_OK)
                {
                    THROW(status);
                }

                // R = (k1 * I) + (k2 * Hp(P))
                status = hw_ge_double_scalarmult(BR, R, BR, L, key_image);

                if (status != OP_OK)
                {
                    THROW(status);
                }

                // add L to the current sum
                hw_sc_add(sum, sum, L);
#undef R
#undef L
#undef SIGNATURE
#undef PUBLIC_KEY
#undef BR
#undef BL
            }

            unsigned char hash[KEY_SIZE] = {0};

            // Hs(prefix + L's + R's)
            const uint16_t status = hw_hash_to_scalar(hash, BUFFER, BUFFER_SIZE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // L'r = Hs(prefix + L's + R's) - sum
            hw_sc_sub(hash, hash, sum);

            reverse32(hash, hash);

            CLOSE_TRY;

            return cx_math_is_zero(hash, KEY_SIZE);
        }
        CATCH_OTHER(e)
        {
            // this would be (false) to mere humans
            return e;
        }
        FINALLY
        {
            // wipe the buffer as we don't want that leaking
            explicit_bzero(BUFFER, BUFFER_SIZE);
        }
    }
    END_TRY;
}

uint16_t hw_complete_ring_signature(
    unsigned char *signature,
    const unsigned char *tx_public_key,
    const size_t output_index,
    const unsigned char *output_key,
    const unsigned char *k,
    const unsigned char *privateView,
    const unsigned char *privateSpend,
    const unsigned char *publicSpend)
{
#define DERIVATION BUFFER
#define PUBLIC_EPHEMERAL DERIVATION + KEY_SIZE
#define PRIVATE_EPHEMERAL PUBLIC_EPHEMERAL + KEY_SIZE

    BEGIN_TRY
    {
        TRY
        {
            // Generate the transaction key derivation D = (rA)
            uint16_t status = hw_generate_key_derivation(DERIVATION, tx_public_key, privateView);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // Generate the public ephemeral for the given output P = H(D || n)G + B
            status = hw_derive_public_key(PUBLIC_EPHEMERAL, DERIVATION, output_index, publicSpend);

            if (status != OP_OK)
            {
                THROW(status);
            }

            /**
             * This checks to verify that the key generation request that we are
             * processing is for an output that was actually sent to us, otherwise, we
             * will fail
             */
            status = os_memcmp(PUBLIC_EPHEMERAL, output_key, KEY_SIZE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // Generate the private ephemeral for the given output x = H(D || N) + b
            status = hw_derive_secret_key(PRIVATE_EPHEMERAL, DERIVATION, output_index, privateSpend);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // Complete the provided ring signature using the supplied k value and
            // private ephemeral
            hw__complete_ring_signature(signature, 0, k, PRIVATE_EPHEMERAL);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            // wipe the buffer as we don't want that leaking
            explicit_bzero(BUFFER, BUFFER_SIZE);
#undef PRIVATE_EPHEMERAL
#undef PUBLIC_EPHEMERAL
#undef DERIVATION
        }
    }
    END_TRY;
}

uint16_t hw_derive_public_key(
    unsigned char *key,
    const unsigned char *derivation,
    const size_t output_index,
    const unsigned char *publicSpend)
{
    unsigned char temp[KEY_SIZE] = {0};

    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = hw_derivation_to_scalar(temp, derivation, output_index);

            if (status != OP_OK)
            {
                THROW(status);
            }

            hw_ge_scalarmult_base(temp, temp);

            CLOSE_TRY;

            return hw_ge_add(key, temp, publicSpend);
        }
        CATCH_OTHER(e)
        {
            return e;
        };
        FINALLY
        {
            explicit_bzero(temp, sizeof(temp));
        };
    }
    END_TRY;
}

uint16_t hw_derive_secret_key(
    unsigned char *key,
    const unsigned char *derivation,
    const size_t output_index,
    const unsigned char *privateSpend)
{
    unsigned char temp[KEY_SIZE] = {0};

    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = hw_derivation_to_scalar(temp, derivation, output_index);

            if (status != OP_OK)
            {
                THROW(status);
            }

            hw_sc_add(key, temp, privateSpend);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        };
        FINALLY
        {
            explicit_bzero(temp, sizeof(temp));
        };
    }
    END_TRY;
}

uint16_t hw_generate_keypair(unsigned char *public, unsigned char *private)
{
    BEGIN_TRY
    {
        TRY
        {
            hw_random_scalar(private);

            hw_ge_scalarmult_base(public, private);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY {}
    }
    END_TRY;
}

uint16_t hw_generate_ring_signatures(
    unsigned char *signatures,
    const unsigned char *tx_public_key,
    const size_t output_index,
    const unsigned char *output_key,
    const unsigned char *tx_prefix_hash,
    const unsigned char *public_keys,
    const size_t real_output_index,
    const unsigned char *privateView,
    const unsigned char *privateSpend,
    const unsigned char *publicSpend)
{
    /**
     * We have to use a local buffer here to avoid running into issues in using
     * the shared working space that hw__generate_ring_signatures() will use
     */
    unsigned char buffer[KEY_SIZE * 2] = {0};

#define LOCAL_BUFFER ((unsigned char *)buffer)
#define DERIVATION LOCAL_BUFFER
#define EPHEMERAL DERIVATION + KEY_SIZE

    BEGIN_TRY
    {
        TRY
        {
            // Generate the transaction key derivation D = (rA)
            uint16_t status = hw_generate_key_derivation(DERIVATION, tx_public_key, privateView);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // Generate the public ephemeral for the given output P = H(D || n)G + B
            status = hw_derive_public_key(EPHEMERAL, DERIVATION, output_index, publicSpend);

            if (status != OP_OK)
            {
                THROW(status);
            }

            /**
             * This checks to verify that the key generation request that we are
             * processing is for an output that was actually sent to us, otherwise, we
             * will fail
             */
            status = os_memcmp(EPHEMERAL, output_key, KEY_SIZE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // Generate the private ephemeral for the given output x = H(D || N) + b
            status = hw_derive_secret_key(EPHEMERAL, DERIVATION, output_index, privateSpend);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // generate the key image and store it in the derivation to reduce memory
            // use
            status = hw__generate_key_image(DERIVATION, output_key, EPHEMERAL);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // generate the ring signatures
            status = hw__generate_ring_signatures(
                signatures, tx_prefix_hash, DERIVATION, public_keys, EPHEMERAL, real_output_index);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            // wipe the buffer as we don't want that leaking
            explicit_bzero(buffer, sizeof(buffer));
#undef PRIVATE_EPHEMERAL
#undef PUBLIC_EPHEMERAL
#undef DERIVATION
#undef LOCAL_BUFFER
        }
    }
    END_TRY;
}

uint16_t
    hw_generate_key_derivation(unsigned char *derivation, const unsigned char *public, const unsigned char *private)
{
    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = hw_ge_scalarmult(derivation, public, private);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            return hw_ge_mul8(derivation, derivation);
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY {}
    }
    END_TRY;
}

uint16_t hw_generate_key_image(
    unsigned char *key_image,
    const unsigned char *tx_public_key,
    const size_t output_index,
    const unsigned char *output_key,
    const unsigned char *privateView,
    const unsigned char *privateSpend,
    const unsigned char *publicSpend)
{
    BEGIN_TRY
    {
#define DERIVATION BUFFER
#define PUBLIC_EPHEMERAL DERIVATION + KEY_SIZE
#define PRIVATE_EPHEMERAL PUBLIC_EPHEMERAL + KEY_SIZE
        TRY
        {
            // Generate the transaction key derivation D = (rA)
            uint16_t status = hw_generate_key_derivation(DERIVATION, tx_public_key, privateView);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // Generate the public ephemeral for the given output P = H(D || n)G + B
            status = hw_derive_public_key(PUBLIC_EPHEMERAL, DERIVATION, output_index, publicSpend);

            if (status != OP_OK)
            {
                THROW(status);
            }

            /**
             * This checks to verify that the key generation request that we are
             * processing is for an output that was actually sent to us, otherwise, we
             * will fail
             */
            status = os_memcmp(PUBLIC_EPHEMERAL, output_key, KEY_SIZE);

            if (status != OP_OK)
            {
                THROW(ERR_PUBKEY_MISMATCH);
            }

            // Generate the private ephemeral for the given output x = H(D || N) + b
            status = hw_derive_secret_key(PRIVATE_EPHEMERAL, DERIVATION, output_index, privateSpend);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            // Generate the key image I = Hp(P)x
            return hw__generate_key_image(key_image, PUBLIC_EPHEMERAL, PRIVATE_EPHEMERAL);
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            // wipe the buffer as we don't want that leaking
            explicit_bzero(BUFFER, BUFFER_SIZE);
#undef PRIVATE_EPHEMERAL
#undef PUBLIC_EPHEMERAL
#undef DERIVATION
        }
    }
    END_TRY;
}

uint16_t hw_generate_key_image_primitive(
    unsigned char *key_image,
    const unsigned char *derivation,
    const size_t output_index,
    const unsigned char *output_key,
    const unsigned char *privateSpend,
    const unsigned char *publicSpend)
{
    BEGIN_TRY
    {
#define PUBLIC_EPHEMERAL BUFFER
#define PRIVATE_EPHEMERAL PUBLIC_EPHEMERAL + KEY_SIZE
        TRY
        {
            // Generate the public ephemeral for the given output P = H(D || n)G + B
            uint16_t status = hw_derive_public_key(PUBLIC_EPHEMERAL, derivation, output_index, publicSpend);

            if (status != OP_OK)
            {
                THROW(status);
            }

            /**
             * This checks to verify that the key generation request that we are
             * processing is for an output that was actually sent to us, otherwise, we
             * will fail
             */
            status = os_memcmp(PUBLIC_EPHEMERAL, output_key, KEY_SIZE);

            if (status != OP_OK)
            {
                THROW(ERR_PUBKEY_MISMATCH);
            }

            // Generate the private ephemeral for the given output x = H(D || N) + b
            status = hw_derive_secret_key(PRIVATE_EPHEMERAL, derivation, output_index, privateSpend);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            // Generate the key image I = Hp(P)x
            return hw__generate_key_image(key_image, PUBLIC_EPHEMERAL, PRIVATE_EPHEMERAL);
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            // wipe the buffer as we don't want that leaking
            explicit_bzero(BUFFER, BUFFER_SIZE);
#undef PRIVATE_EPHEMERAL
#undef PUBLIC_EPHEMERAL
        }
    }
    END_TRY;
}

uint16_t hw_generate_private_view_key(unsigned char *privateView, const unsigned char *privateSpend)
{
    return hw_hash_to_scalar(privateView, privateSpend, KEY_SIZE);
}

uint16_t hw_generate_signature(
    unsigned char *signature,
    const unsigned char *message_digest,
    const unsigned char *public_key,
    const unsigned char *private_key)
{
#define S_COMM_SIZE KEY_SIZE + KEY_SIZE + KEY_SIZE
#define MESSAGE_DIGEST BUFFER
#define KEY MESSAGE_DIGEST + KEY_SIZE
#define COMM KEY + KEY_SIZE
#define K COMM + SIG_SIZE
    BEGIN_TRY
    {
        TRY
        {
            os_memmove(MESSAGE_DIGEST, message_digest, KEY_SIZE);

            os_memmove(KEY, public_key, KEY_SIZE);

            hw_random_scalar(K);

            hw_ge_scalarmult_base(COMM, K);

            const uint16_t status = hw_hash_to_scalar(signature, BUFFER, S_COMM_SIZE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            hw_sc_mulsub(signature + KEY_SIZE, signature, private_key, K);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            // wipe the buffer as we don't want that leaking
            explicit_bzero(BUFFER, BUFFER_SIZE);
#undef K
#undef COMM
#undef KEY
#undef MESSAGE_DIGEST
#undef S_COMM_SIZE
        }
    }
    END_TRY;
}

// cn_fast_hash
uint16_t hw_keccak(const unsigned char *in, size_t length, unsigned char *out)
{
    BEGIN_TRY
    {
        TRY
        {
            cx_sha3_t hw_keccak_context;

            cx_keccak_init(&hw_keccak_context, KECCAK_BITS);

            cx_hash((cx_hash_t *)&hw_keccak_context, CX_LAST, in, length, out, KEY_SIZE);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return ERR_KECCAK;
        }
        FINALLY {}
    }
    END_TRY;
}

uint16_t hw_private_key_to_public_key(unsigned char *public, const unsigned char *private)
{
    BEGIN_TRY
    {
        TRY
        {
            hw_ge_scalarmult_base(public, private);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return ERR_PRIVATE_TO_PUBLIC;
        }
        FINALLY {}
    }
    END_TRY;
}

uint16_t hw_retrieve_private_spend_key(unsigned char *private)
{
    BEGIN_TRY
    {
#define SEED BUFFER
#define KEY SEED + KEY_SIZE + KEY_SIZE
#define CHAIN KEY + KEY_SIZE
        TRY
        {
            uint32_t bip32Path[BIP32_PATH];

            os_memmove(bip32Path, derivePath, sizeof(derivePath));

            // Retrieve the hardware wallet seed for our defined curve and BIP-32 path
            os_perso_derive_node_bip32(CX_CURVE_Ed25519, bip32Path, BIP32_PATH, SEED, CHAIN);

            // Hash that seed
            const uint16_t status = hw_keccak(SEED, KEY_SIZE, KEY);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // Reduce the hash to a scalar
            hw_sc_reduce32(private, KEY);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            // wipe the buffer as we don't want that leaking
            explicit_bzero(BUFFER, BUFFER_SIZE);
#undef CHAIN
#undef KEY
#undef SEED
        }
    }
    END_TRY;
}

uint16_t hw_check_key(const unsigned char *key)
{
    BEGIN_TRY
    {
        TRY
        {
            /**
             * The most common misuse of keys is dropping a public in a private spot
             * or a private in a public spot. This simply checks to make sure that we
             * did not drop a scalar in where we expected a public key as a public key
             * is never a scalar :)
             */
            const uint16_t status = hw_check_scalar(key);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            return OP_NOK;
        }
        CATCH_OTHER(e)
        {
            return OP_OK;
        }
        FINALLY {}
    }
    END_TRY;
}

uint16_t hw_check_scalar(const unsigned char *scalar)
{
    unsigned char reduced[KEY_SIZE] = {0};

    BEGIN_TRY
    {
        TRY
        {
            hw_sc_reduce32(reduced, scalar);

            const uint16_t status = os_memcmp(reduced, scalar, KEY_SIZE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            return OP_NOK;
        }
        CATCH_OTHER(e)
        {
            return OP_OK;
        }
        FINALLY
        {
            explicit_bzero(reduced, KEY_SIZE);
        }
    }
    END_TRY;
}

/**
 * Generates a key image such that
 * I = Hp(P)x
 * @param I the resulting key image
 * @param P the public key (public ephemeral)
 * @param x the private key (private ephemeral)
 */
uint16_t hw__generate_key_image(unsigned char *I, const unsigned char *P, const unsigned char *x)
{
    unsigned char HpP[KEY_SIZE] = {0};

    // Hp(P)
    uint16_t status = hw_hash_to_ec(HpP, P);

    if (status != OP_OK)
    {
        return status;
    }

    // I = Hp(P) * x
    status = hw_ge_scalarmult(I, HpP, x);

    if (status != OP_OK)
    {
        return status;
    }

    return OP_OK;
}

/**
 * Generates the ring signatures for a transaction
 * @param signatures the resulting ring signatures
 * @param tx_prefix_hash the transaction prefix hash
 * @param key_image the key image used for the input
 * @param public_keys the public keys for the input including mixins
 * @param private_ephemeral the private ephemeral of the input
 * @param real_output_index the index of the real output in the public_keys
 */
uint16_t hw__generate_ring_signatures(
    unsigned char *signatures,
    const unsigned char *tx_prefix_hash,
    const unsigned char *key_image,
    const unsigned char *public_keys,
    const unsigned char *private_ephemeral,
    const size_t real_output_index)
{
    unsigned char k[KEY_SIZE] = {0};

    // prepare the ring signatures
    uint16_t status =
        hw__prepare_ring_signatures(signatures, k, tx_prefix_hash, key_image, public_keys, real_output_index);

    if (status != OP_OK)
    {
        return status;
    }

    // complete the ring signatures
    hw__complete_ring_signature(signatures, real_output_index, k, private_ephemeral);

    return OP_OK;
}