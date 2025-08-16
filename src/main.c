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

static void init_array(Buffer *buff, int32 length)
{
    buf_append_8(buff, TYPE_ARRAY);
    buf_append_32(buff, length);
}

static void reply_error(Conn *c, ErrorType err_type, char *msg)
{
    // [type, err_code, strlen, str]
    buf_append_8(c->outgoing, TYPE_ERROR);
    buf_append_32(c->outgoing, err_type);

    size_t msg_len = strlen(msg);
    buf_append_32(c->outgoing, msg_len);
    buf_append(c->outgoing, msg, msg_len);
}

bool entry_equal(HNode *a, HNode *b)
{
    Entry *first = container_of(a, Entry, node);
    Entry *second = container_of(b, Entry, node);

    return strcmp(first->key, second->key) == 0;
}

Entry *get_entry(char *key)
{
    Entry entry = {0};
    entry.key = key;
    entry.node.hash = fnv_32a_str(entry.key, strlen(entry.key));

    HNode *found = hm_get(&map, &entry.node, entry_equal);
    if (found == NULL)
    {
        return NULL;
    }
    else
    {
        return container_of(found, Entry, node);
    }
}

void set_entry(char *key, char *value)
{
    Entry *ent = malloc(sizeof(Entry));

    ent->key = (char *)key;
    ent->value = (char *)value;

    ent->node.hash = fnv_32a_str(ent->key, strlen(ent->key));
    hm_set(&map, &ent->node, entry_equal);
}

int delete_entry(char *key)
{
    Entry e;
    e.key = key;
    e.node.hash = fnv_32a_str(key, strlen(key));
    HNode *res_node = hm_delete(&map, &e.node, entry_equal);
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

    Entry *entry = get_entry(key);
    if (entry == NULL)
    {
        out_nil(c->outgoing);
    }
    else
    {
        out_string(c->outgoing, entry->value);
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

    set_entry(key, value);
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

    printf("TOTAL LENGTH : %zu\n", map_length);
    init_array(c->outgoing, map_length);

    // Loop over the map
    ht_each(&map.newer, &cb_keys, c->outgoing);

    if (map.older.nodes != NULL)
        ht_each(&map.older, &cb_keys, c->outgoing);

    c->want_read = false;
    c->want_write = true;
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

    else
        reply_error(conn, ERR_UNKNOWN, "Unknown command");

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

// int main2()
// {
//     int fd = setup_connection();
//     struct pollfd *fds = NULL;
//     Conn **fd2conn = NULL;
// 
//     arr_init(fds);
//     arr_init(fd2conn);
// 
//     printf("Listening to port %d\n", PORT);
//     for (;;)
//     {
//         arr_len(fds) = 0;
// 
//         struct pollfd pfd = {.fd = fd, .events = POLLIN, .revents = 0};
//         arr_push(fds, pfd);
// 
//         for (int i = 0; i < arr_len(fd2conn); i++)
//         {
//             Conn *c = fd2conn[i];
// 
//             if (c == NULL)
//                 continue;
// 
//             struct pollfd pfd = {.fd = c->fd, .events = POLLERR};
//             if (c->want_read)
//                 pfd.events |= POLLIN;
//             if (c->want_write)
//                 pfd.events |= POLLOUT;
// 
//             arr_push(fds, pfd);
//         }
// 
//         int recv = poll(fds, arr_len(fds), -1);
//         if (recv < 0 && errno == EINTR)
//         {
//             continue;
//         }
//         if (recv < 0)
//         {
//             die("poll()");
//         }
// 
//         if (fds[0].revents)
//         {
//             Conn *c = handle_accept(fds);
//             printf("Accepted : %d\n", c->fd);
//             if (c->fd >= arr_len(fd2conn))
//             {
//                 while (arr_len(fd2conn) < c->fd)
//                 {
//                     arr_push(fd2conn, (void *)NULL);
//                 }
//                 arr_push(fd2conn, c);
//             }
//             else
//             {
//                 fd2conn[c->fd] = c;
//             }
//         }
// 
//         for (int i = 1; i < arr_len(fds); ++i)
//         {
//             short ready = fds[i].revents;
//             if (ready == 0)
//             {
//                 continue;
//             };
// 
//             Conn *c = fd2conn[fds[i].fd];
//             assert(c != NULL);
// 
//             if (ready & POLLIN)
//             {
//                 read_all(c);
//             }
//             if (ready & POLLOUT)
//             {
//                 write_all(c);
//             }
//             if (ready & POLLERR || c->want_close)
//             {
//                 printf("Closing fd : %d...\n", c->fd);
//                 close(c->fd);
//                 fd2conn[c->fd] = NULL;
//                 free_conn(c);
//             }
//         }
//     }
//     return EXIT_SUCCESS;
// }


typedef struct {
    AVLNode node;
    int value;
} TEntry;

#define container_of(ptr, type, attr) (type *)((char *)(ptr) - offsetof(type, attr));


AVLNode *root = NULL;

int compare_tree(AVLNode *left, AVLNode *right) {
    TEntry *entry_left = container_of(left, TEntry, node);
    TEntry *entry_right = container_of(right, TEntry, node);

    if(entry_left->value == entry_right->value ) return 0;
    if(entry_left->value < entry_right->value ) return 1;
    return -1;
}

void add_tree_entry(int value) {
    TEntry *entry = (TEntry *)malloc(sizeof(TEntry));
    init_tree_node(&entry->node);
    entry->value = value;

    add_tree_node(&root, &entry->node, compare_tree);
}

void remove_tree_entry(int value) {
    TEntry entry;
    init_tree_node(&entry.node);
    entry.value = value;

    AVLNode *removed = remove_tree_node(&root, &entry.node, compare_tree);
    if(removed != NULL) {
        TEntry *removed_entry = container_of(removed, TEntry, node);
        free(removed_entry);
    }
}

int cb(AVLNode *left) {
    TEntry *entry = container_of(left, TEntry, node);
    return entry->value;
}

void run_test();
int main() {
    printf("=== TEST: Binary Search Tree ===\n");
    run_test();
    return EXIT_SUCCESS;
}

