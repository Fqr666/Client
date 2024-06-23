#include <stdio.h>
#include <winsock2.h>
#include <crtdefs.h>
#include <process.h>
#include <stdint.h> // For uint8_t, uint32_t

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 12345

// Base64�����
const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// ��������
unsigned __stdcall receive_thread(void* data);
void base64_encode(const char* input, int input_len, char* output);;

int main() {
    WSADATA wsa;
    SOCKET client_sock;
    struct sockaddr_in server_addr;
    char message[1024] = { 0 };
    char buffer[1024] = { 0 };
    int recv_size;

    // ��ʼ��Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    // ����socket
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }

    // ���÷�������ַ
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // ���ӵ�������
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed\n");
        closesocket(client_sock);
        return 1;
    }

    printf("Connected to server %s:%d\n", SERVER_IP, PORT);

    // ���������߳�
    HANDLE recv_thread = (HANDLE)_beginthreadex(NULL, 0, &receive_thread, (void*)&client_sock, 0, NULL);
    if (recv_thread == NULL) {
        printf("Failed to create receive thread\n");
        closesocket(client_sock);
        return 1;
    }

    // ���̴߳����û����������ݷ���
    while (1) {
        printf("Enter message to send (or 'exit' to quit): ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0'; // Remove newline character

        // ����û�����exit�����˳�
        if (strcmp(message, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }

        // ����Ϣ����Base64����
        char base64_encoded[2048] = { 0 }; // Assuming a reasonable buffer size
        base64_encode(message, strlen(message), base64_encoded);

        // ����Base64���������ݸ�������
        if (send(client_sock, base64_encoded, strlen(base64_encoded), 0) < 0) {
            printf("Send failed\n");
            break;
        }
    }

    // �ȴ������߳̽���
    WaitForSingleObject(recv_thread, INFINITE);

    // �ر��߳̾����socket
    CloseHandle(recv_thread);
    closesocket(client_sock);
    WSACleanup();

    return 0;
}

// �����̺߳���
unsigned __stdcall receive_thread(void* data) {
    SOCKET client_sock = *((SOCKET*)data);
    char buffer[1024] = { 0 };
    int recv_size;

    while (1) {
        recv_size = recv(client_sock, buffer, sizeof(buffer), 0);
        if (recv_size < 0) {
            printf("Receive failed\n");
            break;
        }
        else if (recv_size == 0) {
            printf("Server disconnected\n");
            break;
        }
        else {
            buffer[recv_size] = '\0';
            printf("Received from server: %s\n", buffer);
        }
    }

    return 0;
}

// Base64���뺯��
void base64_encode(const char* input, int input_len, char* output) {
    int i = 0, j = 0;
    uint32_t buf = 0;
    int buf_len = 0;

    while (i < input_len) {
        buf = (buf << 8) | input[i++];
        buf_len += 8;

        while (buf_len >= 6) {
            output[j++] = base64_table[(buf >> (buf_len - 6)) & 0x3F];
            buf_len -= 6;
        }
    }

    if (buf_len > 0) {
        buf <<= (6 - buf_len);
        output[j++] = base64_table[buf & 0x3F];
    }

    while (j % 4 != 0) {
        output[j++] = '=';
    }

    output[j] = '\0';
}
