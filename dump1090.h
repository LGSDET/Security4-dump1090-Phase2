
#include <openssl/ssl.h>
#include "rtl-sdr.h"
#include "anet.h"
#include <ctype.h>

#define LG_SECURITY_ENHANCEMENT
#define LG_SECURITY_ENHANCEMENT_TLS
#define LG_SECURITY_ENHANCEMENT_SQLOG
#define LG_SECURITY_ENHANCEMENT_ONLY_SECURE_PORTS

#define MODES_CLIENT_BUF_SIZE 1024
#define MODES_NET_MAX_FD 1024


/* Structure used to describe a networking client. */
struct client {
    int fd;         /* File descriptor. */
    int service;    /* TCP port the client is connected to. */
    char buf[MODES_CLIENT_BUF_SIZE+1];    /* Read buffer. */
    int buflen;                         /* Amount of data on buffer. */
};

/* Program global state. */
struct modesStruct{
    /* Internal state */
    pthread_t reader_thread;
    pthread_mutex_t data_mutex;     /* Mutex to synchronize buffer access. */
    pthread_cond_t data_cond;       /* Conditional variable associated. */
    unsigned char *data;            /* Raw IQ samples buffer */
    uint16_t *magnitude;            /* Magnitude vector */
    uint32_t data_len;              /* Buffer length. */
    int fd;                         /* --ifile option file descriptor. */
    int data_ready;                 /* Data ready to be processed. */
    uint32_t *icao_cache;           /* Recently seen ICAO addresses cache. */
    uint16_t *maglut;               /* I/Q -> Magnitude lookup table. */
    int exit;                       /* Exit from the main loop when true. */

    /* RTLSDR */
    int dev_index;
    int gain;
    int enable_agc;
    rtlsdr_dev_t *dev;
    int freq;

    /* Networking */
    char aneterr[ANET_ERR_LEN];
    struct client *clients[MODES_NET_MAX_FD]; /* Our clients. */
#ifdef LG_SECURITY_ENHANCEMENT_TLS
    SSL_CTX *ctx;
    SSL *ssl[MODES_NET_MAX_FD]; /* SSL **/
#endif
    int maxfd;                      /* Greatest fd currently active. */
#ifdef LG_SECURITY_ENHANCEMENT_TLS
    int tlros; /* TLS-Raw output listening socket. */
    int tlsbsos; /* TLS-SBS output listening socket. */
#endif
    int sbsos;                      /* SBS output listening socket. */
    int ros;                        /* Raw output listening socket. */
    int ris;                        /* Raw input listening socket. */
    int https;                      /* HTTP listening socket. */

    /* Configuration */
    char *filename;                 /* Input form file, --ifile option. */
    int loop;                       /* Read input file again and again. */
    int fix_errors;                 /* Single bit error correction if true. */
    int check_crc;                  /* Only display messages with good CRC. */
    int raw;                        /* Raw output format. */
    int debug;                      /* Debugging mode. */
    int net;                        /* Enable networking. */
    int net_only;                   /* Enable just networking. */
    int interactive;                /* Interactive mode */
    int interactive_rows;           /* Interactive mode: max number of rows. */
    int interactive_ttl;            /* Interactive mode: TTL before deletion. */
    int stats;                      /* Print stats at exit in --ifile mode. */
    int onlyaddr;                   /* Print only ICAO addresses. */
    int metric;                     /* Use metric units. */
    int aggressive;                 /* Aggressive detection algorithm. */

    /* Interactive mode */
    struct aircraft *aircrafts;
    long long interactive_last_update;  /* Last screen update in milliseconds */

    /* Statistics */
    long long stat_valid_preamble;
    long long stat_demodulated;
    long long stat_goodcrc;
    long long stat_badcrc;
    long long stat_fixed;
    long long stat_single_bit_fix;
    long long stat_two_bits_fix;
    long long stat_http_requests;
    long long stat_sbs_connections;

    long long stat_out_of_phase;
};

extern struct modesStruct Modes;

void modesFreeClient(int fd);

void modesSendAllClients(int service, void *msg, int len);
extern ssize_t (*write_ptr)(int, const void*, size_t);
extern int (*SSL_write_ptr)(SSL*, const void*, int);
