/* pmpq_arith -- mpq arithmetic functions
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

#include "pmpq.h"
#include "pgmp-impl.h"

#include "fmgr.h"

/*
 * Unary minus, plus
 */

PGMP_PG_FUNCTION(pmpq_uplus)
{
    const mpq_t     q1;
    mpq_t           qf;

    PGMP_GETARG_MPQ(q1, 0);

    mpq_init(qf);
    mpq_set(qf, q1);

    PGMP_RETURN_MPQ(qf);
}

#define PMPQ_UN(op) \
 \
PGMP_PG_FUNCTION(pmpq_ ## op) \
{ \
    const mpq_t     q1; \
    mpq_t           qf; \
 \
    PGMP_GETARG_MPQ(q1, 0); \
 \
    mpq_init(qf); \
    mpq_ ## op (qf, q1); \
 \
    PGMP_RETURN_MPQ(qf); \
}

PMPQ_UN(neg)


/*
 * Unary abs and inv functions
 */

PMPQ_UN(abs)
PMPQ_UN(inv)


/*
 * Binary operators
 */

/* Template to generate regular binary operators */

#define PMPQ_OP(op) \
 \
PGMP_PG_FUNCTION(pmpq_ ## op) \
{ \
    const mpq_t     q1; \
    const mpq_t     q2; \
    mpq_t           qf; \
 \
    PGMP_GETARG_MPQ(q1, 0); \
    PGMP_GETARG_MPQ(q2, 1); \
 \
    mpq_init(qf); \
    mpq_ ## op (qf, q1, q2); \
 \
    PGMP_RETURN_MPQ(qf); \
}

PMPQ_OP(add)
PMPQ_OP(sub)
PMPQ_OP(mul)


/* Template to generate binary operators that may divide by zero */

#define PMPQ_OP_DIV(op) \
 \
PGMP_PG_FUNCTION(pmpq_ ## op) \
{ \
    const mpq_t     q1; \
    const mpq_t     q2; \
    mpq_t           qf; \
 \
    PGMP_GETARG_MPQ(q2, 1); \
    if (UNLIKELY(MPZ_IS_ZERO(mpq_numref(q2)))) \
    { \
        ereport(ERROR, ( \
            errcode(ERRCODE_DIVISION_BY_ZERO), \
            errmsg("division by zero"))); \
    } \
 \
    PGMP_GETARG_MPQ(q1, 0); \
 \
    mpq_init(qf); \
    mpq_ ## op (qf, q1, q2); \
 \
    PGMP_RETURN_MPQ(qf); \
}

PMPQ_OP_DIV(div)


/* Functions defined on bit count */

#define PMPQ_BIT(op) \
 \
PGMP_PG_FUNCTION(pmpq_ ## op) \
{ \
    const mpq_t     q; \
    unsigned long   b; \
    mpq_t           qf; \
 \
    PGMP_GETARG_MPQ(q, 0); \
    PGMP_GETARG_ULONG(b, 1); \
 \
    mpq_init(qf); \
    mpq_ ## op (qf, q, b); \
 \
    PGMP_RETURN_MPQ(qf); \
}


PMPQ_BIT(mul_2exp)
PMPQ_BIT(div_2exp)


/*
 * Comparison operators
 */

PGMP_PG_FUNCTION(pmpq_cmp)
{
    const mpq_t     q1;
    const mpq_t     q2;

    PGMP_GETARG_MPQ(q1, 0);
    PGMP_GETARG_MPQ(q2, 1);

    PG_RETURN_INT32(mpq_cmp(q1, q2));
}


#define PMPQ_CMP_EQ(op, rel) \
 \
PGMP_PG_FUNCTION(pmpq_ ## op) \
{ \
    const mpq_t     q1; \
    const mpq_t     q2; \
 \
    PGMP_GETARG_MPQ(q1, 0); \
    PGMP_GETARG_MPQ(q2, 1); \
 \
    PG_RETURN_BOOL(mpq_equal(q1, q2) rel 0); \
}

PMPQ_CMP_EQ(eq, !=)     /* note that the operators are reversed */
PMPQ_CMP_EQ(ne, ==)


#define PMPQ_CMP(op, rel) \
 \
PGMP_PG_FUNCTION(pmpq_ ## op) \
{ \
    const mpq_t     q1; \
    const mpq_t     q2; \
 \
    PGMP_GETARG_MPQ(q1, 0); \
    PGMP_GETARG_MPQ(q2, 1); \
 \
    PG_RETURN_BOOL(mpq_cmp(q1, q2) rel 0); \
}

PMPQ_CMP(gt, >)
PMPQ_CMP(ge, >=)
PMPQ_CMP(lt, <)
PMPQ_CMP(le, <=)


