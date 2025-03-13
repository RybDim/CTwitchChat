#ifndef __ENV_PARSER__
#define __ENV_PARSER__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

int read_env_file(char **oauth_token) {
	FILE *env_stream = NULL;
	char *line = NULL;
	size_t line_buffer = 0;
	ssize_t read_bytes = 0;
	bool found_access_token = false;
	bool found_client_id = false;
	bool found_client_secret = false;
	
	char *client_id = NULL;
	char *client_secret = NULL;
	
	char cwd[124] = {'\0'};
	if(getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("Failed to get current working directory\n");
		return -1;
	}
	char env_path[256] = {'\0'};
	strcat(env_path, cwd);
	strcat(env_path, "/.env");
	
	env_stream = fopen(env_path, "r");
	if (env_stream == NULL) {
		perror("Error opening env file");
		return -1;
	}
	
	while ((read_bytes = getline(&line, &line_buffer, env_stream)) != -1) {
		char *key = strtok(line, "=");
		char *value = strtok(NULL, "\n");
		
		if (key && value) {
			if (strcmp(key, "ACCESSTOKEN") == 0) {
				*oauth_token = strdup(value);
				if (*oauth_token == NULL) {
					perror("Memory allocation failed for access token");
					break;
				}
				found_access_token = true;
			} else if (strcmp(key, "CLIENTID") == 0) {
				client_id = strdup(value);
				if (client_id == NULL) {
					perror("Memory allocation failed for client ID");
					break;
				}
				found_client_id = true;
			} else if (strcmp(key, "CLIENTSECRET") == 0) {
				client_secret = strdup(value);
				if (client_secret == NULL) {
					perror("Memory allocation failed for client secret");
					break;
				}
				found_client_secret = true;
			}
		}
	}
	
	free(line);
	fclose(env_stream);

	if (!found_access_token || !found_client_id || !found_client_secret) {

		if(!found_access_token) {
			perror("Missing access token variable\n");
		} 
		
		if(!found_client_id) {
			perror("Missing client id variable\n");
		} 
		
		if(!found_client_secret) {
			perror("Missing client secret variable\n");
		}
		
		if (client_id) free(client_id);
		if (client_secret) free(client_secret);
		
		client_id = NULL;
		client_secret = NULL;
		
		return -1;
	}
	
	return 0;
}

#endif