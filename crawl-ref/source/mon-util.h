/**
 * @file
 * @brief Misc monster related functions.
**/

#ifndef MONUTIL_H
#define MONUTIL_H

#include "enum.h"
#include "mon-enum.h"
#include "player.h"

struct bolt;

struct mon_attack_def
{
    attack_type     type;
    attack_flavour  flavour;
    int             damage;

    static mon_attack_def attk(int dam,
                               attack_type typ = AT_HIT,
                               attack_flavour flav = AF_PLAIN)
    {
        mon_attack_def def = { typ, flav, dam };
        return def;
    }
};

// Amount of mons->speed_increment used by different actions; defaults
// to 10.
#define DEFAULT_ENERGY {10, 10, 10, 10, 10, 10, 10, 100}
struct mon_energy_usage
{
    uint8_t move;
    uint8_t swim;
    uint8_t attack;
    uint8_t missile; // Arrows/crossbows/etc
    uint8_t spell;
    uint8_t special;
    uint8_t item;    // Using an item (i.e., drinking a potion)

    // Percent of mons->speed used when picking up an item; defaults
    // to 100%
    uint8_t pickup_percent;

    static mon_energy_usage attack_cost(int cost, int sw = 10)
    {
        mon_energy_usage me = DEFAULT_ENERGY;
        me.attack = cost;
        me.swim = sw;
        return me;
    }

    static mon_energy_usage missile_cost(int cost)
    {
        mon_energy_usage me = DEFAULT_ENERGY;
        me.missile = cost;
        return me;
    }

    static mon_energy_usage swim_cost (int cost)
    {
        mon_energy_usage me = DEFAULT_ENERGY;
        me.swim = cost;
        return me;
    }

    static mon_energy_usage move_cost(int mv, int sw = 10)
    {
        mon_energy_usage me = DEFAULT_ENERGY;
        me.move = mv;
        me.swim = sw;
        return me;
    }

    mon_energy_usage operator | (const mon_energy_usage &o) const
    {
        mon_energy_usage me;
        me.move = combine(move, o.move);
        me.swim = combine(swim, o.swim);
        me.attack = combine(attack, o.attack);
        me.missile = combine(missile, o.missile);
        me.spell = combine(spell, o.spell);
        me.special = combine(special, o.special);
        me.item = combine(item, o.item);
        me.pickup_percent = combine(pickup_percent, o.pickup_percent, 100);
        return me;
    }

    bool operator == (const mon_energy_usage &rvalue) const
    {
        return move == rvalue.move
               && swim == rvalue.swim
               && attack == rvalue.attack
               && missile == rvalue.missile
               && spell == rvalue.spell
               && special == rvalue.special
               && item == rvalue.item
               && pickup_percent == rvalue.pickup_percent;
    }
private:
    static uint8_t combine(uint8_t a, uint8_t b, uint8_t def = 10)
    {
        return b != def? b : a;
    }
};

struct monsterentry
{
    short mc;            // monster number

    char basechar;
    colour_t colour;
    const char *name;

    monclass_flags_t bitfields;
    resists_t resists;

    // Multiplier for calculated monster XP value; see exper_value() for use.
    int8_t exp_mod;

    monster_type genus,         // "team" the monster plays for
                 species;       // corpse type of the monster

    mon_holy_type holiness;

    short resist_magic;  // (positive sets value, negative is relative to hd)

    // max damage in a turn is total of these four?
    mon_attack_def attack[MAX_NUM_ATTACKS];

    /// Similar to player level; used for misc purposes.
    int HD;
    /// Average hp; multiplied by 10 for precision.
    int avg_hp_10x;

    int8_t AC; // armour class
    int8_t ev; // evasion
    int sec;   // actually mon_spellbook_type
    corpse_effect_type corpse_thingy;
    shout_type         shouts;
    mon_intel_type     intel;
    habitat_type     habitat;
    int8_t           speed;        // How quickly speed_increment increases
    mon_energy_usage energy_usage; // And how quickly it decreases
    mon_itemuse_type gmon_use;
    size_type size;
    mon_body_shape shape;
};

enum mon_threat_level_type
{
    MTHRT_TRIVIAL,
    MTHRT_EASY,
    MTHRT_TOUGH,
    MTHRT_NASTY,
    MTHRT_UNDEF,
};

void set_resist(resists_t &all, mon_resist_flags res, int lev);

// In all cases this will be simplified to a bit field access, so let's let
// the compiler inline it.
static inline int get_resist(resists_t all, mon_resist_flags res)
{
    if (res > MR_LAST_MULTI)
        return all & res ? 1 : 0;
    int v = (all / res) & 7;
    if (v > 4)
        return v - 8;
    return v;
}

dungeon_feature_type habitat2grid(habitat_type ht);

