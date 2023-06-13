#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

constexpr int SERVER_PORT = 12345;
constexpr int BUFFER_SIZE = 1024;

int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: ./observer <ip> <port>" << std::endl;
    return 1;
  }

  const char* address = argv[1];
  int serv_port = std::stoi(argv[2]);

  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket < 0) {
    std::cerr << "Error creating socket." << std::endl;
    return 1;
  }

  sockaddr_in serverAddress{};
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serv_port);

  if (inet_pton(AF_INET, address, &(serverAddress.sin_addr)) <= 0) {
    std::cerr << "Invalid server address." << std::endl;
    return 1;
  }

  if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "Error connecting to server." << std::endl;
    return 1;
  }

  std::cout << "Observer connected to the server.\n";

  ssize_t bytesSent = send(clientSocket, "9", strlen("9"), 0);
  if (bytesSent <= 0) {
    std::cerr << "Error sending mess to serv\n";
    return 1;
  }

  char buffer[BUFFER_SIZE];
  ssize_t bytesRead;

  while (true) {
    memset(buffer, 0, sizeof(buffer));
    bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
      std::cerr << "Error receiving data from server." << std::endl;
      break;
    }

    std::cout << buffer << std::endl;
  }

  std::cout << "Observer has disconnected." << std::endl;

  close(clientSocket);

  return 0;
}
