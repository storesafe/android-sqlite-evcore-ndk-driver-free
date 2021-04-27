/* API version to check: */
#define SQLC_EV_API_VERSION 9

#define SQLC_NULL_HANDLE    0

/* Export some important sqlite open flags to the Java interface (VFS not supported): */
#define SQLC_OPEN_READONLY      0x00001
#define SQLC_OPEN_READWRITE     0x00002
#define SQLC_OPEN_CREATE        0x00004
#define SQLC_OPEN_URI           0x00040
#define SQLC_OPEN_MEMORY        0x00080
#define SQLC_OPEN_NOMUTEX       0x08000
#define SQLC_OPEN_FULLMUTEX     0x10000
#define SQLC_OPEN_SHAREDCACHE   0x20000
#define SQLC_OPEN_PRIVATECACHE  0x40000

/* some important sqlite result codes to the Java interface: */
#define SQLC_RESULT_OK          0
#define SQLC_RESULT_ERROR       1
#define SQLC_RESULT_INTERNAL    2
#define SQLC_RESULT_PERM        3
#define SQLC_RESULT_ABORT       4
/* TBD ... */
#define SQLC_RESULT_CONSTRAINT  19
#define SQLC_RESULT_MISMATCH    20
#define SQLC_RESULT_MISUSE      21
/* TBD ... */
#define SQLC_RESULT_ROW         100
#define SQLC_RESULT_DONE        101

/* and sqlite datatypes: */
#define SQLC_INTEGER    1
#define SQLC_FLOAT      2
#define SQLC_TEXT       3
#define SQLC_BLOB       4
#define SQLC_NULL       5

/* Could not easily get int64_t from stddef.h for gluegen */
typedef long long sqlc_long_t;

/* negative number indicates an error: */
typedef sqlc_long_t sqlc_handle_t;

sqlc_handle_t sqlc_new_ev_dboc(int sqlc_ev_api_version);

/* open database handle & store the open status per dboc handle */
sqlc_handle_t sqlc_ev_db_open(sqlc_handle_t dboc, const char * filename, int flags);

int sqlc_ev_db_open_result(sqlc_handle_t dboc);

void sqlc_ev_dboc_finalize(sqlc_handle_t dboc);

// FUTURE TBD (???) for sqlcipher:
//  int sqlc_db_key_bytes(sqlc_handle_t db, unsigned char *key_bytes, int num_bytes);
//  int sqlc_db_rekey_bytes(sqlc_handle_t db, unsigned char *key_bytes, int num_bytes);

int sqlc_db_key_native_string(sqlc_handle_t db, char *key_string);
// FUTURE TBD (???) for sqlcipher:
//  int sqlc_db_rekey_string_native(sqlc_handle_t db, char *key_string);

sqlc_long_t sqlc_db_last_insert_rowid(sqlc_handle_t db);
int sqlc_db_total_changes(sqlc_handle_t db);

int sqlc_db_errcode(sqlc_handle_t db);
const char * sqlc_db_errmsg_native(sqlc_handle_t db);
const char * sqlc_errstr_native(int errcode);

int sqlc_db_close(sqlc_handle_t db);

sqlc_handle_t sqlc_evcore_db_new_qc(sqlc_handle_t db);

const char * sqlc_evcore_qc_execute(sqlc_handle_t qc, const char * batch_json, int ll);

void sqlc_evcore_qc_finalize(sqlc_handle_t qc);
