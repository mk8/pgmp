--
--  Test mpq datatype
--

-- Compact output
\t
\a


--
-- mpq input and output functions
--

SELECT '0'::mpq;
SELECT '1'::mpq;
SELECT '-1'::mpq;
SELECT '10'::mpq;
SELECT '-10'::mpq;

SELECT '4294967295'::mpq;   -- limbs boundaries
SELECT '4294967296'::mpq;
SELECT '-4294967296'::mpq;
SELECT '-4294967297'::mpq;
SELECT '18446744073709551614'::mpq;
SELECT '18446744073709551615'::mpq;
SELECT '18446744073709551616'::mpq;
SELECT '18446744073709551617'::mpq;
SELECT '-18446744073709551615'::mpq;
SELECT '-18446744073709551616'::mpq;
SELECT '-18446744073709551617'::mpq;
SELECT '-18446744073709551618'::mpq;
SELECT '12345678901234567890123456789012345678901234567890123456789012345678901234567890'::mpq;

SELECT '1/1'::mpq;
SELECT '2/3'::mpq;
SELECT '640/30'::mpq;
SELECT '-640/30'::mpq;

SELECT '18446744073709551616/18446744073709551616'::mpq;
SELECT '12345678901234567890123456789012345678901234567890123456789012345678901234567890/'
       '88888888888888888888888888888888888888888888888888888888888888888888888888888888'::mpq;

SELECT '1/0'::mpq;

--
-- mpq cast
--

SELECT 0::smallint::mpq, (-32768)::smallint::mpq, 32767::smallint::mpq;
SELECT 0::integer::mpq, (-2147483648)::integer::mpq, 2147483647::integer::mpq;
SELECT 0::bigint::mpq, (-9223372036854775808)::bigint::mpq, 9223372036854775807::bigint::mpq;
SELECT 0::numeric::mpq, (-12345678901234567890)::numeric::mpq, 12345678901234567890::numeric::mpq;
SELECT 0::mpz::mpq, (-12345678901234567890)::mpz::mpq, 12345678901234567890::mpz::mpq;
-- TODO - together with conversion from mpf
-- SELECT 0.0::float4::mpq, (-12345.25)::float4::mpq, 12345.25::float4::mpq;
-- SELECT 0.0::float8::mpq, (-123456789012.25)::float8::mpq, 123456789012.25::float8::mpq;
SELECT 0.0::numeric::mpq, (-1234567890.12345)::numeric::mpq, 1234567890.12345::numeric::mpq;

SELECT 0::mpq, 1::mpq, (-1)::mpq;       -- automatic casts
SELECT 1000000::mpq, (-1000000)::mpq;
SELECT 1000000000::mpq, (-1000000000)::mpq;
SELECT 1000000000000000::mpq, (-1000000000000000)::mpq;
SELECT 1000000000000000000000000000000::mpq, (-1000000000000000000000000000000)::mpq;
SELECT 0.0::mpq, (-1234567890.12345)::mpq, 1234567890.12345::mpq;
SELECT 'NaN'::decimal::mpq;

SELECT -1::mpq;       -- these take the unary minus to work
SELECT -1000000::mpq;
SELECT -1000000000::mpq;
SELECT -1000000000000000::mpq;
SELECT -1000000000000000000000000000000::mpq;

SELECT mpq(10, 4), mpq(10, -4);
SELECT mpq(10, 0);
SELECT mpq(47563485764385764395874365986384, 874539847539845639485769837553465);
SELECT mpq(1230::numeric, 123::numeric);
SELECT mpq(123.45::numeric, 1::numeric);
SELECT mpq(1::numeric, 123.45::numeric);
SELECT mpq(123::numeric, 0::numeric);
SELECT mpq(47563485764385764395874365986384::mpz, 874539847539845639485769837553465::mpz);
SELECT mpq('10'::mpz, '0'::mpz);
SELECT num('4/5'::mpq);
SELECT den('4/5'::mpq);


--
-- mpq arithmetic
--

SELECT -('0'::mpq), +('0'::mpq), -('1'::mpq), +('1'::mpq), -('-1'::mpq), +('-1'::mpq);
SELECT -('1234567890123456/7890'::mpq), +('1234567890123456/7890'::mpq);
SELECT '4/5'::mpq + '6/8'::mpq;
SELECT '4/5'::mpq - '6/8'::mpq;
SELECT '4/5'::mpq * '6/8'::mpq;
SELECT '4/5'::mpq / '6/8'::mpq;
SELECT '4/5'::mpq / '0'::mpq;

SELECT '4/5'::mpq << 4;
SELECT '4/5'::mpq << -1;
SELECT '4/5'::mpq >> 4;
SELECT '4/5'::mpq >> -1;

