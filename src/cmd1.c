/* File: cmd1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

/* Purpose: Movement commands (part 1) */

#include "angband.h"
#include "equip.h"

static int _max_vampiric_drain(void)
{
    if (prace_is_(RACE_MON_VAMPIRE) || prace_is_(MIMIC_BAT))
        return 100;
    return 50;
}

static void _rune_sword_kill(object_type *o_ptr, monster_race *r_ptr)
{
    if (o_ptr->curse_flags & TRC_PERMA_CURSE)
    {
        bool feed = FALSE;
        bool unique = (r_ptr->flags1 & RF1_UNIQUE);
                        
        switch (randint1(4))
        {
        case 1:
            if ((r_ptr->level > o_ptr->to_h + o_ptr->to_d + 15))
            {
                /*msg_print("Boost ToHit?");*/
                if (unique || one_in_(6))
                {
                    if (o_ptr->to_h < 50 || one_in_(666))
                    {
                        feed = TRUE;
                        o_ptr->to_h++;
                    }
                }
            }
            break;

        case 2:
            if ((r_ptr->level > o_ptr->to_h + o_ptr->to_d + 15))
            {
                /*msg_print("Boost ToDam?");*/
                if (unique || one_in_(6))
                {
                    if (o_ptr->to_d < 50 || one_in_(666))
                    {
                        feed = TRUE;
                        o_ptr->to_d++;
                    }
                }
            }
            break;

        case 3:
            if ((r_ptr->level > o_ptr->dd * o_ptr->ds))
            {
                /*msg_print("Boost dd?");*/
                if (one_in_((o_ptr->dd + 1) * (o_ptr->ds + 1)) && (unique || one_in_(6)))
                {
                    if (o_ptr->dd < 9 || one_in_(666))
                    {
                        feed = TRUE;
                        o_ptr->dd++;
                    }
                }
            }
            break;

        case 4:
            if ((r_ptr->level > o_ptr->dd * o_ptr->ds))
            {
                /*msg_print("Boost ds?");*/
                if (one_in_((o_ptr->dd + 1) * (o_ptr->ds + 1)) && (unique || one_in_(6)))
                {
                    if (o_ptr->ds < 9 || one_in_(666))
                    {
                        feed = TRUE;
                        o_ptr->ds++;
                    }
                }
            }
            break;
        }

        if (unique && one_in_(200 / MAX(r_ptr->level, 1)))
        {
            switch (randint1(12))
            {
            case 1:
                if (one_in_(6)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_BRAND_POIS); 
                } 
                break;
            case 2:
                if (one_in_(6)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_BRAND_FIRE);
                } 
                break;
            case 3:
                if (one_in_(6)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_BRAND_COLD); 
                } 
                break;
            case 4:
                if (one_in_(26)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_BRAND_ELEC); 
                } 
                break;
            case 5:
                if (one_in_(26)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_SLAY_UNDEAD); 
                } 
                break;
            case 6:
                if (one_in_(26)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_SLAY_DEMON); 
                } 
                break;
            case 7:
                if (one_in_(13)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_SLAY_DRAGON); 
                } 
                break;
            case 8:
                if (one_in_(13)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_SLAY_TROLL); 
                } 
                break;
            case 9:
                if (one_in_(13)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_SLAY_GIANT); 
                } 
                break;
            case 10:
                if (one_in_(66)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_SLAY_HUMAN); 
                } 
                break;
            case 11:
                if (one_in_(666)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_SLAY_EVIL); 
                } 
                break;
            case 12:
                if (one_in_(666)) 
                { 
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_VORPAL2); 
                }
                else if (one_in_(6))
                {
                    feed = TRUE; 
                    add_flag(o_ptr->art_flags, TR_VORPAL); 
                }
                break;
            }
        }

        if (feed)
        {
            if ((o_ptr->curse_flags & TRC_TY_CURSE) == 0
              && o_ptr->dd * o_ptr->ds > 50 )
            {  
                o_ptr->curse_flags |= TRC_TY_CURSE;
                msg_print("Your Rune Sword seeks to dominate you!");
            /*  if (one_in_(13))
                    add_flag(o_ptr->art_flags, TR_AGGRAVATE); */
                if (one_in_(13))
                    add_flag(o_ptr->art_flags, TR_DRAIN_EXP);
            }
            else if ((o_ptr->curse_flags & TRC_AGGRAVATE) == 0
                   && o_ptr->dd * o_ptr->ds > 30 )
            {
                o_ptr->curse_flags |= TRC_AGGRAVATE;
                msg_print("The thirst of your sword redoubles!");
            }
            else
                msg_print("Your rune sword grows more powerful!");
        }
    }
    else
        msg_print("Only cursed Rune Swords may feed.");
}

static void death_scythe_miss(object_type *o_ptr, int hand, int mode)
{
    u32b flgs[TR_FLAG_SIZE];
    int k;
    critical_t crit;

    /* Sound */
    sound(SOUND_HIT);

    /* Message */
    msg_print("Your scythe returns to you!");

    /* Extract the flags */
    object_flags(o_ptr, flgs);

    k = damroll(o_ptr->dd + p_ptr->weapon_info[hand].to_dd, o_ptr->ds + p_ptr->weapon_info[hand].to_ds);
    {
        int mult;
        switch (p_ptr->mimic_form)
        {
        case MIMIC_NONE:
            switch (p_ptr->prace)
            {
                case RACE_YEEK:
                case RACE_KLACKON:
                case RACE_HUMAN:
                case RACE_AMBERITE:
                case RACE_DUNADAN:
                case RACE_BARBARIAN:
                case RACE_BEASTMAN:
                case RACE_DEMIGOD:
                    mult = 25;break;
                case RACE_SNOTLING:
                case RACE_HALF_TROLL:
                case RACE_HALF_OGRE:
                case RACE_HALF_GIANT:
                case RACE_HALF_TITAN:
                case RACE_CYCLOPS:
                case RACE_IMP:
                case RACE_SKELETON:
                case RACE_ZOMBIE:
                case RACE_VAMPIRE:
                case RACE_MON_VAMPIRE:
                case RACE_SPECTRE:
                case RACE_BALROG:
                case RACE_DRACONIAN:
                case RACE_TONBERRY:
                case RACE_MON_LICH:
                case RACE_MON_DRAGON:
                    mult = 30;break;
                default:
                    mult = 10;break;
            }
            break;
        case MIMIC_DEMON:
        case MIMIC_DEMON_LORD:
        case MIMIC_VAMPIRE:
            mult = 30;break;
        default:
            mult = 10;break;
        }

        if (p_ptr->align < 0 && mult < 20)
            mult = 20;
        if (!res_save_default(RES_ACID) && mult < 25)
            mult = 25;
        if (!res_save_default(RES_ELEC) && mult < 25)
            mult = 25;
        if (!res_save_default(RES_FIRE) && mult < 25)
            mult = 25;
        if (!res_save_default(RES_COLD) && mult < 25)
            mult = 25;
        if (!res_save_default(RES_POIS) && mult < 25)
            mult = 25;

        if (p_ptr->tim_slay_sentient && p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS)
        {
            if (mult < 20) mult = 20;
        }

        if ((have_flag(flgs, TR_FORCE_WEAPON) || p_ptr->tim_force) && (p_ptr->csp > (p_ptr->msp / 30)))
        {
            p_ptr->csp -= (1+(p_ptr->msp / 30));
            p_ptr->redraw |= (PR_MANA);
            mult = mult * 3 / 2 + 15;
        }

        k *= mult;
        k /= 10;
    }

    crit = critical_norm(o_ptr->weight, o_ptr->to_h, p_ptr->weapon_info[hand].to_h, mode, hand);
    if (crit.desc)
    {
        k = k * crit.mul/100 + crit.to_d;
        msg_print(crit.desc);
    }

    if (one_in_(6))
    {
        int mult = 2;
        msg_format("Your weapon cuts deep into yourself!");
        /* Try to increase the damage */
        while (one_in_(4))
        {
            mult++;
        }

        k *= mult;
    }
    k += (p_ptr->weapon_info[hand].to_d + o_ptr->to_d);

    if (k < 0) k = 0;

    take_hit(DAMAGE_FORCE, k, "Death scythe", -1);

    redraw_stuff();
}

/*
 * Determine if the player "hits" a monster (normal combat).
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_fire(int chance, int ac, int vis)
{
    int k;

    /* Never hit */
    if (chance <= 0) return (FALSE);

    /* Invisible monsters are harder to hit */
    if (!vis) chance = (chance + 1) / 2;

    /* Percentile dice */
    k = randint0(100);

    /* Hack -- Instant miss or hit */
    if (k < 10) return (k < 5);

    if (p_ptr->personality == PERS_LAZY)
        if (one_in_(20)) return (FALSE);

    /* Power competes against armor */
    if (randint0(chance) < (ac * 3 / 4)) return (FALSE);

    /* Assume hit */
    return (TRUE);
}



/*
 * Determine if the player "hits" a monster (normal combat).
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_norm(int chance, int ac, int vis)
{
    int k;

    /* Wimpy attack never hits */
    if (chance <= 0) return (FALSE);

    /* Penalize invisible targets */
    if (!vis) chance = (chance + 1) / 2;

    /* Percentile dice */
    k = randint0(100);

    /* Hack -- Instant miss or hit */
    if (k < 10) return (k < 5);

    if (p_ptr->personality == PERS_LAZY)
        if (one_in_(20)) return (FALSE);

    /* Power must defeat armor */
    if (randint0(chance) < (ac * 3 / 4)) return (FALSE);

    /* Assume hit */
    return (TRUE);
}



/*
 * Critical hits (from bows/crossbows/slings)
 * Factor in item weight, total plusses, and player level.
 */
critical_t critical_shot(int weight, int plus)
{
    critical_t result = {0};
    int i, k;

    /* Extract "shot" power */
    i = ((p_ptr->shooter_info.to_h + plus) * 4) + (p_ptr->lev * 2);

    /* Snipers can shot more critically with crossbows */
    if (p_ptr->concent) i += ((i * p_ptr->concent) / 10);
    if ((p_ptr->pclass == CLASS_SNIPER) && (p_ptr->shooter_info.tval_ammo == TV_BOLT)) i *= 2;

    /* Critical hit */
    if (randint1(5000) <= i)
    {
        k = weight * randint1(500);

        if (k < 900)
        {
            result.desc = "It was a good hit!";
            result.mul = 150;
        }
        else if (k < 1350)
        {
            result.desc = "It was a great hit!";
            result.mul = 200;
        }
        else
        {
            result.desc = "It was a superb hit!";
            result.mul = 300;
        }
    }

    return result;
}

/*
 * Critical hits (from bows/crossbows/slings)
 * Factor in item weight, total plusses, and player level.
 */
s16b critical_throw(int weight, int plus, int dam)
{
    int i, k;

    /* Extract "shot" power */
    i = ((p_ptr->shooter_info.to_h + plus) * 4) + (p_ptr->lev * 3);

    /* Critical hit */
    if (randint1(5000) <= i)
    {
        k = weight + randint1(650);

        if (k < 400)
        {
            msg_print("It was a good hit!");
            dam += (dam / 2);
        }
        else if (k < 700)
        {
            msg_print("It was a great hit!");
            dam += dam;
        }
        else
        {
            msg_print("It was a superb hit!");
            dam += 3*dam/2;
        }
    }

    return (dam);
}


/*
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
critical_t critical_norm(int weight, int plus, s16b meichuu, int mode, int hand)
{
    critical_t result = {0};
    int i;
    int roll = (p_ptr->pclass == CLASS_NINJA) ? 4444 : 5000;
    int quality = 650;

    if (p_ptr->enhanced_crit)
    {
        weight = weight * 3 / 2;
        weight += 300;
    }

    if ( equip_is_valid_hand(hand)
      && p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS
      && p_ptr->pclass != CLASS_DUELIST 
      && !p_ptr->weapon_info[hand].omoi )
    {
        roll = roll * 2 / 3;
    }

    /* Extract "blow" power */
    i = (weight + (meichuu * 3 + plus * 5) + (p_ptr->lev * 3));

    /* Mauler: Destroyer now scales with level */
    if ( p_ptr->pclass == CLASS_MAULER
      && equip_is_valid_hand(hand)
      && p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS )
    {
        int pct = MIN((weight - 200)/20, 20);
        if (pct > 0)
            pct = pct * p_ptr->lev / 50;
        i += roll * pct / 100;
        quality += quality * pct / 100;
    }

    /* Chance */
    if ( mode == HISSATSU_MAJIN 
      || mode == HISSATSU_3DAN 
      || mode == MAULER_CRITICAL_BLOW
      || mode == GOLEM_BIG_PUNCH
      || randint1(roll) <= i )
    {
        int k = weight + randint1(quality);

        if ( mode == HISSATSU_MAJIN 
          || mode == HISSATSU_3DAN )
        {
            k += randint1(650);
        }
        if (mode == MAULER_CRITICAL_BLOW)
        {
            k += randint1(250*p_ptr->lev/50);
        }

        if (k < 400)
        {
            result.desc = "It was a good hit!";
            result.mul = 200;
        }
        else if (k < 700)
        {
            result.desc = "It was a great hit!";
            result.mul = 250;
        }
        else if (k < 900)
        {
            result.desc = "It was a superb hit!";
            result.mul = 300;
        }
        else if (k < 1300)
        {
            result.desc = "It was a *GREAT* hit!";
            result.mul = 350;
        }
        else
        {
            result.desc = "It was a *SUPERB* hit!";
            result.mul = 400;
        }
    }

    return result;
}

s16b tot_dam_aux_monk(int tdam, monster_type *m_ptr, int mode)
{
    int mult = 10;
    int bonus = 0;
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    const int monk_elem_slay = 17;

    if (have_flag(p_ptr->weapon_info[0].flags, TR_BRAND_ACID) || mode == MYSTIC_ACID)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK)
        {
            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK);
        }
        else if (mode == MYSTIC_ACID)
        {
            mult = MAX(mult, 20);
            bonus = MAX(bonus, 5);
        }
        else
        {
            if (mult < monk_elem_slay) mult = monk_elem_slay;
        }
    }

    if (have_flag(p_ptr->weapon_info[0].flags, TR_BRAND_ELEC) || mode == MYSTIC_ELEC)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK)
        {
            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK);
        }
        else if (mode == MYSTIC_ELEC)
        {
            mult = MAX(mult, 25);
            bonus = MAX(bonus, 7);
        }
        else
        {
            if (mult < monk_elem_slay) mult = monk_elem_slay;
        }
    }

    if (have_flag(p_ptr->weapon_info[0].flags, TR_BRAND_FIRE) || mode == MYSTIC_FIRE)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK)
        {
            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK);
        }
        else
        {
            if (r_ptr->flags3 & RF3_HURT_FIRE)
            {
                mult = MAX(mult, 25);
                if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_HURT_FIRE;
            }
            else if (mode == MYSTIC_FIRE)
            {
                mult = MAX(mult, 17);
                bonus = MAX(bonus, 3);
            }
            else 
                mult = MAX(mult, monk_elem_slay);
        }
    }

    if (have_flag(p_ptr->weapon_info[0].flags, TR_BRAND_COLD) || mode == MYSTIC_COLD)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK)
        {
            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK);
        }
        else
        {
            if (r_ptr->flags3 & RF3_HURT_COLD)
            {
                mult = MAX(mult, 25);
                if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_HURT_COLD;
            }
            else if (mode == MYSTIC_COLD)
            {
                mult = MAX(mult, 17);
                bonus = MAX(bonus, 3);
            }
            else 
                mult = MAX(mult, monk_elem_slay);
        }
    }

    if (have_flag(p_ptr->weapon_info[0].flags, TR_BRAND_POIS) || mode == MYSTIC_POIS)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK)
        {
            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK);
        }
        else if (mode == MYSTIC_POIS)
        {
            mult = MAX(mult, 17);
            bonus = MAX(bonus, 3);
        }
        else
        {
            if (mult < monk_elem_slay) mult = monk_elem_slay;
        }
    }

    return tdam * mult / 10 + bonus;
}

#define _MAX_CHAOS_SLAYS 15

int _chaos_slays[_MAX_CHAOS_SLAYS] = {
    TR_SLAY_ANIMAL,
    TR_SLAY_EVIL,
    TR_SLAY_GOOD,
    TR_SLAY_UNDEAD,
    TR_SLAY_DEMON,
    TR_SLAY_ORC,
    TR_SLAY_TROLL,
    TR_SLAY_GIANT,
    TR_SLAY_DRAGON,
    TR_SLAY_HUMAN,
    TR_BRAND_POIS,
    TR_BRAND_ACID,
    TR_BRAND_ELEC,
    TR_BRAND_FIRE,
    TR_BRAND_COLD,
};    

