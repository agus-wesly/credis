#include "main.h"
#include "common.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> 
#include <time.h>
#define PORT 5555
#define HOST "localhost"
#define MAX_LENGTH 32 << 21

HMap map;

void die(const char *message)
{
    fprintf(stderr, "[%d] %s\n", errno, message);
    abort();
}

void msg(const char *message)
{
    fprintf(stderr, "%s\n", message);
}

void buf_append_8(Buffer *buff, int8 data)
{
    buff_push((buff), data);
}

void buf_append_32(Buffer *buff, int32 data)
{
    buf_append(buff, &data, 4);
}

void buf_append_64(Buffer *buff, int64 data)
{
    buf_append(buff, &data, 8);
}

static void out_string(Buffer *buff, char *data)
{
    size_t resp_len = strlen(data);
    buf_append_8(buff, TYPE_STRING);
    buf_append_32(buff, resp_len);
    buf_append(buff, data, resp_len);
}


static void out_nil(Buffer *buff)
{
    buf_append_8(buff, TYPE_NIL);
}

static void out_int(Buffer *buff, int64 val)
{
    buf_append_8(buff, TYPE_INT);
    buf_append_64(buff, val);
}

static void out_dbl(Buffer *buff, double val)
{
    buf_append_8(buff, TYPE_DOUBLE);
    int64 raw;
    memcpy(&raw, &val, sizeof(raw));  // safe copy of 8 bytes
    buf_append_64(buff, raw);
}

static int init_array(Buffer *buff)
{
    buf_append_8(buff, TYPE_ARRAY);
    buf_append_32(buff, 0);
    return buff_len(buff) - 4;
}

static void end_array(size_t ctx, Buffer *buff, int length) {
    size_t idx = buff->start + ctx;
    assert(buff->data[idx - 1] == TYPE_ARRAY);
    memcpy(&buff->data[idx], &length, 4);
}

static void reply_error(Conn *c, ErrorType err_type, char *msg)
{
    buf_append_8(c->outgoing, TYPE_ERROR);
    buf_append_32(c->outgoing, err_type);

    size_t msg_len = strlen(msg);
    buf_append_32(c->outgoing, msg_len);
    buf_append(c->outgoing, msg, msg_len);
}


bool en_eq(HNode *a, HNode *b)
{
    Entry *first = container_of(a, Entry, node);
    Entry *second = container_of(b, Entry, node);

    return strcmp(first->key, second->key) == 0;
}

Entry *entry_get_from_map(char *key)
{
    Entry entry = {0};
    entry.key = key;
    entry.node.hash = fnv_32a_str(entry.key, strlen(entry.key));

    HNode *found = hm_get(&map, &entry.node, en_eq);
    if (found == NULL)
    {
        return NULL;
    }
    else
    {
        return container_of(found, Entry, node);
    }
}

Entry *entry_str_new(char *key, char *value ) {
    Entry *ent = malloc(sizeof(Entry));

    size_t len = strlen(key) + 1;
    char *new_str = (char *)malloc(len);
    memcpy(new_str, key, len); 
    ent->key = new_str;

    ent->type = ENTRY_STRING;
    ent->str = (char *)value;

    ent->node.hash = fnv_32a_str(ent->key, strlen(ent->key));
    return ent;
}

Entry *entry_zset_new(char *key) {
    Entry *ent = malloc(sizeof(Entry));

    size_t len = strlen(key) + 1;
    char *new_str = (char *)malloc(len);
    memcpy(new_str, key, len); 
    ent->key = new_str;

    ent->type = ENTRY_SET;
    ent->set = new_sorted_set();

    ent->node.hash = fnv_32a_str(ent->key, strlen(ent->key));
    return ent;
}

int delete_entry(char *key)
{
    Entry e;
    e.key = key;
    e.node.hash = fnv_32a_str(key, strlen(key));
    HNode *res_node = hm_delete(&map, &e.node, en_eq);
    if (res_node == NULL)
        return 0;

    Entry *res_entry = container_of(res_node, Entry, node);
    free(res_entry);
    return 1;
}

