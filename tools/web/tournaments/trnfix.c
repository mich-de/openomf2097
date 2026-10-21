// Re-saves a fan-made tournament for the web build's resources:
//
//   trnfix <in.trn> <out.TRN> <pic> <bk>
//
// <pic>: the PIC the tournament's photos are in (renamed with the tournament, so that no two
// tournaments, or the original four, share a name); <bk>: its BK (WORLD.BK where the author's
// own is not in its download). Everything else stays as the author made it.
#include <stdbool.h>
#include <stdio.h>

#include "formats/error.h"
#include "formats/pic.h"
#include "formats/tournament.h"
#include "utils/path.h"

// The tournament loader asks the mod manager for extra photos; there are none here.
bool modmanager_get_player_pics(sd_pic_file *pic) {
    (void)pic;
    return false;
}

int main(int argc, char **argv) {
    if(argc != 5) {
        fprintf(stderr, "usage: %s <in.trn> <out.TRN> <pic> <bk>\n", argv[0]);
        return 1;
    }
    sd_tournament_file trn;
    sd_tournament_create(&trn);
    path p;
    path_from_c(&p, argv[1]);
    if(sd_tournament_load(&trn, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", argv[1]);
        return 1;
    }
    sd_tournament_set_pic_name(&trn, argv[3]);
    sd_tournament_set_bk_name(&trn, argv[4]);
    path_from_c(&p, argv[2]);
    if(sd_tournament_save(&trn, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot write %s\n", argv[2]);
        return 1;
    }
    sd_tournament_free(&trn);
    return 0;
}
