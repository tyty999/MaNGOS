/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Hazzarah
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "zulgurub.h"

enum eHazzarah
{
    SPELL_MANA_BURN             = 26046,
    SPELL_SLEEP                 = 24664,
    SUMMON_NIGHTMARE_ILLUSION_L   = 24729,
    SUMMON_NIGHTMARE_ILLUSION_R  = 24681,
    SUMMON_NIGHTMARE_ILLUSION_B = 24728,

    MODELID_FELGUARD            = 5049,
    MODELID_ABOMINATION         = 1693,
    MODELID_LASHER              = 8172,
    MODELID_DEVILSAUR           = 5240,
	MODELID_BARTHILAS			= 10433,
};

struct MANGOS_DLL_DECL boss_hazzarahAI : public ScriptedAI
{
    boss_hazzarahAI(Creature* pCreature) : ScriptedAI(pCreature) 
	{
		m_bSpawnLightning = false;
		Reset();
	}

    uint32 m_uiManaBurnTimer;
    uint32 m_uiSleepTimer;
    uint32 m_uiIllusionsTimer;
	bool m_bSpawnLightning;

    void Reset()
    {
        m_uiManaBurnTimer = urand(4000, 10000);
        m_uiSleepTimer = urand(10000, 15000);
        m_uiIllusionsTimer = urand(17000, 22000);
    }
    
     void JustReachedHome()
    {
        m_creature->RemoveGuardians();       
    }
    
    void JustSummoned(Creature* pSummoned)
    {
        // Summoned Illusion will have a random model
        uint32 m_uiNewDisplayId = 0;
        switch (rand() % 5)
        {
            case 0: 
                m_uiNewDisplayId = MODELID_ABOMINATION;
                break;
            case 1: 
                m_uiNewDisplayId = MODELID_LASHER;
                break;
            case 2: 
                m_uiNewDisplayId = MODELID_FELGUARD;
                break;
            case 3: 
                m_uiNewDisplayId = MODELID_DEVILSAUR;                
                break;
            case 4: 
                    m_uiNewDisplayId = MODELID_BARTHILAS; 
                break;
        }
        pSummoned->SetFloatValue(OBJECT_FIELD_SCALE_X, 3.0f);
        pSummoned->SetDisplayId(m_uiNewDisplayId);
        pSummoned->SetMaxHealth(500);
        
        Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
        if(pTarget)        
            pSummoned->AI()->AttackStart(pTarget);
        
        pSummoned->StopMoving();
        pSummoned->addUnitState(UNIT_STAT_CAN_NOT_MOVE);        
        
        // no respawns
        pSummoned->SetRespawnEnabled(false);
    }

    void UpdateAI(const uint32 uiDiff)
    {
		if (!m_bSpawnLightning)
		{
			m_creature->CastSpell(m_creature, SPELL_RED_LIGHTNING, true);
			m_bSpawnLightning = true;
		}

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Mana Burn
        if (m_uiManaBurnTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_MANA_BURN);
            m_uiManaBurnTimer = urand(8000, 16000);
        }
        else
            m_uiManaBurnTimer -= uiDiff;

        // Sleep
        if (m_uiSleepTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_SLEEP);
            m_uiSleepTimer = urand(25000, 35000);
        }
        else
            m_uiSleepTimer -= uiDiff;
	
        // Illusions
        if (m_uiIllusionsTimer <= uiDiff)
        {
            m_creature->CastSpell(m_creature, SUMMON_NIGHTMARE_ILLUSION_B, true);
            m_creature->CastSpell(m_creature, SUMMON_NIGHTMARE_ILLUSION_L, true);
            m_creature->CastSpell(m_creature, SUMMON_NIGHTMARE_ILLUSION_R, true);
          /*    
            //We will summon 3 illusions that will spawn on a random player and attack this player
            for (uint8 i = 0; i < 3; ++i)
            {
                float x, y, z;
                short attempts = 0;
                float radius = frand(0, 10);

                // Make 20 attempts at finding a correct spawn position.
                while (attempts < 20)
                {
                    float angle = frand(0, 2 * PI);
                    
                    x = m_creature->GetPositionX() + radius * cosf(angle);
                    y = m_creature->GetPositionY() + radius * sinf(angle);
                    z = m_creature->GetPositionZ();

                    if (m_creature->IsWithinLOS(x, y, z))
                        break;

                    ++attempts;
                }

                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    Creature* pIllusion = m_creature->SummonCreature(NPC_NIGHTMARE_ILLUSION, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
					if (pIllusion)
                    {
                        // Summoned Illusion will to have a random model
                        uint32 m_uiNewDisplayId = 0;
                        switch (rand() % 5)
                        {
                            case 0: m_uiNewDisplayId = MODELID_ABOMINATION;
								pIllusion->SetFloatValue(OBJECT_FIELD_SCALE_X, 3.0f);
								break;
                            case 1: m_uiNewDisplayId = MODELID_LASHER;
								pIllusion->SetFloatValue(OBJECT_FIELD_SCALE_X, 3.0f);
								break;
                            case 2: m_uiNewDisplayId = MODELID_FELGUARD;
								pIllusion->SetFloatValue(OBJECT_FIELD_SCALE_X, 3.0f);
								break;
                            case 3: m_uiNewDisplayId = MODELID_DEVILSAUR; 
								pIllusion->SetFloatValue(OBJECT_FIELD_SCALE_X, 3.0f);
								break;
								case 4: m_uiNewDisplayId = MODELID_BARTHILAS; 
								pIllusion->SetFloatValue(OBJECT_FIELD_SCALE_X, 3.0f);
								break;
                        }

                        pIllusion->SetDisplayId(m_uiNewDisplayId);
                       //pIllusion->SetFloatValue(OBJECT_FIELD_SCALE_X, 2.0f);   // bigger model
                        pIllusion->AI()->AttackStart(pTarget);
                    }
                }
            }*/
            m_uiIllusionsTimer = urand(15000, 21000);
        }
        else
            m_uiIllusionsTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_hazzarah(Creature* pCreature)
{
    return new boss_hazzarahAI(pCreature);
}

/*######
## mob_nightmare_illusion
######*/

struct MANGOS_DLL_DECL mob_nightmare_illusionAI : public ScriptedAI
{
    mob_nightmare_illusionAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
		m_creature->SetAOEImmunity(true);	
        Reset();
    }
    
    uint32 m_uiSpawnTimer;

    void Reset()
    {
        m_uiSpawnTimer = 2500;
    }

	void UpdateAI(const uint32 uiDiff)
    {
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;
        
        if (m_uiSpawnTimer <= uiDiff)
            m_creature->clearUnitState(UNIT_STAT_CAN_NOT_MOVE);
        else
            m_uiSpawnTimer -= uiDiff;
        
        DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_mob_nightmare_illusion(Creature* pCreature)
{
    return new mob_nightmare_illusionAI(pCreature);
}

void AddSC_boss_hazzarah()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "boss_hazzarah";
    pNewScript->GetAI = &GetAI_boss_hazzarah;
    pNewScript->RegisterSelf();

	pNewScript = new Script;
    pNewScript->Name = "mob_nightmare_illusion";
    pNewScript->GetAI = &GetAI_mob_nightmare_illusion;
    pNewScript->RegisterSelf();
}
