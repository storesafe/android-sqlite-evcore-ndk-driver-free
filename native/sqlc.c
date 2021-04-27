#include "sqlc.h"

#include <stdbool.h>
#include <stddef.h> /* for NULL */

#include <android/log.h>

#include "sqlite3.h"

#include "sqlite3_regexp.h"

#include "sqlite3_base64.h"

#define BASE_HANDLE_OFFSET 0x100000000LL

#ifdef SQLC_KEEP_ANDROID_LOG
// ref: http://www.ibm.com/developerworks/opensource/tutorials/os-androidndk/index.html
#define MYLOG(...) __android_log_print(ANDROID_LOG_VERBOSE, "sqlc", __VA_ARGS__)
#else
#define MYLOG(...) ;
#endif

#define HANDLE_FROM_VP(p) ( BASE_HANDLE_OFFSET + ( (unsigned char *)(p) - (unsigned char *)NULL ) )
#define HANDLE_TO_VP(h) (void *)( (unsigned char *)NULL + (ptrdiff_t)((h) - BASE_HANDLE_OFFSET) )

sqlc_handle_t sqlc_evcore_db_open(int sqlc_evcore_api_version, const char * filename, int flags)
{
  sqlite3 *d1;
  int r1;
  const char * err;

  MYLOG("db_open %s %d", filename, flags);

  if (sqlc_evcore_api_version != SQLC_EV_API_VERSION) {
    __android_log_print(ANDROID_LOG_ERROR, "sqlc", "API MISMATCH ERROR");
    return -SQLC_RESULT_ERROR;
  }

  r1 = sqlite3_open_v2(filename, &d1, flags, NULL);

  MYLOG("db_open %s result %d ptr %p", filename, r1, d1);

  if (r1 != 0) return -r1;

  sqlite3_db_config(d1, SQLITE_DBCONFIG_DEFENSIVE, 1, NULL);

  // TBD IGNORE result of sqlite3_regexp_init for now
  sqlite3_regexp_init(d1, &err);

  sqlite3_base64_init(d1);

  return HANDLE_FROM_VP(d1);
}

/** FUTURE TBD (???) for sqlcipher:
int sqlc_db_key_bytes(sqlc_handle_t db, unsigned char *key_bytes, int num_bytes)
{
  sqlite3 *mydb = HANDLE_TO_VP(db);

#ifdef SQLITE_HAS_CODEC
  return sqlite3_key(mydb, key_bytes, num_bytes);
#else
  return SQLITE_ERROR;
#endif
}

int sqlc_db_rekey_bytes(sqlc_handle_t db, unsigned char *key_bytes, int num_bytes)
{
  sqlite3 *mydb = HANDLE_TO_VP(db);

#ifdef SQLITE_HAS_CODEC
  return sqlite3_rekey(mydb, key_bytes, num_bytes);
#else
  return SQLITE_ERROR;
#endif
}
**/

int sqlc_db_key_native_string(sqlc_handle_t db, char *key_string)
{
  // NOT IMPLEMENTED in this version branch:
  return SQLITE_INTERNAL;
}

sqlc_long_t sqlc_db_last_insert_rowid(sqlc_handle_t db)
{
  sqlite3 *mydb = HANDLE_TO_VP(db);

  return sqlite3_last_insert_rowid(mydb);
}

int sqlc_db_total_changes(sqlc_handle_t db)
{
  sqlite3 *mydb = HANDLE_TO_VP(db);

  return sqlite3_total_changes(mydb);
}

int sqlc_db_errcode(sqlc_handle_t db)
{
  sqlite3 *mydb = HANDLE_TO_VP(db);

  return sqlite3_errcode(mydb);
}

const char * sqlc_db_errmsg_native(sqlc_handle_t db)
{
  sqlite3 *mydb = HANDLE_TO_VP(db);

  return sqlite3_errmsg(mydb);
}

const char * sqlc_errstr_native(int errcode)
{
  return sqlite3_errstr(errcode);
}

int sqlc_db_close(sqlc_handle_t db)
{
  sqlite3 * mydb;

  if (db <= 0) {
    __android_log_print(ANDROID_LOG_ERROR, "sqlc", "ERROR: INVALID db handle");
    return SQLC_RESULT_ERROR;
  }

  mydb = HANDLE_TO_VP(db);

  MYLOG("%s %p", __func__, mydb);

// XXX TBD consider sqlite3_close() vs sqlite3_close_v2() ??:
  return sqlite3_close(mydb);
}