s16b tot_dam_aux(object_type *o_ptr, int tdam, monster_type *m_ptr, s16b hand, int mode, bool thrown)
{
    int mult = 10;

    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    int chaos_slay = 0;

    u32b flgs[TR_FLAG_SIZE] = {0};
    char o_name[MAX_NLEN];

    /* Extract the flags */
    if (thrown)
        object_flags(o_ptr, flgs);
    else
        weapon_flags(hand, flgs);

    /* Chaos Weapons now have random slay effects, and the slay so
       chosen will augment any existing slay of the same type. */
    if (have_flag(flgs, TR_CHAOTIC))
    {
        chaos_slay = _chaos_slays[randint0(_MAX_CHAOS_SLAYS)];
        object_desc(o_name, o_ptr, OD_NAME_ONLY);
    }

    /* Some "weapons" and "ammo" do extra damage */
    switch (o_ptr->tval)
    {
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_DIGGING:
        {
            if (p_ptr->tim_blood_seek && monster_living(r_ptr))
            {
                if (mult < 20) mult = 20;
            }

            if (monster_living(r_ptr) && have_flag(flgs, TR_SLAY_LIVING))
            {
                if (mult < 20) mult = 20;
            }

            if (r_ptr->flags3 & RF3_ANIMAL)
            {
                if (chaos_slay == TR_SLAY_ANIMAL)
                {
                    msg_format("%s slays animals.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_ANIMAL;
                    if (have_flag(flgs, TR_KILL_ANIMAL))
                    {
                        if (mult < 50) mult = 50;
                    }
                    else if (have_flag(flgs, TR_SLAY_ANIMAL))
                    {
                        if (mult < 35) mult = 35;
                    }
                    else
                    {
                        if (mult < 25) mult = 25;
                    }
                }
                else
                {
                    if (have_flag(flgs, TR_KILL_ANIMAL))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_ANIMAL;
                        if (mult < 40) mult = 40;
                    }
                    else if (have_flag(flgs, TR_SLAY_ANIMAL))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_ANIMAL;
                        if (mult < 25) mult = 25;
                    }
                }
            }

            if (r_ptr->flags3 & RF3_EVIL)
            {
                if (chaos_slay == TR_SLAY_EVIL)
                {
                    msg_format("%s slays evil.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_EVIL;
                    if (have_flag(flgs, TR_KILL_EVIL))
                    {
                        if (mult < 45) mult = 45;
                    }
                    else if (have_flag(flgs, TR_SLAY_EVIL) || weaponmaster_get_toggle() == TOGGLE_HOLY_BLADE)
                    {
                        if (mult < 30) mult = 30;
                    }
                    else
                    {
                        if (mult < 20) mult = 20;
                    }

                }
                else
                {
                    if (have_flag(flgs, TR_KILL_EVIL))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_EVIL;
                        if (mult < 35) mult = 35;
                    }
                    else if (have_flag(flgs, TR_SLAY_EVIL) || weaponmaster_get_toggle() == TOGGLE_HOLY_BLADE)
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_EVIL;
                        if (mult < 20) mult = 20;
                    }
                }
            }

            if (r_ptr->flags3 & RF3_GOOD)
            {
                if (chaos_slay == TR_SLAY_GOOD)
                {
                    msg_format("%s slays good.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_GOOD;
                    if (have_flag(flgs, TR_SLAY_GOOD))
                    {
                        if (mult < 30) mult = 30;
                    }
                    else
                    {
                        if (mult < 20) mult = 20;
                    }

                }
                else
                {
                    if (have_flag(flgs, TR_SLAY_GOOD))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_GOOD;
                        if (mult < 20) mult = 20;
                    }
                }
            }

            if (r_ptr->flags2 & RF2_HUMAN)
            {
                if (chaos_slay == TR_SLAY_HUMAN)
                {
                    msg_format("%s slays humans.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= RF2_HUMAN;
                    if (have_flag(flgs, TR_KILL_HUMAN))
                    {
                        if (mult < 50) mult = 50;
                    }
                    else if (have_flag(flgs, TR_SLAY_HUMAN))
                    {
                        if (mult < 35) mult = 35;
                    }
                    else
                    {
                        if (mult < 25) mult = 25;
                    }
                }
                else
                {
                    if (have_flag(flgs, TR_KILL_HUMAN))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= RF2_HUMAN;
                        if (mult < 40) mult = 40;
                    }
                    else if (have_flag(flgs, TR_SLAY_HUMAN))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= RF2_HUMAN;
                        if (mult < 25) mult = 25;
                    }
                }
            }

            if (r_ptr->flags3 & RF3_UNDEAD)
            {
                if (chaos_slay == TR_SLAY_UNDEAD)
                {
                    msg_format("%s slays undead.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_UNDEAD;
                    if (have_flag(flgs, TR_KILL_UNDEAD))
                    {
                        if (mult < 60) mult = 60;
                    }
                    else if (have_flag(flgs, TR_SLAY_UNDEAD))
                    {
                        if (mult < 40) mult = 40;
                    }
                    else
                    {
                        if (mult < 30) mult = 30;
                    }
                }
                else
                {
                    if (have_flag(flgs, TR_KILL_UNDEAD))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_UNDEAD;
                        if (mult < 50) mult = 50;
                    }
                    else if (have_flag(flgs, TR_SLAY_UNDEAD))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_UNDEAD;
                        if (mult < 30) mult = 30;
                    }
                }
            }

            if (r_ptr->flags3 & RF3_DEMON)
            {
                if (chaos_slay == TR_SLAY_DEMON)
                {
                    msg_format("%s slays demons.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_DEMON;
                    if (have_flag(flgs, TR_KILL_DEMON))
                    {
                        if (mult < 60) mult = 60;
                    }
                    else if (have_flag(flgs, TR_SLAY_DEMON))
                    {
                        if (mult < 40) mult = 40;
                    }
                    else
                    {
                        if (mult < 30) mult = 30;
                    }
                }
                else
                {
                    if (have_flag(flgs, TR_KILL_DEMON))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_DEMON;
                        if (mult < 50) mult = 50;
                    }
                    else if (have_flag(flgs, TR_SLAY_DEMON))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_DEMON;
                        if (mult < 30) mult = 30;
                    }
                }
            }

            if (r_ptr->flags3 & RF3_ORC)
            {
                if (chaos_slay == TR_SLAY_ORC)
                {
                    msg_format("%s slays orcs.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_ORC;
                    if (have_flag(flgs, TR_KILL_ORC))
                    {
                        if (mult < 60) mult = 60;
                    }
                    else if (have_flag(flgs, TR_SLAY_ORC))
                    {
                        if (mult < 40) mult = 40;
                    }
                    else
                    {
                        if (mult < 30) mult = 30;
                    }
                }
                else
                {
                    if (have_flag(flgs, TR_KILL_ORC))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_ORC;
                        if (mult < 50) mult = 50;
                    }
                    else if (have_flag(flgs, TR_SLAY_ORC))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_ORC;
                        if (mult < 30) mult = 30;
                    }
                }
            }


            if (r_ptr->flags3 & RF3_TROLL)
            {
                if (chaos_slay == TR_SLAY_TROLL)
                {
                    msg_format("%s slays trolls.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_TROLL;
                    if (have_flag(flgs, TR_KILL_TROLL))
                    {
                        if (mult < 60) mult = 60;
                    }
                    else if (have_flag(flgs, TR_SLAY_TROLL))
                    {
                        if (mult < 40) mult = 40;
                    }
                    else
                    {
                        if (mult < 30) mult = 30;
                    }
                }
                else
                {
                    if (have_flag(flgs, TR_KILL_TROLL))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_TROLL;
                        if (mult < 50) mult = 50;
                    }
                    else if (have_flag(flgs, TR_SLAY_TROLL))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_TROLL;
                        if (mult < 30) mult = 30;
                    }
                }
            }

            if (r_ptr->flags3 & RF3_GIANT)
            {
                if (chaos_slay == TR_SLAY_GIANT)
                {
                    msg_format("%s slays giants.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_GIANT;
                    if (have_flag(flgs, TR_KILL_GIANT))
                    {
                        if (mult < 60) mult = 60;
                    }
                    else if (have_flag(flgs, TR_SLAY_GIANT))
                    {
                        if (mult < 40) mult = 40;
                    }
                    else
                    {
                        if (mult < 30) mult = 30;
                    }
                }
                else
                {
                    if (have_flag(flgs, TR_KILL_GIANT))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_GIANT;
                        if (mult < 50) mult = 50;
                    }
                    else if (have_flag(flgs, TR_SLAY_GIANT))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_GIANT;
                        if (mult < 30) mult = 30;
                    }
                }
            }

            if (r_ptr->flags3 & RF3_DRAGON)
            {
                if (chaos_slay == TR_SLAY_DRAGON)
                {
                    msg_format("%s slays dragons.", o_name);
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_DRAGON;
                    if (have_flag(flgs, TR_KILL_DRAGON))
                    {
                        if (mult < 60) mult = 60;
                        if ((o_ptr->name1 == ART_NOTHUNG) && (m_ptr->r_idx == MON_FAFNER))
                            mult *= 3;
                    }
                    else if (have_flag(flgs, TR_SLAY_DRAGON))
                    {
                        if (mult < 40) mult = 40;
                    }
                    else
                    {
                        if (mult < 30) mult = 30;
                    }
                }
                else
                {
                    if (have_flag(flgs, TR_KILL_DRAGON))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_DRAGON;
                        if (mult < 50) mult = 50;
                        if ((o_ptr->name1 == ART_NOTHUNG) && (m_ptr->r_idx == MON_FAFNER))
                            mult *= 3;
                    }
                    else if (have_flag(flgs, TR_SLAY_DRAGON))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_DRAGON;
                        if (mult < 30) mult = 30;
                    }
                }
            }


            /* Hex - Slay Good (Runesword) */
            if (hex_spelling(HEX_RUNESWORD) &&
                (r_ptr->flags3 & RF3_GOOD))
            {
                if (is_original_ap_and_seen(m_ptr))
                {
                    r_ptr->r_flags3 |= RF3_GOOD;
                }

                if (mult < 20) mult = 20;
            }

            /* Brand (Acid) */
            if (have_flag(flgs, TR_BRAND_ACID) || mode == PY_ATTACK_ACID || chaos_slay == TR_BRAND_ACID)
            {
                if (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK)
                {
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK);
                }
                else if (chaos_slay == TR_BRAND_ACID)
                {
                    msg_format("%s is covered in acid.", o_name);
                    if (have_flag(flgs, TR_BRAND_ACID)) mult = MAX(mult, 35);
                    else mult = MAX(mult, 25);
                }
                else mult = MAX(mult, 25);
            }

            /* Brand (Elec) */
            if ( have_flag(flgs, TR_BRAND_ELEC) 
              || mode == HISSATSU_ELEC
              || chaos_slay == TR_BRAND_ELEC )
            {
                if (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK)
                {
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK);
                }
                else if (chaos_slay == TR_BRAND_ELEC)
                {
                    msg_format("%s is covered in electricity.", o_name);
                    if (have_flag(flgs, TR_BRAND_ELEC))
                    {
                        if (mode == HISSATSU_ELEC) mult = MAX(mult, 80);
                        else mult = MAX(mult, 35);
                    }
                    else if (mode == HISSATSU_ELEC) mult = MAX(mult, 60);
                    else mult = MAX(mult, 35);
                }
                else if (have_flag(flgs, TR_BRAND_ELEC))
                {
                    if (mode == HISSATSU_ELEC) mult = MAX(mult, 70);
                    else mult = MAX(mult, 25);
                }
                else if (mode == HISSATSU_ELEC) mult = MAX(mult, 50);
            }

            /* Brand (Fire) */
            if ( have_flag(flgs, TR_BRAND_FIRE) 
              || mode == HISSATSU_FIRE
              || chaos_slay == TR_BRAND_FIRE )
            {
                int tmp = 0;
                if (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK)
                {
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK);
                }
                else if (chaos_slay == TR_BRAND_FIRE)
                {
                    msg_format("%s is covered in fire.", o_name);
                    if (have_flag(flgs, TR_BRAND_FIRE))
                    {
                        if (mode == HISSATSU_FIRE) tmp = 45;
                        else tmp = 35;
                    }
                    else if (mode == HISSATSU_FIRE) tmp = 35;
                    else tmp = 25;
                }
                else if (have_flag(flgs, TR_BRAND_FIRE))
                {
                    if (mode == HISSATSU_FIRE) tmp = 35;
                    else tmp = 25;
                }
                else if (mode == HISSATSU_FIRE) tmp = 25;
                if (tmp > 0 && (r_ptr->flags3 & RF3_HURT_FIRE))
                {    
                    tmp *= 2;
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_HURT_FIRE;
                }
                mult = MAX(mult, tmp);
            }

            /* Brand (Cold) */
            if ( have_flag(flgs, TR_BRAND_COLD) 
              || mode == HISSATSU_COLD
              || chaos_slay == TR_BRAND_COLD ) 
            {
                int tmp = 0;
                if (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK)
                {
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK);
                }
                else if (chaos_slay == TR_BRAND_COLD)
                {
                    msg_format("%s is covered in frost.", o_name);
                    if (have_flag(flgs, TR_BRAND_COLD))
                    {
                        if (mode == HISSATSU_COLD) tmp = 45;
                        else tmp = 35;
                    }
                    else if (mode == HISSATSU_COLD) tmp = 35;
                    else tmp = 25;
                }
                else if (have_flag(flgs, TR_BRAND_COLD))
                {
                    if (mode == HISSATSU_COLD) tmp = 35;
                    else tmp = 25;
                }
                else if (mode == HISSATSU_COLD) tmp = 25;

                if (tmp > 0 && (r_ptr->flags3 & RF3_HURT_COLD))
                {    
                    tmp *= 2;
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_HURT_COLD;
                }
                mult = MAX(mult, tmp);
            }

            /* Brand (Poison) */
            if ( have_flag(flgs, TR_BRAND_POIS) 
              || mode == HISSATSU_POISON
              || chaos_slay == TR_BRAND_POIS )
            {
                if (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK)
                {
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK);
                }
                else if (chaos_slay == TR_BRAND_POIS)
                {
                    msg_format("%s is covered in poison.", o_name);
                    if (have_flag(flgs, TR_BRAND_POIS))
                    {
                        if (mode == HISSATSU_POISON) mult = MAX(mult, 45);
                        else mult = MAX(mult, 35);
                    }
                    else if (mode == HISSATSU_POISON) mult = MAX(mult, 35);
                    else mult = MAX(mult, 25);
                }
                else if (have_flag(flgs, TR_BRAND_POIS))
                {
                    if (mode == HISSATSU_POISON) mult = MAX(mult, 35);
                    else mult = MAX(mult, 25);
                }
                else if (mode == HISSATSU_POISON) mult = MAX(mult, 25);
            }

            if ((mode == HISSATSU_ZANMA) && !monster_living(r_ptr) && (r_ptr->flags3 & RF3_EVIL))
            {
                if (mult < 15) mult = 25;
                else if (mult < 50) mult = MIN(50, mult+20);
            }
            if (mode == HISSATSU_UNDEAD)
            {
                if (r_ptr->flags3 & RF3_UNDEAD)
                {
                    if (is_original_ap_and_seen(m_ptr))
                    {
                        r_ptr->r_flags3 |= RF3_UNDEAD;
                    }
                    if (mult == 10) mult = 70;
                    else if (mult < 140) mult = MIN(140, mult+60);
                }
                if (mult == 10) mult = 40;
                else if (mult < 60) mult = MIN(60, mult+30);
            }
            if ((mode == HISSATSU_SEKIRYUKA) && p_ptr->cut && monster_living(r_ptr))
            {
                int tmp = MIN(100, MAX(10, p_ptr->cut / 10));
                if (mult < tmp) mult = tmp;
            }
            if ((mode == HISSATSU_HAGAN) && (r_ptr->flags3 & RF3_HURT_ROCK))
            {
                if (is_original_ap_and_seen(m_ptr))
                {
                    r_ptr->r_flags3 |= RF3_HURT_ROCK;
                }
                if (mult == 10) mult = 40;
                else if (mult < 60) mult = 60;
            }
            if (p_ptr->tim_slay_sentient && p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS)
            {
                if (r_ptr->flags3 & RF3_NO_STUN)
                {
                    if (is_original_ap_and_seen(m_ptr))    r_ptr->r_flags3 |= RF3_NO_STUN;
                }
                else
                {
                    if (mult < 20) mult = 20;
                }
            }
            if (have_flag(flgs, TR_FORCE_WEAPON) || p_ptr->tim_force)
            {
                int cost = 0;
                int dd = o_ptr->dd + p_ptr->weapon_info[hand].to_dd;
                int ds = o_ptr->ds + p_ptr->weapon_info[hand].to_ds;
                
                if (p_ptr->pclass == CLASS_SAMURAI)
                    cost = (1 + (dd * ds * 2 / 7));
                else
                    cost = (1 + (dd * ds / 7));

                if (p_ptr->csp >= cost)
                {
                    p_ptr->csp -= cost;
                    p_ptr->redraw |= (PR_MANA);
                    mult = mult * 3 / 2 + 15;
                }
            }
            if (p_ptr->tim_blood_feast)
            {
                take_hit(DAMAGE_ATTACK, 15, "blood feast", -1);
            }
            break;
        }
    }
    if (mult > 150) mult = 150;

    /* Return the total damage */
    return (tdam * mult / 10);
}


/*
 * Search for hidden things
 */
void search(void)
{
    int y, x, chance;

    s16b this_o_idx, next_o_idx = 0;

    cave_type *c_ptr;


    /* Start with base search ability */
    chance = p_ptr->skills.srh;

    /* Penalize various conditions */
    if (p_ptr->blind || no_lite()) chance = chance / 10;
    if (p_ptr->confused || p_ptr->image) chance = chance / 10;

    /* Search the nearby grids, which are always in bounds */
    for (y = (py - 1); y <= (py + 1); y++)
    {
        for (x = (px - 1); x <= (px + 1); x++)
        {
            /* Sometimes, notice things */
            if (randint0(100) < chance)
            {
                /* Access the grid */
                c_ptr = &cave[y][x];

                /* Invisible trap */
                if (c_ptr->mimic && is_trap(c_ptr->feat))
                {
                    /* Pick a trap */
                    disclose_grid(y, x);

                    /* Message */
                    msg_print("You have found a trap.");

                    /* Disturb */
                    disturb(0, 0);
                }

                /* Secret door */
                if (is_hidden_door(c_ptr))
                {
                    /* Message */
                    msg_print("You have found a secret door.");

                    /* Disclose */
                    disclose_grid(y, x);

                    /* Disturb */
                    disturb(0, 0);
                }

                /* Scan all objects in the grid */
                for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
                {
                    object_type *o_ptr;

                    /* Acquire object */
                    o_ptr = &o_list[this_o_idx];

                    /* Acquire next object */
                    next_o_idx = o_ptr->next_o_idx;

                    /* Skip non-chests */
                    if (o_ptr->tval != TV_CHEST) continue;

                    /* Skip non-trapped chests */
                    if (!chest_traps[o_ptr->pval]) continue;

                    /* Identify once */
                    if (!object_is_known(o_ptr))
                    {
                        /* Message */
                        msg_print("You have discovered a trap on the chest!");

                        /* Know the trap */
                        object_known(o_ptr);

                        /* Notice it */
                        disturb(0, 0);
                    }
                }
            }
        }
    }
}


/*
 * Helper routine for py_pickup() and py_pickup_floor().
 *
 * Add the given dungeon object to the character's inventory.
 *
 * Delete the object afterwards.
 */
void py_pickup_aux(int o_idx)
{
    int slot, i;

    char o_name[MAX_NLEN];

    object_type *o_ptr;

    o_ptr = &o_list[o_idx];

    /* Carry the object */
    slot = inven_carry(o_ptr);

    /* Get the object again */
    o_ptr = &inventory[slot];

    /* Delete the object */
    delete_object_idx(o_idx);

    if ( p_ptr->personality == PERS_MUNCHKIN 
      || randint0(1000) < virtue_current(VIRTUE_KNOWLEDGE) )
    {
        bool old_known = identify_item(o_ptr);

        /* Auto-inscription/destroy */
        autopick_alter_item(slot, (bool)(destroy_identify && !old_known));

        /* If it is destroyed, don't pick it up */
        if (o_ptr->marked & OM_AUTODESTROY) return;
    }

    /* Describe the object */
    object_desc(o_name, o_ptr, 0);

    /* Message */
    msg_format("You have %s (%c).", o_name, index_to_label(slot));
    strcpy(record_o_name, o_name);

    /* Runes confer benefits even when in inventory */
    p_ptr->update |= PU_BONUS;

    /* Hack: Archaeologists Instantly Pseudo-ID artifacts on pickup */
    if ( p_ptr->pclass == CLASS_ARCHAEOLOGIST 
      && object_is_artifact(o_ptr) 
      && !object_is_known(o_ptr) )
    {
        /* Suppress you are leaving something special behind message ... */
        if (p_ptr->sense_artifact)
        {
            p_ptr->sense_artifact = FALSE;    /* There may be more than one? */
            p_ptr->redraw |= PR_STATUS;
        }

        if (!(o_ptr->ident & (IDENT_SENSE)))
        {
            msg_format("You feel that the %s is %s...", o_name, game_inscriptions[FEEL_SPECIAL]);    
        
            o_ptr->ident |= (IDENT_SENSE);
            o_ptr->feeling = FEEL_SPECIAL;
        }
    }

    record_turn = turn;


    /* Check if completed a quest */
    for (i = 0; i < max_quests; i++)
    {
        if ((quest[i].type == QUEST_TYPE_FIND_ARTIFACT) &&
            (quest[i].status == QUEST_STATUS_TAKEN) &&
               (quest[i].k_idx == o_ptr->name1 || quest[i].k_idx == o_ptr->name3))
        {
            quest[i].status = QUEST_STATUS_COMPLETED;
            quest[i].complev = (byte)p_ptr->lev;
            msg_print("You completed your quest!");
            msg_print(NULL);
        }
    }
}


/*
 * Player "wants" to pick up an object or gold.
 * Note that we ONLY handle things that can be picked up.
 * See "move_player()" for handling of other things.
 */
void carry(bool pickup)
{
    cave_type *c_ptr = &cave[py][px];

    s16b this_o_idx, next_o_idx = 0;

    char    o_name[MAX_NLEN];

    /* Recenter the map around the player */
    verify_panel();

    /* Update stuff */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD);

    /* Handle stuff */
    handle_stuff();

    /* Automatically pickup/destroy/inscribe items */
    autopick_pickup_items(c_ptr);

#ifdef ALLOW_EASY_FLOOR

    if (easy_floor)
    {
        py_pickup_floor(pickup);
        return;
    }

#endif /* ALLOW_EASY_FLOOR */

    /* Scan the pile of objects */
    for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
    {
        object_type *o_ptr;

        /* Acquire object */
        o_ptr = &o_list[this_o_idx];

#ifdef ALLOW_EASY_SENSE /* TNB */

        /* Option: Make item sensing easy */
        if (easy_sense)
        {
            /* Sense the object */
            (void)sense_object(o_ptr);
        }

#endif /* ALLOW_EASY_SENSE -- TNB */

        /* Describe the object */
        object_desc(o_name, o_ptr, 0);

        /* Acquire next object */
        next_o_idx = o_ptr->next_o_idx;

        /* Hack -- disturb */
        disturb(0, 0);

        /* Pick up gold */
        if (o_ptr->tval == TV_GOLD)
        {
            int value = o_ptr->pval;

            /* Delete the gold */
            delete_object_idx(this_o_idx);

            /* Message */
            msg_format("You collect %d gold pieces worth of %s.",
                   value, o_name);

            sound(SOUND_SELL);

            /* Collect the gold */
            p_ptr->au += value;

            /* Redraw gold */
            p_ptr->redraw |= (PR_GOLD);

            /* Window stuff */
            p_ptr->window |= (PW_PLAYER);

            if (prace_is_(RACE_MON_LEPRECHAUN))
                p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
        }

        /* Pick up objects */
        else
        {
            /* Hack - some objects were handled in autopick_pickup_items(). */
            if (o_ptr->marked & OM_NOMSG)
            {
                /* Clear the flag. */
                o_ptr->marked &= ~OM_NOMSG;
            }
            /* Describe the object */
            else if (!pickup)
            {
                msg_format("You see %s.", o_name);

            }

            /* Note that the pack is too full */
            else if (!inven_carry_okay(o_ptr))
            {
                msg_format("You have no room for %s.", o_name);

            }

            /* Pick up the item (if requested and allowed) */
            else
            {
                int okay = TRUE;

                /* Hack -- query every item */
                if (carry_query_flag)
                {
                    char out_val[MAX_NLEN+20];
                    sprintf(out_val, "Pick up %s? ", o_name);

                    okay = get_check(out_val);
                }

                /* Attempt to pick up an object. */
                if (okay)
                {
                    /* Pick up the object */
                    py_pickup_aux(this_o_idx);
                }
            }
        }
    }
}


/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
static int _check_hit(int power)
{
    int k, ac;

    /* Percentile dice */
    k = randint0(100);

    /* Hack -- 5% hit, 5% miss */
    if (k < 10) return (k < 5);

    if (p_ptr->personality == PERS_LAZY)
        if (one_in_(20)) return (TRUE);

    /* Paranoia -- No power */
    if (power <= 0) return (FALSE);

    /* Total armor */
    ac = p_ptr->ac + p_ptr->to_a;

    /* Power competes against Armor */
    if (randint1(power) > ((ac * 3) / 4)) return (TRUE);

    /* Assume miss */
    return (FALSE);
}



/*
 * Handle player hitting a real trap
 */
