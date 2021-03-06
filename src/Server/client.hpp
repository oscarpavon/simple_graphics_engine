#ifndef CLIENT_H
#define CLIENT_H

#define COMMAND_EXIT 5
#define COMMAND_MOVE 1

#include <glm/glm.hpp>

#include "../objects.h"

struct Client{
	int id = -1;
	int client_socket = -1;
	bool connected = false;
	int send_socket = -1;
	bool send_connected = false;
	glm::vec3 position_recieved;
	bool thread_created = false;
};

struct ClientPacket{
	int command = -1;
	glm::vec3 position;
};

struct SendPacket{
	int command = -1;
	int players_count = 0;
	glm::vec3 other_player_position;
};
#endif