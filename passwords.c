/*-
 * Copyright (c) 2017 Joshua Jackson <jjackson@kallisteconsulting.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include "compat/compat.h"
#include "base64/base64.h"
#include "crypto/pkcs5_pbkdf2.h"
#include "scram.h"


int gen_scram_salted_password(char *password, char *salt_b64, int rounds, unsigned char **result) {
    size_t salt_len;
    char *salt = (char *)base64_decode((unsigned char *)salt_b64, strlen(salt_b64), &salt_len);
    /* SaltedPassword  := Hi(Normalize(password), salt, i) */
    *result = (unsigned char *)malloc(20);
    pkcs5_pbkdf2(password, strlen(password), salt, salt_len, *result, 20, rounds);
    freezero(salt, salt_len);
    return SCRAM_OK;
}