static void hit_trap(bool break_trap)
{
    int i, num, dam;
    int x = px, y = py;

    /* Get the cave grid */
    cave_type *c_ptr = &cave[y][x];
    feature_type *f_ptr = &f_info[c_ptr->feat];
    int trap_feat_type = have_flag(f_ptr->flags, FF_TRAP) ? f_ptr->subtype : NOT_TRAP;

    cptr name = "a trap";

    /* Disturb the player */
    disturb(0, 0);

    cave_alter_feat(y, x, FF_HIT_TRAP);

    /* Analyze XXX XXX XXX */
    switch (trap_feat_type)
    {
        case TRAP_TRAPDOOR:
        {
            if (p_ptr->levitation)
            {
                msg_print("You fly over a trap door.");

            }
            else
            {
                msg_print("You have fallen through a trap door!");
                sound(SOUND_FALL);
                dam = damroll(2, 8);
                name = "a trap door";

                take_hit(DAMAGE_NOESCAPE, dam, name, -1);

                /* Still alive and autosave enabled */
                if (autosave_l && (p_ptr->chp >= 0))
                    do_cmd_save_game(TRUE);

                prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_DOWN | CFM_RAND_PLACE | CFM_RAND_CONNECT);

                /* Leaving */
                p_ptr->leaving = TRUE;
            }
            break;
        }

        case TRAP_PIT:
        {
            if (p_ptr->levitation)
            {
                msg_print("You fly over a pit trap.");

            }
            else
            {
                msg_print("You have fallen into a pit!");

                dam = damroll(2, 6);
                name = "a pit trap";

                take_hit(DAMAGE_NOESCAPE, dam, name, -1);
            }
            break;
        }

        case TRAP_SPIKED_PIT:
        {
            if (p_ptr->levitation)
            {
                msg_print("You fly over a spiked pit.");

            }
            else
            {
                msg_print("You fall into a spiked pit!");


                /* Base damage */
                name = "a pit trap";

                dam = damroll(2, 6);

                /* Extra spike damage */
                if (randint0(100) < 50)
                {
                    msg_print("You are impaled!");


                    name = "a spiked pit";

                    dam = dam * 2;
                    (void)set_cut(p_ptr->cut + randint1(dam), FALSE);
                }

                /* Take the damage */
                take_hit(DAMAGE_NOESCAPE, dam, name, -1);
            }
            break;
        }

        case TRAP_POISON_PIT:
        {
            if (p_ptr->levitation)
            {
                msg_print("You fly over a spiked pit.");

            }
            else
            {
                msg_print("You fall into a spiked pit!");


                /* Base damage */
                dam = damroll(2, 6);

                name = "a pit trap";


                /* Extra spike damage */
                if (randint0(100) < 50)
                {
                    msg_print("You are impaled on poisonous spikes!");


                    name = "a spiked pit";


                    dam = dam * 2;
                    (void)set_cut(p_ptr->cut + randint1(dam), FALSE);

                    if (res_save_default(RES_POIS))
                        msg_print("The poison does not affect you!");
                    else
                    {
                        dam = dam * 2;
                        (void)set_poisoned(p_ptr->poisoned + randint1(dam), FALSE);
                    }
                }

                /* Take the damage */
                take_hit(DAMAGE_NOESCAPE, dam, name, -1);
            }

            break;
        }

        case TRAP_TY_CURSE:
        {
            msg_print("There is a flash of shimmering light!");

            num = 2 + randint1(3);
            for (i = 0; i < num; i++)
            {
                (void)summon_specific(0, y, x, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
            }

            if (dun_level > randint1(100)) /* No nasty effect for low levels */
            {
                bool stop_ty = FALSE;
                int count = 0;

                do
                {
                    stop_ty = activate_ty_curse(stop_ty, &count);
                }
                while (one_in_(6));
            }
            break;
        }

        case TRAP_TELEPORT:
        {
            msg_print("You hit a teleport trap!");

            teleport_player(100, TELEPORT_PASSIVE);
            break;
        }

        case TRAP_FIRE:
        {
            msg_print("You are enveloped in flames!");

            dam = damroll(4, 6);
            (void)fire_dam(dam, "a fire trap", -1);

            break;
        }

        case TRAP_ACID:
        {
            msg_print("You are splashed with acid!");

            dam = damroll(4, 6);
            (void)acid_dam(dam, "an acid trap", -1);

            break;
        }

        case TRAP_SLOW:
        {
            if (_check_hit(125))
            {
                msg_print("A small dart hits you!");

                dam = damroll(1, 4);
                take_hit(DAMAGE_ATTACK, dam, "a dart trap", -1);

                if (!CHECK_MULTISHADOW()) (void)set_slow(p_ptr->slow + randint0(20) + 20, FALSE);
            }
            else
            {
                msg_print("A small dart barely misses you.");

            }
            break;
        }

        case TRAP_LOSE_STR:
        {
            if (_check_hit(125))
            {
                msg_print("A small dart hits you!");

                dam = damroll(1, 4);
                take_hit(DAMAGE_ATTACK, dam, "a dart trap", -1);

                if (!CHECK_MULTISHADOW()) (void)do_dec_stat(A_STR);
            }
            else
            {
                msg_print("A small dart barely misses you.");

            }
            break;
        }

        case TRAP_LOSE_DEX:
        {
            if (_check_hit(125))
            {
                msg_print("A small dart hits you!");

                dam = damroll(1, 4);
                take_hit(DAMAGE_ATTACK, dam, "a dart trap", -1);

                if (!CHECK_MULTISHADOW()) (void)do_dec_stat(A_DEX);
            }
            else
            {
                msg_print("A small dart barely misses you.");

            }
            break;
        }

        case TRAP_LOSE_CON:
        {
            if (_check_hit(125))
            {
                msg_print("A small dart hits you!");

                dam = damroll(1, 4);
                take_hit(DAMAGE_ATTACK, dam, "a dart trap", -1);

                if (!CHECK_MULTISHADOW()) (void)do_dec_stat(A_CON);
            }
            else
            {
                msg_print("A small dart barely misses you.");

            }
            break;
        }

        case TRAP_BLIND:
        {
            msg_print("A black gas surrounds you!");
            if (!res_save_default(RES_BLIND))
                (void)set_blind(p_ptr->blind + randint0(50) + 25, FALSE);
            break;
        }

        case TRAP_CONFUSE:
        {
            msg_print("A gas of scintillating colors surrounds you!");
            if (!res_save_default(RES_CONF))
                (void)set_confused(p_ptr->confused + randint0(20) + 10, FALSE);
            break;
        }

        case TRAP_POISON:
        {
            msg_print("A pungent green gas surrounds you!");
            if (!res_save_default(RES_POIS))
                (void)set_poisoned(p_ptr->poisoned + randint0(20) + 10, FALSE);
            break;
        }

        case TRAP_SLEEP:
        {
            msg_print("A strange white mist surrounds you!");

            if (!p_ptr->free_act)
            {
                msg_print("You fall asleep.");


                if (ironman_nightmare)
                {
                    msg_print("A horrible vision enters your mind.");


                    /* Pick a nightmare */
                    get_mon_num_prep(get_nightmare, NULL);

                    /* Have some nightmares */
                    have_nightmare(get_mon_num(MAX_DEPTH));

                    /* Remove the monster restriction */
                    get_mon_num_prep(NULL, NULL);
                }
                (void)set_paralyzed(randint1(4), FALSE);
            }
            break;
        }

        case TRAP_TRAPS:
        {
            msg_print("There is a bright flash of light!");

            /* Make some new traps */
            project(0, 1, y, x, 0, GF_MAKE_TRAP, PROJECT_HIDE | PROJECT_JUMP | PROJECT_GRID, -1);

            break;
        }

        case TRAP_ALARM:
        {
            msg_print("An alarm sounds!");

            aggravate_monsters(0);

            break;
        }

        case TRAP_OPEN:
        {
            msg_print("Suddenly, surrounding walls are opened!");
            /*TODO: Fire beams in 4 principle directions that kill adjacent walls ... */
            project(0, 10, y, x, 0, GF_DISINTEGRATE, PROJECT_GRID | PROJECT_HIDE, -1);
            aggravate_monsters(0);

            break;
        }

        case TRAP_ARMAGEDDON:
        {
            static int levs[10] = {0, 0, 20, 10, 5, 3, 2, 1, 1, 1};
            int evil_idx = 0, good_idx = 0;

            int lev;
            msg_print("Suddenly, you are surrounded by immotal beings!");

            /* Summon Demons and Angels */
            for (lev = dun_level; lev >= 20; lev -= 1 + lev/16)
            {
                num = levs[MIN(lev/10, 9)];
                for (i = 0; i < num; i++)
                {
                    int x1 = rand_spread(x, 7);
                    int y1 = rand_spread(y, 5);

                    /* Skip illegal grids */
                    if (!in_bounds(y1, x1)) continue;

                    /* Require line of projection */
                    if (!projectable(py, px, y1, x1)) continue;

                    if (summon_specific(0, y1, x1, lev, SUMMON_ARMAGE_EVIL, (PM_NO_PET)))
                        evil_idx = hack_m_idx_ii;

                    if (summon_specific(0, y1, x1, lev, SUMMON_ARMAGE_GOOD, (PM_NO_PET)))
                    {
                        good_idx = hack_m_idx_ii;
                    }

                    /* Let them fight each other */
                    if (evil_idx && good_idx)
                    {
                        monster_type *evil_ptr = &m_list[evil_idx];
                        monster_type *good_ptr = &m_list[good_idx];
                        evil_ptr->target_y = good_ptr->fy;
                        evil_ptr->target_x = good_ptr->fx;
                        good_ptr->target_y = evil_ptr->fy;
                        good_ptr->target_x = evil_ptr->fx;
                    }
                }
            }
            break;
        }

        case TRAP_PIRANHA:
        {
            msg_print("Suddenly, the room is filled with water with piranhas!");

            /* Water fills room */
            fire_ball_hide(GF_WATER_FLOW, 0, 1, 10);

            /* Summon Piranhas */
            num = 1 + dun_level/20;
            for (i = 0; i < num; i++)
            {
                (void)summon_specific(0, y, x, dun_level, SUMMON_PIRANHA, (PM_ALLOW_GROUP | PM_NO_PET));
            }
            break;
        }
    }

    if (break_trap && is_trap(c_ptr->feat))
    {
        cave_alter_feat(y, x, FF_DISARM);
        msg_print("You destroyed the trap.");
    }
}


void touch_zap_player(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    if ((r_ptr->flags2 & RF2_AURA_REVENGE) && randint0(120) < r_ptr->level)
    {
        if (p_ptr->lightning_reflexes)
        {
            msg_print("You strike so fast that you avoid a retaliatory strike.");
        }
        else
        {
            retaliation_hack = TRUE;
            make_attack_normal(m_idx);
            retaliation_count++; /* Indexes which blow to use per retaliation, but start at 0 ... See py_attack() for initialization.*/
            retaliation_hack = FALSE;
        }
    }

    if (r_ptr->flags2 & RF2_AURA_FIRE)
    {
        if (p_ptr->lightning_reflexes)
        {
            msg_print("You strike so fast that you avoid getting burned.");
        }
        else
        {
            int dam = damroll(1 + (r_ptr->level / 26), 1 + (r_ptr->level / 17));
            dam = res_calc_dam(RES_FIRE, dam);
            if (dam > 0)
            {
                char buf[80];
                

                monster_desc(buf, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
                msg_print("You are suddenly very hot!");
                take_hit(DAMAGE_NOESCAPE, dam, buf, -1);
                if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= RF2_AURA_FIRE;
                handle_stuff();
            }
        }
    }

    if (r_ptr->flags3 & RF3_AURA_COLD)
    {
        if (p_ptr->lightning_reflexes)
        {
            msg_print("You strike so fast that you avoid getting frozen.");
        }
        else
        {
            int dam = damroll(1 + (r_ptr->level / 26), 1 + (r_ptr->level / 17));
            dam = res_calc_dam(RES_COLD, dam);
            if (dam > 0)
            {
                char buf[80];

                monster_desc(buf, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
                msg_print("You are suddenly very cold!");
                take_hit(DAMAGE_NOESCAPE, dam, buf, -1);
                if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_AURA_COLD;
                handle_stuff();
            }
        }
    }

    if (r_ptr->flags2 & RF2_AURA_ELEC)
    {
        if (p_ptr->lightning_reflexes)
        {
            msg_print("You strike so fast that you avoid getting zapped.");
        }
        else
        {
            int dam = damroll(1 + (r_ptr->level / 26), 1 + (r_ptr->level / 17));
            dam = res_calc_dam(RES_ELEC, dam);

            if (dam > 0)
            {
                char buf[80];

                monster_desc(buf, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
                msg_print("You get zapped!");
                take_hit(DAMAGE_NOESCAPE, dam, buf, -1);
                if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= RF2_AURA_ELEC;
                handle_stuff();
            }
        }
    }
}

/* Fractional Blows: 2.75 is stored as 275 and should give 3 blows 75% of the time. */
static int _get_num_blow(int hand)
{
    int num_blow = NUM_BLOWS(hand);
    int result = num_blow / 100;

    if (randint0(100) < (num_blow % 100))
        result++;

    return result;
}

static int _get_num_blow_innate(int which)
{
    int num_blow = p_ptr->innate_attacks[which].blows;
    int result;

    if (which == 0)
        num_blow += p_ptr->innate_attack_info.xtra_blow;

    result = num_blow / 100;
    if (randint0(100) < (num_blow % 100))
        result++;

    return result;
}

static void do_monster_knockback(int x, int y, int dist);

static void innate_attacks(s16b m_idx, bool *fear, bool *mdeath, int mode)
{
    int             dam, base_dam, effect_pow, to_h, chance;
    monster_type    *m_ptr = &m_list[m_idx];
    monster_race    *r_ptr = &r_info[m_ptr->r_idx];
    char            m_name[80];
    int             i, j, k;
    int             delay_sleep = 0;
    int             delay_stasis = 0;
    int             delay_paralysis = 0;
    bool            delay_quake = FALSE;
    int             drain_amt = 0;
    int             steal_ct = 0;
    int             hit_ct = 0;
    const int       max_drain_amt = _max_vampiric_drain();

    set_monster_csleep(m_idx, 0);
    monster_desc(m_name, m_ptr, 0);

    if (p_ptr->afraid)
    {
        if (!fear_allow_melee(m_idx))
        {
            if (m_ptr->ml)
                msg_format("You are too afraid to attack %s!", m_name);
            else
                msg_format ("There is something scary in your way!");
            return;
        }
    }

    for (i = 0; i < p_ptr->innate_attack_ct; i++)
    {
        innate_attack_ptr a = &p_ptr->innate_attacks[i];
        int               blows = _get_num_blow_innate(i);

        if (a->flags & INNATE_SKIP) continue;

        for (j = 0; j < blows; j++)
        {
            to_h = a->to_h + p_ptr->to_h_m;
            chance = p_ptr->skills.thn + (to_h * BTH_PLUS_ADJ);
            if (test_hit_norm(chance, MON_AC(r_ptr, m_ptr), m_ptr->ml))
            {
                int dd = a->dd + p_ptr->innate_attack_info.to_dd;

                hit_ct++;
                sound(SOUND_HIT);
                msg_format(a->msg, m_name);

                base_dam = damroll(dd, a->ds);
                if ( ((a->flags & INNATE_VORPAL) || mode == DRAGON_REND) 
                  && one_in_(6) )
                {
                    int m = 2;
                    while (one_in_(4))
                        m++;

                    base_dam *= m;
                    switch (m)
                    {
                    case 2: msg_format("You gouge %s!", m_name); break;
                    case 3: msg_format("You maim %s!", m_name); break;
                    case 4: msg_format("You carve %s!", m_name); break;
                    case 5: msg_format("You cleave %s!", m_name); break;
                    case 6: msg_format("You smite %s!", m_name); break;
                    case 7: msg_format("You eviscerate %s!", m_name); break;
                    default: msg_format("You shred %s!", m_name); break;
                    }
                }
                base_dam += a->to_d;
                if (!(a->flags & INNATE_NO_DAM))
                {
                    critical_t crit = critical_norm(a->weight, to_h, 0, mode, HAND_NONE);
                    if (crit.desc)
                    {
                        base_dam = base_dam * crit.mul/100 + crit.to_d;
                        msg_print(crit.desc);
                    }
                }

                dam = base_dam + p_ptr->to_d_m;
                if (a->flags & INNATE_NO_DAM)
                {
                    base_dam = 0;
                    effect_pow = p_ptr->lev * 2;
                    dam = 0;
                }
                else
                    effect_pow = base_dam;
                if (dam < 0) 
                    dam = 0;
                dam = mon_damage_mod(m_ptr, dam, FALSE);
                if (dam > 0) 
                    anger_monster(m_ptr);

                for (k = 0; k < MAX_INNATE_EFFECTS && !*mdeath; k++)
                {
                    int e = a->effect[k];
                    int p = a->effect_chance[k];

                    if (p == 0) p = 100;
                    if (!e && k == 0) 
                    {
                        if (a->flags & INNATE_NO_DAM)
                            continue;
                        e = GF_MISSILE;
                    }

                    /* Hack: When I decreased monster base resistance (89% -> 50%)
                       I inadvertantly made dragon bite attacks too strong. Let's
                       emulate the way branded weapons work, but only when the elemental
                       effect is added on top of some base effect (generally GF_MISSILE). */
                    if (k && mode == DRAGON_DEADLY_BITE)
                    {
                        switch (e)
                        {
                        case GF_ACID:
                            if (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= r_ptr->flagsr & RFR_EFF_IM_ACID_MASK;
                                e = 0;
                            }
                            break;
                        case GF_FIRE:
                            if (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK;
                                e = 0;
                            }
                            break;
                        case GF_COLD:
                            if (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= r_ptr->flagsr & RFR_EFF_IM_COLD_MASK;
                                e = 0;
                            }
                            break;
                        case GF_ELEC:
                            if (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK;
                                e = 0;
                            }
                            break;
                        case GF_POIS:
                            if (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= r_ptr->flagsr & RFR_EFF_IM_POIS_MASK;
                                e = 0;
                            }
                            break;
                        case GF_CONFUSION:
                            if (r_ptr->flags3 & RF3_NO_CONF)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flags3 |= RF3_NO_CONF;
                                e = 0;
                            }
                            break;
                        case GF_SOUND:
                            if (r_ptr->flagsr & RFR_RES_SOUN)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= RFR_RES_SOUN;
                                e = 0;
                            }
                            break;
                        case GF_SHARDS:
                            if (r_ptr->flagsr & RFR_RES_SHAR)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= RFR_RES_SHAR;
                                e = 0;
                            }
                            break;
                        case GF_NETHER:
                            if (r_ptr->flagsr & RFR_RES_NETH)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= RFR_RES_NETH;
                                e = 0;
                            }
                            break;
                        case GF_NEXUS:
                            if (r_ptr->flagsr & RFR_RES_NEXU)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= RFR_RES_NEXU;
                                e = 0;
                            }
                            break;
                        case GF_CHAOS:
                            if (r_ptr->flagsr & RFR_RES_CHAO)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= RFR_RES_CHAO;
                                e = 0;
                            }
                            break;
                        case GF_DISENCHANT:
                            if (r_ptr->flagsr & RFR_RES_DISE)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= RFR_RES_DISE;
                                e = 0;
                            }
                            break;
                        case GF_LITE:
                            if (r_ptr->flagsr & RFR_RES_LITE)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= RFR_RES_LITE;
                                e = 0;
                            }
                            break;
                        case GF_DARK:
                            if (r_ptr->flagsr & RFR_RES_DARK)
                            {
                                if (is_original_ap_and_seen(m_ptr)) 
                                    r_ptr->r_flagsr |= RFR_RES_DARK;
                                e = 0;
                            }
                            break;
                        }
                    }

                    if (!e) continue;
                    if (randint1(100) > p) continue;

                    switch (e)
                    {
                    case GF_MISSILE:
                        *mdeath = mon_take_hit(m_idx, dam, fear, NULL);
                        break;
                    case GF_DISENCHANT:
                        *mdeath = mon_take_hit(m_idx, dam, fear, NULL);
                        if (!(*mdeath) && one_in_(7))
                            dispel_monster_status(m_idx);
                        break;
                    case GF_OLD_SLEEP:
                        delay_sleep += effect_pow/2;
                        break;
                    case GF_STASIS:
                        delay_stasis += effect_pow;
                        break;
                    case GF_PARALYSIS:
                        delay_paralysis += effect_pow;
                        break;
                    case GF_OLD_CONF:
                    case GF_OLD_SLOW:
                    case GF_STUN:
                        project(0, 0, m_ptr->fy, m_ptr->fx, effect_pow, e, PROJECT_KILL|PROJECT_HIDE, -1);
                        *mdeath = (m_ptr->r_idx == 0);
                        break;
                    case GF_DRAIN_MANA:
                    {
                        int amt = MIN(effect_pow, max_drain_amt - drain_amt);
                        if (amt && project(0, 0, m_ptr->fy, m_ptr->fx, amt, e, PROJECT_KILL|PROJECT_HIDE, -1))
                            drain_amt += amt;
                        *mdeath = (m_ptr->r_idx == 0);
                        break;
                    }
                    case GF_OLD_DRAIN:
                        if (monster_living(r_ptr) && project(0, 0, m_ptr->fy, m_ptr->fx, effect_pow, e, PROJECT_KILL|PROJECT_HIDE, -1))
                        {
                            int amt = MIN(effect_pow, max_drain_amt - drain_amt);
                            if (prace_is_(MIMIC_BAT))
                            {
                                vampire_feed(amt);
                            }
                            else
                            {
                                msg_format("You drain life from %s!", m_name);
                                hp_player(amt);
                            }
                            drain_amt += amt;
                        }
                        *mdeath = (m_ptr->r_idx == 0);
                        break;
                    case GF_STEAL:
                        if (leprechaun_steal(m_idx))
                            steal_ct++;
                        break;
                    case GF_QUAKE:
                        if (base_dam > 50 || one_in_(7))
                        {
                            delay_quake = TRUE;
                            project(0, 0, m_ptr->fy, m_ptr->fx, base_dam, GF_STUN, PROJECT_KILL|PROJECT_HIDE, -1);
                        }
                        break;
                    default:
                        project(0, 0, m_ptr->fy, m_ptr->fx, effect_pow, e, PROJECT_KILL|PROJECT_HIDE, -1);
                        *mdeath = (m_ptr->r_idx == 0);
                    }
                }
                /* TODO: Should rings of power brand innate attacks? */
                touch_zap_player(m_idx);

                if (a->flags & INNATE_EXPLODE)
                {
                    possessor_explode(dam);
                    return;
                }

                if (*mdeath) return;

                on_p_hit_m(m_idx);

                if (mode == DRAGON_SNATCH)
                {
                    msg_format("You grab %s in your jaws.", m_name);
                    monster_toss(m_idx);
                    return;
                }
            }
            else
            {
                sound(SOUND_MISS);
                msg_format("You miss %s.", m_name);
            }

            if (mode == WEAPONMASTER_RETALIATION)
                break;
        }
        if (mode == WEAPONMASTER_RETALIATION)
            break;
    }
    if (mode == DRAGON_TAIL_SWEEP && !*mdeath && hit_ct)
    {
        int dist = randint1(1 + p_ptr->lev/20);
        do_monster_knockback(m_ptr->fx, m_ptr->fy, dist);
    }
    if (delay_quake)
        earthquake(py, px, 10);
    if (delay_sleep && !*mdeath)
        project(0, 0, m_ptr->fy, m_ptr->fx, delay_sleep, GF_OLD_SLEEP, PROJECT_KILL|PROJECT_HIDE, -1);
    if (delay_stasis && !*mdeath)
        project(0, 0, m_ptr->fy, m_ptr->fx, delay_stasis, GF_STASIS, PROJECT_KILL|PROJECT_HIDE, -1);
    if (delay_paralysis && !*mdeath)
        project(0, 0, m_ptr->fy, m_ptr->fx, delay_paralysis, GF_PARALYSIS, PROJECT_KILL|PROJECT_HIDE, -1);
    if (steal_ct && !*mdeath)
    {
        if (mon_save_p(m_ptr->r_idx, A_DEX))
            msg_print("You fail to run away!");
        else
            teleport_player(25 + p_ptr->lev/2, 0L);
    }
}

/*
 * Player attacks a (poor, defenseless) creature        -RAK-
 *
 * If no "weapon" is available, then "punch" the monster one time.
 */

static int drain_left = 0;
bool melee_hack = FALSE;
static bool fear_stop = FALSE;

static int calculate_dir(int sx, int sy, int tx, int ty)
{
    int dir;
    for (dir = 0; dir <= 9; dir++)
    {
        int x = sx + ddx[dir];
        int y = sy + ddy[dir];

        if (x == tx && y == ty) return dir;
    }
    return 5;
}

static int get_next_dir(int dir)
{
    switch (dir)
    {
    case 1: return 4;
    case 4: return 7;
    case 7: return 8;
    case 8: return 9;
    case 9: return 6;
    case 6: return 3;
    case 3: return 2;
    case 2: return 1;
    }
    return 5;
}

static void do_monster_knockback(int x, int y, int dist)
{   
    monster_type   *m_ptr = &m_list[cave[y][x].m_idx];   
    int             dir = calculate_dir(px, py, x, y);

    if (dir != 5)
    {
        int i;
        int msec = delay_factor * delay_factor * delay_factor;

        for (i = 0; i < dist; i++)
        {
            int ty = y, tx = x;
            int oy = y, ox = x;

            y += ddy[dir];
            x += ddx[dir];
            if (cave_empty_bold(y, x))
            {
                ty = y;
                tx = x;
            }
            if (ty != oy || tx != ox)
            {
                int m_idx = cave[oy][ox].m_idx;

                cave[oy][ox].m_idx = 0;
                cave[ty][tx].m_idx = m_idx;
                m_ptr->fy = ty;
                m_ptr->fx = tx;
    
                Term_fresh();
                update_mon(m_idx, TRUE);
                lite_spot(oy, ox);
                lite_spot(ty, tx);
    
                if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
                    p_ptr->update |= PU_MON_LITE;
                    
                Term_xtra(TERM_XTRA_DELAY, msec);
                Term_fresh();
            }
            else
                break;
        }
    }
}

