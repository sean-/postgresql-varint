/*
 * pg_varint.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"

#include "fmgr.h"
#include "libpq/pqformat.h" /* needed for send/recv functions */
#include "utils/builtins.h" /* needed for pg_lltoa() */
#include "utils/int8.h"

#include "varint.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/*
 * Since we use V1 function calling convention, all these functions have the
 * same signature as far as C is concerned.  We provide these prototypes just
 * to forestall warnings when compiled with clang -Wmissing-prototypes.
 */
extern Datum varint64_in(PG_FUNCTION_ARGS);
extern Datum varint64_out(PG_FUNCTION_ARGS);
extern Datum varint64_recv(PG_FUNCTION_ARGS);
extern Datum varint64_send(PG_FUNCTION_ARGS);

extern Datum varint64_lt(PG_FUNCTION_ARGS);
extern Datum varint64_le(PG_FUNCTION_ARGS);
extern Datum varint64_eq(PG_FUNCTION_ARGS);
extern Datum varint64_ne(PG_FUNCTION_ARGS);
extern Datum varint64_ge(PG_FUNCTION_ARGS);
extern Datum varint64_gt(PG_FUNCTION_ARGS);
extern Datum varint64_cmp(PG_FUNCTION_ARGS);

extern Datum int2_to_varint64(PG_FUNCTION_ARGS);
extern Datum int4_to_varint64(PG_FUNCTION_ARGS);
extern Datum int8_to_varint64(PG_FUNCTION_ARGS);
extern Datum varint64_to_int2(PG_FUNCTION_ARGS);
extern Datum varint64_to_int4(PG_FUNCTION_ARGS);
extern Datum varint64_to_int8(PG_FUNCTION_ARGS);

extern Datum varint64_sizeof(PG_FUNCTION_ARGS);
extern Datum varint64_sizeof2(PG_FUNCTION_ARGS);

/* Notes:
   struct varattrib_1b == good;
 */

/*****************************************************************************
 * Helper functions
 *****************************************************************************/

static int64_t
varlena_varint64_to_int64(const struct varlena* va) {
  size_t consumed = 0;
  int64_t num;

  varint_to_int64(VARDATA_ANY(va), VARSIZE_ANY(va), &num, &consumed);
  if (consumed == 0) {
    ereport(ERROR, (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                    errmsg("value is out of range for type VARINT64")));
  }
  return num;
}


static int64_t
int64_to_varlena_varint64(const int64_t in) {
  const size_t bufsz = VARINT64_MAX_BYTES;
  char buf[VARINT64_MAX_BYTES];
  size_t buflen = 0;
  char *out;

  int64_to_varint(in, bufsz, buf, &buflen);
  if (buflen == 0) {
    ereport(ERROR, (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                    errmsg("value is out of range for type VARINT64")));
  }
  out = (char*)palloc(buflen + VARHDRSZ);
  SET_VARSIZE(out, buflen + VARHDRSZ);
  memcpy(VARDATA_ANY(out), buf, buflen);
  PG_RETURN_POINTER(out);
}


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(varint64_in);
Datum
varint64_in(PG_FUNCTION_ARGS) {
  char *str = PG_GETARG_CSTRING(0);
  int64 num;

  (void)scanint8(str, false, &num);
  return int64_to_varlena_varint64(num);
}


PG_FUNCTION_INFO_V1(varint64_out);
Datum
varint64_out(PG_FUNCTION_ARGS) {
  struct varlena *varint64 = PG_GETARG_VARLENA_PP(0);
  size_t consumed = 0;
  int64_t num;

  varint_to_int64(VARDATA_ANY(varint64), VARSIZE_ANY(varint64), &num, &consumed);
  if (consumed == 0) {
    ereport(ERROR, (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                    errmsg("value is out of range for type VARINT64")));
  }

  return DirectFunctionCall1(int8out, num);
}


PG_FUNCTION_INFO_V1(varint64_recv);
Datum
varint64_recv(PG_FUNCTION_ARGS) {
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);

  return int64_to_varlena_varint64(pq_getmsgint64(buf));
}


