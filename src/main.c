#include <stdio.h>
#include "rtklib.h"

const char* sys2str(int sys) {
    switch (sys) {
        case SYS_GPS: return "GPS";
        case SYS_GLO: return "GLO";
        case SYS_GAL: return "GAL";
        case SYS_QZS: return "QZS";
        case SYS_CMP: return "BDS";
        case SYS_SBS: return "SBS";
        default:      return "UNK";
    }
}

const char* solq2str(int stat) {
    switch (stat) {
        case SOLQ_NONE:   return "No Solution";
        case SOLQ_FIX:    return "Fix";
        case SOLQ_FLOAT:  return "Float";
        case SOLQ_SBAS:   return "SBAS";
        case SOLQ_DGPS:   return "DGPS";
        case SOLQ_SINGLE: return "Single";
        case SOLQ_PPP:    return "PPP";
        case SOLQ_DR:     return "Dead Reckoning";
        default:          return "Unknown";
    }
}

void print_solution(const sol_t *sol, const double *azel, const ssat_t *ssat, int nsat) {
    double pos[3], enu[3];
    char time_str[64];
    
    time2str(sol->time, time_str, 3);
    
    printf("\n==================== SOLUTION ====================\n");
    printf("Time: %s\n", time_str);
    printf("Status: %s (code=%d)\n", solq2str(sol->stat), sol->stat);
    printf("Valid satellites: %d\n", sol->ns);
    
    if (sol->stat == SOLQ_NONE) {
        printf("No valid solution!\n");
        return;
    }
    
    printf("\n--- ECEF Coordinates ---\n");
    printf("X: %14.4f m\n", sol->rr[0]);
    printf("Y: %14.4f m\n", sol->rr[1]);
    printf("Z: %14.4f m\n", sol->rr[2]);
    
    ecef2pos(sol->rr, pos);
    printf("\n--- Geodetic Coordinates (WGS84) ---\n");
    printf("Latitude:  %14.9f deg (%s)\n", pos[0] * R2D, pos[0] >= 0 ? "N" : "S");
    printf("Longitude: %14.9f deg (%s)\n", pos[1] * R2D, pos[1] >= 0 ? "E" : "W");
    printf("Height:    %14.4f m\n", pos[2]);
    
    printf("\n--- Position Variance (m^2) ---\n");
    printf("Var_XX: %.6f, Var_YY: %.6f, Var_ZZ: %.6f\n", 
           sol->qr[0], sol->qr[1], sol->qr[2]);
    printf("Var_XY: %.6f, Var_YZ: %.6f, Var_ZX: %.6f\n", 
           sol->qr[3], sol->qr[4], sol->qr[5]);
    
    double std_x = sqrt(sol->qr[0]);
    double std_y = sqrt(sol->qr[1]);
    double std_z = sqrt(sol->qr[2]);
    double std_3d = sqrt(sol->qr[0] + sol->qr[1] + sol->qr[2]);
    printf("\n--- Position Std Dev (m) ---\n");
    printf("Std_X: %.4f, Std_Y: %.4f, Std_Z: %.4f\n", std_x, std_y, std_z);
    printf("3D Std: %.4f m\n", std_3d);
    
    printf("\n--- Receiver Clock Bias ---\n");
    printf("GPS:     %14.9f s (%10.3f m)\n", sol->dtr[0], sol->dtr[0] * CLIGHT);
    printf("GLONASS: %14.9f s (%10.3f m)\n", sol->dtr[1], sol->dtr[1] * CLIGHT);
    printf("Galileo: %14.9f s (%10.3f m)\n", sol->dtr[2], sol->dtr[2] * CLIGHT);
    printf("BeiDou:  %14.9f s (%10.3f m)\n", sol->dtr[3], sol->dtr[3] * CLIGHT);
    
    if (ssat && nsat > 0) {
        printf("\n--- Satellite Status ---\n");
        printf("%-4s %-6s %8s %8s %10s %10s %6s\n", 
               "SAT", "SYS", "AZ(deg)", "EL(deg)", "Res_P(m)", "Res_L(m)", "Valid");
        
        int printed = 0;
        for (int i = 0; i < MAXSAT && printed < 20; i++) {
            if (ssat[i].vs) {
                char sat_id[8];
                int prn;
                int sys = satsys(i + 1, &prn);
                satno2id(i + 1, sat_id);
                
                printf("%-4s %-6s %8.2f %8.2f %10.4f %10.4f %6s\n",
                       sat_id, sys2str(sys),
                       ssat[i].azel[0] * R2D,
                       ssat[i].azel[1] * R2D,
                       ssat[i].resp[0],
                       ssat[i].resc[0],
                       ssat[i].vsat[0] ? "YES" : "NO");
                printed++;
            }
        }
    }
}