static bool py_attack_aux(int y, int x, bool *fear, bool *mdeath, s16b hand, int mode)
{
    int             num = 0, k, k2, dam_tot = 0, bonus, chance;
    int             to_h = 0, to_d = 0;
    int             touch_ct = 0;
    critical_t      crit;
    cave_type      *c_ptr = &cave[y][x];
    monster_type   *m_ptr = NULL;
    monster_race   *r_ptr = NULL;
    object_type    *o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);
    char            o_name[MAX_NLEN];
    u32b            flgs[TR_FLAG_SIZE] = {0};
    char            m_name[MAX_NLEN];
    bool            success_hit = FALSE;
    bool            backstab = FALSE;
    bool            vorpal_cut = FALSE;
    int             chaos_effect = 0;
    bool            stab_fleeing = FALSE;
    bool            fuiuchi = FALSE;
    bool            monk_attack = FALSE;
    bool            duelist_attack = FALSE;
    bool            perfect_strike = FALSE;
    bool            do_quake = FALSE;
    bool            weak = FALSE;
    bool            drain_msg = TRUE;
    int             drain_result = 0, drain_heal = 0;
    bool            can_drain = FALSE;
    int             num_blow;
    bool            is_human;
    bool            is_lowlevel;
    bool            zantetsu_mukou = FALSE, e_j_mukou = FALSE;
    int             knock_out = 0;
    int             dd, ds;
    bool            hit_ct = 0;
    bool            poison_needle = FALSE;
    bool            eviscerator = FALSE;

    if (!c_ptr->m_idx)
    {
        msg_print("You swing wildly at nothing.");
        return FALSE;
    }

    m_ptr = &m_list[c_ptr->m_idx];
    r_ptr = &r_info[m_ptr->r_idx];
    is_human = (r_ptr->d_char == 'p');
    is_lowlevel = (r_ptr->level < (p_ptr->lev - 15));

    weapon_flags(hand, flgs);
    if (o_ptr)
    {
        object_desc(o_name, o_ptr, OD_NAME_ONLY);
        if (weaponmaster_get_toggle() == TOGGLE_SHIELD_BASH)
        {
            dd = 3;
            ds = k_info[o_ptr->k_idx].ac;
            to_h = o_ptr->to_a;
            to_d = o_ptr->to_a;

            to_h += 2*o_ptr->to_h;
            to_d += 2*o_ptr->to_d;
        }
        else
        {
            dd = o_ptr->dd;
            ds = o_ptr->ds;
            to_h = o_ptr->to_h;
            to_d = o_ptr->to_d;
        }
        if (o_ptr->name1 == ART_ZANTETSU && r_ptr->d_char == 'j')
            zantetsu_mukou = TRUE;
        if (o_ptr->name1 == ART_EXCALIBUR_J && r_ptr->d_char == 'S')
            e_j_mukou = TRUE;
    }
    else
    {
        sprintf(o_name, "Your fists");
        dd = 1;
        ds = 1;
        to_h = 0;
        to_d = 0;
        if ( (p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_FORCETRAINER || p_ptr->pclass == CLASS_MYSTIC) 
          && !p_ptr->riding )
        {
            monk_attack = TRUE;
        }
    }

    if (p_ptr->painted_target)
    {
        p_ptr->painted_target_idx = 0;
        p_ptr->painted_target_ct = 0;
    }

    switch (p_ptr->pclass)
    {
    case CLASS_DUELIST:
        if (c_ptr->m_idx == p_ptr->duelist_target_idx)
            duelist_attack = TRUE;
        break;

    case CLASS_WEAPONMASTER:
        if (!p_ptr->sneak_attack)
            break;
        /* vvvvv FALL THRU vvvvvv */
    case CLASS_ROGUE:
    case CLASS_NINJA:
        if (o_ptr && !p_ptr->weapon_info[hand].icky_wield)
        {
            int tmp = p_ptr->lev * 6 + (p_ptr->skills.stl + 10) * 4;
            if (p_ptr->monlite && (mode != HISSATSU_NYUSIN)) tmp /= 3;
            if (p_ptr->cursed & TRC_AGGRAVATE) tmp /= 2;
            if (r_ptr->level > (p_ptr->lev * p_ptr->lev / 20 + 10)) tmp /= 3;
            if (MON_CSLEEP(m_ptr) && m_ptr->ml)
            {
                /* Can't backstab creatures that we can't see, right? */
                backstab = TRUE;
            }
            else if ((p_ptr->special_defense & NINJA_S_STEALTH) && (randint0(tmp) > (r_ptr->level+20)) && m_ptr->ml && !(r_ptr->flagsr & RFR_RES_ALL))
            {
                fuiuchi = TRUE;
            }
            else if (MON_MONFEAR(m_ptr) && m_ptr->ml)
            {
                stab_fleeing = TRUE;
            }
        }
        break;

    case CLASS_SCOUT:
        if (p_ptr->ambush && o_ptr && !p_ptr->weapon_info[hand].icky_wield)
        {
            if (MON_CSLEEP(m_ptr) && m_ptr->ml)
                backstab = TRUE;
        }
        break;

    case CLASS_MONSTER:
        if (p_ptr->ambush && o_ptr)
        {
            if (MON_CSLEEP(m_ptr) && m_ptr->ml)
                backstab = TRUE;
        }
        break;
    }

    if (o_ptr)
    {
        if (r_ptr->level + 10 > p_ptr->lev)
            skills_weapon_gain(o_ptr->tval, o_ptr->sval);
    }
    else
    {
        if (r_ptr->level + 10 > p_ptr->lev)
            skills_martial_arts_gain();
    }
    
    set_monster_csleep(c_ptr->m_idx, 0);
    monster_desc(m_name, m_ptr, 0);

    if (mode == PY_POWER_ATTACK)
    {
        to_h += 10;
        to_d += p_ptr->lev / 2;
    }

    /* Calculate the "attack quality" */
    bonus = p_ptr->weapon_info[hand].to_h + to_h;
    if (mode == WEAPONMASTER_KNOCK_BACK) bonus -= 20;
    if (mode == WEAPONMASTER_REAPING) bonus -= 40;
    if (mode == MAULER_KNOCKOUT_BLOW) bonus -= 50;
    if (mode == WEAPONMASTER_CUNNING_STRIKE) bonus += 20;
    if (mode == WEAPONMASTER_SMITE_EVIL && hand == 0 && (r_ptr->flags3 & RF3_EVIL)) bonus += 200;
    if (duelist_attack) bonus += p_ptr->lev;

    chance = (p_ptr->skills.thn + (bonus * BTH_PLUS_ADJ));
    if (chance > 0)
        chance = chance * p_ptr->weapon_info[hand].dual_wield_pct / 1000;

    if (mode == HISSATSU_IAI) chance += 60;
    if (p_ptr->special_defense & KATA_KOUKIJIN) chance += 150;
    if (p_ptr->sutemi) chance = MAX(chance * 3 / 2, chance + 60);

    chance += virtue_current(VIRTUE_VALOUR) / 10;

    num_blow = _get_num_blow(hand);

    if (mode == HISSATSU_COLD) num_blow += 2;
    if (mode == WEAPONMASTER_CUNNING_STRIKE) num_blow = (num_blow + 1)/2;

    if (o_ptr && o_ptr->tval == TV_SWORD && o_ptr->sval == SV_POISON_NEEDLE) 
    {
        poison_needle = TRUE;
        num_blow = 1;
    }
    if (o_ptr && o_ptr->name1 == ART_EVISCERATOR) 
    {
        eviscerator = TRUE;
        num_blow = 1;
    }

    /* Attack once for each legal blow */
    while ((num++ < num_blow) && !p_ptr->is_dead)
    {
    bool do_whirlwind = FALSE;

        /* We now check fear on every blow, and only lose energy equal to the number of blows attempted.
           Monsters with AURA_FEAR can induce fear any time the player damages them! */
        if (p_ptr->afraid)
        {
            if (!fear_allow_melee(c_ptr->m_idx))
            {
                if (m_ptr->ml)
                    msg_format("You are too afraid to attack %s!", m_name);
                else
                    msg_format ("There is something scary in your way!");

                fear_stop = TRUE;
                break;
            }
        }

        if (p_ptr->stun >= 100) /* Grand Master Mystic retaliation knocked the player out! */
            break;

        if (p_ptr->paralyzed)
            break;

        /* Weaponmaster Whirlwind turns a normal strike into a sweeping whirlwind strike */
        if (p_ptr->whirlwind && mode == 0)
        {
            if (one_in_(5))
                do_whirlwind = TRUE;
        }

        if (poison_needle || mode == HISSATSU_KYUSHO || mode == MYSTIC_KILL)
        {
            int n = p_ptr->weapon_ct;

            if (mode == HISSATSU_3DAN)
                n *= 2;

            success_hit = one_in_(n);
        }
        else if ((p_ptr->pclass == CLASS_NINJA) && ((backstab || fuiuchi) && !(r_ptr->flagsr & RFR_RES_ALL))) success_hit = TRUE;
        else if (duelist_attack && one_in_(2))
        {
            perfect_strike = TRUE;
            success_hit = TRUE;
        }
        else if (weaponmaster_get_toggle() == TOGGLE_BURNING_BLADE) success_hit = TRUE;
        else success_hit = test_hit_norm(chance, MON_AC(r_ptr, m_ptr), m_ptr->ml);

        if (mode == HISSATSU_MAJIN)
        {
            if (one_in_(2))
                success_hit = FALSE;
        }

        if (success_hit)
        {
            int vorpal_chance = 4;

            if (have_flag(flgs, TR_VORPAL2))
                vorpal_chance = 2;

            if (eviscerator && !duelist_attack) num_blow++;

            hit_ct++;
            sound(SOUND_HIT);

            /* Uber mega-hack ... aren't they all?! */
            if (weaponmaster_is_(WEAPONMASTER_STAVES))
            {
                bool update = FALSE;
                if (p_ptr->elaborate_defense == 0) update = TRUE;
                p_ptr->elaborate_defense = 1;
                if (update)
                {
                    p_ptr->update |= PU_BONUS;
                    handle_stuff();
                }
            }

            if (backstab) msg_format("You cruelly stab %s!", m_name);
            else if (fuiuchi) msg_format("You make a surprise attack, and hit %s with a powerful blow!", m_name);
            else if (stab_fleeing) msg_format("You backstab %s!",  m_name);
            else if (perfect_strike) msg_format("You land a perfect strike against %s.", m_name);
            else if (!monk_attack) msg_format("You hit %s.", m_name);

            /* Hack -- bare hands do one damage */
            k = 1;

            if ((have_flag(flgs, TR_CHAOTIC)) && one_in_(7))
            {
                if (one_in_(10)) virtue_add(VIRTUE_CHANCE, 1);
                if (randint1(5) < 4) chaos_effect = 1;
                else if (one_in_(5)) chaos_effect = 3;
            }

            /* Vampiric drain */
            if ( have_flag(flgs, TR_VAMPIRIC) 
              || chaos_effect == 1 
              || mode == HISSATSU_DRAIN 
              || hex_spelling(HEX_VAMP_BLADE)
              || weaponmaster_get_toggle() == TOGGLE_BLOOD_BLADE
              || mauler_get_toggle() == MAULER_TOGGLE_DRAIN )
            {
                if (monster_living(r_ptr))
                    can_drain = TRUE;
            }

            vorpal_cut = FALSE;
            if (!zantetsu_mukou) /* No jelly cuts with Zantetsuken */
            {
                if (have_flag(flgs, TR_VORPAL) && p_ptr->vorpal && vorpal_chance > 3) vorpal_chance = 3;
                if (have_flag(flgs, TR_VORPAL) || have_flag(flgs, TR_VORPAL2) || hex_spelling(HEX_RUNESWORD) || p_ptr->vorpal)
                {
                    if (randint1(vorpal_chance*3/2) == 1)
                        vorpal_cut = TRUE;
                }
            }

            if (monk_attack && mode != MYSTIC_KILL)
            {
                int special_effect = 0, stun_effect = 0;
                martial_arts *ma_ptr = &ma_blows[monk_get_attack_idx()];
                int resist_stun = 0;

                if (r_ptr->flags1 & RF1_UNIQUE) resist_stun += (10*r_ptr->level);
                if (r_ptr->flags3 & RF3_NO_CONF) resist_stun += 33;
                if (r_ptr->flags3 & RF3_NO_SLEEP) resist_stun += 33;
                if ((r_ptr->flags3 & RF3_UNDEAD) || (r_ptr->flags3 & RF3_NONLIVING))
                    resist_stun += 66;

                k = damroll(ma_ptr->dd + p_ptr->weapon_info[hand].to_dd, ma_ptr->ds + p_ptr->weapon_info[hand].to_ds);
                k = tot_dam_aux_monk(k, m_ptr, mode);

                if (p_ptr->special_attack & ATTACK_SUIKEN) k *= 2; /* Drunken Boxing! */

                if (ma_ptr->effect == MA_KNEE)
                {
                    if (r_ptr->flags1 & RF1_MALE)
                    {
                        msg_format("You hit %s in the groin with your knee!", m_name);
                        sound(SOUND_PAIN);
                        special_effect = MA_KNEE;
                    }
                    else
                        msg_format(ma_ptr->desc, m_name);
                }

                else if (ma_ptr->effect == MA_SLOW)
                {
                    if (!((r_ptr->flags1 & RF1_NEVER_MOVE) ||
                        my_strchr("~#{}.UjmeEv$,DdsbBFIJQSXclnw!=?", r_ptr->d_char)))
                    {
                        msg_format("You kick %s in the ankle.", m_name);
                        special_effect = MA_SLOW;
                    }
                    else msg_format(ma_ptr->desc, m_name);
                }
                else
                {
                    if (ma_ptr->effect)
                        stun_effect = (ma_ptr->effect / 2) + randint1(ma_ptr->effect / 2);
                    if (mode == MYSTIC_STUN)
                        stun_effect *= 2;
                    msg_format(ma_ptr->desc, m_name);
                }

                crit = monk_get_critical(ma_ptr, hand, mode);
                if (crit.desc)
                {
                    k = k * crit.mul/100 + crit.to_d;
                    msg_print(crit.desc);
                }

                if ((special_effect == MA_KNEE) && ((k + p_ptr->weapon_info[hand].to_d) < m_ptr->hp))
                {
                    msg_format("%^s moans in agony!", m_name);
                    stun_effect = 7 + randint1(13);
                    resist_stun /= 3;
                }

                else if ((special_effect == MA_SLOW) && ((k + p_ptr->weapon_info[hand].to_d) < m_ptr->hp))
                {
                    if (!(r_ptr->flags1 & RF1_UNIQUE) &&
                        (randint1(p_ptr->lev) > r_ptr->level) &&
                        m_ptr->mspeed > 60)
                    {
                        msg_format("%^s starts limping slower.", m_name);
                        m_ptr->mspeed -= 10;
                    }
                }

                /* Massive Hack: Monk stunning is now greatly biffed! */
                if (r_ptr->flags1 & RF1_UNIQUE) stun_effect /= 2;
                if (r_ptr->flags3 & RF3_NO_STUN) stun_effect = 0;
                if (r_ptr->flagsr & RFR_RES_SOUN) stun_effect = 0;

                if (mon_save_p(m_ptr->r_idx, A_DEX))
                    stun_effect = 0;

                if (stun_effect && ((k + p_ptr->weapon_info[hand].to_d) < m_ptr->hp))
                {
                    if (p_ptr->lev > randint1(r_ptr->level + resist_stun + 10))
                    {
                        if (MON_STUNNED(m_ptr))
                            stun_effect /= 4;

                        if (stun_effect == 0)
                        {
                            /* No message */
                        }
                        else if (set_monster_stunned(c_ptr->m_idx, stun_effect + MON_STUNNED(m_ptr)))
                            msg_format("%^s is stunned.", m_name);
                        else
                            msg_format("%^s is more stunned.", m_name);
                    }
                }
                if (mode == MYSTIC_KNOCKOUT)
                {
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_ALL;
                        msg_format("%^s is immune.", m_name);
                    }
                    else if (mon_save_p(m_ptr->r_idx, A_DEX))
                    {
                        msg_format("%^s resists.", m_name);
                    }
                    else
                    {
                        msg_format("%^s is knocked out.", m_name);
                        knock_out++;        
                        /* No more retaliation this round! */                    
                        retaliation_count = 100; /* Any number >= 4 will do ... */
                    }
                }
            }
            /* Handle normal weapon */
            else if (o_ptr)
            {
                if (weaponmaster_get_toggle() == TOGGLE_ORDER_BLADE)
                    k = (dd + p_ptr->weapon_info[hand].to_dd) * (ds + p_ptr->weapon_info[hand].to_ds);
                else
                    k = damroll(dd + p_ptr->weapon_info[hand].to_dd, ds + p_ptr->weapon_info[hand].to_ds);
                k = tot_dam_aux(o_ptr, k, m_ptr, hand, mode, FALSE);

                if (backstab)
                {
                    if (p_ptr->pclass == CLASS_SCOUT)
                        k *= 3;
                    else
                        k *= (3 + (p_ptr->lev / 20));
                }
                else if (fuiuchi)
                {
                    k = k*(5+(p_ptr->lev*2/25))/2;
                }
                else if (stab_fleeing)
                {
                    k = (3 * k) / 2;
                }

                if (mode == MAULER_CRUSHING_BLOW)
                {   /* 2x to 5x */
                    k = k * (5 + randint1(p_ptr->lev/5)) / 3;
                }

                if ( (have_flag(flgs, TR_IMPACT) && (k > 50 || one_in_(7))) 
                  || chaos_effect == 2 
                  || mode == HISSATSU_QUAKE
                  || (mauler_get_toggle() == MAULER_TOGGLE_SHATTER && (k > 50 || one_in_(7))) )
                {
                    do_quake = TRUE;
                    if ( k <= 50
                      || (r_ptr->flagsr & RFR_RES_ALL)
                      || (r_ptr->flags3 & RF3_NO_STUN)
                      || ((r_ptr->flags1 & RF1_UNIQUE) && mon_save_p(m_ptr->r_idx, A_STR)) )
                    {
                    }
                    else
                    {
                        msg_format("%^s is stunned.", m_name);
                        set_monster_stunned(c_ptr->m_idx, MAX(MON_STUNNED(m_ptr), 3 + randint1(3)));
                    }
                }

                if (have_flag(flgs, TR_STUN) && randint1(100) < k)
                {
                    if ( (r_ptr->flagsr & RFR_RES_ALL)
                      || (r_ptr->flags3 & RF3_NO_STUN)
                      || ((r_ptr->flags1 & RF1_UNIQUE) && mon_save_p(m_ptr->r_idx, A_STR)) )
                    {
                    }
                    else
                    {
                        msg_format("%^s is stunned.", m_name);
                        set_monster_stunned(c_ptr->m_idx, MAX(MON_STUNNED(m_ptr), 3 + randint1(3)));
                    }
                }

                if (!poison_needle
                 && mode != HISSATSU_KYUSHO
                 && mode != MYSTIC_KILL
                 && weaponmaster_get_toggle() != TOGGLE_ORDER_BLADE 
                 && !have_flag(flgs, TR_ORDER) )
                {
                    int bonus = 0;
                    if (mode == WEAPONMASTER_SMITE_EVIL && hand == 0 && (r_ptr->flags3 & RF3_EVIL)) bonus = 200;
                    crit = critical_norm(o_ptr->weight, to_h, p_ptr->weapon_info[hand].to_h + bonus, mode, hand);
                    if (crit.desc)
                    {
                        k = k * crit.mul/100 + crit.to_d;
                        msg_print(crit.desc);
                    }
                }

                k += k * p_ptr->weapon_info[hand].to_mult / 100;

                drain_result = k;
                k2 = k;

                if (vorpal_cut)
                {
                    int mult = 2;
                    if (o_ptr->name1 == ART_CHAINSWORD && !one_in_(2))
                    {
                        char chainsword_noise[1024];
                        if (!get_rnd_line("chainswd.txt", 0, chainsword_noise))
                            msg_print(chainsword_noise);
                    }

                    if (o_ptr->name1 == ART_VORPAL_BLADE)
                        msg_print("Your Vorpal Blade goes snicker-snack!");
                    else
                        msg_format("Your weapon cuts deep into %s!", m_name);

                    while (one_in_(vorpal_chance))
                        mult++;

                    k *= mult;

                    /* Ouch! */
                    if (((r_ptr->flagsr & RFR_RES_ALL) ? k/100 : k) > m_ptr->hp)
                        msg_format("You cut %s in half!", m_name);
                    else
                    {
                        switch (mult)
                        {
                        case 2: msg_format("You gouge %s!", m_name); break;
                        case 3: msg_format("You maim %s!", m_name); break;
                        case 4: msg_format("You carve %s!", m_name); break;
                        case 5: msg_format("You cleave %s!", m_name); break;
                        case 6: msg_format("You smite %s!", m_name); break;
                        case 7: msg_format("You eviscerate %s!", m_name); break;
                        default: msg_format("You shred %s!", m_name); break;
                        }
                    }
                    drain_result = drain_result * 3 / 2;
                }

                k += to_d;
                drain_result += to_d;
            }

            /* Apply the player damage bonuses */
            k += p_ptr->weapon_info[hand].to_d;
            drain_result += p_ptr->weapon_info[hand].to_d;

            if (mode == HISSATSU_SUTEMI || mode == HISSATSU_3DAN) k *= 2;
            if (mode == HISSATSU_SEKIRYUKA && !monster_living(r_ptr)) k = 0;
            if (mode == HISSATSU_SEKIRYUKA && !p_ptr->cut) k /= 2;

            if (k < 0) k = 0;

            if (mode == HISSATSU_ZANMA && !(!monster_living(r_ptr) && (r_ptr->flags3 & RF3_EVIL)))
                k = 0;

            if (zantetsu_mukou)
            {
                msg_print("You cannot cut such a elastic thing!");
                k = 0;
            }

            if (e_j_mukou)
            {
                msg_print("Spiders are difficult for you to deal with!");
                k /= 2;
            }

            if (mode == HISSATSU_MINEUCHI)
            {
                int tmp = (10 + randint1(15) + p_ptr->lev / 5);

                k = 0;
                anger_monster(m_ptr);

                if (!(r_ptr->flags3 & (RF3_NO_STUN)))
                {
                    if (MON_STUNNED(m_ptr))
                    {
                        msg_format("%s is more dazed.", m_name);
                        tmp /= 2;
                    }
                    else
                    {
                        msg_format("%s is dazed.", m_name);
                    }
                    (void)set_monster_stunned(c_ptr->m_idx, MON_STUNNED(m_ptr) + tmp);
                }
                else
                {
                    msg_format("%s is not effected.", m_name);
                }
            }

            /* Modify the damage */
            k = mon_damage_mod(
                m_ptr, 
                k, 
                (o_ptr && o_ptr->tval == TV_POLEARM && o_ptr->sval == SV_DEATH_SCYTHE) 
             || (p_ptr->pclass == CLASS_BERSERKER && one_in_(2))
             || mode == WEAPONMASTER_CRUSADERS_STRIKE
            );

            if (duelist_attack)
            {
                int d = k;
                /* Duelist: Careful Aim */
                if (duelist_attack && 
                    p_ptr->lev >= 10) 
                {
                    k += d;
                }
                if ( p_ptr->lev >= 15    /* Hamstring */
                    && !(r_ptr->flags1 & (RF1_UNIQUE))
                    && !mon_save_p(m_ptr->r_idx, A_DEX) )
                {
                    msg_format("You hamstring %s.", m_name);
                    set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50);
                }
                if ( p_ptr->lev >= 20    /* Wounding Strike */
                    && !mon_save_p(m_ptr->r_idx, A_DEX) )
                {
                    msg_format("%^s is dealt a wounding strike.", m_name);
                    k += MIN(m_ptr->hp / 5, randint1(3) * d);
                    drain_result = k;
                }
                if ( p_ptr->lev >= 25    /* Stunning Blow */
                    && !(r_ptr->flags3 & (RF3_NO_STUN))
                    && !mon_save_p(m_ptr->r_idx, A_DEX) )
                {
                    msg_format("%^s is dealt a stunning blow.", m_name);
                    set_monster_stunned(c_ptr->m_idx, MAX(MON_STUNNED(m_ptr), 2));
                }
                if ( p_ptr->lev >= 40    /* Greater Wounding Strike */
                    && !mon_save_p(m_ptr->r_idx, A_DEX) )
                {
                    msg_format("%^s is dealt a *WOUNDING* strike.", m_name);
                    k += MIN(m_ptr->hp * 2 / 5, rand_range(2, 10) * d);
                    drain_result = k;
                }
            }
            else if (p_ptr->pclass == CLASS_DUELIST && p_ptr->lev >= 30)
            {
                /* Duelist: Careful Aim vs a non-target */
                k = k * 3 / 2;
            }

            if (poison_needle || mode == HISSATSU_KYUSHO || mode == MYSTIC_KILL)
            {
                if ( randint1(randint1(r_ptr->level/7)+5) == 1 
                  && (!(r_ptr->flags1 & RF1_UNIQUE) || m_ptr->r_idx == MON_HAGURE2) 
                  && !(r_ptr->flags7 & RF7_UNIQUE2))
                {
                    k = m_ptr->hp + 1;
                    msg_format("You hit %s on a fatal spot!", m_name);
                }
                else k = 1;
            }
            else if ( p_ptr->pclass == CLASS_NINJA 
                   && o_ptr 
                   && !p_ptr->weapon_info[hand].icky_wield 
                   && (p_ptr->cur_lite <= 0 || one_in_(7)) )
            {
                int maxhp = maxroll(r_ptr->hdice, r_ptr->hside);
                if (one_in_(backstab ? 13 : (stab_fleeing || fuiuchi) ? 15 : 27))
                {
                    k *= 5;
                    drain_result *= 2;
                    msg_format("You critically injured %s!", m_name);
                }
                else if ( (m_ptr->hp < maxhp/2 && one_in_(num_blow*10)) 
                       || ( (one_in_(666) || ((backstab || fuiuchi) && one_in_(11)))
                         && !(r_ptr->flags1 & RF1_UNIQUE) 
                         && !(r_ptr->flags7 & RF7_UNIQUE2)) )
                {
                    if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_UNIQUE2) || (m_ptr->hp >= maxhp/2))
                    {
                        k = MAX(k*5, m_ptr->hp/2);
                        drain_result *= 2;
                        msg_format("You fatally injured %s!", m_name);
                    }
                    else
                    {
                        k = m_ptr->hp + 1;
                        msg_format("You hit %s on a fatal spot!", m_name);
                    }
                }
            }

            if (k <= 0) can_drain = FALSE;

            if (drain_result > m_ptr->hp)
                drain_result = m_ptr->hp;

            if (have_flag(flgs, TR_WILD))
                wild_weapon_strike();

            if (mode == ROGUE_ASSASSINATE)
            {
                if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_UNIQUE2) || mon_save_p(m_ptr->r_idx, A_DEX))
                {
                    k = MAX(k*5, m_ptr->hp/2);
                    drain_result *= 2;
                    msg_format("You fatally injured %s!", m_name);
                }
                else
                {
                    k = m_ptr->hp + 1;
                    msg_format("You hit %s on a fatal spot!", m_name);
                }
            }

            if (p_ptr->pclass == CLASS_WEAPONMASTER)
            {
                if (mode == WEAPONMASTER_VICIOUS_STRIKE)
                    k *= 2;

                switch(weaponmaster_get_toggle())
                {
                case TOGGLE_SHIELD_BASH:
                    if (one_in_(12))
                    {
                        if (r_ptr->flagsr & RFR_RES_ALL)
                        {
                            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_ALL;
                            msg_format("%^s is immune.", m_name);
                        }
                        else if (r_ptr->flags3 & RF3_NO_STUN)
                        {
                            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_NO_STUN;
                            msg_format("%^s is immune.", m_name);
                        }
                        else if (mon_save_p(m_ptr->r_idx, A_STR))
                        {
                            msg_format("%^s resists.", m_name);
                        }
                        else
                        {
                            msg_format("%^s is stunned.", m_name);
                            set_monster_stunned(c_ptr->m_idx, MAX(MON_STUNNED(m_ptr), 2));
                        }
                    }
                    break;

                case TOGGLE_BURNING_BLADE:
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        msg_format("%^s is immune.", m_name);
                        k = 0;
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
                    }
                    else if (r_ptr->flagsr & RFR_IM_FIRE)
                    {
                        msg_format("%^s resists alot.", m_name);
                        k /= 9;
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_FIRE);
                    }
                    else if (r_ptr->flags3 & (RF3_HURT_FIRE))
                    {
                        msg_format("%^s is hit hard.", m_name);
                        k *= 2;
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_FIRE);
                    }
                    break;
                case TOGGLE_ICE_BLADE:
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        msg_format("%^s is immune.", m_name);
                        k = 0;
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
                    }
                    else if (r_ptr->flagsr & RFR_IM_COLD)
                    {
                        msg_format("%^s resists alot.", m_name);
                        k /= 9;
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_COLD);
                    }
                    else 
                    {
                        if (r_ptr->flags3 & (RF3_HURT_COLD))
                        {
                            msg_format("%^s is hit hard.", m_name);
                            k *= 2;
                            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_COLD);
                        }
                        if (one_in_(5)
                            && !(r_ptr->flags1 & (RF1_UNIQUE))
                            && !mon_save_p(m_ptr->r_idx, A_STR) )
                        {
                            msg_format("%^s is slowed by the cold.", m_name);
                            set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50);
                        }
                    }
                    break;
                case TOGGLE_THUNDER_BLADE:
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        msg_format("%^s is immune.", m_name);
                        k = 0;
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
                    }
                    else if (r_ptr->flagsr & RFR_IM_ELEC)
                    {
                        msg_format("%^s resists alot.", m_name);
                        k /= 9;
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_ELEC);
                    }
                    else 
                    {
                        if ( one_in_(5)
                          && !(r_ptr->flags3 & RF3_NO_STUN)
                          && !mon_save_p(m_ptr->r_idx, A_STR) )
                        {
                            msg_format("%^s is shocked convulsively.", m_name);
                            set_monster_stunned(c_ptr->m_idx, MAX(MON_STUNNED(m_ptr), 4));
                        }
                    }
                    break;
                case TOGGLE_WILD_BLADE:
                    wild_weapon_strike(); /* TODO: Add EGO_WILD_BLADE and TR_WILD for weapons */
                    break;
                }
            }

            if (mode == WEAPONMASTER_CRUSADERS_STRIKE)
                k = k * 3 / 2;

            dam_tot += k;

            if (mode == WEAPONMASTER_REAPING)
            {
                int              start_dir, x2, y2;
                int                 dir;
                cave_type       *c_ptr2;
                monster_type    *m_ptr2;
                bool             fear2 = FALSE;
                int                 ct = 0;

                k *= 1 + (num_blow + 2)/3;

                /* First hit the chosen target */            
                if (mon_take_hit(c_ptr->m_idx, k, fear, NULL))
                {
                    *mdeath = TRUE;
                    ct += 20;
                }

                msg_format("Your swing your %s about, reaping a harvest of death!", o_name);
            
                /* Next hit all adjacent targets in a swinging circular arc */
                start_dir = calculate_dir(px, py, x, y);
                dir = start_dir;

                for (;;)
                {
                    dir = get_next_dir(dir);
                    if (dir == start_dir || dir == 5) break;

                    x2 = px + ddx[dir];
                    y2 = py + ddy[dir];
                    c_ptr2 = &cave[y2][x2];
                    m_ptr2 = &m_list[c_ptr2->m_idx];

                    if (c_ptr2->m_idx && (m_ptr2->ml || cave_have_flag_bold(y2, x2, FF_PROJECT)))
                    {
                        if (mon_take_hit(c_ptr2->m_idx, k, &fear2, NULL))
                            ct += 10;
                    }
                }

                /* Finally, gain Wraithform */
                set_wraith_form(p_ptr->wraith_form + ct/2, FALSE);

                if (p_ptr->wizard)
                    msg_print("****END REAPING****");
            }
            /* Damage, check for fear and death */
            else if (mon_take_hit(c_ptr->m_idx, k, fear, NULL))
            {
                *mdeath = TRUE;

                if (mauler_get_toggle() == MAULER_TOGGLE_SPLATTER)
                {
                    project(0, 2, y, x, k, 
                            GF_BLOOD, PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_FULL_DAM, -1);
                }

                if ( o_ptr
                  && o_ptr->tval == TV_SWORD
                  && o_ptr->sval == SV_RUNESWORD )
                {
                    _rune_sword_kill(o_ptr, r_ptr);
                }

                if (duelist_attack)
                {
                    p_ptr->duelist_target_idx = 0;
                    p_ptr->redraw |= PR_STATUS;

                    if (p_ptr->lev >= 35)    /* Endless Duel */
                    {
                        /* Hacks so that get_aim_dir() actually allows user to select a new target */
                        target_who = 0;
                        command_dir = 0;
                        msg_print("Your chosen target is vanquished!  Select another.");
                        duelist_issue_challenge();
                    }
                    else
                        msg_print("Your chosen target is vanquished!");
                }

                if ((p_ptr->pclass == CLASS_BERSERKER || mut_present(MUT_FANTASTIC_FRENZY) || p_ptr->tim_shrike) && energy_use)
                {
                    energy_use = 0;
                    if (hand)
                        energy_use += (hand - 1) * 100 / p_ptr->weapon_ct;
                    energy_use += num * (100 / p_ptr->weapon_ct) / num_blow;
                }
                if (o_ptr && o_ptr->name1 == ART_ZANTETSU && is_lowlevel)
                    msg_print("Sigh... Another trifling thing I've cut....");
                break;
            }
            else
            {
                if (o_ptr && o_ptr->name1 == ART_ETERNAL_BLADE)
                {
                    /* Hack: Time Brand. Effectively a 2x slay ... k2 is damage from dice alone.*/
                    project(0, 0, y, x, k2, GF_TIME, PROJECT_STOP | PROJECT_KILL | PROJECT_GRID, -1);
                }

                if (mauler_get_toggle() == MAULER_TOGGLE_DRAIN)
                {
                    int amt = (k+2)/3;
                    m_ptr->maxhp -= amt;
                    msg_format("%^s seems weakened.", m_name);
                }
                if (mode == MAULER_STUNNING_BLOW)
                {
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_ALL;
                        msg_format("%^s is immune.", m_name);
                    }
                    else if (r_ptr->flags3 & RF3_NO_STUN)
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_NO_STUN;
                        msg_format("%^s is immune.", m_name);
                    }
                    else if ((r_ptr->flags1 & RF1_UNIQUE) && mon_save_p(m_ptr->r_idx, A_STR))
                    {
                        msg_format("%^s resists.", m_name);
                    }
                    else
                    {
                        msg_format("%^s is stunned.", m_name);
                        set_monster_stunned(c_ptr->m_idx, MAX(MON_STUNNED(m_ptr), 3 + randint1(3)));
                    }
                }
                if (mode == MAULER_KNOCKOUT_BLOW)
                {
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_ALL;
                        msg_format("%^s is immune.", m_name);
                    }
                    else if (mon_save_p(m_ptr->r_idx, A_STR))
                    {
                        msg_format("%^s resists.", m_name);
                    }
                    else
                    {
                        msg_format("%^s is knocked out.", m_name);
                        set_monster_paralyzed(c_ptr->m_idx, randint1(3));
                        /* No more retaliation this round! */                    
                        retaliation_count = 100; /* Any number >= 4 will do ... */
                    }
                }
                if (mode == MELEE_AWESOME_BLOW)
                {
                    int dir = calculate_dir(px, py, x, y);
                    if (dir != 5)
                    {
                        int ct = 0;
                        int max = 3;
                        int m_idx = c_ptr->m_idx;
                        int ty = y, tx = x;
                        int oy = y, ox = x;

                        if (p_ptr->pclass == CLASS_RAGE_MAGE)
                        {
                            if (p_ptr->shero)
                                max = 6;
                        }
                        else if (p_ptr->pclass == CLASS_MAULER && o_ptr)
                        {
                            int w = o_ptr->weight;
                            max = MIN(p_ptr->lev/5, w/40);
                        }
                        
                        for (ct = 0; ct < max; ct++) 
                        {
                            y += ddy[dir];
                            x += ddx[dir];
                            if (!cave_empty_bold(y, x))
                            {
                                int dam = 50;

                                if ( cave[y][x].m_idx 
                                  || cave_have_flag_bold(y, x, FF_TREE)
                                  || cave[y][x].feat == feat_rubble
                                  || cave[y][x].feat == feat_dark_pit )
                                {
                                    dam = 25;
                                }
                                msg_format("%^s is wounded.", m_name);                            
                                mon_take_hit(m_idx, dam * (max - ct), fear, NULL);
                                break;
                            }
                            else
                            {
                                ty = y;
                                tx = x;
                            }
                            if (ty != oy || tx != ox)
                            {
                                int m_idx = cave[oy][ox].m_idx;

                                cave[oy][ox].m_idx = 0;
                                cave[ty][tx].m_idx = m_idx;
                                m_ptr->fy = ty;
                                m_ptr->fx = tx;
    
                                update_mon(m_idx, TRUE);
                                lite_spot(oy, ox);
                                lite_spot(ty, tx);
                                
                                oy = ty;
                                ox = tx;
    
                                if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
                                    p_ptr->update |= PU_MON_LITE;
                            }
                        }
                    }
                }
                    
                if (mode == WEAPONMASTER_CRUSADERS_STRIKE)
                {
                    msg_format("Your Crusader's Strike drains life from %s!", m_name);
                    hp_player(MIN(k, 150));
                }
            
                /* Clubmaster Hacks.  We do these effects *after* the monster takes damage. */
                if (weaponmaster_is_(WEAPONMASTER_CLUBS) && p_ptr->speciality_equip)
                {
                    int odds = 5;
                
                    if (mode == WEAPONMASTER_CUNNING_STRIKE)
                        odds = 2;

                    if (one_in_(odds))
                    {
                        if (r_ptr->flagsr & RFR_RES_ALL)
                        {
                            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_ALL;
                            msg_format("%^s is immune.", m_name);
                        }
                        else if (r_ptr->flags3 & RF3_NO_CONF)
                        {
                            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_NO_CONF;
                            msg_format("%^s is immune.", m_name);
                        }
                        else if (mon_save_p(m_ptr->r_idx, A_STR))
                        {
                            msg_format("%^s resists.", m_name);
                        }
                        else
                        {
                            msg_format("%^s appears confused.", m_name);
                            set_monster_confused(c_ptr->m_idx, MON_CONFUSED(m_ptr) + 10 + randint0(p_ptr->lev) / 5);
                        }
                    }

                    if (p_ptr->lev >= 20 && one_in_(odds))
                    {
                        if (r_ptr->flagsr & RFR_RES_ALL)
                        {
                            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_ALL;
                            msg_format("%^s is immune.", m_name);
                        }
                        else if (r_ptr->flags3 & RF3_NO_SLEEP)
                        {
                            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_NO_SLEEP;
                            msg_format("%^s is immune.", m_name);
                        }
                        else if (mon_save_p(m_ptr->r_idx, A_STR))
                        {
                            msg_format("%^s resists.", m_name);
                        }
                        else
                        {
                            msg_format("%^s is knocked out.", m_name);
                            knock_out++;        
                            /* No more retaliation this round! */                    
                            retaliation_count = 100; /* Any number >= 4 will do ... */
                        }
                    }

                    if ( mode == WEAPONMASTER_CUNNING_STRIKE 
                      || (p_ptr->lev >= 45 && one_in_(odds)) )
                    {
                        if (r_ptr->flagsr & RFR_RES_ALL)
                        {
                            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_ALL;
                            msg_format("%^s is immune.", m_name);
                        }
                        else if (r_ptr->flags3 & RF3_NO_STUN)
                        {
                            if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_NO_STUN;
                            msg_format("%^s is immune.", m_name);
                        }
                        else if (mon_save_p(m_ptr->r_idx, A_STR))
                        {
                            msg_format("%^s resists.", m_name);
                        }
                        else
                        {
                            msg_format("%^s is stunned.", m_name);
                            set_monster_stunned(c_ptr->m_idx, MAX(MON_STUNNED(m_ptr), 2));
                        }
                    }
                }
            }

            if (k > 0) anger_monster(m_ptr);

            touch_zap_player(c_ptr->m_idx);
            touch_ct++;

            if (can_drain && (drain_result > 0))
            {
                if (o_ptr && o_ptr->name1 == ART_MURAMASA)
                {
                    if (is_human)
                    {
                        int to_h = o_ptr->to_h;
                        int to_d = o_ptr->to_d;
                        int i, flag;

                        flag = 1;
                        for (i = 0; i < to_h + 3; i++) if (one_in_(4)) flag = 0;
                        if (flag) to_h++;

                        flag = 1;
                        for (i = 0; i < to_d + 3; i++) if (one_in_(4)) flag = 0;
                        if (flag) to_d++;

                        if (o_ptr->to_h != to_h || o_ptr->to_d != to_d)
                        {
                            msg_print("Muramasa sucked blood, and became more powerful!");
                            o_ptr->to_h = to_h;
                            o_ptr->to_d = to_d;
                        }
                    }
                }
                else
                {
                    if (drain_result > 5) /* Did we really hurt it? */
                    {
                        drain_heal = damroll(2, drain_result / 6);

                        /* Hex */
                        if (hex_spelling(HEX_VAMP_BLADE)) drain_heal *= 2;
                        if (prace_is_(RACE_MON_VAMPIRE)) drain_heal *= 2;

                        if (drain_left)
                        {
                            if (drain_heal < drain_left)
                                drain_left -= drain_heal;
                            else
                            {
                                drain_heal = drain_left;
                                drain_left = 0;
                            }

                            if (drain_msg)
                            {
                                msg_format("Your weapon drains life from %s!", m_name);
                                drain_msg = FALSE;
                            }
                            drain_heal = (drain_heal * mutant_regenerate_mod) / 100;
                            hp_player_aux(drain_heal);
                        }
                    }
                }
                m_ptr->maxhp -= (k+7)/8;
                if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
                if (m_ptr->maxhp < 1) m_ptr->maxhp = 1;
                weak = TRUE;
            }
            can_drain = FALSE;
            drain_result = 0;

            /* Confusion attack */
            if ( (p_ptr->special_attack & ATTACK_CONFUSE) 
              || chaos_effect == 3 
              || mode == HISSATSU_CONF 
              || mode == MYSTIC_CONFUSE
              || hex_spelling(HEX_CONFUSION)
              || (giant_is_(GIANT_TITAN) && p_ptr->lev >= 30 && one_in_(5)) )
            {
                /* Cancel glowing hands */
                if (p_ptr->special_attack & ATTACK_CONFUSE)
                {
                    p_ptr->special_attack &= ~(ATTACK_CONFUSE);
                    msg_print("Your hands stop glowing.");
                    p_ptr->redraw |= (PR_STATUS);

                }

                /* Confuse the monster */
                if (r_ptr->flags3 & RF3_NO_CONF)
                {
                    if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_NO_CONF;
                    msg_format("%^s is unaffected.", m_name);
                }
                else if (randint0(100) < r_ptr->level)
                    msg_format("%^s is unaffected.", m_name);
                else
                {
                    msg_format("%^s appears confused.", m_name);
                    (void)set_monster_confused(c_ptr->m_idx, MON_CONFUSED(m_ptr) + 10 + randint0(p_ptr->lev) / 5);
                }

                /* Only try to confuse once */
                if (mode == HISSATSU_CONF)
                    mode = 0;
            }

            else if (chaos_effect == 4)
            {
                bool resists_tele = FALSE;

                if (r_ptr->flagsr & RFR_RES_TELE)
                {
                    if (r_ptr->flags1 & RF1_UNIQUE)
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
                        msg_format("%^s is unaffected!", m_name);
                        resists_tele = TRUE;
                    }
                    else if (r_ptr->level > randint1(100))
                    {
                        if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
                        msg_format("%^s resists!", m_name);
                        resists_tele = TRUE;
                    }
                }

                if (!resists_tele)
                {
                    msg_format("%^s disappears!", m_name);
                    teleport_away(c_ptr->m_idx, 50, TELEPORT_PASSIVE);
                    num = num_blow + 1; /* Can't hit it anymore! */
                    *mdeath = TRUE;
                }
            }

            else if ((chaos_effect == 5) && (randint1(90) > r_ptr->level))
            {
                if (!(r_ptr->flags1 & (RF1_UNIQUE | RF1_QUESTOR)) &&
                    !(r_ptr->flagsr & RFR_EFF_RES_CHAO_MASK))
                {
                    if (polymorph_monster(y, x))
                    {
                        msg_format("%^s changes!", m_name);
                        *fear = FALSE;
                        weak = FALSE;
                    }
                    else
                        msg_format("%^s is unaffected.", m_name);

                    m_ptr = &m_list[c_ptr->m_idx];
                    monster_desc(m_name, m_ptr, 0);
                    r_ptr = &r_info[m_ptr->r_idx];
                }
            }
            else if (o_ptr && o_ptr->name1 == ART_G_HAMMER)
            {
                monster_type *m_ptr = &m_list[c_ptr->m_idx];

                if (m_ptr->hold_o_idx)
                {
                    object_type *q_ptr = &o_list[m_ptr->hold_o_idx];
                    char o_name[MAX_NLEN];

                    object_desc(o_name, q_ptr, OD_NAME_ONLY);
                    q_ptr->held_m_idx = 0;
                    q_ptr->marked = OM_TOUCHED;
                    m_ptr->hold_o_idx = q_ptr->next_o_idx;
                    q_ptr->next_o_idx = 0;
                    msg_format("You snatched %s.", o_name);
                    inven_carry(q_ptr);
                }
            }

            if ( p_ptr->pclass == CLASS_DUELIST
              && o_ptr
              && o_ptr->tval == TV_POLEARM 
              && o_ptr->sval == SV_DEATH_SCYTHE
              && !one_in_(3) )
            {
                death_scythe_miss(o_ptr, hand, mode);
            }
        }
        /* Player misses */
        else
        {
            backstab = FALSE; /* Clumsy! */
            fuiuchi = FALSE; /* Clumsy! */

            if (eviscerator && !duelist_attack && one_in_(2)) num_blow++;

            if ( o_ptr
              && o_ptr->tval == TV_POLEARM 
              && o_ptr->sval == SV_DEATH_SCYTHE 
              && one_in_(3) )
            {
                msg_format("You miss %s.", m_name);
                death_scythe_miss(o_ptr, hand, mode);
            }
            else
            {
                sound(SOUND_MISS);
                msg_format("You miss %s.", m_name);
            }
        }
        backstab = FALSE;
        fuiuchi = FALSE;

        /* Hack: Whirlwind first attacks chosen monster, than attempts to strike
           all other monsters adjacent.*/
        if (do_whirlwind)
        {
            int              start_dir, x2, y2;
            int                 dir;
            cave_type       *c_ptr2;
            monster_type    *m_ptr2;
            bool            fear2 = FALSE;
            bool            mdeath2 = FALSE;

            msg_format("Your swing your %s about, striking all nearby foes.", o_name);
            
            start_dir = calculate_dir(px, py, x, y);
            dir = start_dir;

            for (;;)
            {
                dir = get_next_dir(dir);
                if (dir == start_dir || dir == 5) break;

                x2 = px + ddx[dir];
                y2 = py + ddy[dir];
                c_ptr2 = &cave[y2][x2];
                m_ptr2 = &m_list[c_ptr2->m_idx];

                if (c_ptr2->m_idx && (m_ptr2->ml || cave_have_flag_bold(y2, x2, FF_PROJECT)))
                    py_attack_aux(y2, x2, &fear2, &mdeath2, hand, WEAPONMASTER_WHIRLWIND);
            }

            if (p_ptr->wizard)
                msg_print("****END WHIRLWIND****");
        }

        if (mode == WEAPONMASTER_RETALIATION) break;
        if (mode == WEAPONMASTER_CRUSADERS_STRIKE) break;
        if (mode == WEAPONMASTER_MANY_STRIKE) break;
        if (mode == WEAPONMASTER_PIERCING_STRIKE && num > p_ptr->lev/25) break;
        if (mode == WEAPONMASTER_PROXIMITY_ALERT && num > p_ptr->lev/25) break;
        if (mode == WEAPONMASTER_WHIRLWIND) break;
        if (mode == WEAPONMASTER_REAPING) break;
        if (mode == MELEE_AWESOME_BLOW) break;
        if (mode == ROGUE_ASSASSINATE) break;
        if (mauler_get_toggle() == MAULER_TOGGLE_MAUL) break;
        if (mode == MAULER_CRITICAL_BLOW) break;
        if (mode == MAULER_STUNNING_BLOW) break;
        if (mode == MAULER_KNOCKBACK) break;
        if (mode == MAULER_KNOCKOUT_BLOW) break;
        if (mode == MAULER_CRUSHING_BLOW) break;
        if (mode == MAULER_SCATTER) break;
        if (mode == HISSATSU_KYUSHO) break;
        if (mode == HISSATSU_MINEUCHI) break;
        if (mode == HISSATSU_3DAN) break;
        if (mode == HISSATSU_IAI) break;
        if (mode == MYSTIC_KILL) break;
        if (mode == MYSTIC_KNOCKOUT) break;
        if (mode == MYSTIC_CONFUSE) break;
    }

    if (mode == WEAPONMASTER_ELUSIVE_STRIKE && hit_ct)
        teleport_player(10, TELEPORT_LINE_OF_SIGHT);

    if ( (mode == WEAPONMASTER_KNOCK_BACK || mode == MAULER_KNOCKBACK || mode == MAULER_SCATTER) 
      && hit_ct
      && !*mdeath )
    {
        int dist = hit_ct * 2;

        if (mode == MAULER_KNOCKBACK)
            dist = 8;
        if (mode == MAULER_SCATTER)
            dist = 3;

        do_monster_knockback(x, y, dist);
    }

    /* Sleep counter ticks down in energy units ... Also, *lots* of code
       will just wake the monster back up, so we need to be very careful
       about when we call this.  */
    if (knock_out && !(*mdeath))
        set_monster_csleep(c_ptr->m_idx, MON_CSLEEP(m_ptr) + 500);

    if (weaponmaster_get_toggle() == TOGGLE_TRIP && mode == 0 && !(*mdeath) && !fear_stop)
    {
        if (test_hit_norm(chance, MON_AC(r_ptr, m_ptr), m_ptr->ml))
        {
            if (m_ptr->mflag2 & MFLAG2_TRIPPED)
                msg_format("%^s is already tripped up.", m_name);
            else if ( !(r_ptr->flags1 & RF1_UNIQUE)
                   || !mon_save_p(m_ptr->r_idx, A_STR) )
            {
                msg_format("%^s cries 'Help! I've fallen and I can't get up!'", m_name);
                m_ptr->mflag2 |= MFLAG2_TRIPPED;
            }
            else
                msg_format("%^s nimbly dodges your attempt to trip.", m_name);
        }
        else
            msg_format("You attempt to trip %^s but miss.", m_name);
    }

    if (weak && !(*mdeath))
        msg_format("%^s seems weakened.", m_name);

    if (drain_left != _max_vampiric_drain())
    {
        if (one_in_(4))
            virtue_add(VIRTUE_UNLIFE, 1);
    }

    if (touch_ct && !(*mdeath))
        fear_p_touch_m(m_ptr);

    if (do_quake)
        earthquake(py, px, 10);


