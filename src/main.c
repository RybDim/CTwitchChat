#include "irc_twitchclient.h"
#include "env_parser.h"

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s [nickname] [channel]\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *oauth_token = NULL;
	const char *nickname = argv[1];
	const char *channel = argv[2];

	short res = 0;
	if((res = read_env_file(&oauth_token)) != 0) {
		return 1;
	}

	IRCTwitchClient *client = irc_client_init(oauth_token, nickname);
	if (!client)
	{
		return EXIT_FAILURE;
	}


	if(!irc_connect_to_server(client)) {
		printf("Error connecting to server\n");
	}

	if(!irc_request_capabilites(client, IRC_CAP_ALL)) {
		fprintf(stderr, "Error requesting capabilites");
	}

	if(!irc_authenticate(client)) {
		fprintf(stderr, "Erorr in authentication\n");
	}

	if(!irc_join_channel(client, channel)) {
		fprintf(stderr, "Error joining channel\n");
	}

	irc_disconnect(client);
	irc_free_client(client);

	return 0;
}