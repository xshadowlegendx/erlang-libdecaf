// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c et

#include <decaf/shake.h>

/*
 * Erlang NIF functions
 */

#define SHA3_DEFINITION(bits, bytes)                                                                                               \
    static int libdecaf_nif_sha3_##bits##_2_work(ErlNifEnv *env, xnif_slice_t *slice, int *phasep, size_t *offsetp,                \
                                                 size_t reductions);                                                               \
    static ERL_NIF_TERM libdecaf_nif_sha3_##bits##_2_done(ErlNifEnv *env, xnif_slice_t *slice);                                    \
                                                                                                                                   \
    static xnif_slice_func_t libdecaf_nif_sha3_##bits##_2_func = {                                                                 \
        libdecaf_nif_sha3_##bits##_2_work,                                                                                         \
        libdecaf_nif_sha3_##bits##_2_done,                                                                                         \
        NULL,                                                                                                                      \
        NULL,                                                                                                                      \
    };                                                                                                                             \
                                                                                                                                   \
    static ERL_NIF_TERM libdecaf_nif_sha3_##bits##_2(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])                          \
    {                                                                                                                              \
        struct decaf_sha3_##bits##_ctx_s ctxbuf;                                                                                   \
        struct decaf_sha3_##bits##_ctx_s *ctx = &ctxbuf;                                                                           \
        ErlNifBinary input;                                                                                                        \
        unsigned long outlen;                                                                                                      \
        unsigned char *outbuf = NULL;                                                                                              \
        ERL_NIF_TERM out_term;                                                                                                     \
                                                                                                                                   \
        if (argc != 2 || !enif_inspect_iolist_as_binary(env, argv[0], &input) || !enif_get_ulong(env, argv[1], &outlen) ||         \
            outlen > bytes) {                                                                                                      \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
                                                                                                                                   \
        (void)decaf_sha3_##bits##_init(ctx);                                                                                       \
                                                                                                                                   \
        if (input.size <= XNIF_SLICE_MAX_PER_SLICE) {                                                                              \
            outbuf = enif_make_new_binary(env, outlen, &out_term);                                                                 \
            (void)decaf_sha3_##bits##_update(ctx, input.data, input.size);                                                         \
            (void)decaf_sha3_##bits##_final(ctx, outbuf, outlen);                                                                  \
            (void)decaf_sha3_##bits##_destroy(ctx);                                                                                \
            return out_term;                                                                                                       \
        }                                                                                                                          \
                                                                                                                                   \
        ctx = libdecaf_nif_alloc_sha3_##bits##_ctx(env);                                                                           \
        if (ctx == NULL) {                                                                                                         \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
        (void)memcpy(ctx, &ctxbuf, sizeof(struct decaf_sha3_##bits##_ctx_s));                                                      \
                                                                                                                                   \
        xnif_slice_t *slice = xnif_slice_create(env, "sha3_" #bits, &libdecaf_nif_sha3_##bits##_2_func, 0, input.size);            \
        if (slice == NULL) {                                                                                                       \
            (void)enif_release_resource((void *)ctx);                                                                              \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
        ERL_NIF_TERM newargv[3];                                                                                                   \
        newargv[0] = enif_make_resource(env, (void *)ctx);                                                                         \
        newargv[1] = argv[0];                                                                                                      \
        newargv[2] = argv[1];                                                                                                      \
        (void)enif_release_resource((void *)ctx);                                                                                  \
                                                                                                                                   \
        return xnif_slice_schedule(env, slice, 2, newargv);                                                                        \
    }                                                                                                                              \
                                                                                                                                   \
    static int libdecaf_nif_sha3_##bits##_2_work(ErlNifEnv *env, xnif_slice_t *slice, int *phasep, size_t *offsetp,                \
                                                 size_t reductions)                                                                \
    {                                                                                                                              \
        struct decaf_sha3_##bits##_ctx_s *ctx = NULL;                                                                              \
        size_t offset = *offsetp;                                                                                                  \
        ErlNifBinary input;                                                                                                        \
                                                                                                                                   \
        if (slice->argc != 3 || !libdecaf_nif_get_sha3_##bits##_ctx(env, slice->argv[0], &ctx) ||                                  \
            !enif_inspect_iolist_as_binary(env, slice->argv[1], &input)) {                                                         \
            return -1;                                                                                                             \
        }                                                                                                                          \
        (void)decaf_sha3_##bits##_update(ctx, (const uint8_t *)input.data + offset, reductions);                                   \
        *offsetp = offset + reductions;                                                                                            \
        return 0;                                                                                                                  \
    }                                                                                                                              \
                                                                                                                                   \
    static ERL_NIF_TERM libdecaf_nif_sha3_##bits##_2_done(ErlNifEnv *env, xnif_slice_t *slice)                                     \
    {                                                                                                                              \
        struct decaf_sha3_##bits##_ctx_s *ctx = NULL;                                                                              \
        unsigned long outlen;                                                                                                      \
        unsigned char *outbuf = NULL;                                                                                              \
        ERL_NIF_TERM out_term;                                                                                                     \
                                                                                                                                   \
        if (slice->argc != 3 || !libdecaf_nif_get_sha3_##bits##_ctx(env, slice->argv[0], &ctx) ||                                  \
            !enif_get_ulong(env, slice->argv[2], &outlen) || outlen > bytes) {                                                     \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
                                                                                                                                   \
        outbuf = enif_make_new_binary(env, outlen, &out_term);                                                                     \
        (void)decaf_sha3_##bits##_final(ctx, outbuf, outlen);                                                                      \
        (void)decaf_sha3_##bits##_destroy(ctx);                                                                                    \
                                                                                                                                   \
        return out_term;                                                                                                           \
    }                                                                                                                              \
                                                                                                                                   \
    static ERL_NIF_TERM libdecaf_nif_sha3_##bits##_init_0(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])                     \
    {                                                                                                                              \
        struct decaf_sha3_##bits##_ctx_s ctxbuf;                                                                                   \
        struct decaf_sha3_##bits##_ctx_s *ctx = &ctxbuf;                                                                           \
        ERL_NIF_TERM out_term;                                                                                                     \
                                                                                                                                   \
        if (argc != 0) {                                                                                                           \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
                                                                                                                                   \
        (void)decaf_sha3_##bits##_init(ctx);                                                                                       \
                                                                                                                                   \
        ctx = libdecaf_nif_alloc_sha3_##bits##_ctx(env);                                                                           \
        if (ctx == NULL) {                                                                                                         \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
        (void)memcpy(ctx, &ctxbuf, sizeof(struct decaf_sha3_##bits##_ctx_s));                                                      \
                                                                                                                                   \
        out_term = enif_make_resource(env, (void *)ctx);                                                                           \
        (void)enif_release_resource((void *)ctx);                                                                                  \
                                                                                                                                   \
        return out_term;                                                                                                           \
    }                                                                                                                              \
                                                                                                                                   \
    static int libdecaf_nif_sha3_##bits##_update_2_work(ErlNifEnv *env, xnif_slice_t *slice, int *phasep, size_t *offsetp,         \
                                                        size_t reductions);                                                        \
    static ERL_NIF_TERM libdecaf_nif_sha3_##bits##_update_2_done(ErlNifEnv *env, xnif_slice_t *slice);                             \
                                                                                                                                   \
    static xnif_slice_func_t libdecaf_nif_sha3_##bits##_update_2_func = {                                                          \
        libdecaf_nif_sha3_##bits##_update_2_work,                                                                                  \
        libdecaf_nif_sha3_##bits##_update_2_done,                                                                                  \
        NULL,                                                                                                                      \
        NULL,                                                                                                                      \
    };                                                                                                                             \
                                                                                                                                   \
    static ERL_NIF_TERM libdecaf_nif_sha3_##bits##_update_2(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])                   \
    {                                                                                                                              \
        struct decaf_sha3_##bits##_ctx_s *old_ctx = NULL;                                                                          \
        struct decaf_sha3_##bits##_ctx_s *new_ctx = NULL;                                                                          \
        ErlNifBinary input;                                                                                                        \
        ERL_NIF_TERM out_term;                                                                                                     \
                                                                                                                                   \
        if (argc != 2 || !libdecaf_nif_get_sha3_##bits##_ctx(env, argv[0], &old_ctx) ||                                            \
            !enif_inspect_iolist_as_binary(env, argv[1], &input)) {                                                                \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
                                                                                                                                   \
        new_ctx = libdecaf_nif_alloc_sha3_##bits##_ctx(env);                                                                       \
        if (new_ctx == NULL) {                                                                                                     \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
        (void)memcpy(new_ctx, old_ctx, sizeof(struct decaf_sha3_##bits##_ctx_s));                                                  \
                                                                                                                                   \
        if (input.size <= XNIF_SLICE_MAX_PER_SLICE) {                                                                              \
            (void)decaf_sha3_##bits##_update(new_ctx, input.data, input.size);                                                     \
            out_term = enif_make_resource(env, (void *)new_ctx);                                                                   \
            (void)enif_release_resource((void *)new_ctx);                                                                          \
            return out_term;                                                                                                       \
        }                                                                                                                          \
                                                                                                                                   \
        xnif_slice_t *slice =                                                                                                      \
            xnif_slice_create(env, "sha3_" #bits "_update", &libdecaf_nif_sha3_##bits##_update_2_func, 0, input.size);             \
        if (slice == NULL) {                                                                                                       \
            (void)enif_release_resource((void *)new_ctx);                                                                          \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
        ERL_NIF_TERM newargv[2];                                                                                                   \
        newargv[0] = enif_make_resource(env, (void *)new_ctx);                                                                     \
        newargv[1] = argv[1];                                                                                                      \
        (void)enif_release_resource((void *)new_ctx);                                                                              \
                                                                                                                                   \
        return xnif_slice_schedule(env, slice, 2, newargv);                                                                        \
    }                                                                                                                              \
                                                                                                                                   \
    static int libdecaf_nif_sha3_##bits##_update_2_work(ErlNifEnv *env, xnif_slice_t *slice, int *phasep, size_t *offsetp,         \
                                                        size_t reductions)                                                         \
    {                                                                                                                              \
        struct decaf_sha3_##bits##_ctx_s *ctx = NULL;                                                                              \
        size_t offset = *offsetp;                                                                                                  \
        ErlNifBinary input;                                                                                                        \
                                                                                                                                   \
        if (slice->argc != 2 || !libdecaf_nif_get_sha3_##bits##_ctx(env, slice->argv[0], &ctx) ||                                  \
            !enif_inspect_iolist_as_binary(env, slice->argv[1], &input)) {                                                         \
            return -1;                                                                                                             \
        }                                                                                                                          \
        (void)decaf_sha3_##bits##_update(ctx, (const uint8_t *)input.data + offset, reductions);                                   \
        *offsetp = offset + reductions;                                                                                            \
        return 0;                                                                                                                  \
    }                                                                                                                              \
                                                                                                                                   \
    static ERL_NIF_TERM libdecaf_nif_sha3_##bits##_update_2_done(ErlNifEnv *env, xnif_slice_t *slice)                              \
    {                                                                                                                              \
        struct decaf_sha3_##bits##_ctx_s *ctx = NULL;                                                                              \
                                                                                                                                   \
        if (slice->argc != 2 || !libdecaf_nif_get_sha3_##bits##_ctx(env, slice->argv[0], &ctx)) {                                  \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
                                                                                                                                   \
        return slice->argv[0];                                                                                                     \
    }                                                                                                                              \
                                                                                                                                   \
    static ERL_NIF_TERM libdecaf_nif_sha3_##bits##_final_2(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])                    \
    {                                                                                                                              \
        struct decaf_sha3_##bits##_ctx_s *old_ctx = NULL;                                                                          \
        struct decaf_sha3_##bits##_ctx_s new_ctxbuf;                                                                               \
        struct decaf_sha3_##bits##_ctx_s *new_ctx = &new_ctxbuf;                                                                   \
        unsigned long outlen;                                                                                                      \
        unsigned char *outbuf = NULL;                                                                                              \
        ERL_NIF_TERM out_term;                                                                                                     \
                                                                                                                                   \
        if (argc != 2 || !libdecaf_nif_get_sha3_##bits##_ctx(env, argv[0], &old_ctx) || !enif_get_ulong(env, argv[1], &outlen) ||  \
            outlen > bytes) {                                                                                                      \
            return enif_make_badarg(env);                                                                                          \
        }                                                                                                                          \
                                                                                                                                   \
        (void)memcpy(new_ctx, old_ctx, sizeof(struct decaf_sha3_##bits##_ctx_s));                                                  \
        outbuf = enif_make_new_binary(env, outlen, &out_term);                                                                     \
        (void)decaf_sha3_##bits##_final(new_ctx, outbuf, outlen);                                                                  \
        (void)decaf_sha3_##bits##_destroy(new_ctx);                                                                                \
                                                                                                                                   \
        return out_term;                                                                                                           \
    }

SHA3_DEFINITION(224, 28)
SHA3_DEFINITION(256, 32)
SHA3_DEFINITION(384, 48)
SHA3_DEFINITION(512, 64)

#undef SHA3_DEFINITION