#if 1
    if (p_ptr->pclass == CLASS_MAULER)
        c_put_str(TERM_WHITE, format("Maul:%5d", dam_tot), 24, 0);    
    if (p_ptr->pclass == CLASS_DUELIST)
        c_put_str(TERM_WHITE, format("Duel:%5d", dam_tot), 24, 0);    
#endif


    return success_hit;
}

bool random_opponent(int *y, int *x)
{
    int dirs[9];
    int ct = 0;
    int i, tx, ty;
    cave_type *c_ptr;

    for (i = 0; i < 8; i++)
    {
        ty = py + ddy_ddd[i];
        tx = px + ddx_ddd[i];
        c_ptr = &cave[ty][tx];
        if (c_ptr->m_idx)
        {
            dirs[ct] = i;
            ct++;
        }
    }

    if (ct)
    {
        i = randint0(ct);
        *y = py + ddy_ddd[dirs[i]];
        *x = px + ddx_ddd[dirs[i]];
        return TRUE;
    }
    return FALSE;
}

bool py_attack(int y, int x, int mode)
{
    bool            fear = FALSE;
    bool            mdeath = FALSE;
    int             i;
    cave_type       *c_ptr = &cave[y][x];
    monster_type    *m_ptr = &m_list[c_ptr->m_idx];
    monster_race    *r_ptr = &r_info[m_ptr->r_idx];
    char            m_name[80];

    /* Disturb the player */
    disturb(0, 0);

    energy_use = 100;

    if (!p_ptr->weapon_ct && !p_ptr->innate_attack_ct)
    {
        msg_print("You have no melee attacks.");
        energy_use = 0;
        return FALSE;
    }

    if (prace_is_(MIMIC_MIST))
    {
        msg_print("You cannot attack while incorporeal.");
        energy_use = 0;
        return FALSE;
    }

    monster_desc(m_name, m_ptr, 0);
    if (m_ptr->ml)
    {
        if (!p_ptr->image) monster_race_track(m_ptr->ap_r_idx);
        health_track(c_ptr->m_idx);
    }

    if ( (r_ptr->flags1 & RF1_FEMALE) 
      && !(p_ptr->stun || p_ptr->confused || p_ptr->image || !m_ptr->ml)
      && equip_find_artifact(ART_ZANTETSU))
    {
        msg_print("I can not attack women!");
        return FALSE;
    }

    if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
    {
        msg_print("Something prevents you from attacking.");
        return FALSE;
    }

    if ( !is_hostile(m_ptr) 
      && !(p_ptr->stun || p_ptr->confused || p_ptr->image || IS_SHERO() || !m_ptr->ml) )
    {
        if (equip_find_artifact(ART_STORMBRINGER))
        {
            msg_format("Your black blade greedily attacks %s!", m_name);
            virtue_add(VIRTUE_INDIVIDUALISM, 1);
            virtue_add(VIRTUE_HONOUR, -1);
            virtue_add(VIRTUE_JUSTICE, -1);
            virtue_add(VIRTUE_COMPASSION, -1);
        }
        else if (p_ptr->pclass != CLASS_BERSERKER)
        {
            if (get_check("Really hit it? "))
            {
                virtue_add(VIRTUE_INDIVIDUALISM, 1);
                virtue_add(VIRTUE_HONOUR, -1);
                virtue_add(VIRTUE_JUSTICE, -1);
                virtue_add(VIRTUE_COMPASSION, -1);
            }
            else
            {
                msg_format("You stop to avoid hitting %s.", m_name);
                return FALSE;
            }
        }
    }

    if (MON_CSLEEP(m_ptr)) /* It is not honorable etc to attack helpless victims */
    {
        if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) virtue_add(VIRTUE_COMPASSION, -1);
        if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) virtue_add(VIRTUE_HONOUR, -1);
    }

    /* Check for dual wielding skill ... Personally, I think skills should only increase on a hit.*/
    for (i = 0; i < MAX_ARMS; i++)
    {
        int rhand = 2*i;
        int lhand = 2*i+1;
        object_type *robj = NULL, *lobj = NULL;

        if (p_ptr->weapon_info[rhand].wield_how != WIELD_NONE)
            robj = equip_obj(p_ptr->weapon_info[rhand].slot);

        if (p_ptr->weapon_info[lhand].wield_how != WIELD_NONE)
            lobj = equip_obj(p_ptr->weapon_info[lhand].slot);

        if (robj && lobj)
        {
            skills_dual_wielding_gain(r_ptr);
            break;
        }
    }

    if (p_ptr->riding)
        skills_riding_gain_melee(r_ptr);

    riding_t_m_idx = c_ptr->m_idx;

    drain_left = _max_vampiric_drain();
    retaliation_count = 0;
    melee_hack = TRUE;
    fear_stop = FALSE;

    if (weaponmaster_get_toggle() == TOGGLE_MANY_STRIKE && mode == 0)
    {
        int i, j;
        bool stop = FALSE;
        int msec = delay_factor * delay_factor * delay_factor;

        for (i = 0; i < MAX_HANDS && !stop; i++)
        {
            drain_left = _max_vampiric_drain();
            if (p_ptr->weapon_info[i].wield_how != WIELD_NONE && !mdeath && !fear_stop)
            {
                object_type *o_ptr = equip_obj(p_ptr->weapon_info[i].slot);
                int num_blow = _get_num_blow(i);

                for (j = 0; j < num_blow; j++)
                {
                    if (o_ptr) /* paranoia */
                    {
                        char c = object_char(o_ptr);
                        byte a = object_attr(o_ptr);

                        print_rel(c, a, y, x);
                        move_cursor_relative(y, x);
                        Term_fresh();
                        Term_xtra(TERM_XTRA_DELAY, msec);
                        lite_spot(y, x);
                        Term_fresh();
                    }
                    py_attack_aux(y, x, &fear, &mdeath, i, WEAPONMASTER_MANY_STRIKE);
                    if (fear_stop || !random_opponent(&y, &x))
                    {
                        stop = TRUE;
                        break;
                    }
                }
            }
        }
    }
    else if (weaponmaster_get_toggle() == TOGGLE_PIERCING_STRIKE && mode == 0)
    {
    u16b    path[512];
    int        ct = project_path(path, 3, py, px, y, x, PROJECT_PATH | PROJECT_THRU);
    int        msec = delay_factor * delay_factor * delay_factor;

        int i, j, k;

        for (i = 0; i < MAX_HANDS; i++)
        {
            drain_left = _max_vampiric_drain();
            if (p_ptr->weapon_info[i].wield_how != WIELD_NONE && !mdeath && !fear_stop)
            {
                object_type *o_ptr = equip_obj(p_ptr->weapon_info[i].slot);
                int num_blow = _get_num_blow(i);
                for (j = 0; j < num_blow; j++)
                {
                    if (p_ptr->wizard)
                        msg_format("Attack #%d", j+1);

                    for (k = 0; k < ct; k++)
                    {
                        int nx, ny;
                        ny = GRID_Y(path[k]);
                        nx = GRID_X(path[k]);

                        if (p_ptr->wizard)
                            msg_format("  Step #%d", k+1);

                        if (!cave_have_flag_bold(ny, nx, FF_PROJECT)
                         && !cave[ny][nx].m_idx) 
                        {
                            break;
                        }

                        if (!cave[ny][nx].m_idx) break;

                        if (o_ptr && panel_contains(ny, nx) && player_can_see_bold(ny, nx))
                        {
                            char c = object_char(o_ptr);
                            byte a = object_attr(o_ptr);
                            int msec = delay_factor * delay_factor * delay_factor;

                            print_rel(c, a, ny, nx);
                            move_cursor_relative(ny, nx);
                            Term_fresh();
                            Term_xtra(TERM_XTRA_DELAY, msec);
                            lite_spot(ny, nx);
                            Term_fresh();
                        }
                        else
                            Term_xtra(TERM_XTRA_DELAY, msec);
                    
                        if (!py_attack_aux(ny, nx, &fear, &mdeath, i, WEAPONMASTER_PIERCING_STRIKE) || fear_stop) break;
                    }
                }
            }
        }
    }
    else if (mauler_get_toggle() == MAULER_TOGGLE_MAUL && mode == 0)
    {
        int dir = calculate_dir(px, py, x, y);
        int i;
        if (dir != 5)
        {
            for (i = 0; i < MAX_HANDS; i++)
            {
                int j;
                drain_left = _max_vampiric_drain();
                if (p_ptr->weapon_info[i].wield_how != WIELD_NONE && !mdeath && !fear_stop)
                {
                    int num_blow = _get_num_blow(i);
                    for (j = 0; j < num_blow; j++)
                    {
                        int y, x;
                        int ny, nx;
                        int m_idx;
                        cave_type *c_ptr;
                        monster_type *m_ptr;

                        y = py + ddy[dir];
                        x = px + ddx[dir];
                        c_ptr = &cave[y][x];
    
                        if (c_ptr->m_idx)
                            py_attack_aux(y, x, &fear, &mdeath, i, 0);
                        else
                        {
                            msg_print("There is no monster.");
                            break;
                        }

                        /* Monster is dead? */
                        if (!c_ptr->m_idx) break;
    
                        ny = y + ddy[dir];
                        nx = x + ddx[dir];
                        m_idx = c_ptr->m_idx;
                        m_ptr = &m_list[m_idx];
    
                        /* Monster cannot move back? */
                        if (!monster_can_enter(ny, nx, &r_info[m_ptr->r_idx], 0))
                        {
                            /* -more- */
                            if (j < 2) msg_print(NULL);
                            continue;
                        }
    
                        c_ptr->m_idx = 0;
                        cave[ny][nx].m_idx = m_idx;
                        m_ptr->fy = ny;
                        m_ptr->fx = nx;
    
                        update_mon(m_idx, TRUE);
    
                        lite_spot(y, x);
                        lite_spot(ny, nx);
    
                        /* Player can move forward? */
                        if (player_can_enter(c_ptr->feat, 0))
                        {
                            if (!move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP)) break;
                        }
                        else
                        {
                            break;
                        }

                        /* -more- */
                        if (j < 2) msg_print(NULL);
                    }
                }
            }
        }
    }
    else
    {
        for (i = 0; i < MAX_HANDS; i++)
        {
            drain_left = _max_vampiric_drain();
            if (p_ptr->weapon_info[i].wield_how != WIELD_NONE && !mdeath && !fear_stop)
            {
                py_attack_aux(y, x, &fear, &mdeath, i, mode);
                if (mode == WEAPONMASTER_RETALIATION)
                    break;
            }
        }
    }

    if (p_ptr->cleave && !fear_stop)
    {
        int y = 0, x = 0, i, dir = 0;
        cave_type *c_ptr;

        melee_hack = FALSE;
        for (i = 1; i <= 4; i++) /* TODO: Tweak the number of attempts */
        {
            dir = randint0(8);
            y = py + ddy_ddd[dir];
            x = px + ddx_ddd[dir];
            c_ptr = &cave[y][x];

            if (c_ptr->m_idx)
            {
                msg_print("You attempt to cleave another foe!");
                py_attack(y, x, 0);
                break;
            }
        }
        return TRUE;
    }

    if (p_ptr->innate_attack_ct && !mdeath && !fear_stop)
    {
        bool do_innate_attacks = TRUE;

        switch (mode)
        {
        case WEAPONMASTER_RETALIATION:
            if (p_ptr->weapon_ct == 0) /* Kenshirou Possessor can retaliate with innate attacks! */
                break;
        case WEAPONMASTER_PROXIMITY_ALERT:
        case WEAPONMASTER_CRUSADERS_STRIKE:
        case WEAPONMASTER_MANY_STRIKE:
        case WEAPONMASTER_PIERCING_STRIKE:
        case WEAPONMASTER_WHIRLWIND:
        case WEAPONMASTER_REAPING:
        case MELEE_AWESOME_BLOW:
        case ROGUE_ASSASSINATE:
        case MAULER_STUNNING_BLOW:
        case MAULER_KNOCKBACK:
        case MAULER_KNOCKOUT_BLOW:
        case MAULER_CRUSHING_BLOW:
        case MAULER_CRITICAL_BLOW:
        case MAULER_SCATTER:
        case HISSATSU_2:
        case HISSATSU_KYUSHO:
        case HISSATSU_MINEUCHI:
        case HISSATSU_3DAN:
        case HISSATSU_IAI:
        case MYSTIC_KILL:
        case MYSTIC_KNOCKOUT:
        case MYSTIC_CONFUSE:
            do_innate_attacks = FALSE;
            break;
        }

        if (mauler_get_toggle() == MAULER_TOGGLE_MAUL) 
            do_innate_attacks = FALSE;

        if (do_innate_attacks)
        {
            if (mode == DRAGON_TAIL_SWEEP)
            {
                int           ct = 3 + p_ptr->lev / 25;
                int           x2, y2, dir, start_dir;
                int           msec = delay_factor * delay_factor * delay_factor;

                start_dir = calculate_dir(px, py, x, y);
                dir = start_dir;

                while(ct--)
                {
                    x2 = px + ddx[dir];
                    y2 = py + ddy[dir];

                    if (in_bounds(y2, x2))
                    {
                        cave_type    *c_ptr2 = &cave[y2][x2];
                        monster_type *m_ptr2 = &m_list[c_ptr2->m_idx];

                        if (panel_contains(y2, x2) && player_can_see_bold(y2, x2))
                        {
                            char c = '*';
                            byte a = TERM_WHITE;

                            print_rel(c, a, y2, x2);
                            move_cursor_relative(y2, x2);
                            Term_fresh();
                            Term_xtra(TERM_XTRA_DELAY, msec);
                            lite_spot(y2, x2);
                            Term_fresh();
                        }
                        else
                            Term_xtra(TERM_XTRA_DELAY, msec);

                        if (c_ptr2->m_idx && (m_ptr2->ml || cave_have_flag_bold(y2, x2, FF_PROJECT)))
                        {
                            bool fear2 = FALSE, mdeath2 = FALSE;
                            innate_attacks(c_ptr2->m_idx, &fear2, &mdeath2, DRAGON_TAIL_SWEEP);
                        }
                    }
                    dir = get_next_dir(dir);
                    if (dir == start_dir || dir == 5) break;
                }
            }
            else
                innate_attacks(c_ptr->m_idx, &fear, &mdeath, mode);

        }
    }

    melee_hack = FALSE;

    if (fear && m_ptr->ml && !mdeath)
    {
        sound(SOUND_FLEE);
        msg_format("%^s flees in terror!", m_name);
    }

    if ((p_ptr->special_defense & KATA_IAI) && (mode != HISSATSU_IAI || mdeath))
        set_action(ACTION_NONE);

    return mdeath;
}


