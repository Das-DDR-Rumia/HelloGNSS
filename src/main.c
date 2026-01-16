#include <stdio.h>
#include "rtklib.h"

int main() {
    // Print RTKLIB version and copyright info
    printf("Hello GNSS! Using RTKLIB version %s %s\n", VER_RTKLIB, PATCH_LEVEL);
    printf("%s\n\n", COPYRIGHT_RTKLIB);

    // Create and initialize a basic positioning solution
    sol_t sol = {0};
    
    // Display some RTKLIB constants
    printf("GNSS constants from RTKLIB:\n");
    printf("- Speed of light: %.0f m/s\n", CLIGHT);
    printf("- WGS84 earth radius: %.1f m\n", RE_WGS84);
    printf("- GPS L1 frequency: %.1f MHz\n", FREQ1 / 1e6);
    
    // Show supported navigation systems
    printf("\nSupported Navigation Systems:\n");
    printf("- GPS (0x%02X)\n", SYS_GPS);
    printf("- GLONASS (0x%02X)\n", SYS_GLO);
    printf("- Galileo (0x%02X)\n", SYS_GAL);
    printf("- BeiDou (0x%02X)\n", SYS_CMP);
    printf("- QZSS (0x%02X)\n", SYS_QZS);
    printf("- SBAS (0x%02X)\n", SYS_SBS);
    
    // Get current time in GPST
    gtime_t current_time = timeget();
    double ep[6];
    time2epoch(current_time, ep);
    
    char time_str[64];
    time2str(current_time, time_str, 0);
    
    printf("\nCurrent time (GPST): %s\n", time_str);
    printf("Epoch: %.0f/%.0f/%.0f %.0f:%.0f:%.1f\n", 
           ep[0], ep[1], ep[2], ep[3], ep[4], ep[5]);
    
    return 0;
}

