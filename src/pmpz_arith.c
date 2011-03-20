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

#define PMPZ_UN(op, CHECK) \
 \
PGMP_PG_FUNCTION(pmpz_ ## op) \
{ \
    const mpz_t     z1; \
    mpz_t           zf; \
 \
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0)); \
    CHECK(z1); \
 \
    mpz_init(zf); \
    mpz_ ## op (zf, z1); \
 \
    PG_RETURN_MPZ(zf); \
}

PMPZ_UN(neg,    PMPZ_NO_CHECK)
PMPZ_UN(abs,    PMPZ_NO_CHECK)
PMPZ_UN(sqrt,   PMPZ_CHECK_NONEG)


/*
 * Binary operators
 */

/* Operators defined (mpz, mpz) -> mpz.
 *
 * CHECK2 is a check performed on the 2nd argument. Available checks are
 * defined below.
 */

#define PMPZ_OP(op, CHECK2) \
 \
PGMP_PG_FUNCTION(pmpz_ ## op) \
{ \
    const mpz_t     z1; \
    const mpz_t     z2; \
    mpz_t           zf; \
 \
    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0)); \
    mpz_from_pmpz(z2, PG_GETARG_PMPZ(1)); \
    CHECK2(z2); \
 \
    mpz_init(zf); \
    mpz_ ## op (zf, z1, z2); \
 \
    PG_RETURN_MPZ(zf); \
}


/* Operators definitions */

PMPZ_OP(add,        PMPZ_NO_CHECK)
PMPZ_OP(sub,        PMPZ_NO_CHECK)
PMPZ_OP(mul,        PMPZ_NO_CHECK)
PMPZ_OP(tdiv_q,     PMPZ_CHECK_DIV0)
PMPZ_OP(tdiv_r,     PMPZ_CHECK_DIV0)
PMPZ_OP(cdiv_q,     PMPZ_CHECK_DIV0)
PMPZ_OP(cdiv_r,     PMPZ_CHECK_DIV0)
PMPZ_OP(fdiv_q,     PMPZ_CHECK_DIV0)
PMPZ_OP(fdiv_r,     PMPZ_CHECK_DIV0)
PMPZ_OP(divexact,   PMPZ_CHECK_DIV0)


/* Functions defined on unsigned long */

/* TODO: this function could take a INT64 argument */

#define PMPZ_OP_UL(op, CHECK1, CHECK2) \
 \
PGMP_PG_FUNCTION(pmpz_ ## op) \
{ \
    const mpz_t     z; \
    unsigned long   b; \
    mpz_t           zf; \
 \
    mpz_from_pmpz(z, PG_GETARG_PMPZ(0)); \
    CHECK1(z); \
    \
    PGMP_GETARG_ULONG(b, 1); \
    CHECK2(b); \
 \
    mpz_init(zf); \
    mpz_ ## op (zf, z, b); \
 \
    PG_RETURN_MPZ(zf); \
}

PMPZ_OP_UL(pow_ui,  PMPZ_NO_CHECK,      PMPZ_NO_CHECK)
PMPZ_OP_UL(root,    PMPZ_CHECK_NONEG,   PMPZ_CHECK_LONG_POS)


/* Functions defined on bit count
 *
 * mp_bitcnt_t is defined as unsigned long.
 */

#define PMPZ_OP_BITCNT PMPZ_OP_UL

PMPZ_OP_BITCNT(mul_2exp,        PMPZ_NO_CHECK,  PMPZ_NO_CHECK)
PMPZ_OP_BITCNT(tdiv_q_2exp,     PMPZ_NO_CHECK,  PMPZ_NO_CHECK)
PMPZ_OP_BITCNT(tdiv_r_2exp,     PMPZ_NO_CHECK,  PMPZ_NO_CHECK)
PMPZ_OP_BITCNT(cdiv_q_2exp,     PMPZ_NO_CHECK,  PMPZ_NO_CHECK)
PMPZ_OP_BITCNT(cdiv_r_2exp,     PMPZ_NO_CHECK,  PMPZ_NO_CHECK)
PMPZ_OP_BITCNT(fdiv_q_2exp,     PMPZ_NO_CHECK,  PMPZ_NO_CHECK)
PMPZ_OP_BITCNT(fdiv_r_2exp,     PMPZ_NO_CHECK,  PMPZ_NO_CHECK)


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
 * Unary predicates
 */

PGMP_PG_FUNCTION(pmpz_perfect_power)
{
    const mpz_t     z1;

    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));

    PG_RETURN_BOOL(mpz_perfect_power_p(z1));
}

PGMP_PG_FUNCTION(pmpz_perfect_square)
{
    const mpz_t     z1;

    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));

    PG_RETURN_BOOL(mpz_perfect_square_p(z1));
}

PGMP_PG_FUNCTION(pmpz_rootrem)
{
    const mpz_t     z1;
    mpz_t           zroot;
    mpz_t           zrem;
    unsigned long   n;

    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));
    PMPZ_CHECK_NONEG(z1);

    PGMP_GETARG_ULONG(n, 1);
    PMPZ_CHECK_LONG_POS(n);

    mpz_init(zroot);
    mpz_init(zrem);
    mpz_rootrem (zroot, zrem, z1, n);

    PG_RETURN_MPZ_MPZ(zroot, zrem);
}

PGMP_PG_FUNCTION(pmpz_sqrtrem)
{
    const mpz_t     z1;
    mpz_t           zroot;
    mpz_t           zrem;

    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));

    mpz_init(zroot);
    mpz_init(zrem);
    mpz_sqrtrem(zroot, zrem, z1);

    PG_RETURN_MPZ_MPZ(zroot, zrem);
}