bool pattern_seq(int c_y, int c_x, int n_y, int n_x)
{
    feature_type *cur_f_ptr = &f_info[cave[c_y][c_x].feat];
    feature_type *new_f_ptr = &f_info[cave[n_y][n_x].feat];
    bool is_pattern_tile_cur = have_flag(cur_f_ptr->flags, FF_PATTERN);
    bool is_pattern_tile_new = have_flag(new_f_ptr->flags, FF_PATTERN);
    int pattern_type_cur, pattern_type_new;

    if (!is_pattern_tile_cur && !is_pattern_tile_new) return TRUE;

    pattern_type_cur = is_pattern_tile_cur ? cur_f_ptr->subtype : NOT_PATTERN_TILE;
    pattern_type_new = is_pattern_tile_new ? new_f_ptr->subtype : NOT_PATTERN_TILE;

    if (pattern_type_new == PATTERN_TILE_START)
    {
        if (!is_pattern_tile_cur && !p_ptr->confused && !p_ptr->stun && !p_ptr->image)
        {
            if (get_check("If you start walking the Pattern, you must walk the whole way. Ok? "))
                return TRUE;
            else
                return FALSE;
        }
        else
            return TRUE;
    }
    else if ((pattern_type_new == PATTERN_TILE_OLD) ||
         (pattern_type_new == PATTERN_TILE_END) ||
         (pattern_type_new == PATTERN_TILE_WRECKED))
    {
        if (is_pattern_tile_cur)
        {
            return TRUE;
        }
        else
        {
            msg_print("You must start walking the Pattern from the startpoint.");

            return FALSE;
        }
    }
    else if ((pattern_type_new == PATTERN_TILE_TELEPORT) ||
         (pattern_type_cur == PATTERN_TILE_TELEPORT))
    {
        return TRUE;
    }
    else if (pattern_type_cur == PATTERN_TILE_START)
    {
        if (is_pattern_tile_new)
            return TRUE;
        else
        {
            msg_print("You must walk the Pattern in correct order.");

            return FALSE;
        }
    }
    else if ((pattern_type_cur == PATTERN_TILE_OLD) ||
         (pattern_type_cur == PATTERN_TILE_END) ||
         (pattern_type_cur == PATTERN_TILE_WRECKED))
    {
        if (!is_pattern_tile_new)
        {
            msg_print("You may not step off from the Pattern.");

            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        if (!is_pattern_tile_cur)
        {
            msg_print("You must start walking the Pattern from the startpoint.");

            return FALSE;
        }
        else
        {
            byte ok_move = PATTERN_TILE_START;
            switch (pattern_type_cur)
            {
                case PATTERN_TILE_1:
                    ok_move = PATTERN_TILE_2;
                    break;
                case PATTERN_TILE_2:
                    ok_move = PATTERN_TILE_3;
                    break;
                case PATTERN_TILE_3:
                    ok_move = PATTERN_TILE_4;
                    break;
                case PATTERN_TILE_4:
                    ok_move = PATTERN_TILE_1;
                    break;
                default:
                    if (p_ptr->wizard)
                        msg_format("Funny Pattern walking, %d.", pattern_type_cur);

                    return TRUE; /* Goof-up */
            }

            if ((pattern_type_new == ok_move) ||
                (pattern_type_new == pattern_type_cur))
                return TRUE;
            else
            {
                if (!is_pattern_tile_new)
                    msg_print("You may not step off from the Pattern.");
                else
                    msg_print("You must walk the Pattern in correct order.");

                return FALSE;
            }
        }
    }
}


bool player_can_enter(s16b feature, u16b mode)
{
    feature_type *f_ptr = &f_info[feature];

    if (p_ptr->riding) return monster_can_cross_terrain(feature, &r_info[m_list[p_ptr->riding].r_idx], mode | CEM_RIDING);

    /* Pattern */
    if (have_flag(f_ptr->flags, FF_PATTERN))
    {
        if (!(mode & CEM_P_CAN_ENTER_PATTERN)) return FALSE;
    }

    /* "CAN" flags */
    if (have_flag(f_ptr->flags, FF_CAN_FLY) && p_ptr->levitation) return TRUE;
    if (have_flag(f_ptr->flags, FF_CAN_SWIM) && p_ptr->can_swim) return TRUE;
    if (have_flag(f_ptr->flags, FF_CAN_PASS) && p_ptr->pass_wall) return TRUE;

    if (!have_flag(f_ptr->flags, FF_MOVE)) return FALSE;

    return TRUE;
}


/*
 * Move the player
 */
bool move_player_effect(int ny, int nx, u32b mpe_mode)
{
    cave_type *c_ptr = &cave[ny][nx];
    feature_type *f_ptr = &f_info[c_ptr->feat];

    if (!(mpe_mode & MPE_STAYING))
    {
        int oy = py;
        int ox = px;
        cave_type *oc_ptr = &cave[oy][ox];
        int om_idx = oc_ptr->m_idx;
        int nm_idx = c_ptr->m_idx;

        /* Move the player */
        py = ny;
        px = nx;

        /* Hack -- For moving monster or riding player's moving */
        if (!(mpe_mode & MPE_DONT_SWAP_MON))
        {
            /* Swap two monsters */
            c_ptr->m_idx = om_idx;
            oc_ptr->m_idx = nm_idx;

            if (om_idx > 0) /* Monster on old spot (or p_ptr->riding) */
            {
                monster_type *om_ptr = &m_list[om_idx];
                om_ptr->fy = ny;
                om_ptr->fx = nx;
                update_mon(om_idx, TRUE);
            }

            if (nm_idx > 0) /* Monster on new spot */
            {
                monster_type *nm_ptr = &m_list[nm_idx];
                nm_ptr->fy = oy;
                nm_ptr->fx = ox;
                update_mon(nm_idx, TRUE);
            }
        }

        /* Redraw old spot */
        lite_spot(oy, ox);

        /* Redraw new spot */
        lite_spot(ny, nx);

        /* Check for new panel (redraw map) */      
        verify_panel();

        if (mpe_mode & MPE_FORGET_FLOW)
        {
            forget_flow();

            /* Mega-Hack -- Forget the view */
            p_ptr->update |= (PU_UN_VIEW);

            /* Redraw map */
            p_ptr->redraw |= (PR_MAP);
        }

        /* Update stuff */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE | PU_DISTANCE);

        if (!view_unsafe_grids)
            p_ptr->redraw |= PR_STATUS;

        {
            class_t *class_ptr = get_class_t();
            race_t  *race_ptr = get_race_t();
            if (class_ptr->move_player)
                class_ptr->move_player();
            if (race_ptr->move_player)
                race_ptr->move_player();

            if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
                wilderness_move_player(ox, oy);
        }

        /* Window stuff */
        p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

        /* Remove "unsafe" flag */
        if ((!p_ptr->blind && !no_lite()) || !is_trap(c_ptr->feat)) c_ptr->info &= ~(CAVE_UNSAFE);

        /* For get everything when requested hehe I'm *NASTY* */
        if (dun_level && (d_info[dungeon_type].flags1 & DF1_FORGET)) wiz_dark();

        /* Handle stuff */
        if (mpe_mode & MPE_HANDLE_STUFF) handle_stuff();

        if (p_ptr->pclass == CLASS_NINJA || p_ptr->tim_superstealth)
        {
            if (c_ptr->info & (CAVE_GLOW)) set_superstealth(FALSE);
            else if (p_ptr->cur_lite <= 0) set_superstealth(TRUE);
        }

        if ((p_ptr->action == ACTION_QUICK_WALK) &&
            (!have_flag(f_ptr->flags, FF_PROJECT) ||
             (!p_ptr->levitation && have_flag(f_ptr->flags, FF_DEEP))))
        {
            msg_print("You cannot run in here.");
            set_action(ACTION_NONE);

            if (weaponmaster_get_toggle() == TOGGLE_SHADOW_STANCE)
            {
                msg_print("Your shadow stance is disrupted!");
                weaponmaster_set_toggle(TOGGLE_NONE);
            }
        }
    }

    if (mpe_mode & MPE_ENERGY_USE)
    {
        if (music_singing(MUSIC_WALL))
        {
            (void)project(0, 0, py, px, (60 + p_ptr->lev), GF_DISINTEGRATE,
                PROJECT_KILL | PROJECT_ITEM, -1);

            if (!player_bold(ny, nx) || p_ptr->is_dead || p_ptr->leaving) return FALSE;
        }

        /* Spontaneous Searching */
        if ((p_ptr->skills.fos >= 50) || (0 == randint0(50 - p_ptr->skills.fos)))
        {
            search();
        }

        /* Continuous Searching */
        if (p_ptr->action == ACTION_SEARCH)
        {
            search();
        }
    }

    /* Handle "objects" */
    if (!(mpe_mode & MPE_DONT_PICKUP))
    {
        carry((mpe_mode & MPE_DO_PICKUP) ? TRUE : FALSE);
    }

    /* Handle "store doors" */
    if (have_flag(f_ptr->flags, FF_STORE))
    {
        /* Disturb */
        disturb(0, 0);

        energy_use = 0;
        /* Hack -- Enter store */
        command_new = SPECIAL_KEY_STORE;
    }

    /* Handle "building doors" -KMW- */
    else if (have_flag(f_ptr->flags, FF_BLDG))
    {
        /* Disturb */
        disturb(0, 0);

        energy_use = 0;
        /* Hack -- Enter building */
        command_new = SPECIAL_KEY_BUILDING;
    }

    /* Handle quest areas -KMW- */
    else if (have_flag(f_ptr->flags, FF_QUEST_ENTER))
    {
        /* Disturb */
        disturb(0, 0);

        energy_use = 0;
        /* Hack -- Enter quest level */
        command_new = SPECIAL_KEY_QUEST;
    }

    else if (have_flag(f_ptr->flags, FF_QUEST_EXIT))
    {
        if (quest[p_ptr->inside_quest].type == QUEST_TYPE_FIND_EXIT)
        {
            quest[p_ptr->inside_quest].status = QUEST_STATUS_COMPLETED;
            quest[p_ptr->inside_quest].complev = (byte)p_ptr->lev;
            msg_print("You accomplished your quest!");
            msg_print(NULL);
        }

        leave_quest_check();

        p_ptr->inside_quest = c_ptr->special;
        dun_level = 0;
        p_ptr->oldpx = 0;
        p_ptr->oldpy = 0;

        p_ptr->leaving = TRUE;
    }

    /* Set off a trap */
    else if (have_flag(f_ptr->flags, FF_HIT_TRAP) && !(mpe_mode & MPE_STAYING))
    {
        /* Disturb */
        disturb(0, 0);

        /* Hidden trap */
        if (c_ptr->mimic || have_flag(f_ptr->flags, FF_SECRET))
        {
            /* Message */
            msg_print("You found a trap!");

            /* Pick a trap */
            disclose_grid(py, px);
        }

        /* Hit the trap */
        hit_trap((mpe_mode & MPE_BREAK_TRAP) ? TRUE : FALSE);

        if (!player_bold(ny, nx) || p_ptr->is_dead || p_ptr->leaving) return FALSE;
    }

    /* Warn when leaving trap detected region */
    if (!(mpe_mode & MPE_STAYING) && (disturb_trap_detect || alert_trap_detect)
        && p_ptr->dtrap && !(c_ptr->info & CAVE_IN_DETECT))
    {
        /* No duplicate warning */
        p_ptr->dtrap = FALSE;

        /* You are just on the edge */
        if (!(c_ptr->info & CAVE_UNSAFE))
        {
            if (alert_trap_detect)
            {
                msg_print("*Leaving trap detect region!*");
            }

            if (disturb_trap_detect) disturb(0, 0);
        }
    }

    return player_bold(ny, nx) && !p_ptr->is_dead && !p_ptr->leaving;
}