monsterentry *get_monster_data(monster_type mc) IMMUTABLE;
int get_mons_class_ac(monster_type mc) IMMUTABLE;
int get_mons_class_ev(monster_type mc) IMMUTABLE;
resists_t get_mons_class_resists(monster_type mc) IMMUTABLE;
resists_t get_mons_resists(const monster* mon);
int get_mons_resist(const monster* mon, mon_resist_flags res);
const bool monster_resists_this_poison(const monster* mons, bool force = false);

void init_monsters();
void init_monster_symbols();

monster *monster_at(const coord_def &pos);

// this is the old moname()
string mons_type_name(monster_type type, description_level_type desc);

bool give_monster_proper_name(monster* mon, bool orcs_only = true);

bool mons_flattens_trees(const monster* mon);
size_type mons_class_body_size(monster_type mc);
bool mons_class_res_wind(monster_type mc);

mon_itemuse_type mons_class_itemuse(monster_type mc);
mon_itemuse_type mons_itemuse(const monster* mon);

bool mons_can_be_blinded(monster_type mc);
bool mons_can_be_dazzled(monster_type mc);

int get_shout_noise_level(const shout_type shout);
shout_type mons_shouts(monster_type mclass, bool demon_shout = false);

bool mons_is_ghost_demon(monster_type mc);
bool mons_is_unique(monster_type mc);
bool mons_is_or_was_unique(const monster& mon);
bool mons_is_pghost(monster_type mc);
bool mons_is_draconian_job(monster_type mc);
bool mons_is_demonspawn_job(monster_type mc);
bool mons_is_job(monster_type mc);
bool mons_is_hepliaklqana_ancestor(monster_type mc);

int mutant_beast_tier(int xl);

int mons_avg_hp(monster_type mc);
int mons_max_hp(monster_type mc, monster_type mbase_typeg = MONS_NO_MONSTER);
int exper_value(const monster* mon, bool real = true);

int hit_points(int avg_hp, int scale = 10);

int mons_class_hit_dice(monster_type mc);
int mons_class_res_magic(monster_type type, monster_type base);
bool mons_class_sees_invis(monster_type type, monster_type base);

bool mons_immune_magic(const monster* mon);

mon_attack_def mons_attack_spec(const monster* mon, int attk_number, bool base_flavour = false);

corpse_effect_type mons_corpse_effect(monster_type mc);

bool mons_class_flag(monster_type mc, monclass_flags_t bits);

mon_holy_type holiness_by_name(string name);
const char * holiness_name(mon_holy_type_flags which_holiness);
string holiness_description(mon_holy_type holiness);
mon_holy_type mons_class_holiness(monster_type mc);

void discover_mimic(const coord_def& pos);
void discover_shifter(monster* shifter);

bool mons_is_statue(monster_type mc);
bool mons_is_demon(monster_type mc);
bool mons_is_draconian(monster_type mc);
bool mons_is_base_draconian(monster_type mc);
bool mons_is_demonspawn(monster_type mc);
bool mons_is_conjured(monster_type mc);
bool mons_is_beast(monster_type mc);
bool mons_is_avatar(monster_type mc);
int mons_demon_tier(monster_type mc);

bool mons_class_wields_two_weapons(monster_type mc);
bool mons_wields_two_weapons(const monster* m);
bool mons_self_destructs(const monster* m);

mon_intel_type mons_class_intel(monster_type mc);
mon_intel_type mons_intel(const monster* mon);

// Use mons_habitat() and mons_primary_habitat() wherever possible,
// since the class variants do not handle zombies correctly.
habitat_type mons_habitat(const monster* mon, bool real_amphibious = false);
habitat_type mons_class_primary_habitat(monster_type mc);
habitat_type mons_primary_habitat(const monster* mon);
habitat_type mons_class_secondary_habitat(monster_type mc);
habitat_type mons_secondary_habitat(const monster* mon);

bool intelligent_ally(const monster* mon);

bool mons_skeleton(monster_type mc);
bool mons_zombifiable(monster_type mc);

int max_corpse_chunks(monster_type mc);
int mons_class_base_speed(monster_type mc);
mon_energy_usage mons_class_energy(monster_type mc);
mon_energy_usage mons_energy(const monster* mon);
int mons_class_zombie_base_speed(monster_type zombie_base_mc);
int mons_base_speed(const monster* mon, bool known = false);

