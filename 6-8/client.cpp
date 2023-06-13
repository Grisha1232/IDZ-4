#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

constexpr int SERVER_PORT = 12345;
constexpr int MAX_MEALS = 3;

int main(int argc, char *argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: ./client <philosopher_index> <ip> <port>" << std::endl;
    return 1;
  }

  int philosopherIndex = std::stoi(argv[1]);
  const char* address = argv[2];
  int serv_port = std::stoi(argv[3]);
  if (philosopherIndex < 0 || philosopherIndex >= 5) {
    std::cerr << "Invalid philosopher index. Must be between 0 and 4." << std::endl;
    return 1;
  }

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

  std::cout << "Phil connected to server waiting for start\n";

  char buffer[1024];
  // отправляем индекс философа
  buffer[0] = philosopherIndex + '0';
  ssize_t bytesSent = send(clientSocket, buffer, strlen(buffer), 0);
  if (bytesSent <= 0) {
    std::cerr << "Error sending data to server." << std::endl;
    return 1;
  }

  ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
  if (bytesRead <= 0) {
    std::cerr << "Error receiving data from server." << std::endl;
    return 1;
  }

  std::string startMessage(buffer, bytesRead);
  if (startMessage != "start") {
    std::cerr << "Unexpected start message from server: " << startMessage << std::endl;
    return 1;
  }

  int numMeals = 0;
  int state = 0;
  while (numMeals < MAX_MEALS) {
    if (state == 0) {
      std::cout << "Philosopher " << philosopherIndex << " is thinking." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // Simulating thinking

      memset(buffer, 0, sizeof(buffer));
      buffer[0] = philosopherIndex + '0';
      bytesSent = send(clientSocket, buffer, strlen(buffer), 0);
      if (bytesSent <= 0) {
        std::cerr << "Error sending data to server." << std::endl;
        break;
      }
      std::cout << "phil trying to take to forks\n";
      state = 1;
    } else if (state == 1) {
      bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
      if (bytesRead <= 0) {
        std::cerr << "Error receiving data from server when reserve forks\n";
        break;
      }

      if (buffer[0] == 'y') {
        std::cout << "Phil take two forks and now eat\n";
        state = 2;
      } else {
        std::cout << "Phil failed to take two forks\n";
        state = 0;
      }
    } else {
      memset(buffer, 0, sizeof(buffer));
      std::cout << "Phil taking back two forks\n";
      buffer[0] = (philosopherIndex + 5) + '0';
      bytesSent = send(clientSocket, buffer, strlen(buffer), 0);
      if (bytesSent <= 0) {
        std::cerr << "Failed to sent message about taking back forks\n";
        break;
      }
      state = 0;
      numMeals++;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  std::cout << "Philosopher " << philosopherIndex << " has finished dining." << std::endl;

  close(clientSocket);

  return 0;
}