bool trap_can_be_ignored(int feat)
{
    feature_type *f_ptr = &f_info[feat];

    if (!have_flag(f_ptr->flags, FF_TRAP)) return TRUE;

    switch (f_ptr->subtype)
    {
    case TRAP_TRAPDOOR:
    case TRAP_PIT:
    case TRAP_SPIKED_PIT:
    case TRAP_POISON_PIT:
        if (p_ptr->levitation) return TRUE;
        break;
    case TRAP_TELEPORT:
        if (p_ptr->anti_tele) return TRUE;
        break;
    case TRAP_FIRE:
        if (res_can_ignore(RES_FIRE)) return TRUE;
        break;
    case TRAP_ACID:
        if (res_can_ignore(RES_ACID)) return TRUE;
        break;
    case TRAP_BLIND:
        if (res_can_ignore(RES_BLIND)) return TRUE;
        break;
    case TRAP_CONFUSE:
        if (res_can_ignore(RES_CONF)) return TRUE;
        break;
    case TRAP_POISON:
        if (res_can_ignore(RES_POIS)) return TRUE;
        break;
    case TRAP_SLEEP:
        if (p_ptr->free_act) return TRUE;
        break;
    }

    return FALSE;
}


/*
 * Determine if a "boundary" grid is "floor mimic"
 */
#define boundary_floor(C, F, MF) \
    ((C)->mimic && permanent_wall(F) && \
     (have_flag((MF)->flags, FF_MOVE) || have_flag((MF)->flags, FF_CAN_FLY)) && \
     have_flag((MF)->flags, FF_PROJECT) && \
     !have_flag((MF)->flags, FF_OPEN))

/*
 * Move player in the given direction, with the given "pickup" flag.
 *
 * This routine should (probably) always induce energy expenditure.
 *
 * Note that moving will *always* take a turn, and will *always* hit
 * any monster which might be in the destination grid.  Previously,
 * moving into walls was "free" and did NOT hit invisible monsters.
 */