bool mons_class_can_regenerate(monster_type mc);
bool mons_can_regenerate(const monster* mon);
bool mons_class_fast_regen(monster_type mc);
int mons_zombie_size(monster_type mc);
monster_type mons_zombie_base(const monster* mon);
bool mons_class_is_zombified(monster_type mc);
bool mons_class_is_hybrid(monster_type mc);
bool mons_class_is_animated_weapon(monster_type type);
monster_type mons_base_type(const monster* mon);
bool mons_class_can_leave_corpse(monster_type mc);
bool mons_class_leaves_hide(monster_type mc);
bool mons_is_zombified(const monster* mons);
bool mons_class_can_be_zombified(monster_type mc);
bool mons_can_be_zombified(const monster* mon);
bool mons_class_can_use_stairs(monster_type mc);
bool mons_can_use_stairs(const monster* mon,
                         dungeon_feature_type stair = DNGN_UNSEEN);
bool mons_enslaved_body_and_soul(const monster* mon);
bool mons_enslaved_soul(const monster* mon);
void name_zombie(monster* mon, monster_type mc, const string &mon_name);
void name_zombie(monster* mon, const monster* orig);

int mons_power(monster_type mc);

ucs_t mons_char(monster_type mc);
char mons_base_char(monster_type mc);

int mons_class_colour(monster_type mc);

monster_type royal_jelly_ejectable_monster();
monster_type random_draconian_monster_species();
monster_type random_demonspawn_monster_species();

bool init_abomination(monster* mon, int hd);
void define_monster(monster* mons);

void mons_pacify(monster* mon, mon_attitude_type att = ATT_GOOD_NEUTRAL,
                 bool no_xp = false);

bool mons_should_fire(bolt &beam, bool ignore_good_idea = false);

bool mons_has_los_ability(monster_type mon_type);
bool mons_has_ranged_spell(const monster* mon, bool attack_only = false,
                           bool ench_too = true);
bool mons_has_ranged_attack(const monster* mon);
bool mons_has_known_ranged_attack(const monster* mon);
bool mons_can_attack(const monster* mon);
bool mons_has_incapacitating_spell(const monster* mon, const actor* foe);
bool mons_has_incapacitating_ranged_attack(const monster* mon, const actor* foe);

const char *mons_pronoun(monster_type mon_type, pronoun_type variant,
                         bool visible = true);

bool mons_aligned(const actor *m1, const actor *m2);
bool mons_atts_aligned(mon_attitude_type fr1, mon_attitude_type fr2);

bool mons_att_wont_attack(mon_attitude_type fr);
mon_attitude_type mons_attitude(const monster* m);

bool mons_is_native_in_branch(const monster* mons,
                              const branch_type branch = you.where_are_you);
bool mons_is_poisoner(const monster* mon);

// Whether the monster is temporarily confused (class_too = false)
// or confused at all (class_too = true; temporarily or by class).
bool mons_is_confused(const monster* m, bool class_too = false);

bool mons_is_wandering(const monster* m);
bool mons_is_seeking(const monster* m);
bool mons_is_fleeing(const monster* m);
bool mons_is_retreating(const monster* m);
bool mons_is_cornered(const monster* m);
bool mons_is_batty(const monster* m);
bool mons_is_influenced_by_sanctuary(const monster* m);
bool mons_is_fleeing_sanctuary(const monster* m);
bool mons_just_slept(const monster* m);
bool mons_class_is_slime(monster_type mc);
bool mons_is_slime(const monster* mon);
bool mons_class_is_plant(monster_type mc);
bool mons_is_plant(const monster* mon);
bool mons_eats_items(const monster* mon);
monster_type mons_genus(monster_type mc);
monster_type mons_species(monster_type mc);
monster_type draco_or_demonspawn_subspecies(const monster* mon);
monster_type mons_detected_base(monster_type mt);
bool mons_is_siren_beholder(monster_type mc);
bool mons_is_siren_beholder(const monster* mons);

bool mons_looks_stabbable(const monster* m);
bool mons_looks_distracted(const monster* m);

void mons_start_fleeing_from_sanctuary(monster* mons);
void mons_stop_fleeing_from_sanctuary(monster* mons);

bool mons_landlubbers_in_reach(const monster* mons);

bool mons_class_is_stationary(monster_type mc);
bool mons_class_is_firewood(monster_type mc);
bool mons_is_firewood(const monster* mon);
bool mons_is_active_ballisto(const monster* mon);
bool mons_has_body(const monster* mon);
bool mons_has_flesh(const monster* mon);
bool mons_is_abyssal_only(monster_type mc);

bool herd_monster(const monster* mon);

int cheibriados_monster_player_speed_delta(const monster* mon);
bool cheibriados_thinks_mons_is_fast(const monster* mon);
bool mons_is_fiery(const monster* mon);
bool mons_is_projectile(monster_type mc);
bool mons_is_projectile(const monster* mon);
bool mons_is_boulder(const monster* mon);
bool mons_can_cling_to_walls(const monster* mon);
bool mons_is_object(monster_type mc);
bool mons_has_blood(monster_type mc);
bool mons_is_sensed(monster_type mc);
bool mons_allows_beogh(const monster* mon);
bool mons_allows_beogh_now(const monster* mon);

