/* pmpz_arith -- mpz arithmetic functions
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

#include "funcapi.h"


/*
 * Unary minus, plus
 */

PGMP_PG_FUNCTION(pmpz_uplus)
{
    const pmpz      *pz1;
    pmpz            *res;

    pz1 = PG_GETARG_PMPZ(0);

	res = (pmpz *)palloc(VARSIZE(pz1));
	memcpy(res, pz1, VARSIZE(pz1));

    PG_RETURN_POINTER(res);
}


/* Template to generate unary functions */

#define PMPZ_UN(op) \
 \
PGMP_PG_FUNCTION(pmpz_ ## op) \
{ \
    const mpz_t     z1; \
    mpz_t           zf; \
 \
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0)); \
 \
    mpz_init_set(zf, z1); \
    mpz_ ## op (zf, zf); \
 \
    PG_RETURN_MPZ(zf); \
}

PMPZ_UN(neg)
PMPZ_UN(abs)

/*
 * Binary operators
 */

/* Template to generate regular binary operators */

#define PMPZ_OP(op) \
 \
PGMP_PG_FUNCTION(pmpz_ ## op) \
{ \
    const mpz_t     z1; \
    const mpz_t     z2; \
    mpz_t           zf; \
 \
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0)); \
    mpz_from_pmpz(z2, PG_GETARG_PMPZ(1)); \
 \
    mpz_init(zf); \
    mpz_ ## op (zf, z1, z2); \
 \
    PG_RETURN_MPZ(zf); \
}

PMPZ_OP(add)
PMPZ_OP(sub)
PMPZ_OP(mul)


/* Template to generate binary operators that may divide by zero */

#define PMPZ_OP_DIV(op) \
 \
PGMP_PG_FUNCTION(pmpz_ ## op) \
{ \
    const mpz_t     z1; \
    const mpz_t     z2; \
    mpz_t           zf; \
 \
    mpz_from_pmpz(z2, PG_GETARG_PMPZ(1)); \
    if (UNLIKELY(MPZ_IS_ZERO(z2))) \
    { \
        ereport(ERROR, ( \
            errcode(ERRCODE_DIVISION_BY_ZERO), \
            errmsg("division by zero"))); \
    } \
 \
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0)); \
 \
    mpz_init(zf); \
    mpz_ ## op (zf, z1, z2); \
 \
    PG_RETURN_MPZ(zf); \
}

PMPZ_OP_DIV(tdiv_q)
PMPZ_OP_DIV(tdiv_r)
PMPZ_OP_DIV(cdiv_q)
PMPZ_OP_DIV(cdiv_r)
PMPZ_OP_DIV(fdiv_q)
PMPZ_OP_DIV(fdiv_r)
PMPZ_OP_DIV(divexact)


/* Functions defined on unsigned long */

/* TODO: this function could take a INT64 argument */

#define PMPZ_OP_UL(op) \
 \
PGMP_PG_FUNCTION(pmpz_ ## op) \
{ \
    const mpz_t     z; \
    long            b; \
    mpz_t           zf; \
 \
    mpz_from_pmpz(z, PG_GETARG_PMPZ(0)); \
    b = PG_GETARG_INT32(1); \
 \
    if (UNLIKELY(b < 0)) { \
        ereport(ERROR, ( \
            errcode(ERRCODE_INVALID_PARAMETER_VALUE), \
            errmsg("op2 can't be negative") )); \
    } \
 \
    mpz_init(zf); \
    mpz_ ## op (zf, z, (unsigned long)b); \
 \
    PG_RETURN_MPZ(zf); \
}

PMPZ_OP_UL(pow_ui)


/* Functions defined on bit count
 *
 * mp_bitcnt_t is defined as unsigned long.
 */

#define PMPZ_OP_BITCNT PMPZ_OP_UL

PMPZ_OP_BITCNT(mul_2exp)
PMPZ_OP_BITCNT(tdiv_q_2exp)
PMPZ_OP_BITCNT(tdiv_r_2exp)
PMPZ_OP_BITCNT(cdiv_q_2exp)
PMPZ_OP_BITCNT(cdiv_r_2exp)
PMPZ_OP_BITCNT(fdiv_q_2exp)
PMPZ_OP_BITCNT(fdiv_r_2exp)


/*
 * Comparison operators
 */

PGMP_PG_FUNCTION(pmpz_cmp)
{
    const mpz_t     z1;
    const mpz_t     z2;

    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));
    mpz_from_pmpz(z2, PG_GETARG_PMPZ(1));

    PG_RETURN_INT32(mpz_cmp(z1, z2));
}


