#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

constexpr int SERVER_PORT = 12345;
constexpr int NUM_PHILOSOPHERS = 5;
constexpr int NUM_FORKS = 5;
constexpr int MAX_MEALS = 3;

bool forks[NUM_FORKS];
std::vector<int> clientSockets;
std::vector<int> viewer_sock;
int philosopher_meals[NUM_PHILOSOPHERS];
std::mutex mutex;

void sendToViewers(const char *message) {
  for (auto sock : viewer_sock) {
    send(sock, message, strlen(message), 0);
  }
}

void closeViewers() {
  for (auto sock : viewer_sock) {
    close(sock);
  }
}

void initializeForks() {
  for (int i = 0; i < NUM_FORKS; ++i) {
    forks[i] = true;
  }
}

bool acquireFork(int fork1Index, int fork2Index) {
  if (forks[fork1Index] && forks[fork2Index]) {
    forks[fork1Index] = false;
    forks[fork2Index] = false;
    return true;
  }
  return false;
}

void releaseFork(int forkIndex) {
  forks[forkIndex] = true;
}

void handleClient(int clientSocket, int philosopherIndex) {
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));
  ssize_t bytesRead;

  while (philosopher_meals[philosopherIndex] < MAX_MEALS) {
    memset(buffer, 0, sizeof(buffer));
    bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
      std::cerr << "Error receiving data from philosopher " << philosopherIndex << "." << std::endl;
      break;
    }

    int fork1Index = buffer[0] - '0';
    int fork2Index = (fork1Index + 1) % 5;

    mutex.lock();
    if (fork1Index == philosopherIndex) {
      if (acquireFork(fork1Index, fork2Index)) {
        std::cout << "Philosopher " << philosopherIndex << " is taking two forks and eating\n";
        send(clientSocket, "y", strlen("y"), 0);
        std::stringstream ss;
        ss << "Philosopher " << philosopherIndex << " is taking two forks and eating";
        auto str = ss.str();
        const char *mess = str.c_str();
        sendToViewers(mess);
      } else {
        std::cout << "Philosopher " << philosopherIndex << " is waiting for forks\n";
        send(clientSocket, "n", strlen("n"), 0);
        std::stringstream ss;
        ss << "Philosopher " << philosopherIndex << " is waiting for forks";
        auto str = ss.str();
        const char *mess = str.c_str();
        sendToViewers(mess);
      }
    } else {
      fork1Index = fork1Index - 5;
      fork2Index = (fork1Index + 1) % 5;
      releaseFork(fork1Index);
      releaseFork(fork2Index);
      philosopher_meals[philosopherIndex]++;
      std::cout << "Philosopher " << philosopherIndex << " take back forks\n";
      std::stringstream ss;
      ss << "Philosopher " << philosopherIndex << " take back forks";
      auto str = ss.str();
      const char *mess = str.c_str();
      sendToViewers(mess);
    }
    mutex.unlock();
  }

  std::cout << "Philosopher " << philosopherIndex << " has finished dining." << std::endl;

  std::stringstream ss;
  ss << "Philosopher " << philosopherIndex << " has finished dining.";
  auto str = ss.str();
  const char *mess = str.c_str();
  sendToViewers(mess);

  close(clientSocket);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "./server <ip> <port>\n";
    return 1;
  }
  const char *address = argv[1];
  int port = std::stoi(argv[2]);

  initializeForks();

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    std::cerr << "Error creating socket." << std::endl;
    return 1;
  }

  sockaddr_in serverAddress{};
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  inet_pton(AF_INET, address, &(serverAddress.sin_addr));

  if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "Error binding socket." << std::endl;
    return 1;
  }

  if (listen(serverSocket, NUM_PHILOSOPHERS) < 0) {
    std::cerr << "Error listening on socket." << std::endl;
    return 1;
  }
  std::cout << "Waiting for connections\n";

  clientSockets.resize(5);
  int connected = 0;
  while (connected < NUM_PHILOSOPHERS) {
    sockaddr_in clientAddress{};
    socklen_t clientAddressLength = sizeof(clientAddress);

    int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);
    if (clientSocket < 0) {
      std::cerr << "Error accepting connection." << std::endl;
      continue;
    }

    char buffer[1024];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
      std::cerr << "Error receiving data from philosopher." << std::endl;
      close(clientSocket);
      return 1;
    }

    int index = buffer[0] - '0';
    if (index < 5) {
      connected++;
      clientSockets[index] = clientSocket;
      std::cout << "Philosopher " << index << " connected to the server." << std::endl;

      std::stringstream ss;
      ss << "Philosopher " << index << " connected to the server.";
      auto str = ss.str();
      const char *mess = str.c_str();
      sendToViewers(mess);
    } else {
      viewer_sock.push_back(clientSocket);
    }
  }

  // Send start signal to all philosophers
  for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
    send(clientSockets[i], "start", strlen("start"), 0);
    std::stringstream ss;
    ss << "Server sent message \'start\' to phil " << i;
    auto str = ss.str();
    const char *mess = str.c_str();
    sendToViewers(mess);
  }

  std::vector<std::thread> clientThreads;

  clientThreads.reserve(NUM_PHILOSOPHERS);
  for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
    clientThreads.emplace_back(handleClient, clientSockets[i], i);
  }

  // Wait for client threads to finish
  for (auto &thread : clientThreads) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  // Close client sockets
  for (int clientSocket : clientSockets) {
    close(clientSocket);
  }

  std::cout << "Server has shut down." << std::endl;
  sendToViewers("Server has shut down.");
  closeViewers();


  close(serverSocket);

  return 0;
}