static void handle_get(Conn *c, Request *r)
{
    if (r->nstrings != 2)
    {
        reply_error(c, ERR_UNKNOWN, "ERR wrong number of arguments for 'get' command");
        return;
    }

    char *key = read_next(r);
    assert(key);

    Entry *entry = entry_get_from_map(key);
    if (entry == NULL)
    {
        out_nil(c->outgoing);
    }
    else
    {
        if (IS_ENTRY_STRING(entry)) {
            out_string(c->outgoing, entry->str);
        } else if(IS_ENTRY_SET(entry)) {
            assert(0 && "TODO : stringify the set");
        }
        else {
            assert(0 && "Unexpected");
        }
    }

    free(key);
}

static void handle_set(Conn *c, Request *r)
{
    if (r->nstrings != 3)
    {
        reply_error(c, ERR_UNKNOWN, "ERR wrong number of arguments for 'set' command");
        return;
    }

    char *key = read_next(r);
    assert(key);

    char *value = read_next(r);
    assert(value);

    Entry *e = entry_str_new(key, value);
    hm_set(&map, &e->node, en_eq);
    out_nil(c->outgoing);
}

static void handle_delete(Conn *c, Request *r)
{
    if (r->nstrings <= 1)
    {
        reply_error(c, ERR_UNKNOWN, "ERR wrong number of arguments for 'del' command");
        return;
    }

    size_t deleted = 0;
    char *key = NULL;

    key = read_next(r);
    while (key != NULL)
    {
        if (delete_entry(key) > 0)
        {
            ++deleted;
        }

        free(key);
        key = read_next(r);
    }

    out_int(c->outgoing, deleted);
}

void cb_keys(HNode *val, void *buff)
{
    Entry *entry = container_of(val, Entry, node);
    out_string((Buffer *)buff, entry->key);
}

static void handle_keys(Conn *c, Request *r)
{
    if (r->nstrings != 1)
    {
        reply_error(c, ERR_UNKNOWN, "ERR wrong number of arguments for 'keys' command");
        return;
    }

    size_t map_length = map.newer.length;
    if (map.older.nodes != NULL)
        map_length += map.older.length;

    size_t ctx = init_array(c->outgoing);
    ht_each(&map.newer, &cb_keys, c->outgoing);
    if (map.older.nodes != NULL)
        ht_each(&map.older, &cb_keys, c->outgoing);
    end_array(ctx, c->outgoing, map_length);

    c->want_read = false;
    c->want_write = true;
}

static bool char2float(char *chardata, float *data) {
    char *endptr;
    errno = 0;
    float res = strtof(chardata, &endptr);

    if (chardata == endptr) return false;

    while (*endptr != '\0') {
        if (isspace((unsigned char)*endptr)) return false;
        endptr++;
    }

    if (errno == ERANGE) return false;

    *data = res;
    return true;
}

static bool char2int(char *chardata, int *data) {
    char *endptr;
    errno = 0;
    float res = strtol(chardata, &endptr, 10);

    if (chardata == endptr) return false;

    while (*endptr != '\0') {
        if (isspace((unsigned char)*endptr)) return false;
        endptr++;
    }

    if (errno == ERANGE) return false;

    *data = res;
    return true;
}

static void handle_z_add(Conn *c, Request *r) {
    // ZADD <set_name> <score> <key>
    if (r->nstrings != 4) {
        reply_error(c, ERR_UNKNOWN, "ERR wrong number of arguments for 'zadd' command");
        return;
    }
    char *set_key = read_next(r);
    char *score = read_next(r);
    char *key = read_next(r);

    Entry *entry = entry_get_from_map(set_key);
    if (entry == NULL) {
        entry  = entry_zset_new(set_key);
        hm_set(&map, &entry->node, en_eq);
        out_int(c->outgoing, 1);

    } else {
        if (!IS_ENTRY_SET(entry)) {
            reply_error(c, ERR_UNKNOWN, "Entry must be a set");
            goto end;
        } 
    }

    ZSet *set = entry->set;
    float data = {0};
    if (!char2float(score, &data)) {
        reply_error(c, ERR_UNKNOWN, "value is not a valid float");
        goto end;
    } 
    int ret = zset_add(set, data, key, strlen(key));
    out_int(c->outgoing, ret);

end:
    free(set_key);
    free(score);
    free(key);
}

