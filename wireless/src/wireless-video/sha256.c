#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Small, one-shot SHA-256 verifier for the MST2/J5e installer.
 *
 * The target image has no sha256/sha256sum/openssl command.  This helper is
 * uploaded only to the volatile installation staging directory, verifies a
 * file read-only, and is removed with that directory after the transaction.
 * It is not installed as a service and is never part of the runtime path.
 */

typedef struct {
    uint8_t block[64];
    uint32_t block_len;
    uint64_t total_bits;
    uint32_t state[8];
} sha256_ctx;

static const uint32_t k[64] = {
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
    0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
    0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
    0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
    0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
    0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
};

static uint32_t rotr32(uint32_t value, uint32_t count)
{
    return (value >> count) | (value << (32U - count));
}

static void sha256_transform(sha256_ctx *ctx, const uint8_t block[64])
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;
    unsigned int i;

    for (i = 0U; i < 16U; ++i) {
        unsigned int p = i * 4U;
        w[i] = ((uint32_t)block[p] << 24) |
               ((uint32_t)block[p + 1U] << 16) |
               ((uint32_t)block[p + 2U] << 8) |
               (uint32_t)block[p + 3U];
    }
    for (i = 16U; i < 64U; ++i) {
        uint32_t s0 = rotr32(w[i - 15U], 7U) ^
                      rotr32(w[i - 15U], 18U) ^ (w[i - 15U] >> 3);
        uint32_t s1 = rotr32(w[i - 2U], 17U) ^
                      rotr32(w[i - 2U], 19U) ^ (w[i - 2U] >> 10);
        w[i] = w[i - 16U] + s0 + w[i - 7U] + s1;
    }

    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    for (i = 0U; i < 64U; ++i) {
        uint32_t s1 = rotr32(e, 6U) ^ rotr32(e, 11U) ^ rotr32(e, 25U);
        uint32_t choose = (e & f) ^ ((~e) & g);
        uint32_t s0 = rotr32(a, 2U) ^ rotr32(a, 13U) ^ rotr32(a, 22U);
        uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
        t1 = h + s1 + choose + k[i] + w[i];
        t2 = s0 + majority;
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b;
    ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f;
    ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(sha256_ctx *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->state[0] = 0x6a09e667U; ctx->state[1] = 0xbb67ae85U;
    ctx->state[2] = 0x3c6ef372U; ctx->state[3] = 0xa54ff53aU;
    ctx->state[4] = 0x510e527fU; ctx->state[5] = 0x9b05688cU;
    ctx->state[6] = 0x1f83d9abU; ctx->state[7] = 0x5be0cd19U;
}

static void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t length)
{
    size_t i;
    for (i = 0U; i < length; ++i) {
        ctx->block[ctx->block_len++] = data[i];
        if (ctx->block_len == 64U) {
            sha256_transform(ctx, ctx->block);
            ctx->total_bits += 512U;
            ctx->block_len = 0U;
        }
    }
}

static void sha256_final(sha256_ctx *ctx, uint8_t digest[32])
{
    uint64_t total_bits = ctx->total_bits + ((uint64_t)ctx->block_len * 8U);
    unsigned int i;

    ctx->block[ctx->block_len++] = 0x80U;
    if (ctx->block_len > 56U) {
        while (ctx->block_len < 64U) ctx->block[ctx->block_len++] = 0U;
        sha256_transform(ctx, ctx->block);
        ctx->block_len = 0U;
    }
    while (ctx->block_len < 56U) ctx->block[ctx->block_len++] = 0U;
    for (i = 0U; i < 8U; ++i) {
        ctx->block[63U - i] = (uint8_t)(total_bits >> (i * 8U));
    }
    sha256_transform(ctx, ctx->block);

    for (i = 0U; i < 8U; ++i) {
        digest[i * 4U] = (uint8_t)(ctx->state[i] >> 24);
        digest[i * 4U + 1U] = (uint8_t)(ctx->state[i] >> 16);
        digest[i * 4U + 2U] = (uint8_t)(ctx->state[i] >> 8);
        digest[i * 4U + 3U] = (uint8_t)ctx->state[i];
    }
}

static int hash_file(const char *path, uint8_t digest[32])
{
    FILE *stream;
    uint8_t buffer[32768];
    size_t count;
    sha256_ctx ctx;

    stream = fopen(path, "rb");
    if (stream == NULL) return 10;
    sha256_init(&ctx);
    while ((count = fread(buffer, 1U, sizeof(buffer), stream)) != 0U) {
        sha256_update(&ctx, buffer, count);
    }
    if (ferror(stream)) {
        fclose(stream);
        return 11;
    }
    if (fclose(stream) != 0) return 12;
    sha256_final(&ctx, digest);
    memset(&ctx, 0, sizeof(ctx));
    memset(buffer, 0, sizeof(buffer));
    return 0;
}

#ifndef CPMAP_SHA256_EMBEDDED
static int normalize_expected(const char *input, char output[65])
{
    size_t i;
    if (strlen(input) != 64U) return -1;
    for (i = 0U; i < 64U; ++i) {
        unsigned char value = (unsigned char)input[i];
        if (!isxdigit(value)) return -1;
        output[i] = (char)toupper(value);
    }
    output[64] = '\0';
    return 0;
}

int main(int argc, char **argv)
{
    uint8_t digest[32];
    char actual[65];
    char expected[65];
    unsigned int i;
    int result;

    if (argc != 2 && argc != 3) return 2;
    result = hash_file(argv[1], digest);
    if (result != 0) return result;
    for (i = 0U; i < 32U; ++i) {
        static const char hex[] = "0123456789ABCDEF";
        actual[i * 2U] = hex[digest[i] >> 4];
        actual[i * 2U + 1U] = hex[digest[i] & 0x0fU];
    }
    actual[64] = '\0';
    memset(digest, 0, sizeof(digest));

    if (argc == 2) {
        puts(actual);
        return 0;
    }
    if (normalize_expected(argv[2], expected) != 0) return 2;
    return strcmp(actual, expected) == 0 ? 0 : 3;
}
#endif
