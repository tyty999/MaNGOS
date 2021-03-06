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
SDName: Hinterlands
SD%Complete: 100
SDComment: Quest support: 863, 2742
SDCategory: The Hinterlands
EndScriptData */

/* ContentData
npc_00x09hl
npc_rinji
mob_vilebranch_kidnapper
go_lard_picnic_basket
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

 /*######
## npc_00x09hl
######*/

enum
{
    SAY_OOX_START           = -1000287,
    SAY_OOX_AGGRO1          = -1000288,
    SAY_OOX_AGGRO2          = -1000289,
    SAY_OOX_AMBUSH          = -1000290,
    SAY_OOX_END             = -1000292,

    QUEST_RESQUE_OOX_09     = 836,

    NPC_MARAUDING_OWL       = 7808,
    NPC_VILE_AMBUSHER       = 7809
};

struct MANGOS_DLL_DECL npc_00x09hlAI : public npc_escortAI
{
    npc_00x09hlAI(Creature* pCreature) : npc_escortAI(pCreature) 
    { 
        m_bFirstSpawned = false;
        m_bSecondSpawned = false; 

        Reset(); 
    }

    bool m_bFirstSpawned;
    bool m_bSecondSpawned;

    void Reset() { }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 26:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);
                break;
            case 43:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);
                break;
            case 64:
                DoScriptText(SAY_OOX_END, m_creature);
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_RESQUE_OOX_09, m_creature);
                break;
        }
    }

    void WaypointStart(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 5:
                {
                    // Reset the two owl encounters.
                    m_bFirstSpawned = false;
                    m_bSecondSpawned = false;
                    break;
                }
            case 27:
                if (!m_bFirstSpawned)
                {
                    for(uint8 i = 0; i < 3; ++i)
                    {
                        float fX, fY, fZ;
                        m_creature->GetRandomPoint(147.927444f, -3851.513428f, 130.893f, 7.0f, fX, fY, fZ);

                        m_creature->SummonCreature(NPC_MARAUDING_OWL, fX, fY, fZ, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 25000);
                    }

                    m_bFirstSpawned = true;
                }
                break;
            case 44:
                if (!m_bSecondSpawned)
                {
                    for(uint8 i = 0; i < 3; ++i)
                    {
                        float fX, fY, fZ;
                        m_creature->GetRandomPoint(-141.151581f, -4291.213867f, 120.130f, 7.0f, fX, fY, fZ);

                        m_creature->SummonCreature(NPC_VILE_AMBUSHER, fX, fY, fZ, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 25000);
                    }

                    m_bSecondSpawned = true;
                }
                break;
        }
    }

    void Aggro(Unit* pWho)
    {
        if (pWho->GetEntry() == NPC_MARAUDING_OWL || pWho->GetEntry() == NPC_VILE_AMBUSHER)
            return;

        if (urand(0, 1))
            DoScriptText(SAY_OOX_AGGRO1, m_creature);
        else
            DoScriptText(SAY_OOX_AGGRO2, m_creature);
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->GetMotionMaster()->MovePoint(0, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ());
    }
};