/*
 * MPZ Number Theoretic Functions
 */

PGMP_PG_FUNCTION(pmpz_probab_prime_p)
{
    const mpz_t     z1;
    int             reps;

    mpz_from_pmpz(z1, PG_GETARG_PMPZ(0));
    reps = PG_GETARG_INT32(1);

    PG_RETURN_INT32(mpz_probab_prime_p (z1, reps));
}

PMPZ_UN(nextprime,  PMPZ_NO_CHECK)

PGMP_PG_FUNCTION(pmpz_gcd)
{
    const mpz_t     z1;
    const mpz_t     z2;
    mpz_t           zf;

    mpz_from_pmpz (z1, PG_GETARG_PMPZ(0));
    mpz_from_pmpz (z2, PG_GETARG_PMPZ(1));
    mpz_init (zf);

    mpz_gcd (zf, z1, z2);
    
    PG_RETURN_MPZ(zf);
}

PGMP_PG_FUNCTION(pmpz_gcdext)
{
    const mpz_t     z1;
    const mpz_t     z2;
    mpz_t           zf;
    mpz_t           zs;
    mpz_t           zt;

    mpz_from_pmpz (z1, PG_GETARG_PMPZ(0));
    mpz_from_pmpz (z2, PG_GETARG_PMPZ(1));
    mpz_init (zf);
    mpz_init (zs);
    mpz_init (zt);

    mpz_gcdext (zf, zs, zt, z1, z2);
    
    PG_RETURN_MPZ_MPZ_MPZ (zf, zs, zt);
}

PGMP_PG_FUNCTION(pmpz_lcm)
{
    const mpz_t     z1;
    const mpz_t     z2;
    mpz_t           zf;

    mpz_from_pmpz (z1, PG_GETARG_PMPZ(0));
    mpz_from_pmpz (z2, PG_GETARG_PMPZ(1));
    mpz_init (zf);

    mpz_lcm (zf, z1, z2);
    
    PG_RETURN_MPZ(zf);
}

PGMP_PG_FUNCTION(pmpz_invert)
{
    const mpz_t     z1;
    const mpz_t     z2;
    mpz_t           zf;
    int             ret;

    mpz_from_pmpz (z1, PG_GETARG_PMPZ(0));
    mpz_from_pmpz (z2, PG_GETARG_PMPZ(1));
    mpz_init (zf);

    ret = mpz_invert (zf, z1, z2);

    if (ret != 0)
        PG_RETURN_MPZ(zf);
    else
        PG_RETURN_NULL();
}

PGMP_PG_FUNCTION(pmpz_jacobi)
{
    const mpz_t     z1;
    const mpz_t     z2;
    int             ret;

    mpz_from_pmpz (z1, PG_GETARG_PMPZ(0));
    mpz_from_pmpz (z2, PG_GETARG_PMPZ(1));

    // TODO: check for z2 odd otherwise the function is not defined.
    ret = mpz_jacobi (z1, z2);

    PG_RETURN_INT32(ret);
}

PGMP_PG_FUNCTION(pmpz_legendre)
{
    const mpz_t     z1;
    const mpz_t     z2;
    int             ret;

    mpz_from_pmpz (z1, PG_GETARG_PMPZ(0));
    mpz_from_pmpz (z2, PG_GETARG_PMPZ(1));

    // TODO: check for z2 odd positive prime otherwise the function is not defined.
    ret = mpz_legendre (z1, z2);

    PG_RETURN_INT32(ret);
}

PGMP_PG_FUNCTION(pmpz_kronecker)
{
    const mpz_t     z1;
    const mpz_t     z2;
    int             ret;

    mpz_from_pmpz (z1, PG_GETARG_PMPZ(0));
    mpz_from_pmpz (z2, PG_GETARG_PMPZ(1));

    // TODO: check for z2 odd positive prime otherwise the function is not defined.
    ret = mpz_kronecker (z1, z2);

    PG_RETURN_INT32(ret);
}

PGMP_PG_FUNCTION(pmpz_remove)
{
    const mpz_t      z1;
    const mpz_t      z2;
    mpz_t            zf;
    unsigned long int ret;

    mpz_from_pmpz (z1, PG_GETARG_PMPZ(0));
    mpz_from_pmpz (z2, PG_GETARG_PMPZ(1));

    mpz_init (zf);
    
    // TODO: actually the ret value is not returned. Decide if create a new function that retun two values
    ret = mpz_remove (zf, z1, z2);

    PG_RETURN_MPZ(zf);
}

/*
— Function: unsigned long int mpz_gcd_ui (mpz_t rop, mpz_t op1, unsigned long int op2)
— Function: void mpz_lcm_ui (mpz_t rop, mpz_t op1, unsigned long op2)
— Function: int mpz_kronecker_si (mpz_t a, long b)
— Function: int mpz_kronecker_ui (mpz_t a, unsigned long b)
— Function: int mpz_si_kronecker (long a, mpz_t b)
— Function: int mpz_ui_kronecker (unsigned long a, mpz_t b)
— Function: void mpz_fac_ui (mpz_t rop, unsigned long int op)
— Function: void mpz_bin_ui (mpz_t rop, mpz_t n, unsigned long int k)
— Function: void mpz_bin_uiui (mpz_t rop, unsigned long int n, unsigned long int k)
— Function: void mpz_fib_ui (mpz_t fn, unsigned long int n)
— Function: void mpz_fib2_ui (mpz_t fn, mpz_t fnsub1, unsigned long int n)
— Function: void mpz_lucnum_ui (mpz_t ln, unsigned long int n)
— Function: void mpz_lucnum2_ui (mpz_t ln, mpz_t lnsub1, unsigned long int n)
*/
