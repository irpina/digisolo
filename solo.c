/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SOLO & MUTE GROUPS for the Digitone mk1 and Digitone Keys (core-dn1 5.0.1), on the stock global mutes, the ones
 * FUNC + a track key toggles (bit t of core_mutes() for track t: 0-3 the synth tracks, 4-7 the MIDI tracks).
 *
 * Solo: hold a track key and press FUNC (FUNC first and then a track key is the stock mute, untouched). The track
 * plays alone among its four: every other synth track is muted, or every other MIDI track for a MIDI track, and
 * the solo track is unmuted. The same again (that track held, FUNC) gives the four their mutes back as they were
 * before the solo. A mute the stock sets meanwhile (FUNC + a track key) ends the solo where it is: nothing is
 * given back. SOLO in the Mod Menu solos the held track the same way. A solo's FUNC press and release are taken,
 * so the FUNC overlay does not flash.
 *
 * Mute groups: MUTE GROUPS in the Mod Menu opens RECALL 1-4 and STORE 1-4. STORE n keeps the eight tracks' mutes
 * as group n; RECALL n sets them (a group never stored does nothing). The groups are saved with the project
 * (project data "MUTG"). Each action shows a word in the screen's corner for a second. */
#include "digitone-mk1/core3.h"

typedef void (*rect_fn)(void *bmp, int x0, int y0, int x1, int y1, int colour);
typedef void (*text_fn)(void *bmp, const void *font, int x, int y, int maxlen, const char *fmt, ...);

#define KEY_FUNC  1
#define KEY_T1    42
#define GROUPS    4

struct save {
    uint32_t stored;                    /* bit n: group n was stored */
    uint32_t mask[GROUPS];              /* the eight tracks' mutes, bit t */
};
static struct save save;

static int32_t held_key;                /* the track key down on its own, or 0 */
static int32_t func_taken;              /* FUNC's press was a solo: take its repeats and release */
static int32_t soloed = -1;             /* the solo track, or -1 */
static uint32_t before;                 /* the mutes before the solo */
static int32_t busy;                    /* our own mute writes: solo_mute ignores them */
static const char *note;                /* what the corner shows, and for how long */
static int32_t note_arg, note_ticks;

static void say(const char *what, int32_t arg)
{
    note = what;
    note_arg = arg;
    note_ticks = 30;
}

/* Every track of `group` (0 the synth tracks, 4 the MIDI tracks) set to its bit of `mask`, where it differs. */
static void apply(uint32_t mask, int32_t group)
{
    uint32_t now = core_mutes();
    busy = 1;
    for (int32_t t = group; t < group + 4; t++)
        if (((now ^ mask) >> t) & 1)
            core_mute_set(t, (mask >> t) & 1);
    busy = 0;
}

static void solo(int32_t track)
{
    int32_t group = track & 4;
    if (track < 0 || track > 7)
        return;
    if (soloed == track) {              /* the same solo again: the mutes back */
        apply(before, group);
        soloed = -1;
        say("UNSOLO", 0);
        return;
    }
    if (soloed >= 0 && (soloed & 4) != group) {
        apply(before, soloed & 4);      /* a solo in the other four ends first */
        soloed = -1;
    }
    if (soloed < 0)
        before = core_mutes();
    apply(0xffu & ~(1u << track), group);
    soloed = track;
    say("SOLO T%d", (track & 3) + 1);
}

int32_t solo_key(void *brain, void *ev)
{
    int32_t key = CORE_KEY_ID(ev);
    uint32_t flags = CORE_KEY_FLAGS(ev);
    (void)brain;
    if (key >= KEY_T1 && key < KEY_T1 + 4) {
        if ((flags & CORE_KEY_UP) || !(flags & CORE_KEY_DOWN))
            held_key = 0;               /* released (flags 0 when FUNC went down between) */
        else if (!(flags & (CORE_KEY_FUNC | CORE_KEY_REPEAT)))
            held_key = key;
        return 0;
    }
    if (key != KEY_FUNC)
        return 0;
    if (func_taken) {                   /* the solo's FUNC: its repeats and release */
        if ((flags & CORE_KEY_UP) || !(flags & CORE_KEY_DOWN))
            func_taken = 0;
        return 1;
    }
    if ((flags & CORE_KEY_DOWN) && !(flags & CORE_KEY_REPEAT) && held_key) {
        solo(fw_active_track);          /* the track key selected its track as it went down */
        func_taken = 1;
        return 1;
    }
    return 0;
}