bool QuestAccept_npc_00x09hl(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_RESQUE_OOX_09)
    {
        pCreature->SetStandState(UNIT_STAND_STATE_STAND);

        if (pPlayer->GetTeam() == ALLIANCE)
            pCreature->setFaction(FACTION_ESCORT_A_PASSIVE);
        else if (pPlayer->GetTeam() == HORDE)
            pCreature->setFaction(FACTION_ESCORT_H_PASSIVE);

        DoScriptText(SAY_OOX_START, pCreature, pPlayer);

        if (npc_00x09hlAI* pEscortAI = dynamic_cast<npc_00x09hlAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

CreatureAI* GetAI_npc_00x09hl(Creature* pCreature)
{
    return new npc_00x09hlAI(pCreature);
}

/*######
## npc_rinji
######*/

enum
{
    SAY_RIN_FREE            = -1000403,
    SAY_RIN_BY_OUTRUNNER    = -1000404,
    SAY_RIN_HELP_1          = -1000405,
    SAY_RIN_HELP_2          = -1000406,
    SAY_RIN_COMPLETE        = -1000407,
    SAY_RIN_PROGRESS_1      = -1000408,
    SAY_RIN_PROGRESS_2      = -1000409,

    QUEST_RINJI_TRAPPED     = 2742,
    NPC_RANGER              = 2694,
    NPC_OUTRUNNER           = 2691,
    GO_RINJI_CAGE           = 142036
};

struct Location
{
    float m_fX, m_fY, m_fZ;
};

Location m_afAmbushSpawn[] =
{
    {191.296204f, -2839.329346f, 107.388f},
    {70.972466f,  -2848.674805f, 109.459f}
};

Location m_afAmbushMoveTo[] =
{
    {166.630386f, -2824.780273f, 108.153f},
    {70.886589f,  -2874.335449f, 116.675f}
};

struct MANGOS_DLL_DECL npc_rinjiAI : public npc_escortAI
{
    npc_rinjiAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_bIsByOutrunner = false;
        m_iSpawnId = 0;
        Reset();
    }

    bool m_bIsByOutrunner;
    uint32 m_uiPostEventCount;
    uint32 m_uiPostEventTimer;
    int m_iSpawnId;

    void Reset()
    {
        m_uiPostEventCount = 0;
        m_uiPostEventTimer = 3000;
    }

    void JustRespawned()
    {
        m_bIsByOutrunner = false;
        m_iSpawnId = 0;

        npc_escortAI::JustRespawned();
    }

    void Aggro(Unit* pWho)
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            if (pWho->GetEntry() == NPC_OUTRUNNER && !m_bIsByOutrunner)
            {
                DoScriptText(SAY_RIN_BY_OUTRUNNER, pWho);
                m_bIsByOutrunner = true;
            }

            if (urand(0, 3))
                return;

            //only if attacked and escorter is not in combat?
            DoScriptText(urand(0, 1) ? SAY_RIN_HELP_1 : SAY_RIN_HELP_2, m_creature);
        }
    }

    void DoSpawnAmbush(bool bFirst)
    {
        if (!bFirst)
            m_iSpawnId = 1;

        m_creature->SummonCreature(NPC_RANGER,
            m_afAmbushSpawn[m_iSpawnId].m_fX, m_afAmbushSpawn[m_iSpawnId].m_fY, m_afAmbushSpawn[m_iSpawnId].m_fZ, 0.0f,
            TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);

        for(int i = 0; i < 2; ++i)
        {
            m_creature->SummonCreature(NPC_OUTRUNNER,
                m_afAmbushSpawn[m_iSpawnId].m_fX, m_afAmbushSpawn[m_iSpawnId].m_fY, m_afAmbushSpawn[m_iSpawnId].m_fZ, 0.0f,
                TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->RemoveSplineFlag(SPLINEFLAG_WALKMODE);
        pSummoned->GetMotionMaster()->MovePoint(0, m_afAmbushMoveTo[m_iSpawnId].m_fX, m_afAmbushMoveTo[m_iSpawnId].m_fY, m_afAmbushMoveTo[m_iSpawnId].m_fZ);
    }

    void WaypointReached(uint32 uiPointId)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch(uiPointId)
        {
            case 1:
                DoScriptText(SAY_RIN_FREE, m_creature, pPlayer);
                break;
            case 7:
                DoSpawnAmbush(true);
                break;
            case 13:
                DoSpawnAmbush(false);
                break;
            case 17:
                DoScriptText(SAY_RIN_COMPLETE, m_creature, pPlayer);
                pPlayer->GroupEventHappens(QUEST_RINJI_TRAPPED, m_creature);
                SetRun();
                m_uiPostEventCount = 1;
                break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        //Check if we have a current target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING) && m_uiPostEventCount)
            {
                if (m_uiPostEventTimer < uiDiff)
                {
                    m_uiPostEventTimer = 3000;

                    if (Player* pPlayer = GetPlayerForEscort())
                    {
                        switch(m_uiPostEventCount)
                        {
                            case 1:
                                DoScriptText(SAY_RIN_PROGRESS_1, m_creature, pPlayer);
                                ++m_uiPostEventCount;
                                break;
                            case 2:
                                DoScriptText(SAY_RIN_PROGRESS_2, m_creature, pPlayer);
                                m_uiPostEventCount = 0;
                                break;
                        }
                    }
                    else
                    {
                        m_creature->ForcedDespawn();
                        return;
                    }
                }
                else
                    m_uiPostEventTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_rinji(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_RINJI_TRAPPED)
    {
        if (GameObject* pGo = GetClosestGameObjectWithEntry(pCreature, GO_RINJI_CAGE, INTERACTION_DISTANCE))
            pGo->UseDoorOrButton();

        if (npc_rinjiAI* pEscortAI = dynamic_cast<npc_rinjiAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

CreatureAI* GetAI_npc_rinji(Creature* pCreature)
{
    return new npc_rinjiAI(pCreature);
}

/*######
## mob_vilebranch_kidnapper
######*/

#define SPELL_EXECUTE 7160

struct MANGOS_DLL_DECL mob_vilebranch_kidnapperAI : public ScriptedAI
{
	mob_vilebranch_kidnapperAI(Creature* pCreature) : ScriptedAI(pCreature)
	{Reset();}

	uint32 m_uiExecute_Timer;

	void Reset()
	{
		m_uiExecute_Timer = 2000;
	}

	void UpdateAI(const uint32 uiDiff)
	{
		//return since we have no target
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
			return;

		//if me target have less as 21% HP, execute him
		Unit* mTarget = m_creature->getVictim();
		if (((mTarget->GetHealth() * 100) / mTarget->GetMaxHealth()) < 21)
		{
			if (m_uiExecute_Timer <= uiDiff)
			{
				DoCastSpellIfCan(m_creature->getVictim(), SPELL_EXECUTE);
				m_uiExecute_Timer = 10000;
			}
				else m_uiExecute_Timer -= uiDiff;
		}

		DoMeleeAttackIfReady();
	}
};
CreatureAI* GetAI_mob_vilebranch_kidnapper(Creature* pCreature)
{
	return new mob_vilebranch_kidnapperAI (pCreature);
}

/*######
## go_lard_picnic_basket
######*/

#define QUEST_LARD_LOST_HIS_LUNCH 7840
#define NPC_VILEBRANCH_KIDNAPPER  14748

bool GOUse_go_lard_picnic_basket(Player* pPlayer, GameObject* pGo)
{
	Creature* Raider[3];
	for(uint8 i = 0; i < 3; ++i)
		Raider[i] = NULL;

	if (pPlayer->GetQuestStatus(QUEST_LARD_LOST_HIS_LUNCH) == QUEST_STATUS_INCOMPLETE)
	{
		Raider[0] = pGo->SummonCreature(NPC_VILEBRANCH_KIDNAPPER, 412.82f, -4804.24f, 12.32f, 4.52f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
		if (Raider[0]) {Raider[0]->AI()->AttackStart(pPlayer); Raider[0]->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.7f);}
		Raider[1] = pGo->SummonCreature(NPC_VILEBRANCH_KIDNAPPER, 429.69f, -4822.37f, 14.78f, 3.40f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
		if (Raider[1]) {Raider[1]->AI()->AttackStart(pPlayer); Raider[1]->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);}
		Raider[2] = pGo->SummonCreature(NPC_VILEBRANCH_KIDNAPPER, 391.45f, -4807.61f, 10.22f, 5.45f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
		if (Raider[2]) {Raider[2]->AI()->AttackStart(pPlayer); Raider[2]->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.3f);}

		debug_log("SD2: Creatures for quest Lard Lost His Lunch summoned and start attacking players.");
	}
	return true;
}

/*####
# npc_elder_torntusk
####*/

enum
{
	TORNTUSK_SAY						= -1720086,

	QUEST_ID_RECOVER_THE_KEY			= 7846,
};

struct MANGOS_DLL_DECL npc_elder_torntuskAI : public npc_escortAI
{
    npc_elder_torntuskAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }
	
	uint8 m_uiSpeechStep;
	uint32 m_uiSpeechTimer;
	bool m_bOutro;

	ObjectGuid m_uiPlayerGUID;

    void Reset()
	{
		m_bOutro = false;
		m_uiSpeechStep = 1;
		m_uiSpeechTimer = 0;
		m_uiPlayerGUID.Clear();
	}

	void WaypointReached(uint32 /*uiPointId*/)
    {
	}

	void JustStartedEscort()
    {
    }

	void StartOutro(ObjectGuid pPlayerGUID)
	{
		if (!pPlayerGUID)
            return;

        m_uiPlayerGUID = pPlayerGUID;

		m_bOutro = true; 
		m_uiSpeechTimer = 1000;
		m_uiSpeechStep = 1;
		m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP + UNIT_NPC_FLAG_QUESTGIVER);
		m_creature->SetStandState(UNIT_STAND_STATE_STAND);
	}

	void UpdateAI(const uint32 uiDiff)
    {
		npc_escortAI::UpdateAI(uiDiff);

		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
	}

	void UpdateEscortAI(const uint32 uiDiff)
    {
		if (m_uiSpeechTimer && m_bOutro)							// handle RP at quest end
		{
			if (!m_uiSpeechStep)
				return;
		
			if (m_uiSpeechTimer <= uiDiff)
            {			
                switch(m_uiSpeechStep)
                {
					case 1:
						m_creature->HandleEmote(EMOTE_ONESHOT_ROAR);
						m_uiSpeechTimer = 3000;
						break;
					case 2:
						DoScriptText(TORNTUSK_SAY, m_creature);
                        m_uiSpeechTimer = 3000;
						break;
					case 3:
						m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP + UNIT_NPC_FLAG_QUESTGIVER);
						m_uiSpeechTimer = 10000;
						break;
					case 4:
						m_creature->ForcedDespawn();
						m_creature->SetRespawnTime(5);						
						m_bOutro = false;
						break;
                    /*default:
                        m_uiSpeechStep = 0;
                        return;*/
                }
                ++m_uiSpeechStep;
            }
            else
                m_uiSpeechTimer -= uiDiff;
		}
	}
};

CreatureAI* GetAI_npc_elder_torntusk(Creature* pCreature)
{
    return new npc_elder_torntuskAI(pCreature);
}

bool OnQuestRewarded_npc_elder_torntusk(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
	if (pQuest->GetQuestId() == QUEST_ID_RECOVER_THE_KEY)
    {
		if (npc_elder_torntuskAI* pEscortAI = dynamic_cast<npc_elder_torntuskAI*>(pCreature->AI()))
			pEscortAI->StartOutro(pPlayer->GetObjectGuid());
	}
	return true;
}

/* AddSC */

void AddSC_hinterlands()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "npc_00x09hl";
    pNewscript->GetAI = &GetAI_npc_00x09hl;
    pNewscript->pQuestAcceptNPC = &QuestAccept_npc_00x09hl;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_rinji";
    pNewscript->GetAI = &GetAI_npc_rinji;
    pNewscript->pQuestAcceptNPC = &QuestAccept_npc_rinji;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "mob_vilebranch_kidnapper";
    pNewscript->GetAI = &GetAI_mob_vilebranch_kidnapper;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "go_lard_picnic_basket";
    pNewscript->pGOUse = &GOUse_go_lard_picnic_basket;
    pNewscript->RegisterSelf();

	pNewscript = new Script;
    pNewscript->Name = "npc_elder_torntusk";
    pNewscript->GetAI = &GetAI_npc_elder_torntusk;
    pNewscript->pQuestRewardedNPC = &OnQuestRewarded_npc_elder_torntusk;
    pNewscript->RegisterSelf();
}