bool invalid_monster(const monster* mon);
bool invalid_monster_type(monster_type mt);
bool invalid_monster_index(int i);

void mons_load_spells(monster* mon);

void mons_remove_from_grid(const monster* mon);

bool monster_shover(const monster* m);

bool monster_senior(const monster* first, const monster* second,
                    bool fleeing = false);
string ugly_thing_colour_name(colour_t colour);
colour_t ugly_thing_random_colour();
int str_to_ugly_thing_colour(const string &s);
colour_t random_monster_colour();
int ugly_thing_colour_offset(colour_t colour);
string  draconian_colour_name(monster_type mon_type);
monster_type draconian_colour_by_name(const string &colour);
string  demonspawn_base_name(monster_type mon_type);
monster_type demonspawn_base_by_name(const string &colour);
mon_spell_slot drac_breath(monster_type drac_type);

monster_type random_monster_at_grid(const coord_def& p, bool species = false);

void         init_mon_name_cache();
monster_type get_monster_by_name(string name, bool substring = false);

string do_mon_str_replacements(const string &msg, const monster* mons,
                               int s_type = -1);

mon_body_shape get_mon_shape(const monster* mon);
mon_body_shape get_mon_shape(const monster_type mc);

string get_mon_shape_str(const mon_body_shape shape);

bool mons_class_can_pass(monster_type mc, const dungeon_feature_type grid);
bool mons_can_open_door(const monster* mon, const coord_def& pos);
bool mons_can_eat_door(const monster* mon, const coord_def& pos);
bool mons_can_destroy_door(const monster* mon, const coord_def& pos);
bool mons_can_traverse(const monster* mon, const coord_def& pos,
                       bool only_in_sight = false,
                       bool checktraps = true);

mon_inv_type equip_slot_to_mslot(equipment_type eq);
mon_inv_type item_to_mslot(const item_def &item);

bool player_or_mon_in_sanct(const monster* mons);
bool mons_is_immotile(const monster* mons);

int get_dist_to_nearest_monster();
bool monster_nearby();
actor *actor_by_mid(mid_t m, bool require_valid = false);
monster *monster_by_mid(mid_t m, bool require_valid = false);
bool mons_is_recallable(actor* caller, monster* targ);
void init_anon();
actor *find_agent(mid_t m, kill_category kc);
const char* mons_class_name(monster_type mc);
mon_threat_level_type mons_threat_level(const monster *mon,
                                        bool real = false);
int count_monsters(monster_type mtyp, bool friendly_only);
int count_allies();

bool mons_foe_is_marked(const monster* mons);
vector<monster* > get_on_level_followers();

bool mons_stores_tracking_data(const monster* mons);

bool mons_is_player_shadow(const monster* mon);

void reset_all_monsters();
void debug_mondata();
void debug_monspells();

bool choose_any_monster(const monster* mon);
monster *choose_random_nearby_monster(
    int weight,
    bool (*suitable)(const monster* mon) =
        choose_any_monster,
    bool prefer_named_or_priest = false);

monster *choose_random_monster_on_level(
    int weight,
    bool (*suitable)(const monster* mon) =
        choose_any_monster,
    bool prefer_named_or_priest = false);

void update_monster_symbol(monster_type mtype, cglyph_t md);

void normalize_spell_freq(monster_spells &spells, int hd);

enum mon_dam_level_type
{
    MDAM_OKAY,
    MDAM_LIGHTLY_DAMAGED,
    MDAM_MODERATELY_DAMAGED,
    MDAM_HEAVILY_DAMAGED,
    MDAM_SEVERELY_DAMAGED,
    MDAM_ALMOST_DEAD,
    MDAM_DEAD,
};

void print_wounds(const monster* mons);
bool wounded_damaged(mon_holy_type holi);

mon_dam_level_type mons_get_damage_level(const monster* mons);

string get_damage_level_string(mon_holy_type holi, mon_dam_level_type mdam);
bool mons_class_can_display_wounds(monster_type mc);
bool mons_can_display_wounds(const monster* mon);
bool mons_class_is_threatening(monster_type mo);
bool mons_is_threatening(const monster* mon);
bool mons_class_gives_xp(monster_type mc, bool indirect = false);
bool mons_gives_xp(const monster* mon, const actor* agent);
bool mons_is_notable(const monster& mon);

int max_mons_charge(monster_type m);

void init_mutant_beast(monster &mon, short HD, vector<int> beast_facets,
                       set<int> avoid_facets);
void radiate_pain_bond(const monster* mon, int damage);
void throw_monster_bits(const monster* mon);
void set_ancestor_spells(monster &ancestor, bool notify = false);
#endif