static void handle_z_rem(Conn *c, Request *r) {
    // ZREM <set_name> <key>
    if (r->nstrings != 3) {
        reply_error(c, ERR_UNKNOWN, "ERR wrong number of arguments for command");
        return;
    }
    char *set_key = read_next(r);
    char *key = read_next(r);

    Entry *entry = entry_get_from_map(set_key);   
    if (entry == NULL) {
        entry  = entry_zset_new(set_key);
        hm_set(&map, &entry->node, en_eq);
        out_nil(c->outgoing);
    } else {
        ZSet *s = entry->set;
        if (!zset_rem(s, key, strlen(key))) {
            out_int(c->outgoing, 0);
        } else {
            out_int(c->outgoing, 1);
        }
    }

    free(set_key);
    free(key);
}

static void handle_z_query(Conn *c, Request *r) {
    // ZQUERY <set_name> <score> <key> <offset> <limit>

    if (r->nstrings != 6) {
        reply_error(c, ERR_UNKNOWN, "ERR wrong number of arguments for 'zquery' command");
        return;
    }

    char *set_key = read_next(r);
    char *score = read_next(r);
    char *key = read_next(r);
    char *offset = read_next(r);
    char *limit = read_next(r);

    float f_score;
    int i_offset;
    int i_limit;

    if (!char2float(score, &f_score)) {
        reply_error(c, ERR_UNKNOWN, "'score' is not a valid float");
    };
    if (!char2int(offset, &i_offset)) {
        reply_error(c, ERR_UNKNOWN, "'offset' is not a valid int");
    };
    if (!char2int(limit, &i_limit)) {
        reply_error(c, ERR_UNKNOWN, "'limit' is not a valid int");
    };
    if (i_limit == 0) i_limit = -1; // if user provide 0, than it means no limit

    Entry *entry = entry_get_from_map(set_key);
    if (entry == NULL) {
        out_nil(c->outgoing);
    } else {
        size_t arr_ctx = init_array(c->outgoing);
        size_t l = 0;
        ZSet *set = entry->set;
        ZNode *curr = zset_find_ge(set, f_score, key, strlen(key));
        curr = zset_offset(curr, i_offset);
        while (i_limit > 0) {
            if (!curr) break;
            out_string(c->outgoing, curr->key);
            out_dbl(c->outgoing, (double)curr->score);
            l += 2;

            curr = zset_offset(curr, +1);
            --i_limit;
        }
        end_array(arr_ctx, c->outgoing, l);
    }

    free(set_key);
    free(score);
    free(key);
    free(offset);
    free(limit);
}


static void handle_z_rank(Conn *c, Request *r) {
    if (r->nstrings != 3) {
        reply_error(c, ERR_UNKNOWN, "ERR wrong number of arguments for 'zrank' command");
        return;
    }

    char *set_key = read_next(r);
    char *key = read_next(r);

    Entry *entry = entry_get_from_map(set_key);
    if (!entry) {
        out_nil(c->outgoing);
        goto end;
    }
    ZSet *set = entry->set;
    ZNode *node = zset_hm_lookup(set, key, strlen(key));
    if (!node) {
        out_nil(c->outgoing);
        goto end;
    }
    size_t rank = zset_rank(set, node);
    out_int(c->outgoing, rank);

end:
    free(set_key);
    free(key);
}