void process_spp(obs_t *obs, nav_t *nav, prcopt_t *opt, int max_epochs) {
    sol_t sol = {0};
    double azel[MAXOBS * 2] = {0};
    ssat_t ssat[MAXSAT] = {0};
    char msg[128] = "";
    
    int epoch_count = 0;
    int success_count = 0;
    int i = 0;
    
    double *all_pos = (double*)malloc(max_epochs * 3 * sizeof(double));
    int valid_count = 0;
    
    printf("\n============ SINGLE POINT POSITIONING ============\n");
    
    while (i < obs->n && epoch_count < max_epochs) {
        int n = 0;
        gtime_t time = obs->data[i].time;
        
        while (i + n < obs->n && 
               fabs(timediff(obs->data[i + n].time, time)) < DTTOL) {
            n++;
        }
        
        if (n > 0) {
            char time_str[64];
            time2str(time, time_str, 3);
            
            memset(ssat, 0, sizeof(ssat));
            memset(&sol, 0, sizeof(sol));
            msg[0] = '\0';
            
            int ret = pntpos(&obs->data[i], n, nav, opt, &sol, azel, ssat, msg);
            
            printf("\n----- Epoch #%d: %s -----\n", epoch_count + 1, time_str);
            printf("Observations in epoch: %d\n", n);
            printf("pntpos return: %d\n", ret);
            
            if (msg[0]) {
                printf("Message: %s\n", msg);
            }
            
            if (ret && sol.stat != SOLQ_NONE) {
                print_solution(&sol, azel, ssat, MAXSAT);
                success_count++;
                
                if (valid_count < max_epochs) {
                    all_pos[valid_count * 3 + 0] = sol.rr[0];
                    all_pos[valid_count * 3 + 1] = sol.rr[1];
                    all_pos[valid_count * 3 + 2] = sol.rr[2];
                    valid_count++;
                }
            } else {
                printf("Positioning failed!\n");
            }
            
            epoch_count++;
        }
        
        i += n;
    }
    
    printf("\n============ POSITIONING SUMMARY ============\n");
    printf("Total epochs processed: %d\n", epoch_count);
    printf("Successful solutions:   %d\n", success_count);
    printf("Success rate:           %.1f%%\n", 
           epoch_count > 0 ? 100.0 * success_count / epoch_count : 0.0);
    
    if (valid_count > 0) {
        double mean_pos[3] = {0};
        double pos_llh[3];
        
        for (int j = 0; j < valid_count; j++) {
            mean_pos[0] += all_pos[j * 3 + 0];
            mean_pos[1] += all_pos[j * 3 + 1];
            mean_pos[2] += all_pos[j * 3 + 2];
        }
        mean_pos[0] /= valid_count;
        mean_pos[1] /= valid_count;
        mean_pos[2] /= valid_count;
        
        double std_pos[3] = {0};
        for (int j = 0; j < valid_count; j++) {
            std_pos[0] += pow(all_pos[j * 3 + 0] - mean_pos[0], 2);
            std_pos[1] += pow(all_pos[j * 3 + 1] - mean_pos[1], 2);
            std_pos[2] += pow(all_pos[j * 3 + 2] - mean_pos[2], 2);
        }
        if (valid_count > 1) {
            std_pos[0] = sqrt(std_pos[0] / (valid_count - 1));
            std_pos[1] = sqrt(std_pos[1] / (valid_count - 1));
            std_pos[2] = sqrt(std_pos[2] / (valid_count - 1));
        }
        
        ecef2pos(mean_pos, pos_llh);
        
        printf("\n--- Mean Position (from %d solutions) ---\n", valid_count);
        printf("ECEF X: %14.4f m (std: %.4f m)\n", mean_pos[0], std_pos[0]);
        printf("ECEF Y: %14.4f m (std: %.4f m)\n", mean_pos[1], std_pos[1]);
        printf("ECEF Z: %14.4f m (std: %.4f m)\n", mean_pos[2], std_pos[2]);
        printf("\nGeodetic (WGS84):\n");
        printf("Latitude:  %14.9f deg\n", pos_llh[0] * R2D);
        printf("Longitude: %14.9f deg\n", pos_llh[1] * R2D);
        printf("Height:    %14.4f m\n", pos_llh[2]);
        printf("\n3D position std: %.4f m\n", 
               sqrt(std_pos[0]*std_pos[0] + std_pos[1]*std_pos[1] + std_pos[2]*std_pos[2]));
    }
    
    free(all_pos);
}

