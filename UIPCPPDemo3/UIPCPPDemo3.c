#include <stdio.h>
#include <windows.h>
#include "FSUIPC_User64.h"

char* pszErrors[] = {
    "Okay",
    "Attempt to Open when already Open",
    "Cannot link to FSUIPC or WideClient",
    "Failed to Register common message with Windows",
    "Failed to create Atom for mapping filename",
    "Failed to create a file mapping object",
    "Failed to open a view to the file map",
    "Incorrect version of FSUIPC, or not FSUIPC",
    "Sim is not version requested",
    "Call cannot execute, link not Open",
    "Call cannot execute: no requests accumulated",
    "IPC timed out all retries",
    "IPC sendmessage failed all retries",
    "IPC request contains bad data",
    "Maybe running on WideClient, but FS not running on Server, or wrong FSUIPC",
    "Read or Write request cannot be added, memory for Process is full",
};
void printBits(const unsigned char* data, int size) {
    int lo = 0;
    int hi = size * 8;
    for (int i = lo; i <= hi; i++) {
        int byteIndex = i / 8;
        int bitIndex = i % 8;
        int bitValue = (data[byteIndex] >> bitIndex) & 1;
        printf("%d", bitValue);
    }
}
int main(int argc, char* argv[])
{
    DWORD dwResult;

    if (FSUIPC_Open(SIM_ANY, &dwResult))
    {
        char chMsg[128], chTimeMsg[64];
        char chTime[3];
        char acft_icao[256];
        BOOL fTimeOk = TRUE;
        BOOL facftOk = TRUE;

        static char* pFS[] = { "FS98", "FS2000", "CFS2", "CFS1", "Fly!", "FS2002", "FS2004", "FSX 32-bit", "ESP", "P3D 32-bit", "FSX 64-bit", "P3D 64-bit", "MSFS2020", "MSFS2024"};
        /*

        if (!FSUIPC_Read(0x238, 3, chTime, &dwResult) ||
            !FSUIPC_Process(&dwResult))
            fTimeOk = FALSE;

        if (fTimeOk)
            nprintf(chTimeMsg, sizeof(chTimeMsg), "Request for time ok: FS clock = %02d:%02d:%02d", chTime[0], chTime[1], chTime[2]);
        else
            snprintf(chTimeMsg, sizeof(chTimeMsg), "Request for time failed: %s", pszErrors[dwResult]);

        if (!FSUIPC_Read(0x3D00, 256, acft_icao, &dwResult) ||
            !FSUIPC_Process(&dwResult))
            facftOk = FALSE;

        if (facftOk)
            snprintf(chTimeMsg, sizeof(chTimeMsg), "Request for aircraft ICAO ok: %s", acft_icao);
        else
            snprintf(chTimeMsg, sizeof(chTimeMsg), "Request for aircraft ICAO failed: %s", pszErrors[dwResult]);


        */
        printf("Sim is %s,   FSUIPC Version = %c.%c%c%c%c\n",
            (FSUIPC_FS_Version && (FSUIPC_FS_Version <= 15)) ? pFS[FSUIPC_FS_Version - 1] : "Post FS2024",
            '0' + (0x0f & (FSUIPC_Version >> 28)),
            '0' + (0x0f & (FSUIPC_Version >> 24)),
            '0' + (0x0f & (FSUIPC_Version >> 20)),
            '0' + (0x0f & (FSUIPC_Version >> 16)),
            (FSUIPC_Version & 0xffff) ? 'a' + (FSUIPC_Version & 0xff) - 1 : ' '
            );

        printf("UIPC: Link established to FSUIPC\n\n");
        while(1)
        {
			DWORD dwOffset;
			DWORD dwSize;
			// ask user for an offset and size
			printf("UIPC: Enter offset and size (hex): ");
            while(scanf_s("%lx", &dwOffset) != 1) {
                printf("Invalid input for offset.\n");
            }
            while(scanf_s("%d", &dwSize) != 1) {
                printf("Invalid input for size.\n");
            }
			char response[256] = { 0 };
            BOOL success = TRUE;
            // obtain data given the offset and the size
            if (!FSUIPC_Read(dwOffset, dwSize, response, &dwResult) ||
                !FSUIPC_Process(&dwResult))
				success = FALSE;
            if (success) {
                printf("Read Successful.\n");
				printf("->in string: %s\n", response);
				printf("->in ASCII: ");
				int i = 0; while (response[i]) printf("%i ", response[i++]);
				printf("\n");
                // Interpret the response as a double if the size is 8 bytes
                if (dwSize == 8) {
                    double dValue;
                    // Copy the bytes into a double variable (assuming little-endian)
                    memcpy(&dValue, response, sizeof(double));
                    printf("->in double: %lf\n", dValue);
                } else if (dwSize == 4) {
                    DWORD dwordValue;
                    memcpy(&dwordValue, response, sizeof(DWORD));
                    printf("->in double: %lf\n", (double)dwordValue);
                } else {
                    printf("->in double: (not applicable for size %lu)\n", dwSize);
                }
                if (dwSize >= 4) {
                    DWORD dwordValue;
                    memcpy(&dwordValue, response, sizeof(DWORD));
                    printf("->in int32: %lu\n", dwordValue);
                } else if(dwSize >= 2) {
                    DWORD dwordValue;
                    memcpy(&dwordValue, response, sizeof(DWORD));
                    printf("->in int16: %u\n", dwordValue);
                }
                else {
                    printf("->in 16/32-bit int: (not applicable for size %lu)\n", dwSize);
                }
                printf("->in 4 digits BCD format: ");
                if (dwSize >= 2) {
					// Assume the BCD is in the first 2 bytes of response
					unsigned short bcd = *(unsigned short*)response;
					int d1 = (bcd >> 12) & 0xF;
					int d2 = (bcd >> 8) & 0xF;
					int d3 = (bcd >> 4) & 0xF;
					int d4 = bcd & 0xF;
					printf("%d%d%d%d\n", d1, d2, d3, d4);
				} else {
					printf("(not enough data)\n");
				} 
                printf("->in bits from lo to hi: "); printBits(response, dwSize); printf("\n");
            }
            printf("\n");
		}
    }
    else
    {
        printf("UIPC: Failed to open link to FSUIPC\n%s\n", pszErrors[dwResult]);
    }

    FSUIPC_Close();
    return 0;
}
