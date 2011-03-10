/* pmpz_io -- mpz Input/Output functions
 *
 * Copyright (C) 2011 Daniele Varrazzo
 *
 * This file is part of the PostgreSQL GMP Module
 *
 * The PostgreSQL GMP Module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * The PostgreSQL GMP Module is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the PostgreSQL GMP Module.  If not, see
 * http://www.gnu.org/licenses/.
 */

#include "pmpz.h"
#include "pgmp-impl.h"

#include "fmgr.h"

#include <limits.h>


/*
 * Input/Output functions
 */

PGMP_PG_FUNCTION(pmpz_in)
{
    char    *str;
    mpz_t   z;

    str = PG_GETARG_CSTRING(0);

    /* TODO: make base variable */
    if (0 != mpz_init_set_str(z, str, 10))
    {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for mpz: \"%s\"",
                        str)));
    }

    PG_RETURN_MPZ(z);
}

PGMP_PG_FUNCTION(pmpz_out)
{
    const pmpz      *pz;
    const mpz_t     z;
    char            *res;

    pz = PG_GETARG_PMPZ(0);
    mpz_from_pmpz(z, pz);

    /* TODO: make base variable */
    res = mpz_get_str(NULL, 10, z);
    PG_RETURN_CSTRING(res);
}


/*
 * Cast functions
 */

static Datum _pmpz_from_long(long in);

PGMP_PG_FUNCTION(pmpz_from_int2)
{
    int16 in = PG_GETARG_INT16(0);
    return _pmpz_from_long(in);
}

PGMP_PG_FUNCTION(pmpz_from_int4)
{
    int32 in = PG_GETARG_INT32(0);
    return _pmpz_from_long(in);
}

PGMP_PG_FUNCTION(pmpz_from_int8)
{
    int64   in = PG_GETARG_INT64(0);

#if LONG_MAX == INT64_MAX

    return _pmpz_from_long(in);

#elif LONG_MAX == INT32_MAX

    int         neg = 0;
    uint32      lo;
    uint32      hi;
    mpz_t       z;

    if (LIKELY(in != INT64_MIN))
    {
        if (in < 0) {
            neg = 1;
            in = -in;
        }

        lo = in & 0xFFFFFFFFUL;
        hi = in >> 32;

        if (hi) {
            mpz_init_set_ui(z, hi);
            mpz_mul_2exp(z, z, 32);
            mpz_add_ui(z, z, lo);
        }
        else {
            mpz_init_set_ui(z, lo);
        }

        if (neg) {
            mpz_neg(z, z);
        }
    }
    else {
        /* this would overflow the long */
        mpz_init_set_si(z, 1L);
        mpz_mul_2exp(z, z, 63);
        mpz_neg(z, z);
    }

    PG_RETURN_MPZ(z);

#endif
}

static Datum
_pmpz_from_long(long in)
{
    mpz_t   z;

    mpz_init_set_si(z, in);

    PG_RETURN_MPZ(z);
}


PGMP_PG_FUNCTION(pmpz_to_int4)
{
    const pmpz      *pz;
    const mpz_t     q;
    int32           out;

    pz = PG_GETARG_PMPZ(0);
    mpz_from_pmpz(q, pz);

    if (!mpz_fits_sint_p(q)) {
        ereport(ERROR,
                (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                 errmsg("numeric value too big to be converted in integer data type")));
    }

    out = mpz_get_si(q);
    PG_RETURN_INT32(out);
}

PGMP_PG_FUNCTION(pmpz_to_int2)
{
    const pmpz      *pz;
    const mpz_t     q;
    int16           out;

    pz = PG_GETARG_PMPZ(0);
    mpz_from_pmpz(q, pz);

    if (!mpz_fits_sshort_p(q)) {
        ereport(ERROR,
                (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                 errmsg("numeric value too big to be converted in smallint data type")));
    }

    out = mpz_get_si(q);
    PG_RETURN_INT16(out);
}

PGMP_PG_FUNCTION(pmpz_to_int8)
{
    const pmpz      *pz;
    const mpz_t     z;
    int64           out;
    char            *buffer;
    mp_limb_t       msLimb;

    pz = PG_GETARG_PMPZ(0);
    mpz_from_pmpz(z, pz);

#if LONG_MAX == INT64_MAX

    if (!mpz_fits_slong_p(z)) {
        goto errorNotInt8Value;
    } else {
        out = mpz_get_si(z);
    }

#elif LONG_MAX == INT32_MAX

    buffer = mpz_get_str(NULL, 10, z);

    if (mpz_size(z) > 2) {
        goto errorNotInt8Value;
    }
    if (mpz_size(z) == 2) {
        msLimb = mpz_getlimbn(z,1);
        if (msLimb > 0x7fffffff) {
            goto errorNotInt8Value;
        }
    }

    sscanf(buffer,"%lld",&out);

#endif
    PG_RETURN_INT64(out);

errorNotInt8Value:
    ereport(ERROR,
            (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
             errmsg("numeric value too big to be converted in biginteger data type")));

}