struct qc_s {
  sqlite3 * mydb;
  //void * cleanup1;
  void * cleanup2;
};

sqlc_handle_t sqlc_evcore_db_new_qc(sqlc_handle_t db)
{
  sqlite3 * mydb;
  struct qc_s * myqc;

  if (db <= 0) {
    __android_log_print(ANDROID_LOG_ERROR, "sqlc", "ERROR: INVALID db handle");
    return SQLC_RESULT_ERROR;
  }

  mydb = HANDLE_TO_VP(db);

  myqc = malloc(sizeof(struct qc_s));

  myqc->mydb = mydb;
  //myqc->cleanup1 = NULL;
  myqc->cleanup2 = NULL;

  return HANDLE_FROM_VP(myqc);
}

void sqlc_evcore_qc_finalize(sqlc_handle_t qc)
{
  struct qc_s * myqc;

  if (qc <= 0) {
    __android_log_print(ANDROID_LOG_ERROR, "sqlc", "ERROR: INVALID qc handle");
    return;
  }

  myqc = HANDLE_TO_VP(qc);

  //free(myqc->cleanup1);
  free(myqc->cleanup2);
  free(myqc);
}

int sj(const char * j, int tl, char * a)
{
  int ti=0;
  int ai=0;
  while (ti<tl) {
    const uint8_t c = j[ti];
    if (c == '\\') {
      // XXX TODO TODO TODO TODO TODO
      switch(j[ti+1]) {
      case '\"':
        a[ai++] = '\"';
        ti += 2;
        break;

      case '\\':
        a[ai++] = '\\';
        ti += 2;
        break;

      case 'r':
        a[ai++] = '\r';
        ti += 2;
        break;

      case 'n':
        a[ai++] = '\n';
        ti += 2;
        break;

      case 't':
        a[ai++] = '\t';
        ti += 2;
        break;

      default:
        // XXX TODO what to do??
        ti += 2;
        break;
      }
    } else if (c >= 0xf0) {
      a[ai++]=j[ti++];
      a[ai++]=j[ti++];
      a[ai++]=j[ti++];
      a[ai++]=j[ti++];
    } else if (c >= 0xe0) {
      a[ai++]=j[ti++];
      a[ai++]=j[ti++];
      a[ai++]=j[ti++];
    } else if (c >= 0xc0) {
      a[ai++]=j[ti++];
      a[ai++]=j[ti++];
    } else if (c >= 128) {
      sprintf(a+ai, "-%02x-", c);
      ai += strlen(a+ai);
      ti += 1;
    } else {
      a[ai++]=j[ti++];
    }
  }

  return ai;
}