static void handle_z_score(Conn *c, Request *r) {
    // ZSCORE <set_name> <key> -> score
    if (r->nstrings != 3) {
        reply_error(c, ERR_UNKNOWN, "ERR wrong number of arguments for 'zscore' command");
        return;
    }

    char *set_key = read_next(r);
    char *key = read_next(r);

    Entry *entry = entry_get_from_map(set_key);
    if (entry == NULL) {
        out_nil(c->outgoing);
    } else {
        ZSet *s = entry->set;
        ZNode *s_entry = zset_hm_lookup(s, key, strlen(key)); 
        if (s_entry == NULL) {
            out_nil(c->outgoing);
        } else {
            char buff[128];
            sprintf(buff, "%f", s_entry->score);
            out_string(c->outgoing, buff);
        }
    }

    free(set_key);
    free(key);
}

bool process_one_request(Conn *conn, int8 *request, int len)
{
    // payload = [nstrings, nchar, @@@... nchar, @@@...]
    Request *req = new_request(request, len);
    if (req->nstrings > MAX_LENGTH)
    {
        reply_error(conn, ERR_TO_BIG, "Request length too big");
        conn->want_close = true;
    }

    char *data = read_next(req);

    if (strcmp(data, "GET") == 0)
        handle_get(conn, req);

    else if (strcmp(data, "SET") == 0)
        handle_set(conn, req);

    else if (strcmp(data, "DEL") == 0)
        handle_delete(conn, req);

    else if (strcmp(data, "KEYS") == 0)
        handle_keys(conn, req);

    else if(strcmp(data, "ZADD") == 0) 
        handle_z_add(conn, req);

    else if(strcmp(data, "ZQUERY") == 0) 
        handle_z_query(conn, req);

    else if(strcmp(data, "ZRANK") == 0) 
        handle_z_rank(conn, req);

    else if(strcmp(data, "ZREM") == 0) 
        handle_z_rem(conn, req);

    else if(strcmp(data, "ZSCORE") == 0) 
        handle_z_score(conn, req);

    else {
        printf("data %s\n", data);
        reply_error(conn, ERR_UNKNOWN, "Unknown command");
    }

    conn->want_read = false;
    conn->want_write = true;

    free(data);
    free_request(req);
    return true;
}

static void response_begin(Conn *c, size_t *curr_idx)
{
    *curr_idx = buff_len(c->outgoing);
    // reserve 4 byte on the outgoing for the size
    buf_append_32(c->outgoing, 0);
}

static void response_end(Conn *c, size_t curr_idx)
{
    size_t final_size = buff_len(c->outgoing) - curr_idx - 4;
    memcpy(&buff_data(c->outgoing)[curr_idx], &final_size, 4);
}

bool try_one_request(Conn *c)
{
    if (buff_len(c->incoming) < 4)
    {
        return false;
    }
    int len = {0};
    memcpy(&len, buff_data(c->incoming), 4);

    if (len > MAX_LENGTH)
    {
        fprintf(stderr, "Too long\n");
        c->want_close = true;
        return false;
    }

    if (4 + len > (int)buff_len(c->incoming))
    {
        return false;
    }

    int8 *request = &buff_data(c->incoming)[4];

    size_t curr_idx = {0};
    response_begin(c, &curr_idx);
    bool done = process_one_request(c, request, len);
    response_end(c, curr_idx);

    buf_consume(c->incoming, 4 + len);

    return done;
}

void write_all(Conn *c)
{
    assert(buff_len(c->outgoing) > 0);

    int recv = write(c->fd, buff_data(c->outgoing), buff_len(c->outgoing));
    if (recv == -1)
    {
        fprintf(stderr, "write()\n");
        c->want_close = true;
        return;
    }

    if (recv == -1 && errno == EAGAIN)
    {
        msg("Not yet ready\n");
        return;
    }

    buf_consume(c->outgoing, recv);

    // Check if all data is writted
    if (buff_len(c->outgoing) == 0)
    {
        c->want_read = true;
        c->want_write = false;
    }
}

