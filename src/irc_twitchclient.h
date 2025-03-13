#ifndef __IRC_TWITCH_CLIENT__
#define __IRC_TWITCH_CLIENT__

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <stdbool.h>

#define TWITCH_IRC_NOSSL_SERVER "irc.chat.twitch.tv"
#define TWITCH_IRC_NOSSL_PORT 6667

#define MESSAGE_BUFFER 2048

#define IRC_AUTH "PASS oauth:%s\r\n"
#define IRC_NICK "NICK %s\r\n"
#define IRC_JOIN "JOIN #%s\r\n"

#define LOG_ERROR(msg) fprintf(stderr, "[ERROR] %s : %s\n", msg, strerror(errno))
#define LOG_SUCCESS(msg, ...) fprintf(stdout, "[SUCCESS] " msg "\n", ##__VA_ARGS__)
#define LOG_INFO(msg, ...) printf("[INFO] " msg "\n", ##__VA_ARGS__)
#define LOG_RECEIVED(msg) printf("[RECEIVED] %s", msg)

typedef struct IRCTwitchClient IRCTwitchClient;
typedef enum
{
	IRC_CAP_NONE = 0,
	IRC_CAP_MEMBERSHIP = 1 << 0, // twitch.tv/membership
	IRC_CAP_TAGS = 1 << 1,			 // twitch.tv/tags
	IRC_CAP_COMMANDS = 1 << 2,	 // twitch.tv/commands
	IRC_CAP_ALL = IRC_CAP_MEMBERSHIP | IRC_CAP_TAGS | IRC_CAP_COMMANDS
} IRCCapabilities;

IRCTwitchClient *irc_client_init(const char *, const char *);
void irc_free_client(IRCTwitchClient *);
bool irc_connect_to_server(IRCTwitchClient *);
bool irc_send_message(IRCTwitchClient *, const char *);
bool irc_read_response(IRCTwitchClient *, char *, size_t);
bool irc_request_capabilites(IRCTwitchClient *, IRCCapabilities);
bool irc_authenticate(IRCTwitchClient *);
bool irc_join_channel(IRCTwitchClient *, const char *);
void irc_disconnect(IRCTwitchClient *);

#endif