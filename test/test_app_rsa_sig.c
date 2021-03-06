/** @file */
/*****************************************************************************
* Copyright (c) 2019, Cisco Systems, Inc.
* All rights reserved.

* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/
//
// Created by edaw on 2019-01-07.
//

#ifdef ACVP_NO_RUNTIME

#include "ut_common.h"
#include "acvp_lcl.h"

ACVP_CTX *ctx;
ACVP_TEST_CASE *test_case;
ACVP_RSA_SIG_TC *rsa_sig_tc;
ACVP_RESULT rv;

void free_rsa_sig_tc(ACVP_RSA_SIG_TC *stc) {
    if (stc->msg) { free(stc->msg); }
    if (stc->e) { free(stc->e); }
    if (stc->n) { free(stc->n); }
    if (stc->signature) { free(stc->signature); }
    if (stc->salt) { free(stc->salt); }
    memzero_s(stc, sizeof(ACVP_RSA_SIG_TC));
}

int initialize_rsa_sig_tc(ACVP_CIPHER cipher,
                      ACVP_RSA_SIG_TC *stc,
                      int tgId,
                      ACVP_RSA_SIG_TYPE sig_type,
                      unsigned int mod,
                      ACVP_HASH_ALG hash_alg,
                      char *e,
                      char *n,
                      char *msg,
                      char *signature,
                      char *salt,
                      int salt_len,
                      int corrupt) {
    ACVP_RESULT rv;
    
    memzero_s(stc, sizeof(ACVP_RSA_SIG_TC));
    
    stc->salt = calloc(ACVP_RSA_SIGNATURE_MAX, sizeof(char));
    if (!stc->salt) { goto err; }
    
    if (msg) {
        stc->msg = calloc(ACVP_RSA_MSGLEN_MAX, sizeof(char));
        if (!stc->msg) { goto err; }
        rv = acvp_hexstr_to_bin(msg, stc->msg, ACVP_RSA_MSGLEN_MAX, &(stc->msg_len));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (msg)");
            goto err;
        }
    }
    
    if (cipher == ACVP_RSA_SIGVER) {
        stc->sig_mode = ACVP_RSA_SIGVER;
    
        if (e) {
            stc->e = calloc(ACVP_RSA_EXP_LEN_MAX, sizeof(char));
            if (!stc->e) { goto err; }
            rv = acvp_hexstr_to_bin(e, stc->e, ACVP_RSA_EXP_LEN_MAX, &(stc->e_len));
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("Hex conversion failure (e)");
                goto err;
            }
        }
        if (n) {
            stc->n = calloc(ACVP_RSA_EXP_LEN_MAX, sizeof(char));
            if (!stc->n) { goto err; }
            rv = acvp_hexstr_to_bin(n, stc->n, ACVP_RSA_EXP_LEN_MAX, &(stc->n_len));
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("Hex conversion failure (n)");
                goto err;
            }
        }
        if (signature) {
            stc->signature = calloc(ACVP_RSA_SIGNATURE_MAX, sizeof(char));
            if (!stc->signature) { goto err; }
            rv = acvp_hexstr_to_bin(signature, stc->signature, ACVP_RSA_SIGNATURE_MAX, &stc->sig_len);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("Hex conversion failure (signature)");
                goto err;
            }
        }
    } else {
        stc->sig_mode = ACVP_RSA_SIGGEN;
        if (!corrupt) {
            stc->e = calloc(ACVP_RSA_EXP_LEN_MAX, sizeof(char));
            if (!stc->e) { goto err; }
            stc->n = calloc(ACVP_RSA_EXP_LEN_MAX, sizeof(char));
            if (!stc->n) { goto err; }
            stc->signature = calloc(ACVP_RSA_SIGNATURE_MAX, sizeof(char));
            if (!stc->signature) { goto err; }
        }
    }
    
    if (salt_len) {
        if (salt) {
            memcpy_s(stc->salt, ACVP_RSA_SIGNATURE_MAX,
                     salt, strnlen_s((const char *)salt, 256));
        }
    }
    stc->salt_len = salt_len;
    
    stc->tg_id = tgId;
    stc->modulo = mod;
    stc->hash_alg = hash_alg;
    stc->sig_type = sig_type;
    
    return 1;
    
err:
    free_rsa_sig_tc(stc);
    return 0;
}

/*
 * invalid hash alg rsa sig handler
 */
Test(APP_RSA_SIG_HANDLER, invalid_hash_alg) {
    ACVP_CIPHER cipher = ACVP_RSA_SIGGEN;
    int tgid = 0;
    int mod = 2048;
    int hash_alg = 0;
    char *e = "aa";
    char *n = "aa";
    char *msg = "aa";
    char *signature = "aa";
    char *salt = "aa";
    int salt_len = 0;
    int corrupt = 0;
    int sig_type = ACVP_RSA_SIG_TYPE_X931;
    
    rsa_sig_tc = calloc(1, sizeof(ACVP_RSA_SIG_TC));
    
    if (!initialize_rsa_sig_tc(cipher, rsa_sig_tc, tgid, sig_type,
            mod, hash_alg, e, n, msg, signature, salt, salt_len,
            corrupt)) {
        cr_assert_fail("rsa sig init tc failure");
    }
    test_case = calloc(1, sizeof(ACVP_TEST_CASE));
    test_case->tc.rsa_sig = rsa_sig_tc;
    
    rv = app_rsa_sig_handler(test_case);
    cr_assert_neq(rv, 0);
    
    free_rsa_sig_tc(rsa_sig_tc);
}

#endif // ACVP_NO_RUNTIME

