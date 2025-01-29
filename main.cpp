#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <cstdio>
#include <locale>
#include <stdio.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

int main()
{
    setlocale(LC_ALL, "Russian");

    char sendAdr[128], hostIP[128];
    const char *sendData = "package";

    WSADATA wsaData;
    WORD sockVer = MAKEWORD(2, 2);
    WSAStartup(sockVer, &wsaData);

    printf("Enter the address: ");
    fgets(sendAdr, sizeof(sendAdr), stdin);
    sendAdr[strcspn(sendAdr, "\n")] = 0;

    struct hostent *host = gethostbyname(sendAdr);
    if (host != NULL)
    {
        unsigned long minTime = LONG_MAX, maxTime = 0, time = 0, pack_rec = 0, timeout = 1000;

        strcpy_s(hostIP, inet_ntoa(*((in_addr *)host->h_addr_list[0])));
        unsigned long ipaddr = inet_addr(hostIP);
        HANDLE hIcmpFile = IcmpCreateFile();
        unsigned long massageSize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData);
        LPVOID massageBuffer = malloc(massageSize);

        if (strcmp(sendAdr, hostIP))
            printf("Exchange packets with %s [%s] with %d bytes of data:\n", sendAdr, hostIP, massageSize);
        else
            printf("Exchange packets with %s with %d bytes of data:\n", sendAdr, massageSize);

        for (int i = 0; i < 4; i++)
        {

            DWORD echoReq = IcmpSendEcho(hIcmpFile, ipaddr, (LPVOID)sendData, (WORD)strlen(sendData), NULL, massageBuffer, massageSize, timeout);

            if (echoReq)
            {
                PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)massageBuffer;
                printf("    Reply from %s: ", hostIP);
                printf("Number of bytes = %d ", massageSize);
                printf("Time = %ld ms ", pEchoReply->RoundTripTime);
                printf("TTL = %d\n", (int)pEchoReply->Options.Ttl);

                if (minTime > pEchoReply->RoundTripTime)
                    minTime = pEchoReply->RoundTripTime;
                if (maxTime < pEchoReply->RoundTripTime)
                    maxTime = pEchoReply->RoundTripTime;
                time += pEchoReply->RoundTripTime;
                pack_rec += 1;
            }
            else
                printf("Waiting interval exceeded\n");
        }

        printf("Ping statistics for %s:\n", hostIP);
        printf("    Packets: transmitted = 4, received = %d, loss = %d\n", pack_rec, 4 - pack_rec);
        printf("    (%.0f%% loss)\n", (100.0 - pack_rec / 4.0 * 100.0));

        if (pack_rec != 0)
        {
            printf("Approximate transmit-receive time in ms:\n");
            printf("    Min = %ld ms, Max = %ld ms, Mean = %ld ms\n", minTime, maxTime, time / pack_rec);
        }

        free(massageBuffer);
        IcmpCloseHandle(hIcmpFile);
    }
    else
        printf("This address does not exist!\n");

    WSACleanup();
    system("pause");
    return 0;
}