const char *sqlc_evcore_qc_execute(sqlc_handle_t qc, const char * batch_json, int ll)
{
  struct qc_s * myqc;
  sqlite3 * mydb;

  const char * jp1 = batch_json;
  const char * jp2;
  char jv;

  int r = -1;

  int as = 0;
  int flen = 0;
  char nf[22];
  int nflen = 0;

  int fi = 0;
  sqlite3_stmt * s = NULL;
  int rv = -1;
  int jj, cc;

  int param_count = 0;
  int bi = 0;

  const int FIRST_ALLOC = 10000;
  const int NEXT_ALLOC = 80; // (extra padding)

  char * rr;
  int rrlen = 0;
  int arlen = 0;

  const char * pptext = 0;
  int pplen = 0;

  if (qc <= 0) {
    __android_log_print(ANDROID_LOG_ERROR, "sqlc", "ERROR: INVALID qc handle");
    return "[\"batcherror\", \"internal error\", \"extra\"]";
  }

  myqc = HANDLE_TO_VP(qc);
  mydb = myqc->mydb;

#define EXTRA_ALLOC ((rrlen < 1000000) ? rrlen : (rrlen >> 1))

// EXTRA NOTE: Use of realloc seems to break under some bulk scenarios.
// WORKAROUND is to do malloc/memcpy/free instead.
// TBD VERY strange!

  //free(myqc->cleanup1);
  //myqc->cleanup1 = NULL;
  free(myqc->cleanup2);
  myqc->cleanup2 = NULL;

  if (*jp1 != '[') return "[\"batcherror\", \"internal error: json parse error 1\", \"extra\"]";

  ++jp1;

  // dbid (ignored)
  while ((jv = *jp1) != ',') ++jp1;

  ++jp1;

  // flen (batch length)
  jp2 = jp1;
  while ((*jp2) != ',') ++jp2;

  nflen = (jp2-jp1);
  strncpy(nf, jp1, nflen);
  nf[nflen] = '\0';
  flen = atoi(nf);

  jp1 = jp2 + 1;

  myqc->cleanup2 = rr = malloc(arlen = FIRST_ALLOC);
  if (rr == NULL) goto batchmemoryerror;

  strcpy(rr, "[");
  rrlen = 1;

  for (fi=0; fi<flen; ++fi) {
    int tc0 = sqlite3_total_changes(mydb);

    if (*jp1 != '"') return "[\"batcherror\", \"internal error 2\", \"extra\"]";
    ++jp1;

    jp2 = jp1;
    while ((jv = *jp2) != '"')
      jp2 += (jv == '\\') ? 2 : 1;

    {
      // XXX FUTURE TODO keep buffer & free at the end
      int tl = (jp2-jp1);
      char * a = malloc(tl+100); // extra padding
      int ai = (a == NULL) ? -1 : sj(jp1, tl, a);
      if (a == NULL) goto batchmemoryerror;
      rv = sqlite3_prepare_v2(mydb, a, ai, &s, NULL);
      free(a);
    }

    jp1 = jp2 + 2;

    jp2 = jp1;
    while ((*jp2) != ',') ++jp2;

    nflen = (jp2-jp1);
    strncpy(nf, jp1, nflen);
    nf[nflen] = '\0';
    param_count = atoi(nf);

    jp1 = jp2 + 1;

    if (rv != SQLITE_OK) {
      // SKIP:
      for (bi=1; bi<=param_count; ++bi) {
        jp2 = jp1;
        while ((*jp2) != ',') ++jp2;
        jp1 = jp2 + 1;
      }
    } else {
      for (bi=1; bi<=param_count; ++bi) {
        // FUTURE TBD BLOB
        // NOTE: Only the LAST bind result will be checked:
        jv = *jp1;
        if (jv != '"') {
          jp2 = jp1;
          while ((*jp2) != ',') ++jp2;

          if (jv == 'n') {
            rv = sqlite3_bind_null(s, bi);
          } else if (jv == 't' || jv == 'f') {
            // NOT EXPECTED:
            return "[\"batcherror\", \"internal error 3\", \"extra\"]";
          } else {
            bool f=false;
            const char * jp3;

            for (jp3=jp1; jp3<jp2; ++jp3) {
              if (*jp3=='.') {
                f = true;
                break;
              }
            }

            nflen = (jp2-jp1);
            strncpy(nf, jp1, nflen);
            nf[nflen] = '\0';

            if (f) {
              rv = sqlite3_bind_double(s, bi, atof(nf));
            } else {
              rv = sqlite3_bind_int64(s, bi, atoll(nf));
            }
          }

          jp1 = jp2 + 1;
        } else {
          // FUTURE TBD keep buffer & free at the end??
          ++jp1;

          jp2 = jp1;
          while ((jv = *jp2) != '"')
            jp2 += (jv == '\\') ? 2 : 1;

          {
            int tl = (jp2-jp1);
            char * a = malloc(tl+100); // extra padding
            int ai = (a==NULL) ? -1 : sj(jp1, tl, a);
            if (a == NULL) goto batchmemoryerror1;
            rv = sqlite3_bind_text(s, bi, a, ai, SQLITE_TRANSIENT);
            free(a);
          }
          jp1 = jp2 + 2;
        }
      }

      if (rv == SQLITE_OK) {
        rv = sqlite3_step(s);
      }

      if (rv == SQLITE_ROW) {
        strcpy(rr+rrlen, "\"okrows\",");
        rrlen += 9;
        do {
          cc = sqlite3_column_count(s);
          sprintf(nf, "%d", cc);
          strcpy(rr+rrlen, nf);
          rrlen += strlen(nf);
          strcpy(rr+rrlen, ",");
          ++rrlen;

          for (jj=0; jj<cc; ++jj) {
            int ct = sqlite3_column_type(s, jj);

            strcpy(rr+rrlen, "\"");
            rrlen += 1;
            pptext = sqlite3_column_name(s, jj);
            pplen = strlen(pptext);
            if (rrlen + pplen + NEXT_ALLOC > arlen) {
              char * old = rr;
              arlen += EXTRA_ALLOC + pplen + NEXT_ALLOC;
              //myqc->cleanup2 = rr = realloc(rr, arlen);
              myqc->cleanup2 = rr = malloc(arlen);
              if (rr != NULL) memcpy(rr, old, rrlen);
              free(old);
              if (rr == NULL) goto batchmemoryerror1;
            }
            strcpy(rr+rrlen, pptext);
            rrlen += pplen;
            strcpy(rr+rrlen, "\",");
            rrlen += 2;

            if (ct == SQLITE_NULL) {
              strcpy(rr+rrlen, "null,");
              rrlen += 5;
            } else {
              // (const char *) cast is used to avoid a conversion warning as
              // sqlite3_column_text() returns pointer to unsigned char
              // (same thing as pointer to uint8_t)
              pptext = (const char *)sqlite3_column_text(s, jj);
              pplen = strlen(pptext);

              // NOTE: add double pplen for JSON encoding
              // XXX FUTURE TBD add 3x/4x pplen to deal with certain UTF-8 chars
              if (rrlen + pplen + pplen + NEXT_ALLOC > arlen) {
                char * old = rr;
                arlen += EXTRA_ALLOC + pplen + pplen + NEXT_ALLOC;
                //myqc->cleanup2 = rr = realloc(rr, arlen);
                myqc->cleanup2 = rr = malloc(arlen);
                if (rr != NULL) memcpy(rr, old, rrlen);
                free(old);
                if (rr == NULL) goto batchmemoryerror1;
              }

              if (ct == SQLITE_INTEGER || ct == SQLITE_FLOAT) {
                strcpy(rr+rrlen, pptext);
                rrlen += pplen;
                strcpy(rr+rrlen, ",");
                rrlen += 1;
              } else {
                int pi=0;
                // FUTURE TBD BLOB
                strcpy(rr+rrlen, "\"");
                rrlen += 1;

                while (pi < pplen) {
                  // Use uint8_t (unsigned char) to avoid unwanted conversion
                  // with sign extension:
                  // sqlite3_column_text() returns pointer to unsigned char
                  // (same thing as pointer to uint8_t)
                  // THANKS to @spacepope (Hannes Petersen) for
                  // pointing this one out.
                  const uint8_t pc = pptext[pi];

                  if (pc == '\\') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = '\\';
                    pi += 1;
                  } else if (pc == '\"') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = '\"';
                    pi += 1;
                  } else if (pc >= 32 && pc < 127) {
                    rr[rrlen++] = pptext[pi++];
                  } else if (pc >= 0xf0) {
                    rr[rrlen++] = pptext[pi++];
                    rr[rrlen++] = pptext[pi++];
                    rr[rrlen++] = pptext[pi++];
                    rr[rrlen++] = pptext[pi++];
                  } else if (pc >= 0xe0) {
                    rr[rrlen++] = pptext[pi++];
                    rr[rrlen++] = pptext[pi++];
                    rr[rrlen++] = pptext[pi++];
                  } else if (pc >= 0xc0) {
                    rr[rrlen++] = pptext[pi++];
                    rr[rrlen++] = pptext[pi++];
                  } else if (pc >= 128) {
                    sprintf(rr+rrlen, "?");
                    rrlen += strlen(rr+rrlen);
                    pi += 1;
                  } else if (pc == '\t') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 't';
                    pi += 1;
                  } else if (pc == '\r') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 'r';
                    pi += 1;
                  } else if (pc == '\n') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 'n';
                    pi += 1;
                  } else {
                    sprintf(rr+rrlen, "?%02x?", pc);
                    rrlen += strlen(rr+rrlen);
                    pi += 1;
                  }
                }
                strcpy(rr+rrlen, "\",");
                rrlen += 2;
              }
            }
          }
          rv=sqlite3_step(s);
        } while (rv == SQLITE_ROW);
        strcpy(rr+rrlen, "\"endrows\",");
        rrlen += 10;
      } else if (rv == SQLITE_OK || rv == SQLITE_DONE) {
        int rowsAffected = sqlite3_total_changes(mydb) - tc0;

        if (rrlen + 200 > arlen) {
          char * old = rr;
          arlen += EXTRA_ALLOC + 200 + 50;
          //myqc->cleanup2 = rr = realloc(rr, arlen);
          myqc->cleanup2 = rr = malloc(arlen);
          if (rr != NULL) memcpy(rr, old, rrlen);
          free(old);
          if (rr == NULL) goto batchmemoryerror1;
        }

        if (rowsAffected > 0) {
          int insertId = sqlite3_last_insert_rowid(mydb);

          strcpy(rr+rrlen, "\"ch2\",");
          rrlen += 6;

          sprintf(nf, "%d", rowsAffected);
          strcpy(rr+rrlen, nf);
          rrlen += strlen(nf);
          strcpy(rr+rrlen, ",");
          ++rrlen;

          sprintf(nf, "%d", insertId);
          strcpy(rr+rrlen, nf);
          rrlen += strlen(nf);
          strcpy(rr+rrlen, ",");
          ++rrlen;
        } else {
          strcpy(rr+rrlen, "\"ok\",");
          rrlen += 5;
        }
      }

    }

    if (rv != SQLITE_OK && rv != SQLITE_DONE) {
      const char * em = sqlite3_errmsg(mydb);
      int emlen = strlen(em);
      int pi = 0;

      if (rrlen + emlen + emlen + 100 + NEXT_ALLOC > arlen) {
        char * old = rr;
        arlen += EXTRA_ALLOC + emlen + emlen + 100;
        //myqc->cleanup2 = rr = realloc(rr, arlen);
        myqc->cleanup2 = rr = malloc(arlen);
        if (rr != NULL) memcpy(rr, old, rrlen);
        free(old);
        if (rr == NULL) goto batchmemoryerror1;
      }

      // QUICK FIX to closely emulate error code mapping of other platform implementations:
      if (rv == 19)
        strcpy(rr+rrlen, "\"error\",6,null,\"constraint fail error code: 19 message: ");
      else if (rv == 1)
        strcpy(rr+rrlen, "\"error\",5,null,\"syntax error or other error code: 1 message: ");
      else
        sprintf(rr+rrlen, "\"error\",0,null,\"other error code: %d message: ", rv);
      rrlen += strlen(rr+rrlen);


        {
            {
                int pi=0;
                const char * pptext = em;
                const int pplen = emlen;

                while (pi < pplen) {
                  // Use uint8_t (unsigned char) to avoid unwanted conversion
                  // with sign extension:
                  // sqlite3_column_text() returns pointer to unsigned char
                  // (same thing as pointer to uint8_t)
                  // THANKS to @spacepope (Hannes Petersen) for
                  // pointing this one out.
                  const uint8_t pc = pptext[pi];

                  if (pc == '\\') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = '\\';
                    pi += 1;
                  } else if (pc == '\"') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = '\"';
                    pi += 1;
                  } else if (pc >= 32 && pc < 127) {
                    rr[rrlen++] = pptext[pi++];
                  } else if (pc == '\t') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 't';
                    pi += 1;
                  } else if (pc == '\r') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 'r';
                    pi += 1;
                  } else if (pc == '\n') {
                    rr[rrlen++] = '\\';
                    rr[rrlen++] = 'n';
                    pi += 1;
                  } else {
                    sprintf(rr+rrlen, "?%02x?", pc);
                    rrlen += strlen(rr+rrlen);
                    pi += 1;
                  }
                }
                strcpy(rr+rrlen, "\",");
                rrlen += 2;
            }
        }
    }

    // FUTURE TODO what to do in case this returns an error
    sqlite3_finalize(s);
  }

  strcpy(rr+rrlen, "\"extra\"]");

  return rr;

batchmemoryerror1:
  // FUTURE TODO what to do in case this returns an error
  sqlite3_finalize(s);

batchmemoryerror:
  //free(myqc->cleanup1);
  //myqc->cleanup1 = NULL;
  free(myqc->cleanup2);
  myqc->cleanup2 = NULL;

  return "[\"batcherror\", \"memory error\", \"extra\"]";
}