void set_default_prcopt(prcopt_t *opt) {
    *opt = prcopt_default;
    
    opt->mode = PMODE_SINGLE;
    opt->navsys = SYS_GPS | SYS_GLO | SYS_GAL | SYS_CMP;
    opt->nf = 1;
    opt->elmin = 10.0 * D2R;
    opt->sateph = EPHOPT_BRDC;
    opt->ionoopt = IONOOPT_BRDC;
    opt->tropopt = TROPOPT_SAAS;
    opt->dynamics = 0;
    opt->tidecorr = 0;
    
    opt->eratio[0] = 100.0;
    opt->err[1] = 0.003;
    opt->err[2] = 0.003;
    opt->err[3] = 0.0;
    opt->err[4] = 1.0;
}

int main() {
    printf("Hello GNSS! Using RTKLIB version %s %s\n", VER_RTKLIB, PATCH_LEVEL);
    printf("%s\n\n", COPYRIGHT_RTKLIB);

    printf("GNSS constants from RTKLIB:\n");
    printf("- Speed of light: %.0f m/s\n", CLIGHT);
    printf("- WGS84 earth radius: %.1f m\n", RE_WGS84);
    printf("- GPS L1 frequency: %.1f MHz\n", FREQ1 / 1e6);

    printf("\nSupported Navigation Systems:\n");
    printf("- GPS (0x%02X)\n", SYS_GPS);
    printf("- GLONASS (0x%02X)\n", SYS_GLO);
    printf("- Galileo (0x%02X)\n", SYS_GAL);
    printf("- BeiDou (0x%02X)\n", SYS_CMP);
    
    char obs_file[260] = "";
    char nav_file[260] = "";

    printf("\nEnter observation file path: ");
    scanf_s("%259s", obs_file, (unsigned)_countof(obs_file));
    printf("Enter navigation file path: ");
    scanf_s("%259s", nav_file, (unsigned)_countof(nav_file));

    obs_t obs = {0};
    nav_t nav = {0};
    sta_t sta = {0};
    
    printf("\nReading observation file: %s\n", obs_file);
    int ret1 = readrnx(obs_file, 1, "", &obs, &nav, &sta);
    printf("Result: %d (obs=%d, nav=%d)\n", ret1, obs.n, nav.n);
    
    printf("Reading navigation file: %s\n", nav_file);
    int ret2 = readrnx(nav_file, 1, "", &obs, &nav, NULL);
    printf("Result: %d (obs=%d, nav=%d)\n", ret2, obs.n, nav.n);
    
    if (obs.n == 0) {
        printf("Error: No observation data!\n");
        return -1;
    }
    if (nav.n == 0 && nav.ng == 0) {
        printf("Error: No navigation data!\n");
        return -1;
    }
    
    sortobs(&obs);
    
    uniqnav(&nav);
    
    printf("\nAfter sorting: %d observations, %d GPS eph, %d GLO eph\n", 
           obs.n, nav.n, nav.ng);
    
    if (sta.name[0]) {
        printf("\n--- Station Info ---\n");
        printf("Name: %s\n", sta.name);
        printf("Marker: %s\n", sta.marker);
        printf("Receiver: %s %s\n", sta.rectype, sta.recver);
        printf("Antenna: %s\n", sta.antdes);
        if (norm(sta.pos, 3) > 0) {
            double pos_llh[3];
            ecef2pos(sta.pos, pos_llh);
            printf("Approx Position: %.4f, %.4f, %.4f (ECEF)\n", 
                   sta.pos[0], sta.pos[1], sta.pos[2]);
            printf("                 %.9f, %.9f, %.4f (LLH)\n",
                   pos_llh[0] * R2D, pos_llh[1] * R2D, pos_llh[2]);
        }
    }
    
    prcopt_t opt;
    set_default_prcopt(&opt);
    
    printf("\n--- Processing Options ---\n");
    printf("Mode: Single Point Positioning\n");
    printf("Navigation Systems: GPS%s%s%s\n",
           (opt.navsys & SYS_GLO) ? "+GLO" : "",
           (opt.navsys & SYS_GAL) ? "+GAL" : "",
           (opt.navsys & SYS_CMP) ? "+BDS" : "");
    printf("Elevation mask: %.1f deg\n", opt.elmin * R2D);
    printf("Ionosphere: %s\n", opt.ionoopt == IONOOPT_BRDC ? "Broadcast" : "Off");
    printf("Troposphere: %s\n", opt.tropopt == TROPOPT_SAAS ? "Saastamoinen" : "Off");
    
    int max_epochs = 10;
    printf("\nProcessing up to %d epochs...\n", max_epochs);
    
    process_spp(&obs, &nav, &opt, max_epochs);
    
    freeobs(&obs);
    freenav(&nav, 0xFF);
    
    printf("\n============ END OF PROCESSING ============\n");
    
    return 0;
}