#define PMPZ_CMP(op, rel) \
 \
PGMP_PG_FUNCTION(pmpz_ ## op) \
{ \
    const mpz_t     z1; \
    const mpz_t     z2; \
 \
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0)); \
    mpz_from_pmpz(z2, PG_GETARG_PMPZ(1)); \
 \
    PG_RETURN_BOOL(mpz_cmp(z1, z2) rel 0); \
}

PMPZ_CMP(eq, ==)
PMPZ_CMP(ne, !=)
PMPZ_CMP(gt, >)
PMPZ_CMP(ge, >=)
PMPZ_CMP(lt, <)
PMPZ_CMP(le, <=)

/*
 * Mathematics functions
 */
PGMP_PG_FUNCTION(pmpz_sqrt)
{
    const mpz_t     z1;
    mpz_t           zf;
    
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));

    mpz_init_set(zf, z1);
    mpz_sqrt (zf, zf);

    PG_RETURN_MPZ(zf);
}

PGMP_PG_FUNCTION(pmpz_root)
{
    const mpz_t     z1;
    mpz_t           zf;
    unsigned long   root;
    
#if LONG_MAX == INT64_MAX
    root = PG_GETARG_UINT64(1);
#else
    root = PG_GETARG_UINT32(1);
#endif
        
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));

    mpz_init_set(zf, z1);
    mpz_root (zf, zf, root);

    PG_RETURN_MPZ(zf);
}

PGMP_PG_FUNCTION(pmpz_perfect_power)
{
    const mpz_t     z1;
    
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));

    PG_RETURN_BOOL(mpz_perfect_power_p (z1));
}

PGMP_PG_FUNCTION(pmpz_perfect_square)
{
    const mpz_t     z1;
    
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));

    PG_RETURN_BOOL(mpz_perfect_square_p (z1));
}

PGMP_PG_FUNCTION(pmpz_rootrem)
{
    const mpz_t     z1;
    mpz_t           zroot;
    mpz_t           zrem;
    unsigned long   n;
    TupleDesc       tupdesc;
    AttInMetadata  *attinmeta;
    char          **values;
    HeapTuple       tuple;
    Datum           result;

#if LONG_MAX == INT64_MAX
    n = PG_GETARG_UINT64(1);
#else
    n = PG_GETARG_UINT32(1);
#endif
    
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));

    mpz_init_set(zroot, z1);
    mpz_init_set(zrem, z1);
    mpz_rootrem (zroot, zrem, z1, n);
    
    /* Build a tuple descriptor for our result type */
    if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
        ereport(ERROR,
            (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
             errmsg("function returning composite called in context that cannot accept type composite")));

    /*
     * generate attribute metadata needed later to produce tuples
     */
    attinmeta = TupleDescGetAttInMetadata(tupdesc);

    /*
     * Prepare a values array for building the returned tuple.
     * This should be an array of C strings which will
     * be processed later by the type input functions.
     */
    values = (char **) palloc(2 * sizeof(char *));
    values[0] = (char *) palloc(mpz_sizeinbase(zroot, 10) + 2);
    values[1] = (char *) palloc(mpz_sizeinbase(zrem, 10) + 2);

    mpz_get_str(values[0], 10, zroot);
    mpz_get_str(values[1], 10, zrem);

    /* build a tuple */
    tuple = BuildTupleFromCStrings(attinmeta, values);

    /* make the tuple into a datum */
    result = HeapTupleGetDatum(tuple);

    /* clean up (this is not really necessary) */
    pfree(values[0]);
    pfree(values[1]);
    pfree(values);

    PG_RETURN_DATUM(result);
}

PMPZ_UN(nextprime)
