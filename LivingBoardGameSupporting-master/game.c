#include "game.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

class_entry classes[NUM_CLASSES] = {
    [0] = {0},
    [DOG] = {
        .title = "--MIGHTY DOG--",
        .body = " Once a loyal companion, he now scoures the wastelands to resurect his owner.",
        .portrait = DOG,
        .stats = {
            .health = 8,
            .armor = 3,
            .gold = 100,
            .attack = 5,
            .speed = 3,
            .view = 1,
            .special = 0
        }
    },
    [SCIENTIST] = {
        .title = "-SCIENCE MAN--",
        .body = "What wonders await in the fallout? What new creatures are there to be discoverd?",
        .portrait = SCIENTIST,
        .stats = {
            .health = 5,
            .armor = 2,
            .gold = 800,
            .attack = 4,
            .speed = 3,
            .view = 4,
            .special = 0
        }
    },
    [GOBLIN] = {
        .title = "----GOBLIN----",
        .body = "One of the creatures formed from the conditions of the fallout. Heart of and heart for gold",
        .portrait = GOBLIN,
        .stats = {
            .health = 6,
            .armor = 3,
            .gold = 600,
            .attack = 6,
            .speed = 4,
            .view = 2,
            .special = 0
        }
    },
    [ROBOT] = {
        .title = "---IRON MAN---",
        .body = "The fallout had little effect on its body, only its mind. Allowing it to bypass its programming.",
        .portrait = ROBOT,
        .stats = {
            .health = 12,
            .armor = 5,
            .gold = 0,
            .attack = 4,
            .speed = 1,
            .view = 3,
            .special = 0
        }
    },
    [MUTANT] = {
        .title = " ---MUTANT--- ",
        .body = "An unfortunate victim of the fallout. Left him with usual strength and abilities.",
        .portrait = MUTANT,
        .stats = {
            .health = 15,
            .armor = 0,
            .gold = 300,
            .attack = 6,
            .speed = 2,
            .view = 0,
            .special = 0
        }
    },
    [STRAW] = {
        .title = "--SCARECROW-- ",
        .body = "Through the power of friendship (and radiation), the lovable scarecrow was brought to life.",
        .portrait = STRAW,
        .stats = {
            .health = 4,
            .armor = 5,
            .gold = 300,
            .attack = 7,
            .speed = 3,
            .view = 2,
            .special = 0
        }
    }
};


void fill_stats(uint8_t *text_buff, stat_block stats){
    sprintf(text_buff, "SPEED:%-1d VIEW:%-1dHEALTH:%-+7dATTACK:%-+7dARMOR:%-+8dGOLD:%-+9dSPECIAL:0x%-+06x", stats.speed, stats.view, stats.health, stats.attack, stats.armor, stats.gold, stats.special);
}

// Will need to modify
void add_stats(stat_block *end_stats, stat_block *changing_stats){
    end_stats->view += changing_stats->view;
    end_stats->speed += changing_stats->speed;
    end_stats->health += changing_stats->health;
    //end_stats->gold += changing_stats->gold;
    end_stats->armor += changing_stats->armor;
    end_stats->attack += changing_stats->attack;
}

// Will need to modify
void sub_stats(stat_block *end_stats, stat_block *changing_stats){
    end_stats->view -= changing_stats->view;
    end_stats->speed -= changing_stats->speed;
    end_stats->health -= changing_stats->health;
    //end_stats->gold -= changing_stats->gold;
    end_stats->armor -= changing_stats->armor;
    end_stats->attack -= changing_stats->attack;
}