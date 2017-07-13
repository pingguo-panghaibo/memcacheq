#include "qnet.h"

int new_socket(struct addrinfo *ai)
{
    int sfd, flags;
    if ((sfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
        perror("socket()");
        return -1;
    }
    if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
        fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("setting O_NONBLOCK");
        close(sfd);
        return -1;
    }
    return sfd;
}

int server_socket(const int port, const bool is_udp)
{
    int sfd;
    struct linger ling = {0, 0};
    struct addrinfo *ai;
    struct addrinfo *next;
    struct addrinfo hints;
    char port_buf[NI_MAXSERV];
    int error;
    int success = 0;
    int flags =1;
    memset(&hints, 0, sizeof (hints));
    hints.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;
    if (is_udp)
    {
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_family = AF_INET; /* This left here because of issues with OSX 10.5 */
    } else {
        hints.ai_family = AF_UNSPEC;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_socktype = SOCK_STREAM;
    }

    snprintf(port_buf, NI_MAXSERV, "%d", port);
    error= getaddrinfo(settings.inter, port_buf, &hints, &ai);
    if (error != 0) {
      if (error != EAI_SYSTEM)
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(error));
      else
        perror("getaddrinfo()");

      return 1;
    }
    for (next= ai; next; next= next->ai_next) {
        conn *listen_conn_add;
        if ((sfd = new_socket(next)) == -1) {
            freeaddrinfo(ai);
            return 1;
        }
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
        if (is_udp) {
            maximize_sndbuf(sfd);
        } else {
            setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
            setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
            setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
        }
        if (bind(sfd, next->ai_addr, next->ai_addrlen) == -1) {
            if (errno != EADDRINUSE) {
                perror("bind()");
                close(sfd);
                freeaddrinfo(ai);
                return 1;
            }
            close(sfd);
            continue;
        } else {
          success++;
          if (!is_udp && listen(sfd, 1024) == -1) {
              perror("listen()");
              close(sfd);
              freeaddrinfo(ai);
              return 1;
          }
      }
      if (is_udp)
      {
        int c;

        for (c = 0; c < settings.num_threads; c++) {
            /* this is guaranteed to hit all threads because we round-robin */
            dispatch_conn_new(sfd, conn_read, EV_READ | EV_PERSIST,
                              UDP_READ_BUFFER_SIZE, 1);
        }
      } else {
        if (!(listen_conn_add = conn_new(sfd, conn_listening,
                                         EV_READ | EV_PERSIST, 1, false, main_base))) {
            fprintf(stderr, "failed to create listening connection\n");
            exit(EXIT_FAILURE);
        }

        listen_conn_add->next = listen_conn;
        listen_conn = listen_conn_add;
      }
    }
    freeaddrinfo(ai);
    return success == 0;
}

int new_socket_unix(void) {
    int sfd;
    int flags;

    if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        return -1;
    }

    if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
        fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("setting O_NONBLOCK");
        close(sfd);
        return -1;
    }
    return sfd;
}

int server_socket_unix(const char *path, int access_mask)
{
    int sfd;
    struct linger ling = {0, 0};
    struct sockaddr_un addr;
    struct stat tstat;
    int flags =1;
    int old_umask;
    if (!path) {
        return 1;
    }
    if ((sfd = new_socket_unix()) == -1) {
        return 1;
    }
    /*
     * Clean up a previous socket file if we left it around
     */
    if (lstat(path, &tstat) == 0) {
        if (S_ISSOCK(tstat.st_mode))
            unlink(path);
    }
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
    setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
    setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
    /*
     * the memset call clears nonstandard fields in some impementations
     * that otherwise mess things up.
     */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    old_umask=umask( ~(access_mask&0777));
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind()");
        close(sfd);
        umask(old_umask);
        return 1;
    }
    umask(old_umask);
    if (listen(sfd, 1024) == -1) {
        perror("listen()");
        close(sfd);
        return 1;
    }
    if (!(listen_conn = conn_new(sfd, conn_listening,
                                     EV_READ | EV_PERSIST, 1, false, main_base))) {
        fprintf(stderr, "failed to create listening connection\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}
