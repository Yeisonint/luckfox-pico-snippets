#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define QUEUE_SIZE 32

typedef enum {
    EVENT_TX,
    EVENT_RX
} event_type_t;

typedef struct {
    event_type_t type;
    struct can_frame frame;
} can_event_t;

pthread_mutex_t mutex;
pthread_cond_t cond;

can_event_t queue[QUEUE_SIZE];

int head = 0;
int tail = 0;
int count = 0;

int sock_tx;
int sock_rx;

volatile sig_atomic_t running = 1;

char tx_ifname[IFNAMSIZ] = "can1";
char rx_ifname[IFNAMSIZ] = "can0";

void signal_handler(int sig)
{
    (void)sig;
    running = 0;
    pthread_cond_broadcast(&cond);
}

void push_event(can_event_t *event)
{
    pthread_mutex_lock(&mutex);

    if (count < QUEUE_SIZE)
    {
        queue[tail] = *event;
        tail = (tail + 1) % QUEUE_SIZE;
        count++;

        pthread_cond_signal(&cond);
    }

    pthread_mutex_unlock(&mutex);
}

int pop_event_timeout(can_event_t *event, int timeout_ms)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;

    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }

    pthread_mutex_lock(&mutex);

    while (count == 0 && running)
    {
        if (pthread_cond_timedwait(&cond, &mutex, &ts) == ETIMEDOUT)
        {
            pthread_mutex_unlock(&mutex);
            return 0;
        }
    }

    if (!running)
    {
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    *event = queue[head];
    head = (head + 1) % QUEUE_SIZE;
    count--;

    pthread_mutex_unlock(&mutex);

    return 1;
}

int open_can_socket(const char *ifname)
{
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (s < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    strcpy(ifr.ifr_name, ifname);

    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0)
    {
        perror("ioctl");
        close(s);
        exit(EXIT_FAILURE);
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(s);
        exit(EXIT_FAILURE);
    }

    return s;
}

void *can_tx_thread(void *arg)
{
    (void)arg;
    struct can_frame frame;

    while (running)
    {
        frame.can_id = rand() % 0x7FF;
        frame.can_dlc = 8;

        for (int i = 0; i < 8; i++)
            frame.data[i] = rand() % 256;

        if (write(sock_tx, &frame, sizeof(frame)) > 0)
        {
            can_event_t ev;
            ev.type = EVENT_TX;
            ev.frame = frame;

            push_event(&ev);
        }

        sleep(2);
    }

    return NULL;
}

void *can_rx_thread(void *arg)
{
    (void)arg;
    struct can_frame frame;

    while (running)
    {
        int nbytes = read(sock_rx, &frame, sizeof(frame));

        if (nbytes > 0)
        {
            can_event_t ev;
            ev.type = EVENT_RX;
            ev.frame = frame;

            push_event(&ev);
        }
        else if (nbytes < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                printf("[RX] timeout\n");
            else
                perror("CAN read");
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    pthread_t tx_thread;
    pthread_t rx_thread;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    if (argc >= 2)
        snprintf(rx_ifname, IFNAMSIZ, "%s", argv[1]);

    if (argc >= 3)
        snprintf(tx_ifname, IFNAMSIZ, "%s", argv[2]);

    printf("RX interface: %s\n", rx_ifname);
    printf("TX interface: %s\n", tx_ifname);

    sock_rx = open_can_socket(rx_ifname);
    sock_tx = open_can_socket(tx_ifname);

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    setsockopt(sock_rx, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    pthread_create(&tx_thread, NULL, can_tx_thread, NULL);
    pthread_create(&rx_thread, NULL, can_rx_thread, NULL);

    while (running)
    {
        printf("Doing something important...\n");

        can_event_t ev;

        if (pop_event_timeout(&ev, 1000))
        {
            if (ev.type == EVENT_TX)
                printf("[TX] ");
            else
                printf("[RX] ");

            printf("ID=0x%X DLC=%d Data:",
                ev.frame.can_id,
                ev.frame.can_dlc);

            for (int i = 0; i < ev.frame.can_dlc; i++)
                printf(" %02X", ev.frame.data[i]);

            printf("\n");
        }
    }

    printf("Shutting down...\n");

    pthread_join(tx_thread, NULL);
    pthread_join(rx_thread, NULL);

    close(sock_rx);
    close(sock_tx);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("Clean shutdown\n");

    return 0;
}