PG_FUNCTION_INFO_V1(varint64_send);
Datum
varint64_send(PG_FUNCTION_ARGS) {
  struct varlena *varint64 = PG_GETARG_VARLENA_PP(0);
  StringInfoData buf;
  int64_t num;
  size_t consumed = 0;

  varint_to_int64(VARDATA_ANY(varint64), VARSIZE_ANY(varint64), &num, &consumed);
  if (consumed == 0) {
    ereport(ERROR, (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                    errmsg("value is out of range for type VARINT64")));
  }

  pq_begintypsend(&buf);
  pq_sendint64(&buf, (int64)num);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}


/*****************************************************************************
 * Operator class for defining B-tree index
 *
 * It's essential that the comparison operators and support function for a
 * B-tree index opclass always agree on the relative ordering of any two data
 * values.  Experience has shown that it's depressingly easy to write
 * unintentionally inconsistent functions.  One way to reduce the odds of
 * making a mistake is to make all the functions simple wrappers around an
 * internal three-way-comparison function, as we do here.
 *****************************************************************************/

static int
varint64_cmp_internal(const int64_t a, const int64_t b) {
  if (a < b)
    return -1;
  if (a > b)
    return 1;
  return 0;
}


PG_FUNCTION_INFO_V1(varint64_lt);
Datum
varint64_lt(PG_FUNCTION_ARGS) {
  struct varlena *lhs_var = PG_GETARG_VARLENA_PP(0);
  struct varlena *rhs_var = PG_GETARG_VARLENA_PP(1);

  int64_t lhs = varlena_varint64_to_int64(lhs_var);
  int64_t rhs = varlena_varint64_to_int64(rhs_var);

  PG_RETURN_BOOL(varint64_cmp_internal(lhs, rhs) < 0);
}


PG_FUNCTION_INFO_V1(varint64_le);
Datum
varint64_le(PG_FUNCTION_ARGS) {
  struct varlena *lhs_var = PG_GETARG_VARLENA_PP(0);
  struct varlena *rhs_var = PG_GETARG_VARLENA_PP(1);

  int64_t lhs = varlena_varint64_to_int64(lhs_var);
  int64_t rhs = varlena_varint64_to_int64(rhs_var);

  PG_RETURN_BOOL(varint64_cmp_internal(lhs, rhs) <= 0);
}


PG_FUNCTION_INFO_V1(varint64_eq);
Datum
varint64_eq(PG_FUNCTION_ARGS) {
  struct varlena *lhs_var = PG_GETARG_VARLENA_PP(0);
  struct varlena *rhs_var = PG_GETARG_VARLENA_PP(1);

  int64_t lhs = varlena_varint64_to_int64(lhs_var);
  int64_t rhs = varlena_varint64_to_int64(rhs_var);

  PG_RETURN_BOOL(varint64_cmp_internal(lhs, rhs) == 0);
}


PG_FUNCTION_INFO_V1(varint64_ne);
Datum
varint64_ne(PG_FUNCTION_ARGS) {
  struct varlena *lhs_var = PG_GETARG_VARLENA_PP(0);
  struct varlena *rhs_var = PG_GETARG_VARLENA_PP(1);

  int64_t lhs = varlena_varint64_to_int64(lhs_var);
  int64_t rhs = varlena_varint64_to_int64(rhs_var);

  PG_RETURN_BOOL(varint64_cmp_internal(lhs, rhs) != 0);
}


PG_FUNCTION_INFO_V1(varint64_ge);
Datum
varint64_ge(PG_FUNCTION_ARGS) {
  struct varlena *lhs_var = PG_GETARG_VARLENA_PP(0);
  struct varlena *rhs_var = PG_GETARG_VARLENA_PP(1);

  int64_t lhs = varlena_varint64_to_int64(lhs_var);
  int64_t rhs = varlena_varint64_to_int64(rhs_var);

  PG_RETURN_BOOL(varint64_cmp_internal(lhs, rhs) >= 0);
}


