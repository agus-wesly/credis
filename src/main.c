#include "main.h"
#include "common.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 5555
#define HOST "localhost"
#define MAX_LENGTH 32 << 21

void die(const char *message)
{
    fprintf(stderr, "[%d] %s\n", errno, message);
    abort();
}

void msg(const char *message)
{
    fprintf(stderr, "%s\n", message);
}

static void reply(Conn *c, char *response)
{

    int resp_len = strlen(response);

    buf_append(c->outgoing, &resp_len, 4);
    buf_append(c->outgoing, response, resp_len);

    c->want_read = false;
    c->want_write = true;
}

static void close_conn(Conn *c, char *response)
{
    reply(c, response);
    c->want_close = true;
}

static void handle_get(Conn *c, Request *r)
{
    // Get the key
    char *key = read_next(r);
    if (key == NULL)
    {
        close_conn(c, "Please provide key");
        return;
    }

    printf("Request key : %s\n", key);

    reply(c, key);
    free(key);
}

static void handle_set(Conn *c)
{
    close_conn(c, "TODO : SET");
}

static void handle_delete(Conn *c)
{
    close_conn(c, "TODO : DELETE");
}

bool process_one_request(Conn *conn, int8 *request, int len)
{
    // payload = [nstrings, nchar, @@@... nchar, @@@...]
    Request *req = new_request(request, len);

    char *data = read_next(req);
    if (strcmp(data, "GET") == 0)
    {
        handle_get(conn, req);
    }
    else if (strcmp(data, "SET") == 0)
    {
        handle_set(conn);
    }
    else if (strcmp(data, "DELETE") == 0)
    {
        handle_delete(conn);
    }
    else
    {
        reply(conn, "Bad Request");
        conn->want_close = true;
    }

    free(data);
    free_request(req);
    return true;
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
    // printf("Retrieved data => %.*s\n", len, request);

    bool done = process_one_request(c, request, len);
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

    buf_append(c->incoming, buff, sz);
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

int main2()
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
    return 0;
}

int main()
{
    Map m = {0};
    init_map(&m);

    char k[64];
    for (int i = 0; i < 1000; ++i)
    {
        memset(k, 0, sizeof(k));
        sprintf(k, "%d", i);
        // printf("%s, len : %zu\n", k, strlen(k));
        map_set(&m, k, k);
    }
    printf("\n");

    map_delete(&m, "3");
    map_delete(&m, "2");
    map_delete(&m, "4");

    map_set(&m, "4", "abc");
    Value v = {0};
    if (map_get(&m, "4", &v))
    {
        printf("%s\n", v);
    };

    print_map(&m);

    free_map(&m);
    return 0;
}
