#include "irc_twitchclient.h"

struct IRCTwitchClient
{
	int socket;
	char *oauth_token;
	char *nickname;
};

IRCTwitchClient *irc_client_init(const char *_oauth_token, const char *_nickname)
{
	assert(_oauth_token);
	assert(_nickname);

	IRCTwitchClient *client = (IRCTwitchClient *)malloc(sizeof(IRCTwitchClient));
	memset(client, 0, sizeof(IRCTwitchClient));
	client->socket = -1;
	client->oauth_token = strdup(_oauth_token);
	client->nickname = strdup(_nickname);

	return client;
}

void irc_free_client(IRCTwitchClient *_client)
{
	assert(_client);
	if (_client->socket >= 0)
	{
		close(_client->socket);
	}

	free(_client->oauth_token);
	free(_client->nickname);
	free(_client);
}

bool irc_connect_to_server(IRCTwitchClient *_client)
{
	assert(_client);
	_client->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_client->socket == -1)
	{
		LOG_ERROR("Failed to create socket");
		return false;
	}

	struct hostent *host = gethostbyname(TWITCH_IRC_NOSSL_SERVER);
	if (host == NULL)
	{
		LOG_ERROR("Error resolving twitch server");
		return false;
	}

	struct sockaddr_in server_addr = {0};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(TWITCH_IRC_NOSSL_PORT);

	memcpy(&server_addr.sin_addr, host->h_addr_list[0], host->h_length);

	if (connect(_client->socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		LOG_ERROR("Error connecting to server");
		return false;
	}

	LOG_SUCCESS("Connection successful");
	return true;
}

bool irc_send_message(IRCTwitchClient *_client, const char *message)
{
	assert(_client);
	assert(message);

	if (send(_client->socket, message, strlen(message), 0) < 0)
	{
		LOG_ERROR("Error sending message");
		return false;
	}
	return true;
}

bool irc_read_response(IRCTwitchClient *_client, char *buffer, size_t buffer_size)
{
	memset(buffer, 0, buffer_size);
	char temp_buffer[buffer_size];
	size_t total_bytes_read = 0;
	int bytes_read = 0;
	bool message_complete = false;

	while (!message_complete && total_bytes_read < buffer_size - 1)
	{
		bytes_read = read(_client->socket, temp_buffer,
											MIN(sizeof(temp_buffer) - 1, buffer_size - total_bytes_read - 1));

		if (bytes_read < 0)
		{
			LOG_ERROR("Error receiving message");
			return false;
		}

		if (bytes_read == 0)
		{
			if (total_bytes_read == 0)
			{
				return false;
			}
			break;
		}

		temp_buffer[bytes_read] = '\0';
		strncat(buffer + total_bytes_read, temp_buffer, buffer_size - total_bytes_read - 1);
		total_bytes_read += bytes_read;

		if (total_bytes_read >= 2 &&
				buffer[total_bytes_read - 2] == '\r' &&
				buffer[total_bytes_read - 1] == '\n')
		{
			message_complete = true;
		}
	}

	buffer[total_bytes_read] = '\0';

	if (total_bytes_read > 0)
	{
		LOG_INFO("Received: %s", buffer);
		return true;
	}

	return false;
}

bool irc_request_capabilites(IRCTwitchClient *_client, IRCCapabilities caps)
{
	assert(_client);
	if (caps == IRC_CAP_NONE)
	{
		return true;
	}

	char cap_request[256] = "CAP REQ :";
	bool first = true;

	if (caps & IRC_CAP_MEMBERSHIP)
	{
		strcat(cap_request, "twitch.tv/membership");
		first = false;
	}

	if (caps & IRC_CAP_TAGS)
	{
		if (!first)
		{
			strcat(cap_request, " ");
		}
		strcat(cap_request, "twitch.tv/tags");
		first = false;
	}

	if (caps & IRC_CAP_COMMANDS)
	{
		if (!first)
		{
			strcat(cap_request, " ");
		}
		strcat(cap_request, "twitch.tv/commands");
		first = false;
	}

	strcat(cap_request, "\r\n");
	return irc_send_message(_client, cap_request);
}

bool irc_authenticate(IRCTwitchClient *_client)
{
	assert(_client);
	char buffer[MESSAGE_BUFFER];
	char auth_message[MESSAGE_BUFFER];
	char nick_message[MESSAGE_BUFFER];

	snprintf(auth_message, MESSAGE_BUFFER, IRC_AUTH, _client->oauth_token);
	if (!irc_send_message(_client, auth_message))
	{
		printf("Erorr in sending the auth message");
		return false;
	}

	snprintf(nick_message, MESSAGE_BUFFER, IRC_NICK, _client->nickname);
	if (!irc_send_message(_client, nick_message))
	{
		return false;
	}

	if (!irc_read_response(_client, buffer, MESSAGE_BUFFER))
	{
		return false;
	}

	return true;
}

bool irc_join_channel(IRCTwitchClient *_client, const char *channel)
{
	assert(_client);
	char buffer[MESSAGE_BUFFER];
	char join_message[MESSAGE_BUFFER];

	snprintf(join_message, MESSAGE_BUFFER, IRC_JOIN, channel);
	if (!irc_send_message(_client, join_message))
	{
		return false;
	}

	if (!irc_read_response(_client, buffer, MESSAGE_BUFFER))
	{
		return false;
	}
	LOG_INFO("Channel %s joined successfully\n", channel);
	memset(buffer, '\0', MESSAGE_BUFFER);
	while(1) {
		if(!irc_read_response(_client, buffer, MESSAGE_BUFFER)) {
			break;
		}
	}

	return true;
}

void irc_disconnect(IRCTwitchClient *_client)
{
	assert(_client);
	if (_client->socket >= 0)
	{
		close(_client->socket);
		_client->socket = -1;
		LOG_INFO("Connection to server closed");
	}
}