PG_FUNCTION_INFO_V1(varint64_gt);
Datum
varint64_gt(PG_FUNCTION_ARGS) {
  struct varlena *lhs_var = PG_GETARG_VARLENA_PP(0);
  struct varlena *rhs_var = PG_GETARG_VARLENA_PP(1);

  int64_t lhs = varlena_varint64_to_int64(lhs_var);
  int64_t rhs = varlena_varint64_to_int64(rhs_var);

  PG_RETURN_BOOL(varint64_cmp_internal(lhs, rhs) > 0);
}


PG_FUNCTION_INFO_V1(varint64_cmp);
Datum
varint64_cmp(PG_FUNCTION_ARGS) {
  struct varlena *lhs_var = PG_GETARG_VARLENA_PP(0);
  struct varlena *rhs_var = PG_GETARG_VARLENA_PP(1);

  int64_t lhs = varlena_varint64_to_int64(lhs_var);
  int64_t rhs = varlena_varint64_to_int64(rhs_var);

  PG_RETURN_INT32(varint64_cmp_internal(lhs, rhs));
}


/*
 * Conversion operators
 */
PG_FUNCTION_INFO_V1(int2_to_varint64);
Datum
int2_to_varint64(PG_FUNCTION_ARGS) {
  int16 in = PG_GETARG_INT16(0);

  return int64_to_varlena_varint64((int64_t)in);
}


PG_FUNCTION_INFO_V1(int4_to_varint64);
Datum
int4_to_varint64(PG_FUNCTION_ARGS) {
  int32 in = PG_GETARG_INT32(0);

  return int64_to_varlena_varint64((int64_t)in);
}


PG_FUNCTION_INFO_V1(int8_to_varint64);
Datum
int8_to_varint64(PG_FUNCTION_ARGS) {
  int64 in = PG_GETARG_INT64(0);

  return int64_to_varlena_varint64((int64_t)in);
}


PG_FUNCTION_INFO_V1(varint64_to_int2);
Datum
varint64_to_int2(PG_FUNCTION_ARGS) {
  struct varlena *var = PG_GETARG_VARLENA_PP(0);

  int64_t num = varlena_varint64_to_int64(var);
  int16 result = (int16)num;
  if ((int64)result != num) {
    ereport(ERROR,
            (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
             errmsg("integer out of range")));
  }

  PG_RETURN_INT16((int16) result);
}


PG_FUNCTION_INFO_V1(varint64_to_int4);
Datum
varint64_to_int4(PG_FUNCTION_ARGS) {
  struct varlena *var = PG_GETARG_VARLENA_PP(0);

  int64_t num = varlena_varint64_to_int64(var);
  int32 result = (int32)num;
  if ((int64)result != num) {
    ereport(ERROR,
            (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
             errmsg("integer out of range")));
  }

  PG_RETURN_INT32((int32) result);
}


PG_FUNCTION_INFO_V1(varint64_to_int8);
Datum
varint64_to_int8(PG_FUNCTION_ARGS) {
  struct varlena *var = PG_GETARG_VARLENA_PP(0);

  PG_RETURN_INT64((int64) varlena_varint64_to_int64(var));
}


PG_FUNCTION_INFO_V1(varint64_sizeof);
Datum
varint64_sizeof(PG_FUNCTION_ARGS) {
  struct varlena *var = PG_GETARG_VARLENA_PP(0);

  PG_RETURN_INT32(VARSIZE_ANY(var));
}


PG_FUNCTION_INFO_V1(varint64_sizeof2);
Datum
varint64_sizeof2(PG_FUNCTION_ARGS) {
  struct varlena *var = PG_GETARG_VARLENA_PP(0);
  bool include_varlena = PG_GETARG_BOOL(1);

  if (include_varlena)
    PG_RETURN_INT32(VARSIZE_ANY(var));

  if (VARATT_IS_SHORT(var))
    PG_RETURN_INT32(VARSIZE_ANY(var) - 1 /* 1 byte VARLENA header */);
  else
    PG_RETURN_INT32(VARSIZE_ANY(var) - 4 /* 4 byte VARLENA header */);
}