/* A mute the stock sets ends a solo where it is. */
void solo_mute(int32_t where, int32_t track, int32_t muted, int32_t row)
{
    (void)track; (void)muted; (void)row;
    if (!busy && where == CORE_MUTE_GLOBAL)
        soloed = -1;
}

/* ---- the Mod Menu ---------------------------------------------------------------------------- */
struct item {
    const char *name;
    void (*open)(void *brain, void *ev, int32_t track);
    unsigned long tag;
    const unsigned short *icon;
};

static const unsigned short solo_icon[16] = {          /* one bar up, three down */
    0x0000, 0x0000, 0x0700, 0x0700, 0x0700, 0x0700, 0x0700, 0x0700,
    0x0700, 0x0700, 0x0700, 0x7777, 0x7777, 0x0000, 0xffff, 0x0000
};
static const unsigned short groups_icon[16] = {        /* four boxes, two filled */
    0x0000, 0x7c7c, 0x7c44, 0x7c44, 0x7c44, 0x7c7c, 0x0000, 0x0000,
    0x7c7c, 0x447c, 0x447c, 0x447c, 0x7c7c, 0x0000, 0xffff, 0x0000
};
static const unsigned short store_icon[16] = {         /* an arrow into a box */
    0x0180, 0x0180, 0x0180, 0x0990, 0x05a0, 0x03c0, 0x0180, 0x0000,
    0x7ffe, 0x4002, 0x4002, 0x4002, 0x7ffe, 0x0000, 0xffff, 0x0000
};

static void solo_open(void *brain, void *event, int32_t track)
{
    (void)brain; (void)event; (void)track;
    solo(fw_active_track);
}

static void groups_open(void *brain, void *event, int32_t track);
const struct item solo_menu = { "SOLO", solo_open, CORE_MENU_ICON, solo_icon };
const struct item solo_groups_menu = { "MUTE GROUPS", groups_open, CORE_MENU_ICON, groups_icon };

static const struct item group_items[2 * GROUPS] = {
    { "RECALL 1", 0, CORE_MENU_ICON, groups_icon }, { "RECALL 2", 0, CORE_MENU_ICON, groups_icon },
    { "RECALL 3", 0, CORE_MENU_ICON, groups_icon }, { "RECALL 4", 0, CORE_MENU_ICON, groups_icon },
    { "STORE 1", 0, CORE_MENU_ICON, store_icon }, { "STORE 2", 0, CORE_MENU_ICON, store_icon },
    { "STORE 3", 0, CORE_MENU_ICON, store_icon }, { "STORE 4", 0, CORE_MENU_ICON, store_icon },
};
static const void *const group_list[2 * GROUPS + 1] = {
    &group_items[0], &group_items[1], &group_items[2], &group_items[3],
    &group_items[4], &group_items[5], &group_items[6], &group_items[7], 0
};

static void group_pick(void *brain, void *event, int32_t track, int32_t index)
{
    (void)brain; (void)event; (void)track;
    if (index >= GROUPS) {
        index -= GROUPS;
        save.mask[index] = core_mutes() & 0xff;
        save.stored |= 1u << index;
        say("STORED %d", index + 1);
    } else if ((save.stored >> index) & 1) {
        soloed = -1;
        apply(save.mask[index], 0);
        apply(save.mask[index], 4);
        say("GROUP %d", index + 1);
    } else {
        say("EMPTY %d", index + 1);
    }
}

static void groups_open(void *brain, void *event, int32_t track)
{
    (void)brain; (void)event; (void)track;
    core_menu_open(group_list, 0, group_pick);
}

/* ---- the project's groups -------------------------------------------------------------------- */
static void solo_loaded(int32_t found)
{
    (void)found;                        /* none found: save holds zeros, no group stored */
    soloed = -1;
}
const struct core_projdata solo_projdata = { 0x4d555447u /* "MUTG" */, sizeof(save), &save, solo_loaded };

/* ---- the corner ------------------------------------------------------------------------------ */
void solo_tick(void *ctrl)
{
    if (note_ticks > 0) {
        note_ticks--;
        *((unsigned char *)ctrl + 0x20) = 1;
    }
}

void solo_draw(void *bmp, void *ctrl)
{
    (void)ctrl;
    if (note_ticks <= 0 || !note)
        return;
    ((rect_fn)fw_fillrect)(bmp, 84, 54, 127, 63, 0);
    ((text_fn)fw_textf)(bmp, (const void *)fw_font5, 87, 57, -1, note, (int)note_arg);
    ((rect_fn)fw_fillrect)(bmp, 84, 54, 127, 63, -1);
}