void read_all(Conn *c)
{
    int8 buff[64 * 1024] = {0};
    int sz = read(c->fd, buff, sizeof(buff));
    if (sz == -1)
    {
        msg("read()");
        c->want_close = true;
    }

    if (sz == 0)
    {
        c->want_read = false;
        c->want_close = true;
        return;
    }

    buf_append(c->incoming, buff, (size_t)sz);
    while (try_one_request(c))
    {
    };
    // Write directly from here
    if (buff_len(c->outgoing) > 0)
    {
        c->want_read = false;
        c->want_write = true;
        write_all(c);
        return;
    }
}

void fd_set_nb(int fd)
{
    errno = 0;
    (void)fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL) | O_NONBLOCK));
    if (errno)
    {
        die("fcntl()");
    }
}

Conn *handle_accept(struct pollfd *fds)
{
    struct sockaddr_in client_addr = {0};
    socklen_t addrlen = sizeof(client_addr);
    int conn_fd = accept(fds[0].fd, (struct sockaddr *)&client_addr, &addrlen);

    if (conn_fd == -1)
    {
        die("accept()");
    }

    fd_set_nb(conn_fd);

    Conn *c = (void *)malloc(sizeof(Conn));
    c->fd = conn_fd;
    c->want_read = true;
    c->want_write = false;
    c->want_close = false;

    c->incoming = new_buffer();
    c->outgoing = new_buffer();
    return c;
}

int setup_connection()
{
    int fd;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
    };

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        die("setsockopt()");
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(0);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        die("bind()");
    }

    if (listen(fd, SOMAXCONN) < 0)
    {
        die("listen()");
    }
    fd_set_nb(fd);
    return fd;
}

void free_conn(Conn *c)
{
    free_buff(c->incoming);
    free_buff(c->outgoing);
    free(c);
}

int main()
{
    int fd = setup_connection();
    struct pollfd *fds = NULL;
    Conn **fd2conn = NULL;

    arr_init(fds);
    arr_init(fd2conn);

    printf("Listening to port %d\n", PORT);
    for (;;)
    {
        arr_len(fds) = 0;

        struct pollfd pfd = {.fd = fd, .events = POLLIN, .revents = 0};
        arr_push(fds, pfd);

        for (int i = 0; i < arr_len(fd2conn); i++)
        {
            Conn *c = fd2conn[i];

            if (c == NULL)
                continue;

            struct pollfd pfd = {.fd = c->fd, .events = POLLERR};
            if (c->want_read)
                pfd.events |= POLLIN;
            if (c->want_write)
                pfd.events |= POLLOUT;

            arr_push(fds, pfd);
        }

        int recv = poll(fds, arr_len(fds), -1);
        if (recv < 0 && errno == EINTR)
        {
            continue;
        }
        if (recv < 0)
        {
            die("poll()");
        }

        if (fds[0].revents)
        {
            Conn *c = handle_accept(fds);
            printf("Accepted : %d\n", c->fd);
            if (c->fd >= arr_len(fd2conn))
            {
                while (arr_len(fd2conn) < c->fd)
                {
                    arr_push(fd2conn, (void *)NULL);
                }
                arr_push(fd2conn, c);
            }
            else
            {
                fd2conn[c->fd] = c;
            }
        }

        for (int i = 1; i < arr_len(fds); ++i)
        {
            short ready = fds[i].revents;
            if (ready == 0)
            {
                continue;
            };

            Conn *c = fd2conn[fds[i].fd];
            assert(c != NULL);

            if (ready & POLLIN)
            {
                read_all(c);
            }
            if (ready & POLLOUT)
            {
                write_all(c);
            }
            if (ready & POLLERR || c->want_close)
            {
                printf("Closing fd : %d...\n", c->fd);
                close(c->fd);
                fd2conn[c->fd] = NULL;
                free_conn(c);
            }
        }
    }
    return EXIT_SUCCESS;
}