void move_player(int dir, bool do_pickup, bool break_trap)
{
    /* Find the result of moving */
    int y = py + ddy[dir];
    int x = px + ddx[dir];

    /* Examine the destination */
    cave_type *c_ptr = &cave[y][x];

    feature_type *f_ptr = &f_info[c_ptr->feat];

    monster_type *m_ptr;

    monster_type *riding_m_ptr = &m_list[p_ptr->riding];
    monster_race *riding_r_ptr = &r_info[p_ptr->riding ? riding_m_ptr->r_idx : 0]; /* Paranoia */

    char m_name[80];

    bool p_can_enter = player_can_enter(c_ptr->feat, CEM_P_CAN_ENTER_PATTERN);
    bool p_can_kill_walls = FALSE;
    bool stormbringer = FALSE;
    bool shadow_strike = FALSE;
    bool oktomove = TRUE;
    bool do_past = FALSE;

    bool ring_lev = FALSE;

    if (p_ptr->prace == RACE_MON_RING && p_ptr->levitation)
        ring_lev = TRUE;

    /* Get the monster */
    m_ptr = &m_list[c_ptr->m_idx];


    if (equip_find_artifact(ART_STORMBRINGER)) stormbringer = TRUE;

    /* Player can not walk through "walls"... */
    /* unless in Shadow Form */
    p_can_kill_walls = p_ptr->kill_wall && have_flag(f_ptr->flags, FF_HURT_DISI) &&
        (!p_can_enter || !have_flag(f_ptr->flags, FF_LOS)) &&
        !have_flag(f_ptr->flags, FF_PERMANENT);

    /* Hack -- attack monsters */
    if (c_ptr->m_idx && (m_ptr->ml || p_can_enter || p_can_kill_walls))
    {
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Attack -- only if we can see it OR it is not in a wall */
        if (!is_hostile(m_ptr) &&
            !(p_ptr->confused || p_ptr->image || !m_ptr->ml || p_ptr->stun ||
            (mut_present(MUT_BERS_RAGE) && IS_SHERO())) &&
            pattern_seq(py, px, y, x) && (p_can_enter || p_can_kill_walls))
        {
            /* Disturb the monster */
            (void)set_monster_csleep(c_ptr->m_idx, 0);

            /* Extract monster name (or "it") */
            monster_desc(m_name, m_ptr, 0);

            if (m_ptr->ml)
            {
                /* Auto-Recall if possible and visible */
                if (!p_ptr->image) monster_race_track(m_ptr->ap_r_idx);

                /* Track a new monster */
                health_track(c_ptr->m_idx);
            }

            /* displace? */
            if ((stormbringer && (randint1(1000) > 666)) || (p_ptr->pclass == CLASS_BERSERKER))
            {
                py_attack(y, x, 0);
                oktomove = FALSE;
            }
            else if (monster_can_cross_terrain(cave[py][px].feat, r_ptr, 0))
            {
                do_past = TRUE;
            }
            else
            {
                msg_format("%^s is in your way!", m_name);

                energy_use = 0;
                oktomove = FALSE;
            }

            /* now continue on to 'movement' */
        }
        else
        {
            py_attack(y, x, 0);
            if (weaponmaster_get_toggle() == TOGGLE_SHADOW_STANCE)
                shadow_strike = TRUE;
            else
                oktomove = FALSE;
        }
    }

    if (oktomove && (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC))
    {
        monster_race *r_ptr = &r_info[p_ptr->current_r_idx];
        if (r_ptr->flags1 & RF1_NEVER_MOVE)
        {
            energy_use *= 3;
            /*msg_print("You can't move!");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);*/
        }
        else if (have_flag(f_ptr->flags, FF_CAN_FLY) && ((r_ptr->flags7 & RF7_CAN_FLY) || p_ptr->levitation))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_CAN_SWIM) && (r_ptr->flags7 & RF7_CAN_SWIM))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_WATER) &&
            !(r_ptr->flags7 & RF7_AQUATIC) &&
            (have_flag(f_ptr->flags, FF_DEEP) || (r_ptr->flags2 & RF2_AURA_FIRE)))
        {
            msg_print("You can't swim.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (!have_flag(f_ptr->flags, FF_WATER) && (r_ptr->flags7 & RF7_AQUATIC))
        {
            msg_print("You can't move onto dry land.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
    }

    if (oktomove && p_ptr->prace == RACE_MON_RING && !p_ptr->riding)
    {
        msg_print("You can't move! Try using your Glitter power to lure a ringbearer instead.");
        energy_use = 0;
        oktomove = FALSE;
        disturb(0, 0);
    }

    if (oktomove && p_ptr->riding)
    {
        if (riding_r_ptr->flags1 & RF1_NEVER_MOVE)
        {
            msg_print("You can't move!");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (MON_CSLEEP(riding_m_ptr))
        {
            char m_name[80];
            monster_desc(m_name, riding_m_ptr, 0);
            msg_format("%^s is sleeping.", m_name);
            oktomove = FALSE;
            disturb(0,0);
        }
        else if (riding_m_ptr->paralyzed)
        {
            char m_name[80];
            monster_desc(m_name, riding_m_ptr, 0);
            msg_format("%^s is paralyzed.", m_name);
            oktomove = FALSE;
            disturb(0,0);
        }
        else if (MON_MONFEAR(riding_m_ptr))
        {
            char m_name[80];

            /* Acquire the monster name */
            monster_desc(m_name, riding_m_ptr, 0);

            /* Dump a message */
            msg_format("%^s is too scared to control.", m_name);
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (p_ptr->riding_ryoute)
        {
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (have_flag(f_ptr->flags, FF_CAN_FLY) && ((riding_r_ptr->flags7 & RF7_CAN_FLY) || ring_lev))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_CAN_SWIM) && ((riding_r_ptr->flags7 & RF7_CAN_SWIM) || ring_lev))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_WATER) &&
            !(riding_r_ptr->flags7 & RF7_AQUATIC) &&
            (have_flag(f_ptr->flags, FF_DEEP) || (riding_r_ptr->flags2 & RF2_AURA_FIRE)))
        {
            msg_print("Can't swim.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (!have_flag(f_ptr->flags, FF_WATER) && (riding_r_ptr->flags7 & RF7_AQUATIC))
        {
            msg_print("Can't land.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (have_flag(f_ptr->flags, FF_LAVA) && !(riding_r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK))
        {
            msg_print("Too hot to go through.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }

        if (oktomove && MON_STUNNED(riding_m_ptr) && one_in_(2))
        {
            char m_name[80];
            monster_desc(m_name, riding_m_ptr, 0);
            msg_format("You cannot control stunned %s!",m_name);
            oktomove = FALSE;
            disturb(0, 0);
        }
    }

    if (!oktomove)
    {
    }

    else if ( !have_flag(f_ptr->flags, FF_MOVE) 
           && have_flag(f_ptr->flags, FF_CAN_FLY) 
           && !(p_ptr->levitation || ring_lev) )
    {
        msg_format("You need to fly to go through the %s.", f_name + f_info[get_feat_mimic(c_ptr)].name);

        if (!shadow_strike)
            energy_use = 0;
        running = 0;
        oktomove = FALSE;
    }

    /*
     * Player can move through trees and
     * has effective -10 speed
     */
    else if (have_flag(f_ptr->flags, FF_TREE) && !p_can_kill_walls)
    {
        if ( p_ptr->pclass != CLASS_RANGER
          && p_ptr->pclass != CLASS_SCOUT
          && !prace_is_(RACE_ENT) 
          && !prace_is_(RACE_CENTAUR)
          && !prace_is_(RACE_WOOD_ELF)
          && !p_ptr->levitation 
          && (!p_ptr->riding || !(riding_r_ptr->flags8 & RF8_WILD_WOOD))) 
        {
            energy_use *= 2;
        }
    }
    else if (have_flag(f_ptr->flags, FF_WEB) && !prace_is_(RACE_MON_SPIDER))
        energy_use *= 2;

#ifdef ALLOW_EASY_DISARM /* TNB */

    /* Disarm a visible trap */
    else if ((do_pickup != easy_disarm) && have_flag(f_ptr->flags, FF_DISARM) && !c_ptr->mimic)
    {
        if (!trap_can_be_ignored(c_ptr->feat))
        {
            (void)do_cmd_disarm_aux(y, x, dir);
            return;
        }
    }

#endif /* ALLOW_EASY_DISARM -- TNB */

    /* Player can not walk through "walls" unless in wraith form...*/
    else if (!p_can_enter && !p_can_kill_walls)
    {
        /* Feature code (applying "mimic" field) */
        s16b feat = get_feat_mimic(c_ptr);
        feature_type *mimic_f_ptr = &f_info[feat];
        cptr name = f_name + mimic_f_ptr->name;

        oktomove = FALSE;

        /* Disturb the player */
        disturb(0, 0);

        /* Notice things in the dark */
        if (!(c_ptr->info & CAVE_MARK) && !player_can_see_bold(y, x))
        {
            /* Boundary floor mimic */
            if (boundary_floor(c_ptr, f_ptr, mimic_f_ptr))
            {
                msg_print("You feel you cannot go any more.");
            }

            /* Wall (or secret door) */
            else
            {
                msg_format("You feel %s %s blocking your way.",
                    is_a_vowel(name[0]) ? "an" : "a", name);

                c_ptr->info |= (CAVE_MARK);
                lite_spot(y, x);
            }
        }

        /* Notice things */
        else
        {
            /* Boundary floor mimic */
            if (boundary_floor(c_ptr, f_ptr, mimic_f_ptr))
            {
                msg_print("You cannot go any more.");

                if (!(p_ptr->confused || p_ptr->stun || p_ptr->image))
                {
                    if (!shadow_strike)
                        energy_use = 0;
                }
            }

            /* Wall (or secret door) */
            else
            {
#ifdef ALLOW_EASY_OPEN
                /* Closed doors */
                if (easy_open && is_closed_door(feat) && easy_open_door(y, x))
                {
                    /* Hack. Try to deduce what happened since easy_open_door hides this.
                       Try to repeat attempting to unlock the door, but do a quick check
                       for jammed doors so we don't waste 99 turns. Also, only make
                       99 attempts to pick the lock ... But using command_rep would be
                       unwise since we will then run thru the door once we pick the lock! */
                    if (always_repeat)
                    {
                        static int _repeat_count = 0;

                        cave_type *c_ptr = &cave[y][x];
                        feature_type *f_ptr = &f_info[c_ptr->feat];

                        if (is_closed_door(c_ptr->feat) && have_flag(f_ptr->flags, FF_OPEN))
                        {
                            if (_repeat_count == 0)
                                _repeat_count = 99;
                            else
                                --_repeat_count;

                            if (_repeat_count)
                                command_rep = 1;
                        }
                        else
                            _repeat_count = 0;
                    }
                    return;
                }
#endif /* ALLOW_EASY_OPEN */

                msg_format("There is %s %s blocking your way.",
                    is_a_vowel(name[0]) ? "an" : "a", name);

                /*
                 * Well, it makes sense that you lose time bumping into
                 * a wall _if_ you are confused, stunned or blind; but
                 * typing mistakes should not cost you a turn...
                 */
                if (!(p_ptr->confused || p_ptr->stun || p_ptr->image))
                {
                    if (!shadow_strike)
                        energy_use = 0;
                }
            }
        }

        /* Sound */
        if (!boundary_floor(c_ptr, f_ptr, mimic_f_ptr)) sound(SOUND_HITWALL);
    }

    if (oktomove)
    {
        if (have_flag(f_ptr->flags, FF_CAN_PASS) && !p_can_kill_walls && !elemental_is_(ELEMENTAL_EARTH))
            energy_use *= 2;

        if (have_flag(f_ptr->flags, FF_LAVA) && elemental_is_(ELEMENTAL_FIRE))
            energy_use /= 2;

        if (have_flag(f_ptr->flags, FF_WATER) && elemental_is_(ELEMENTAL_WATER))
            energy_use /= 2;
    }

    /* Normal movement */
    if (oktomove && !pattern_seq(py, px, y, x))
    {
        if (!(p_ptr->confused || p_ptr->stun || p_ptr->image))
        {
            if (!shadow_strike)
                energy_use = 0;
        }

        /* To avoid a loop with running */
        disturb(0, 0);

        oktomove = FALSE;
    }

    /* Normal movement */
    if (oktomove)
    {
        u32b mpe_mode = MPE_ENERGY_USE;

        if (p_ptr->warning)
        {
            if (!process_warning(x, y))
            {
                if (!shadow_strike)
                    energy_use = 25;
                return;
            }
        }

        if (do_past)
        {
            msg_format("You push past %s.", m_name);
        }

        /* Change oldpx and oldpy to place the player well when going back to big mode */
        if (p_ptr->wild_mode)
        {
            if (ddy[dir] > 0)  p_ptr->oldpy = 1;
            if (ddy[dir] < 0)  p_ptr->oldpy = MAX_HGT - 2;
            if (ddy[dir] == 0) p_ptr->oldpy = MAX_HGT / 2;
            if (ddx[dir] > 0)  p_ptr->oldpx = 1;
            if (ddx[dir] < 0)  p_ptr->oldpx = MAX_WID - 2;
            if (ddx[dir] == 0) p_ptr->oldpx = MAX_WID / 2;
            p_ptr->wilderness_dx = 0;
            p_ptr->wilderness_dy = 0;
        }

        if (p_can_kill_walls)
        {
            cave_alter_feat(y, x, FF_HURT_DISI);

            /* Update some things -- similar to GF_KILL_WALL */
            p_ptr->update |= (PU_FLOW);
        }

        /* Sound */
        /* sound(SOUND_WALK); */

#ifdef ALLOW_EASY_DISARM /* TNB */

        if (do_pickup != always_pickup) mpe_mode |= MPE_DO_PICKUP;

#else /* ALLOW_EASY_DISARM -- TNB */

        if (do_pickup) mpe_mode |= MPE_DO_PICKUP;

#endif /* ALLOW_EASY_DISARM -- TNB */

        if (break_trap) mpe_mode |= MPE_BREAK_TRAP;

        /* Move the player */
        (void)move_player_effect(y, x, mpe_mode);
    }
}


static bool ignore_avoid_run;

/*
 * Hack -- Check for a "known wall" (see below)
 */
static int see_wall(int dir, int y, int x)
{
    cave_type   *c_ptr;

    /* Get the new location */
    y += ddy[dir];
    x += ddx[dir];

    /* Illegal grids are not known walls */
    if (!in_bounds2(y, x)) return (FALSE);

    /* Access grid */
    c_ptr = &cave[y][x];

    /* Must be known to the player */
    if (c_ptr->info & (CAVE_MARK))
    {
        /* Feature code (applying "mimic" field) */
        s16b         feat = get_feat_mimic(c_ptr);
        feature_type *f_ptr = &f_info[feat];

        /* Wall grids are known walls */
        if (!player_can_enter(feat, 0)) return !have_flag(f_ptr->flags, FF_DOOR);

        /* Don't run on a tree unless explicitly requested */
        if (have_flag(f_ptr->flags, FF_AVOID_RUN) && !ignore_avoid_run)
            return TRUE;

        /* Don't run in a wall */
        if (!have_flag(f_ptr->flags, FF_MOVE) && !have_flag(f_ptr->flags, FF_CAN_FLY))
            return !have_flag(f_ptr->flags, FF_DOOR);
    }

    return FALSE;
}


/*
 * Hack -- Check for an "unknown corner" (see below)
 */
static int see_nothing(int dir, int y, int x)
{
    /* Get the new location */
    y += ddy[dir];
    x += ddx[dir];

    /* Illegal grids are unknown */
    if (!in_bounds2(y, x)) return (TRUE);

    /* Memorized grids are always known */
    if (cave[y][x].info & (CAVE_MARK)) return (FALSE);

    /* Viewable door/wall grids are known */
    if (player_can_see_bold(y, x)) return (FALSE);

    /* Default */
    return (TRUE);
}





/*
 * The running algorithm:                       -CJS-
 *
 * In the diagrams below, the player has just arrived in the
 * grid marked as '@', and he has just come from a grid marked
 * as 'o', and he is about to enter the grid marked as 'x'.
 *
 * Of course, if the "requested" move was impossible, then you
 * will of course be blocked, and will stop.
 *
 * Overview: You keep moving until something interesting happens.
 * If you are in an enclosed space, you follow corners. This is
 * the usual corridor scheme. If you are in an open space, you go
 * straight, but stop before entering enclosed space. This is
 * analogous to reaching doorways. If you have enclosed space on
 * one side only (that is, running along side a wall) stop if
 * your wall opens out, or your open space closes in. Either case
 * corresponds to a doorway.
 *
 * What happens depends on what you can really SEE. (i.e. if you
 * have no light, then running along a dark corridor is JUST like
 * running in a dark room.) The algorithm works equally well in
 * corridors, rooms, mine tailings, earthquake rubble, etc, etc.
 *
 * These conditions are kept in static memory:
 * find_openarea         You are in the open on at least one
 * side.
 * find_breakleft        You have a wall on the left, and will
 * stop if it opens
 * find_breakright       You have a wall on the right, and will
 * stop if it opens
 *
 * To initialize these conditions, we examine the grids adjacent
 * to the grid marked 'x', two on each side (marked 'L' and 'R').
 * If either one of the two grids on a given side is seen to be
 * closed, then that side is considered to be closed. If both
 * sides are closed, then it is an enclosed (corridor) run.
 *
 * LL           L
 * @x          LxR
 * RR          @R
 *
 * Looking at more than just the immediate squares is
 * significant. Consider the following case. A run along the
 * corridor will stop just before entering the center point,
 * because a choice is clearly established. Running in any of
 * three available directions will be defined as a corridor run.
 * Note that a minor hack is inserted to make the angled corridor
 * entry (with one side blocked near and the other side blocked
 * further away from the runner) work correctly. The runner moves
 * diagonally, but then saves the previous direction as being
 * straight into the gap. Otherwise, the tail end of the other
 * entry would be perceived as an alternative on the next move.
 *
 * #.#
 * ##.##
 * .@x..
 * ##.##
 * #.#
 *
 * Likewise, a run along a wall, and then into a doorway (two
 * runs) will work correctly. A single run rightwards from @ will
 * stop at 1. Another run right and down will enter the corridor
 * and make the corner, stopping at the 2.
 *
 * ##################
 * o@x       1
 * ########### ######
 * #2          #
 * #############
 *
 * After any move, the function area_affect is called to
 * determine the new surroundings, and the direction of
 * subsequent moves. It examines the current player location
 * (at which the runner has just arrived) and the previous
 * direction (from which the runner is considered to have come).
 *
 * Moving one square in some direction places you adjacent to
 * three or five new squares (for straight and diagonal moves
 * respectively) to which you were not previously adjacent,
 * marked as '!' in the diagrams below.
 *
 *   ...!              ...
 *   .o@!  (normal)    .o.!  (diagonal)
 *   ...!  (east)      ..@!  (south east)
 *                      !!!
 *
 * You STOP if any of the new squares are interesting in any way:
 * for example, if they contain visible monsters or treasure.
 *
 * You STOP if any of the newly adjacent squares seem to be open,
 * and you are also looking for a break on that side. (that is,
 * find_openarea AND find_break).
 *
 * You STOP if any of the newly adjacent squares do NOT seem to be
 * open and you are in an open area, and that side was previously
 * entirely open.
 *
 * Corners: If you are not in the open (i.e. you are in a corridor)
 * and there is only one way to go in the new squares, then turn in
 * that direction. If there are more than two new ways to go, STOP.
 * If there are two ways to go, and those ways are separated by a
 * square which does not seem to be open, then STOP.
 *
 * Otherwise, we have a potential corner. There are two new open
 * squares, which are also adjacent. One of the new squares is
 * diagonally located, the other is straight on (as in the diagram).
 * We consider two more squares further out (marked below as ?).
 *
 * We assign "option" to the straight-on grid, and "option2" to the
 * diagonal grid, and "check_dir" to the grid marked 's'.
 *
 * ##s
 * @x?
 * #.?
 *
 * If they are both seen to be closed, then it is seen that no benefit
 * is gained from moving straight. It is a known corner.  To cut the
 * corner, go diagonally, otherwise go straight, but pretend you
 * stepped diagonally into that next location for a full view next
 * time. Conversely, if one of the ? squares is not seen to be closed,
 * then there is a potential choice. We check to see whether it is a
 * potential corner or an intersection/room entrance.  If the square
 * two spaces straight ahead, and the space marked with 's' are both
 * unknown space, then it is a potential corner and enter if
 * find_examine is set, otherwise must stop because it is not a
 * corner. (find_examine option is removed and always is TRUE.)
 */




/*
 * Hack -- allow quick "cycling" through the legal directions
 */
static byte cycle[] =
{ 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };

/*
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static byte chome[] =
{ 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };

/*
 * The direction we are running
 */
static byte find_current;

/*
 * The direction we came from
 */
static byte find_prevdir;

/*
 * We are looking for open area
 */
static bool find_openarea;

/*
 * We are looking for a break
 */
static bool find_breakright;
static bool find_breakleft;



/*
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. ???
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #                  #
 *       #x#                 @x#
 *       @p.                  p
 */
static void run_init(int dir)
{
    int             row, col, deepleft, deepright;
    int             i, shortleft, shortright;


    /* Save the direction */
    find_current = dir;

    /* Assume running straight */
    find_prevdir = dir;

    /* Assume looking for open area */
    find_openarea = TRUE;

    /* Assume not looking for breaks */
    find_breakright = find_breakleft = FALSE;

    /* Assume no nearby walls */
    deepleft = deepright = FALSE;
    shortright = shortleft = FALSE;

    p_ptr->run_py = py;
    p_ptr->run_px = px;

    /* Find the destination grid */
    row = py + ddy[dir];
    col = px + ddx[dir];

    ignore_avoid_run = cave_have_flag_bold(row, col, FF_AVOID_RUN);

    /* Extract cycle index */
    i = chome[dir];

    /* Check for walls */
    if (see_wall(cycle[i+1], py, px))
    {
        find_breakleft = TRUE;
        shortleft = TRUE;
    }
    else if (see_wall(cycle[i+1], row, col))
    {
        find_breakleft = TRUE;
        deepleft = TRUE;
    }

    /* Check for walls */
    if (see_wall(cycle[i-1], py, px))
    {
        find_breakright = TRUE;
        shortright = TRUE;
    }
    else if (see_wall(cycle[i-1], row, col))
    {
        find_breakright = TRUE;
        deepright = TRUE;
    }

    /* Looking for a break */
    if (find_breakleft && find_breakright)
    {
        /* Not looking for open area */
        find_openarea = FALSE;

        /* Hack -- allow angled corridor entry */
        if (dir & 0x01)
        {
            if (deepleft && !deepright)
            {
                find_prevdir = cycle[i - 1];
            }
            else if (deepright && !deepleft)
            {
                find_prevdir = cycle[i + 1];
            }
        }

        /* Hack -- allow blunt corridor entry */
        else if (see_wall(cycle[i], row, col))
        {
            if (shortleft && !shortright)
            {
                find_prevdir = cycle[i - 2];
            }
            else if (shortright && !shortleft)
            {
                find_prevdir = cycle[i + 2];
            }
        }
    }
}


/*
 * Update the current "run" path
 *
 * Return TRUE if the running should be stopped
 */
static bool run_test(void)
{
    int         prev_dir, new_dir, check_dir = 0;
    int         row, col;
    int         i, max, inv;
    int         option = 0, option2 = 0;
    cave_type   *c_ptr;
    s16b        feat;
    feature_type *f_ptr;

    /* Where we came from */
    prev_dir = find_prevdir;


    /* Range of newly adjacent grids */
    max = (prev_dir & 0x01) + 1;

    /* break run when leaving trap detected region */
    if ((disturb_trap_detect || alert_trap_detect)
        && p_ptr->dtrap && !(cave[py][px].info & CAVE_IN_DETECT))
    {
        /* No duplicate warning */
        p_ptr->dtrap = FALSE;

        /* You are just on the edge */
        if (!(cave[py][px].info & CAVE_UNSAFE))
        {
            if (alert_trap_detect)
            {
                msg_print("*Leaving trap detect region!*");
            }

            if (disturb_trap_detect)
            {
                /* Break Run */
                return(TRUE);
            }
        }
    }

    /* Look at every newly adjacent square. */
    for (i = -max; i <= max; i++)
    {
        s16b this_o_idx, next_o_idx = 0;

        /* New direction */
        new_dir = cycle[chome[prev_dir] + i];

        /* New location */
        row = py + ddy[new_dir];
        col = px + ddx[new_dir];

        /* Access grid */
        c_ptr = &cave[row][col];

        /* Feature code (applying "mimic" field) */
        feat = get_feat_mimic(c_ptr);
        f_ptr = &f_info[feat];

        /* Visible monsters abort running */
        if (c_ptr->m_idx)
        {
            monster_type *m_ptr = &m_list[c_ptr->m_idx];

            /* Visible monster */
            if (m_ptr->ml) return (TRUE);
        }

        /* Visible objects abort running */
        for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
        {
            object_type *o_ptr;

            /* Acquire object */
            o_ptr = &o_list[this_o_idx];

            /* Acquire next object */
            next_o_idx = o_ptr->next_o_idx;

            /* Visible object */
            if (o_ptr->marked & OM_FOUND) return (TRUE);
        }

        /* Assume unknown */
        inv = TRUE;

        /* Check memorized grids */
        if (c_ptr->info & (CAVE_MARK))
        {
            bool notice = have_flag(f_ptr->flags, FF_NOTICE);

            if (notice && have_flag(f_ptr->flags, FF_MOVE))
            {
                /* Open doors */
                if (find_ignore_doors && have_flag(f_ptr->flags, FF_DOOR) && have_flag(f_ptr->flags, FF_CLOSE))
                {
                    /* Option -- ignore */
                    notice = FALSE;
                }

                /* Stairs */
                else if (find_ignore_stairs && have_flag(f_ptr->flags, FF_STAIRS))
                {
                    /* Option -- ignore */
                    notice = FALSE;
                }

                /* Lava */
                else if (have_flag(f_ptr->flags, FF_LAVA) && (res_pct(RES_FIRE) >= 100 || IS_INVULN()))
                {
                    /* Ignore */
                    notice = FALSE;
                }

                /* Deep water */
                else if (have_flag(f_ptr->flags, FF_WATER) && have_flag(f_ptr->flags, FF_DEEP) &&
                         (p_ptr->levitation || p_ptr->can_swim || (p_ptr->total_weight <= weight_limit())))
                {
                    /* Ignore */
                    notice = FALSE;
                }
            }

            /* Interesting feature */
            if (notice) return (TRUE);

            /* The grid is "visible" */
            inv = FALSE;
        }

        /* Analyze unknown grids and floors considering mimic */
        if (inv || !see_wall(0, row, col))
        {
            /* Looking for open area */
            if (find_openarea)
            {
                /* Nothing */
            }

            /* The first new direction. */
            else if (!option)
            {
                option = new_dir;
            }

            /* Three new directions. Stop running. */
            else if (option2)
            {
                return (TRUE);
            }

            /* Two non-adjacent new directions.  Stop running. */
            else if (option != cycle[chome[prev_dir] + i - 1])
            {
                return (TRUE);
            }

            /* Two new (adjacent) directions (case 1) */
            else if (new_dir & 0x01)
            {
                check_dir = cycle[chome[prev_dir] + i - 2];
                option2 = new_dir;
            }

            /* Two new (adjacent) directions (case 2) */
            else
            {
                check_dir = cycle[chome[prev_dir] + i + 1];
                option2 = option;
                option = new_dir;
            }
        }

        /* Obstacle, while looking for open area */
        else
        {
            if (find_openarea)
            {
                if (i < 0)
                {
                    /* Break to the right */
                    find_breakright = TRUE;
                }

                else if (i > 0)
                {
                    /* Break to the left */
                    find_breakleft = TRUE;
                }
            }
        }
    }

    /* Looking for open area */
    if (find_openarea)
    {
        /* Hack -- look again */
        for (i = -max; i < 0; i++)
        {
            /* Unknown grid or non-wall */
            if (!see_wall(cycle[chome[prev_dir] + i], py, px))
            {
                /* Looking to break right */
                if (find_breakright)
                {
                    return (TRUE);
                }
            }

            /* Obstacle */
            else
            {
                /* Looking to break left */
                if (find_breakleft)
                {
                    return (TRUE);
                }
            }
        }

        /* Hack -- look again */
        for (i = max; i > 0; i--)
        {
            /* Unknown grid or non-wall */
            if (!see_wall(cycle[chome[prev_dir] + i], py, px))
            {
                /* Looking to break left */
                if (find_breakleft)
                {
                    return (TRUE);
                }
            }

            /* Obstacle */
            else
            {
                /* Looking to break right */
                if (find_breakright)
                {
                    return (TRUE);
                }
            }
        }
    }

    /* Not looking for open area */
    else
    {
        /* No options */
        if (!option)
        {
            return (TRUE);
        }

        /* One option */
        else if (!option2)
        {
            /* Primary option */
            find_current = option;

            /* No other options */
            find_prevdir = option;
        }

        /* Two options, examining corners */
        else if (!find_cut)
        {
            /* Primary option */
            find_current = option;

            /* Hack -- allow curving */
            find_prevdir = option2;
        }

        /* Two options, pick one */
        else
        {
            /* Get next location */
            row = py + ddy[option];
            col = px + ddx[option];

            /* Don't see that it is closed off. */
            /* This could be a potential corner or an intersection. */
            if (!see_wall(option, row, col) ||
                !see_wall(check_dir, row, col))
            {
                /* Can not see anything ahead and in the direction we */
                /* are turning, assume that it is a potential corner. */
                if (see_nothing(option, row, col) &&
                    see_nothing(option2, row, col))
                {
                    find_current = option;
                    find_prevdir = option2;
                }

                /* STOP: we are next to an intersection or a room */
                else
                {
                    return (TRUE);
                }
            }

            /* This corner is seen to be enclosed; we cut the corner. */
            else if (find_cut)
            {
                find_current = option2;
                find_prevdir = option2;
            }

            /* This corner is seen to be enclosed, and we */
            /* deliberately go the long way. */
            else
            {
                find_current = option;
                find_prevdir = option2;
            }
        }
    }

    /* About to hit a known wall, stop */
    if (see_wall(find_current, py, px))
    {
        return (TRUE);
    }

    /* Failure */
    return (FALSE);
}



/*
 * Take one step along the current "run" path
 */
void run_step(int dir)
{
    /* Start running */
    if (dir)
    {
        /* Ignore AVOID_RUN on a first step */
        ignore_avoid_run = TRUE;

        /* Hack -- do not start silly run */
        if (see_wall(dir, py, px))
        {
            /* Message */
            msg_print("You cannot run in that direction.");

            /* Disturb */
            disturb(0, 0);

            /* Done */
            return;
        }

        /* Initialize */
        run_init(dir);
    }

    /* Keep running */
    else
    {
        /* Update run */
        if (run_test())
        {
            /* Disturb */
            disturb(0, 0);

            /* Done */
            return;
        }
    }

    /* Decrease the run counter */
    if (--running <= 0) return;

    /* Take time */
    energy_use = 100;

    /* Move the player, using the "pickup" flag */
#ifdef ALLOW_EASY_DISARM /* TNB */

    move_player(find_current, FALSE, FALSE);

#else /* ALLOW_EASY_DISARM -- TNB */

    move_player(find_current, always_pickup, FALSE);

#endif /* ALLOW_EASY_DISARM -- TNB */

    if (player_bold(p_ptr->run_py, p_ptr->run_px))
    {
        p_ptr->run_py = 0;
        p_ptr->run_px = 0;
        disturb(0, 0);
    }
}


#ifdef TRAVEL
/*
 * Test for traveling
 */
static bool travel_test(void)
{
    int prev_dir, new_dir;
    int row, col;
    int i, max;
    bool stop = TRUE;
    cave_type *c_ptr;

    /* Where we came from */
    prev_dir = find_prevdir;

    /* Range of newly adjacent grids */
    max = (prev_dir & 0x01) + 1;

    for (i = 0; i < 8; i++)
    {
        if (travel.cost[py+ddy_ddd[i]][px+ddx_ddd[i]] < travel.cost[py][px]) stop = FALSE;
    }

    if (stop) return (TRUE);

    /* break run when leaving trap detected region */
    if ((disturb_trap_detect || alert_trap_detect)
        && p_ptr->dtrap && !(cave[py][px].info & CAVE_IN_DETECT))
    {
        /* No duplicate warning */
        p_ptr->dtrap = FALSE;

        /* You are just on the edge */
        if (!(cave[py][px].info & CAVE_UNSAFE))
        {
            if (alert_trap_detect)
            {
                msg_print("*Leaving trap detect region!*");
            }

            if (disturb_trap_detect)
            {
                /* Break Run */
                return(TRUE);
            }
        }
    }

    /* Cannot travel when blind */
    if (p_ptr->blind || no_lite())
    {
        msg_print("You cannot see!");
        return (TRUE);
    }

    /* Look at every newly adjacent square. */
    for (i = -max; i <= max; i++)
    {
        /* New direction */
        new_dir = cycle[chome[prev_dir] + i];

        /* New location */
        row = py + ddy[new_dir];
        col = px + ddx[new_dir];

        /* Access grid */
        c_ptr = &cave[row][col];


        /* Visible monsters abort running */
        if (c_ptr->m_idx)
        {
            monster_type *m_ptr = &m_list[c_ptr->m_idx];

            /* Visible monster */
            if (m_ptr->ml) return (TRUE);
        }
    }

    /* Failure */
    return (FALSE);
}


/*
 * Travel command
 */
void travel_step(void)
{
    int i;
    int dir = travel.dir;
    int old_run = travel.run;

    find_prevdir = dir;

    /* disturb */
    if (travel_test())
    {
        if (travel.run == 255)
        {
            msg_print("No route is found!");
        }
        disturb(0, 0);
        return;
    }

    energy_use = 100;

    for (i = 1; i <= 9; i++)
    {
        if (i == 5) continue;

        if (travel.cost[py+ddy[i]][px+ddx[i]] < travel.cost[py+ddy[dir]][px+ddx[dir]])
        {
            dir = i;
        }
    }

    /* Close door */
    if (!easy_open && is_closed_door(cave[py+ddy[dir]][px+ddx[dir]].feat))
    {
        disturb(0, 0);
        return;
    }

    travel.dir = dir;
    move_player(dir, always_pickup, easy_disarm);
    Term_xtra(TERM_XTRA_DELAY, delay_factor * delay_factor * delay_factor);
    Term_fresh();
    travel.run = old_run;

    if ((py == travel.y) && (px == travel.x))
        travel.run = 0;
    else
        travel.run--;
}
#endif
