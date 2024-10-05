#include "settings.h"
#pragma warning(disable: 4100)
#include <chrono>
#include <iostream>
#include <windows.h>

// using Clock = std::chrono::steady_clock;
// std::chrono::time_point<std::chrono::steady_clock> start, now;
// std::chrono::milliseconds                          duration;

//#define PI 3.1415926535f
using writeLock = std::unique_lock<std::shared_mutex>;
using readLock = std::shared_lock<std::shared_mutex>;


void dodge::init() {
	_precision_API = reinterpret_cast<PRECISION_API::IVPrecision1*>(PRECISION_API::RequestPluginAPI());
	if (_precision_API) {
		_precision_API->AddPreHitCallback(SKSE::GetPluginHandle(), DodgeCallback_PreHit);
		logger::info("Enabled compatibility with Precision");
	}
}

void interruptattack(RE::Actor *me)
{
	me->NotifyAnimationGraph("attackStop");
	me->NotifyAnimationGraph("bashStop");
	me->NotifyAnimationGraph("blockStop");
	me->NotifyAnimationGraph("staggerStop");
	me->NotifyAnimationGraph("recoilStop");
}

//Native Functions for Papyrus
float dodge::GetProtaganist_ReflexScore(RE::Actor* a_actor){
	float Score = 0.0f;

	return Score;
}

bool dodge::BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm)
{
	vm->RegisterFunction("GetProtaganist_ReflexScore", "_SM_UND_NativeFunctions", GetProtaganist_ReflexScore);
	// vm->RegisterFunction("Protagnist_can_dodge", "_SM_UND_NativeFunctions", Protagnist_can_dodge);
	return true;
}

PRECISION_API::PreHitCallbackReturn dodge::DodgeCallback_PreHit(const PRECISION_API::PrecisionHitData& a_precisionHitData)
{
	PRECISION_API::PreHitCallbackReturn returnData;
	if (!a_precisionHitData.target || !a_precisionHitData.target->Is(RE::FormType::ActorCharacter)) {
		return returnData;
	}

	auto actor = a_precisionHitData.target->As<RE::Actor>();

	if (actor->IsPlayerRef()) {
		return returnData;
	}

	if (!Utils::Actor::isHumanoid(actor)) {
		return returnData;
	}

	if (!ValhallaUtils::is_adversary(actor, a_precisionHitData.attacker)) {
		return returnData;
	}

	bool bIsDodging = false;

	if ((actor)
			->GetGraphVariableBool("bIsDodging", bIsDodging) &&
		bIsDodging) {
		returnData.bIgnoreHit = true;
	}

	return returnData;
}

/*Get the dodge chance of a reactive dodger in case of an incoming attack.*/
float dodge::get_dodge_chance(RE::Actor* a_actor, const Armour_factors& Armour, const PReflex_factors& Protagnist_Reflexes, const CStyle_factors& CStyle)
{
	float Score = 0.0f;

	/////////////////////////////////////////////////Armour Weighting////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	auto Helm = a_actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHair);

	auto Chest = a_actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);

	auto Gauntlet = a_actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHands);

	auto Boots = a_actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet);

	auto Shield = a_actor->GetEquippedObject(true);

	if (Helm) {
		switch (Helm->GetArmorType()) {
		case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
			Score += Armour.Helm_weight * Armour.Heavyarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kLightArmor:
			Score += Armour.Helm_weight * Armour.Lightarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kClothing:
			Score += Armour.Helm_weight * Armour.clothing_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		default:
			Score += Armour.Helm_weight * Protagnist_Reflexes.Armour_Weighting;
		}
	} else {
		Score += Armour.Helm_weight * Protagnist_Reflexes.Armour_Weighting;
	}

	if (Chest) {
		switch (Chest->GetArmorType()) {
		case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
			Score += Armour.Chest_weight * Armour.Heavyarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kLightArmor:
			Score += Armour.Chest_weight * Armour.Lightarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kClothing:
			Score += Armour.Chest_weight * Armour.clothing_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		default:
			Score += Armour.Chest_weight * Protagnist_Reflexes.Armour_Weighting;
		}
	} else {
		Score += Armour.Chest_weight * Protagnist_Reflexes.Armour_Weighting;
	}

	if (Gauntlet) {
		switch (Gauntlet->GetArmorType()) {
		case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
			Score += Armour.Gauntlet_weight * Armour.Heavyarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kLightArmor:
			Score += Armour.Gauntlet_weight * Armour.Lightarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kClothing:
			Score += Armour.Gauntlet_weight * Armour.clothing_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		default:
			Score += Armour.Gauntlet_weight * Protagnist_Reflexes.Armour_Weighting;
		}
	} else {
		Score += Armour.Gauntlet_weight * Protagnist_Reflexes.Armour_Weighting;
	}

	if (Boots) {
		switch (Boots->GetArmorType()) {
		case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
			Score += Armour.Boots_weight * Armour.Heavyarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kLightArmor:
			Score += Armour.Boots_weight * Armour.Lightarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kClothing:
			Score += Armour.Boots_weight * Armour.clothing_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		default:
			Score += Armour.Boots_weight * Protagnist_Reflexes.Armour_Weighting;
		}
	} else {
		Score += Armour.Boots_weight * Protagnist_Reflexes.Armour_Weighting;
	}

	if (Shield && Shield->IsArmor()) {
		// if (Shield->HasKeywordByEditorID())
		switch (Shield->As<RE::TESObjectARMO>()->GetArmorType()) {  //function tests for biped model; need some king of flag or keyword instead for sheilds, else crash
		case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
			Score += Armour.Shield_weight * Armour.Heavyarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kLightArmor:
			Score += Armour.Shield_weight * Armour.Lightarm_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		case RE::BIPED_MODEL::ArmorType::kClothing:
			Score += Armour.Shield_weight * Armour.clothing_mult * Protagnist_Reflexes.Armour_Weighting;
			break;
		default:
			Score += Armour.Shield_weight * Protagnist_Reflexes.Armour_Weighting;
			break;
		}
	} else {
		Score += Armour.Shield_weight * Protagnist_Reflexes.Armour_Weighting;
	}

	/////////////////////////////////////////////////Defensive & Skirmish Weighting ///////////////////////////////////////////////////////////////////////////////////////////

	if (a_actor->GetActorRuntimeData().combatController) {
		RE::TESCombatStyle* style = a_actor->GetActorRuntimeData().combatController->combatStyle;
		if (style) {
			Score += style->generalData.defensiveMult * Protagnist_Reflexes.Defensive_Weighting;

			Score += style->generalData.avoidThreatChance * CStyle.Skirmish_AvoidThreat_Weighting * Protagnist_Reflexes.Skirmish_Weighting;
			Score += style->closeRangeData.circleMult * CStyle.Skirmish_Circle_Weighting * Protagnist_Reflexes.Skirmish_Weighting;
			Score += style->closeRangeData.fallbackMult * CStyle.Skirmish_Fallback_Weighting * Protagnist_Reflexes.Skirmish_Weighting;
			Score += style->longRangeData.strafeMult * CStyle.Skirmish_Strafe_Weighting * Protagnist_Reflexes.Skirmish_Weighting;
		}
	}

	/////////////////////////////////////////////////Sneak Skill Weighting /////////////////////////////////////////////////////////////////////////////////////////////////////////

	Score += (a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSneak) / 100.0f) * Protagnist_Reflexes.Sneak_Weighting;

	return Score;
}

float dodge::get_stamina_basecost(RE::Actor* a_actor, const Stamina_factors& Stamina, bool DodgeRoll)
{
	float A_Score = Stamina.fSideStep_staminacost;

	if (settings::bStaminaCost_ArmourBasedSystem_enable){
		auto Helm = a_actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHair);

		auto Chest = a_actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);

		auto Gauntlet = a_actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHands);

		auto Boots = a_actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet);

		auto Shield = a_actor->GetEquippedObject(true);

		auto Heavy_Skill = (a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHeavyArmor) / 100.0f);

		auto Light_skill = (a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLightArmor) / 100.0f);

		if (Helm) {
			switch (Helm->GetArmorType()) {
			case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
				A_Score += (Helm->GetWeight() * Stamina.Stamina_Helm_weight * Stamina.Stamina_HeavyArmour_mult) / Heavy_Skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kLightArmor:
				A_Score += (Helm->GetWeight() * Stamina.Stamina_Helm_weight * Stamina.Stamina_LightArmour_mult) / Light_skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kClothing:
				A_Score += (Helm->GetWeight() * Stamina.Stamina_Helm_weight * Stamina.Stamina_Clothing_mult);
				break;
			default:
				A_Score += (Helm->GetWeight() * Stamina.Stamina_Helm_weight * Stamina.Stamina_Clothing_mult);
			}
		}

		if (Chest) {
			switch (Chest->GetArmorType()) {
			case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
				A_Score += (Chest->GetWeight() * Stamina.Stamina_Chest_weight * Stamina.Stamina_HeavyArmour_mult) / Heavy_Skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kLightArmor:
				A_Score += (Chest->GetWeight() * Stamina.Stamina_Chest_weight * Stamina.Stamina_LightArmour_mult) / Light_skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kClothing:
				A_Score += (Chest->GetWeight() * Stamina.Stamina_Chest_weight * Stamina.Stamina_Clothing_mult);
				break;
			default:
				A_Score += (Chest->GetWeight() * Stamina.Stamina_Chest_weight * Stamina.Stamina_Clothing_mult);
			}
		}

		if (Gauntlet) {
			switch (Gauntlet->GetArmorType()) {
			case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
				A_Score += (Gauntlet->GetWeight() * Stamina.Stamina_Gauntlet_weight * Stamina.Stamina_HeavyArmour_mult) / Heavy_Skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kLightArmor:
				A_Score += (Gauntlet->GetWeight() * Stamina.Stamina_Gauntlet_weight * Stamina.Stamina_LightArmour_mult) / Light_skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kClothing:
				A_Score += (Gauntlet->GetWeight() * Stamina.Stamina_Gauntlet_weight * Stamina.Stamina_Clothing_mult);
				break;
			default:
				A_Score += (Gauntlet->GetWeight() * Stamina.Stamina_Gauntlet_weight * Stamina.Stamina_Clothing_mult);
			}
		}

		if (Boots) {
			switch (Boots->GetArmorType()) {
			case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
				A_Score += (Boots->GetWeight() * Stamina.Stamina_Boots_weight * Stamina.Stamina_HeavyArmour_mult) / Heavy_Skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kLightArmor:
				A_Score += (Boots->GetWeight() * Stamina.Stamina_Boots_weight * Stamina.Stamina_LightArmour_mult) / Light_skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kClothing:
				A_Score += (Boots->GetWeight() * Stamina.Stamina_Boots_weight * Stamina.Stamina_Clothing_mult);
				break;
			default:
				A_Score += (Boots->GetWeight() * Stamina.Stamina_Boots_weight * Stamina.Stamina_Clothing_mult);
			}
		}

		if (Shield && Shield->IsArmor()) {
			// if (Shield->HasKeywordByEditorID())
			switch (Shield->As<RE::TESObjectARMO>()->GetArmorType()) {  //function tests for biped model; need some king of flag or keyword instead for sheilds, else crash
			case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
				A_Score += (Shield->GetWeight() * Stamina.Stamina_Shield_weight * Stamina.Stamina_HeavyArmour_mult) / Heavy_Skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kLightArmor:
				A_Score += (Shield->GetWeight() * Stamina.Stamina_Shield_weight * Stamina.Stamina_LightArmour_mult) / Light_skill;
				break;
			case RE::BIPED_MODEL::ArmorType::kClothing:
				A_Score += (Shield->GetWeight() * Stamina.Stamina_Shield_weight * Stamina.Stamina_Clothing_mult);
				break;
			default:
				A_Score += (Shield->GetWeight() * Stamina.Stamina_Shield_weight * Stamina.Stamina_Clothing_mult);
			}
		}
	}

	
	if (DodgeRoll){
		if (Stamina.fSideStep_staminacost == 0.0) {
			return A_Score * Stamina.fSideStep_staminacost;

		} else {
			return A_Score * Stamina.fDodgeRoll_staminacost;
		}

	}else {
		if(Stamina.fSideStep_staminacost == 0.0) {
			return A_Score * Stamina.fSideStep_staminacost;

		} else {
			return A_Score;
		}
	}
}

std::vector<RE::TESForm*> dodge::GetEquippedForm(RE::Actor* actor)
{
	std::vector<RE::TESForm*> Hen;

	if (actor->GetEquippedObject(true)){
		Hen.push_back(actor->GetEquippedObject(true));
	}
	if (actor->GetEquippedObject(false)){
		Hen.push_back(actor->GetEquippedObject(false));
	}

	return Hen;
}

bool dodge::GetEquippedType_IsMelee(RE::Actor* actor)
{
	bool result = false;
	auto form_list = GetEquippedForm(actor);

	if (!form_list.empty()){
		for (auto form : form_list) {
			if (form){
				switch (*form->formType) {
				case RE::FormType::Weapon:
					if (const auto equippedWeapon = form->As<RE::TESObjectWEAP>()) {
						switch (equippedWeapon->GetWeaponType()) {
						case RE::WEAPON_TYPE::kHandToHandMelee:
						case RE::WEAPON_TYPE::kOneHandSword:
						case RE::WEAPON_TYPE::kOneHandDagger:
						case RE::WEAPON_TYPE::kOneHandAxe:
						case RE::WEAPON_TYPE::kOneHandMace:
						case RE::WEAPON_TYPE::kTwoHandSword:
						case RE::WEAPON_TYPE::kTwoHandAxe:
							result = true;
							break;
						default:
							break;
						}
					}
					break;
				case RE::FormType::Armor:
					if (auto equippedShield = form->As<RE::TESObjectARMO>()) {
						result = true;
					}
					break;
				default:
					break;
				}
				if (result) {
					break;
				}
			}
			continue;
		}
	}
	return result;
}

bool dodge::is_melee(RE::Actor* actor){
	return GetEquippedType_IsMelee(actor);
}

bool dodge::IsMeleeOnly(RE::Actor* a_actor)
{
	using TYPE = RE::CombatInventoryItem::TYPE;

	auto result = true;
		
	auto combatCtrl = a_actor->GetActorRuntimeData().combatController;
	auto CombatInv = combatCtrl ? combatCtrl->inventory : nullptr;
	if (CombatInv) {
		for (const auto item : CombatInv->equippedItems) {
			if (item.item) {
				switch (item.item->GetType()) {
				case TYPE::kMagic:
				case TYPE::kRanged:
				case TYPE::kScroll:
				case TYPE::kStaff:

					result = false;
					break;

				default:
					break;
				}
			}
		}
	}

	return result;
}

float dodge::confidence_threshold(RE::Actor* a_actor)
{
	float result = 0.0f;

	switch (static_cast<int>(a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kConfidence)))
	{
	case 0:
		result = 1.25f;
		break;

	case 1:
		result = 1.0f;
		break;

	case 2:
		result = 0.75f;
		break;

	case 3:
		result = 0.5f;
		break;

	case 4:
		result = 0.25f;
		break;

	default:
		break;
	}

	return result;
	
}

std::pair<float, float> dodge::Get_ReactiveDodge_Distance(RE::Actor* actor)
{
	float distance = 200.0f;
	float attack_speed = 0.0f;
	bool IsLeftAttack = false;
	
	auto aiProcess = actor->GetActorRuntimeData().currentProcess;

	if (aiProcess && aiProcess->high && aiProcess->high->attackData) {
		// const RE::TESForm* 
		auto equippedLi = aiProcess->high->attackData.get();
		if (equippedLi) {
			IsLeftAttack = equippedLi->IsLeftAttack();
			const RE::TESForm* equipped = IsLeftAttack ? aiProcess->GetEquippedLeftHand() : aiProcess->GetEquippedRightHand();
			if (equipped && equipped->IsWeapon()) {

				attack_speed = dodge::GetSingleton()->Get_Attack_Speed(actor, equipped->As<RE::TESObjectWEAP>(), IsLeftAttack);

				switch (equipped->As<RE::TESObjectWEAP>()->GetWeaponType()) {
				case RE::WEAPON_TYPE::kOneHandSword:
					distance = 310.0f;
					break;
				case RE::WEAPON_TYPE::kOneHandAxe:
					distance = 305.0f;
					break;
				case RE::WEAPON_TYPE::kOneHandMace:
					distance = 300.0f;
					break;
				case RE::WEAPON_TYPE::kOneHandDagger:
					distance = 250.0f;
					break;
				case RE::WEAPON_TYPE::kTwoHandAxe:
					distance = 350.0f;
					break;
				case RE::WEAPON_TYPE::kTwoHandSword:
					distance = 370.0f;
					break;
				case RE::WEAPON_TYPE::kHandToHandMelee:
					if (!Utils::Actor::isHumanoid(actor)) {
						distance = 350.0f;
					} else {
						distance = 150.0f;
					}
					break;
				case RE::WEAPON_TYPE::kBow:
					distance = 3000.0f;
					break;
				case RE::WEAPON_TYPE::kCrossbow:
					distance = 3000.0f;
					break;
				case RE::WEAPON_TYPE::kStaff:
					distance = 320.0f;
					break;
				default:
					distance = 150.0f;
					break;
				}

			} else if (equipped && equipped->IsArmor()) {
				distance = 250.0f;
				attack_speed = (actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeedMult)/100.0f);

			} else {
				if (!Utils::Actor::isHumanoid(actor)) {
					distance = 350.0f;
				} else {
					distance = 150.0f;
				}
			}
		} else {
			if (!Utils::Actor::isHumanoid(actor)) {
				distance = 350.0f;
			} else {
				distance = 150.0f;
			}
		}
		
	}else {
		if (!Utils::Actor::isHumanoid(actor)) {
			distance = 350.0f;
		} else {
			distance = 200.0f;
		}
	}

	return { distance, attack_speed };
}

void dodge::retreive_execute_attacks(RE::Actor *a_actor, bool melee, bool melee_normal, bool ranged, bool bash, bool bash_sprint){
	auto it = dodge::GetSingleton()->Get_ReactiveDodge_Distance(a_actor);
	if (melee){
		dodge::GetSingleton()->react_to_melee(a_actor, it.first, it.second);
	}
	if (melee_normal) {
		dodge::GetSingleton()->react_to_melee_normal(a_actor, it.first, it.second);
	}
	if (ranged) {
		dodge::GetSingleton()->react_to_ranged(a_actor, it.first, it.second);
	}
	if (bash) {
		dodge::GetSingleton()->react_to_bash(a_actor, it.first);
	}
	if (bash_sprint) {
		dodge::GetSingleton()->react_to_bash_sprint(a_actor, it.first, it.second);
	}
}

float dodge::Get_ReactiveDodge_Reach(RE::Actor* actor)
{
	float reach = Actor_GetReach(actor);
	return reach;
}

float dodge::Get_Attack_Speed(RE::Actor* actor, const RE::TESObjectWEAP* a_weapon, bool IsLeftAttack)
{
	float total_speed = 1.0f;
	if (IsLeftAttack){
		float av_speed = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftWeaponSpeedMultiply);
		if (av_speed == 0.0f){
			av_speed = 1.0f;
		}
		float weapon_speed = a_weapon->GetSpeed();

		total_speed = av_speed * weapon_speed;

	}else{

		float av_speed = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kWeaponSpeedMult);
		if (av_speed == 0.0f) {
			av_speed = 1.0f;
		}
		float weapon_speed = a_weapon->GetSpeed();

		total_speed = av_speed * weapon_speed;
	}

	return total_speed;
}

bool dodge::GetAttackSpell(RE::Actor* actor, bool lefthand) {

	bool result = false;
	auto limbospell = actor->GetActorRuntimeData().currentProcess;
	static auto fireKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageFire");
	static auto frostKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageFrost");
	static auto ShockKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageShock");

	if (limbospell) {
		if (lefthand){
			auto eSpell = limbospell->GetEquippedLeftHand();
			if (eSpell && eSpell->Is(RE::FormType::Spell)) {
				auto rSpell = eSpell->As<RE::SpellItem>();
				if (!(rSpell->GetSpellType() == RE::MagicSystem::SpellType::kPower || rSpell->GetSpellType() == RE::MagicSystem::SpellType::kLesserPower)){
					auto Effect_List = rSpell->effects;
					for (auto Effect : Effect_List) {
						if (Effect && Effect->baseEffect) {
							if (Effect->baseEffect->data.flags.all(RE::EffectSetting::EffectSettingData::Flag::kHostile)) {
								result = true;
								break;
							} else if (Effect->baseEffect->HasKeyword(fireKeyword) || Effect->baseEffect->HasKeyword(frostKeyword) || Effect->baseEffect->HasKeyword(ShockKeyword)) {
								result = true;
								break;
							}
						}
						continue;
					}
				}	
			}
		}else {
			auto eSpell = limbospell->GetEquippedRightHand();
			if (eSpell && eSpell->Is(RE::FormType::Spell)) {
				auto rSpell = eSpell->As<RE::SpellItem>();
				if (!(rSpell->GetSpellType() == RE::MagicSystem::SpellType::kPower || rSpell->GetSpellType() == RE::MagicSystem::SpellType::kLesserPower)) {
					auto Effect_List = rSpell->effects;
					for (auto Effect : Effect_List) {
						if (Effect && Effect->baseEffect) {
							if (Effect->baseEffect->data.flags.all(RE::EffectSetting::EffectSettingData::Flag::kHostile)) {
								result = true;
								break;
							} else if (Effect->baseEffect->HasKeyword(fireKeyword) || Effect->baseEffect->HasKeyword(frostKeyword) || Effect->baseEffect->HasKeyword(ShockKeyword)) {
								result = true;
								break;
							}
						}
						continue;
					}
				}
			}
		}
	}
	return result;
}

std::pair<bool, float> dodge::GetAttackSpell_Alt(RE::SpellItem* a_spell)
{
	bool result = false;
	float time = 0.0f;
	static auto fireKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageFire");
	static auto frostKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageFrost");
	static auto ShockKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageShock");
	try
	{
		auto Effect_List = a_spell->effects;
		for (auto Effect : Effect_List) {
			if (Effect && Effect->baseEffect) {
				if (Effect->baseEffect->data.flags.all(RE::EffectSetting::EffectSettingData::Flag::kHostile)) {
					result = true;
					break;
				} else if (Effect->baseEffect->HasKeyword(fireKeyword) || Effect->baseEffect->HasKeyword(frostKeyword) || Effect->baseEffect->HasKeyword(ShockKeyword)) {
					result = true;
					break;
				}
			}
			continue;
		}
		for (auto Effect : Effect_List) {
			if (Effect && Effect->baseEffect) {
				if (Effect->baseEffect->data.projectileBase) {
					float speed = Effect->baseEffect->data.projectileBase->data.speed;
					if (speed && speed != 0.0f) {
						time = speed;
						//3000.0f / (speed * 2.0f);
						//logger::info("Name {} projectilespeed{}"sv, "Actor", speed);
						break;
					}
				}
			}
			continue;
		}
	}
	catch(...)
	{
		return { result, time };
	}
	return { result, time };
}

bool dodge::GetEquippedShout(RE::Actor* actor){
	bool result = false;
	auto limboshout = actor->GetActorRuntimeData().selectedPower;
	auto currentVar = actor->GetActorRuntimeData().currentProcess->high->currentShoutVariation;
	static auto fireKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageFire");
	static auto frostKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageFrost");
	static auto ShockKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("MagicDamageShock");

	if (limboshout && limboshout->Is(RE::FormType::Shout) && currentVar){
		try
		{
			if (limboshout->As<RE::TESShout>()->variations && limboshout->As<RE::TESShout>()->variations[currentVar].spell) {
				auto eSpell = limboshout->As<RE::TESShout>()->variations[currentVar].spell;
				if (!(eSpell->GetSpellType() == RE::MagicSystem::SpellType::kPower || eSpell->GetSpellType() == RE::MagicSystem::SpellType::kLesserPower)){
					auto Effect_List = eSpell->effects;
					for (auto Effect : Effect_List) {
						if (Effect && Effect->baseEffect) {
							if (Effect->baseEffect->data.flags.all(RE::EffectSetting::EffectSettingData::Flag::kHostile)) {
								result = true;
								break;
							} else if (Effect->baseEffect->HasKeyword(fireKeyword) || Effect->baseEffect->HasKeyword(frostKeyword) || Effect->baseEffect->HasKeyword(ShockKeyword)) {
								result = true;
								break;
							}
						}
						continue;
					}
				}
			}
		}
		catch(...)
		{
			return result;
		}
		
	}
	return result;
}

float dodge::GetShoutRange_Reaction(RE::Actor* actor, float distance){
	float result = 0;
	auto limboshout = actor->GetActorRuntimeData().selectedPower;
	auto currentVar = actor->GetActorRuntimeData().currentProcess->high->currentShoutVariation;

	if (limboshout && limboshout->Is(RE::FormType::Shout) && currentVar){
		if (limboshout->As<RE::TESShout>()->variations && limboshout->As<RE::TESShout>()->variations[currentVar].spell) {
			auto eSpell = limboshout->As<RE::TESShout>()->variations[currentVar].spell;
			if (eSpell->GetDelivery() == RE::MagicSystem::Delivery::kAimed) {
				auto Effect_List = eSpell->effects;
				for (auto Effect : Effect_List) {
					if (Effect && Effect->baseEffect) {
						if (Effect->baseEffect->data.projectileBase) {
							float speed = Effect->baseEffect->data.projectileBase->data.speed;
							if (speed && speed != 0.0) {
								result = distance / speed;
								break;
							}
						}
					}
					continue;
				}
			}
		}
	}
	return result;
}

float dodge::GetSpellRange_Reaction(RE::Actor* actor, float distance, bool lefthand){
	float result = 0;
	auto limbospell = actor->GetActorRuntimeData().currentProcess;

	if (limbospell){
		if (lefthand) {
			auto eSpell = limbospell->GetEquippedLeftHand();
			if (eSpell && eSpell->Is(RE::FormType::Spell)){
				if (eSpell->As<RE::SpellItem>()->GetDelivery() == RE::MagicSystem::Delivery::kAimed) {
					auto Effect_List = eSpell->As<RE::SpellItem>()->effects;
					for (auto Effect : Effect_List) {
						if (Effect && Effect->baseEffect) {
							if (Effect->baseEffect->data.projectileBase) {
								float speed = Effect->baseEffect->data.projectileBase->data.speed;
								if (speed && speed != 0.0) {
									result = distance / speed;
									break;
								}
							}
						}
						continue;
					}
				}
			}
		} else {
			auto eSpell = limbospell->GetEquippedRightHand();
			if (eSpell && eSpell->Is(RE::FormType::Spell)) {
				if (eSpell->As<RE::SpellItem>()->GetDelivery() == RE::MagicSystem::Delivery::kAimed) {
					auto Effect_List = eSpell->As<RE::SpellItem>()->effects;
					for (auto Effect : Effect_List) {
						if (Effect && Effect->baseEffect) {
							if (Effect->baseEffect->data.projectileBase) {
								float speed = Effect->baseEffect->data.projectileBase->data.speed;
								if (speed && speed != 0.0) {
									result = distance / speed;
									break;
								}
							}
						}
						continue;
					}
				}
			}
		}
	}
	return result;
}

bool dodge::is_adequate_threat(RE::Actor* protagonist, RE::Actor* attacker)
{
	auto adequate_threat = false;
	float protagonist_threat = 0.0f;
	float attacker_threat = 0.0f;

	auto combatGroup = protagonist->GetCombatGroup();
	if (combatGroup) {
		for (auto it = combatGroup->members.begin(); it != combatGroup->members.end(); ++it) {
			if (it->memberHandle && it->memberHandle.get().get() && it->memberHandle.get().get() == protagonist) {
				protagonist_threat += it->threatValue;
				break;
			}
			continue;
		}
	}
	auto EnemyGroup = attacker->GetCombatGroup();
	if (EnemyGroup) {
		for (auto it = EnemyGroup->members.begin(); it != EnemyGroup->members.end(); ++it) {
			if (it->memberHandle && it->memberHandle.get().get() && it->memberHandle.get().get() == attacker) {
				attacker_threat += it->threatValue;
				break;
			}
			continue;
		}
	}

	if (protagonist_threat > 0 && attacker_threat > 0) {
		if (settings::bThreatlogging_enable) {
			logger::info("Name {} RSS_foe_threat {}"sv, protagonist->GetName(), (protagonist_threat / attacker_threat));
		}
		if ((protagonist_threat / attacker_threat) < dodge::GetSingleton()->confidence_threshold(protagonist)) {
			adequate_threat = true;
		}
	}

	return adequate_threat;
}


void dodge::Set_iFrames(RE::Actor* actor){
	actor->SetGraphVariableBool("bIframeActive", true);
	actor->SetGraphVariableBool("bInIframe", true);
}

void dodge::Reset_iFrames(RE::Actor* actor){
	actor->SetGraphVariableBool("bIframeActive", false);
	actor->SetGraphVariableBool("bInIframe", false);
}

void dodge::send_UNDdodge__event(RE::Actor* a_actor)
{
	SKSE::ModCallbackEvent modEvent{
		RE::BSFixedString("UND_DodgeEvent"),
		RE::BSFixedString(),
		0.0f,
		a_actor
	};

	SKSE::GetModCallbackEventSource()->SendEvent(&modEvent);
	if (settings::bCombatlogging_enable) {
		logger::info("Sent UND dodge event");
	}
}

bool dodge::getrace_VLserana(RE::Actor* a_actor)
{
	bool result = false;
	const auto race = a_actor->GetRace();
	const auto raceEDID = race->formEditorID;
	if (raceEDID == "DLC1VampireBeastRace") {
		if (a_actor->HasKeywordString("VLS_Serana_Key") || a_actor->HasKeywordString("VLS_Valerica_Key")) {
			result = true;
		}
	}
	return result;
}

/*Trigger reactive AI surrounding the attacker.*/
void dodge::react_to_melee(RE::Actor* a_attacker, float attack_range, float attack_speed)
{
	if (!settings::bDodgeAI_Reactive_enable) {
		return;
	}
	auto combatGroup = a_attacker->GetCombatGroup();
	if (combatGroup) {
		for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
			if (it->targetHandle && it->targetHandle.get().get()) {
				RE::Actor* refr = it->targetHandle.get().get();
				if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) <= attack_range) {
					
					if (refr->IsPlayerRef() || refr->IsDead() || !refr->IsInCombat()) {
						continue;
					}
					if (!Utils::Actor::isHumanoid(refr)) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (!(refr->HasKeywordString("ActorTypeNPC") || refr->HasKeywordString("DLC2ActorTypeMiraak"))) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (refr->HasKeywordString("UND_ExcludeDodge")) {
						continue;
					}
					auto CTarget = refr->GetActorRuntimeData().currentCombatTarget.get().get();
					if (!CTarget) {
						continue;
					}
					// if (!ValhallaUtils::is_adversary(refr, a_attacker)) {
					// 	continue;
					// }

					RE::BGSAttackData* attackdata = Utils::get_attackData(a_attacker);
					auto angle = Utils::get_angle_he_me(refr, a_attacker, attackdata);

					float attackAngle = attackdata ? attackdata->data.strikeAngle : 35.0f;

					if (abs(angle) > attackAngle) {
						continue;
					}

					switch (settings::iDodgeAI_Framework) {
					case 0:
						dodge::GetSingleton()->attempt_dodge(refr, &dodge_directions_tk_reactive, attack_speed);
						break;
					case 1:
						// dodge::GetSingleton()->attempt_dodge(refr, &dodge_directions_dmco_all);
						break;
					}
				}
				continue;
			}
		}
	}
}

void dodge::react_to_melee_power(RE::Actor* a_attacker, float attack_range)
{
	if (!settings::bDodgeAI_Reactive_enable) {
		return;
	}
	auto combatGroup = a_attacker->GetCombatGroup();
	if (combatGroup) {
		for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
			if (it->targetHandle && it->targetHandle.get().get()) {
				RE::Actor* refr = it->targetHandle.get().get();
				if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) <= attack_range) {
					
					if (refr->IsPlayerRef() || refr->IsDead() || !refr->IsInCombat()) {
						continue;
					}
					if (!Utils::Actor::isHumanoid(refr)) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (!(refr->HasKeywordString("ActorTypeNPC") || refr->HasKeywordString("DLC2ActorTypeMiraak"))) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (refr->HasKeywordString("UND_ExcludeDodge")) {
						continue;
					}
					auto CTarget = refr->GetActorRuntimeData().currentCombatTarget.get().get();
					if (!CTarget) {
						continue;
					}
					// if (!ValhallaUtils::is_adversary(refr, a_attacker)) {
					// 	continue;
					// }

					RE::BGSAttackData* attackdata = Utils::get_attackData(a_attacker);
					auto angle = Utils::get_angle_he_me(refr, a_attacker, attackdata);

					float attackAngle = attackdata ? attackdata->data.strikeAngle : 35.0f;

					if (abs(angle) > attackAngle) {
						continue;
					}

					switch (settings::iDodgeAI_Framework) {
					case 0:
						dodge::GetSingleton()->Powerattack_attempt_dodge(refr, &dodge_directions_tk_reactive);
						break;
					case 1:
						// dodge::GetSingleton()->attempt_dodge(refr, &dodge_directions_dmco_all);
						break;
					}
				}
				continue;
			}
		}
	}
}

void dodge::react_to_melee_normal(RE::Actor* a_attacker, float attack_range, float attack_speed)
{
	if (!settings::bDodgeAI_Reactive_enable) {
		return;
	}
	auto combatGroup = a_attacker->GetCombatGroup();
	if (combatGroup) {
		for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
			if (it->targetHandle && it->targetHandle.get().get()) {
				RE::Actor* refr = it->targetHandle.get().get();
				if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) <= attack_range) {
					
					if (refr->IsPlayerRef() || refr->IsDead() || !refr->IsInCombat()) {
						continue;
					}
					if (!Utils::Actor::isHumanoid(refr)) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (!(refr->HasKeywordString("ActorTypeNPC") || refr->HasKeywordString("DLC2ActorTypeMiraak"))) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (refr->HasKeywordString("UND_ExcludeDodge")) {
						continue;
					}
					auto CTarget = refr->GetActorRuntimeData().currentCombatTarget.get().get();
					if (!CTarget) {
						continue;
					}
					// if (!ValhallaUtils::is_adversary(refr, a_attacker)) {
					// 	continue;
					// }

					RE::BGSAttackData* attackdata = Utils::get_attackData(a_attacker);
					auto angle = Utils::get_angle_he_me(refr, a_attacker, attackdata);

					float attackAngle = attackdata ? attackdata->data.strikeAngle : 35.0f;

					if (abs(angle) > attackAngle) {
						continue;
					}

					switch (settings::iDodgeAI_Framework) {
					case 0:
						dodge::GetSingleton()->NormalAttack_attempt_dodge(refr, &dodge_directions_tk_reactive, attack_speed);
						break;
					case 1:
						// dodge::GetSingleton()->attempt_dodge(refr, &dodge_directions_dmco_all);
						break;
					}
				}
				continue;
			}
		}
	}
}

void dodge::react_to_bash(RE::Actor* a_attacker, float attack_range)
{
	if (!settings::bDodgeAI_Reactive_enable) {
		return;
	}
	auto combatGroup = a_attacker->GetCombatGroup();
	if (combatGroup) {
		for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
			if (it->targetHandle && it->targetHandle.get().get()) {
				RE::Actor* refr = it->targetHandle.get().get();
				if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) <= attack_range) {
					
					if (refr->IsPlayerRef() || refr->IsDead() || !refr->IsInCombat()) {
						continue;
					}
					if (!Utils::Actor::isHumanoid(refr)) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (!(refr->HasKeywordString("ActorTypeNPC") || refr->HasKeywordString("DLC2ActorTypeMiraak"))) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (refr->HasKeywordString("UND_ExcludeDodge")) {
						continue;
					}
					auto CTarget = refr->GetActorRuntimeData().currentCombatTarget.get().get();
					if (!CTarget) {
						continue;
					}
					// if (!ValhallaUtils::is_adversary(refr, a_attacker)) {
					// 	continue;
					// }

					RE::BGSAttackData* attackdata = Utils::get_attackData(a_attacker);
					auto angle = Utils::get_angle_he_me(refr, a_attacker, attackdata);

					float attackAngle = attackdata ? attackdata->data.strikeAngle : 35.0f;

					if (abs(angle) > attackAngle) {
						continue;
					}

					switch (settings::iDodgeAI_Framework) {
					case 0:
						dodge::GetSingleton()->Bash_attempt_dodge(refr, &dodge_directions_tk_reactive);
						break;
					case 1:
						// dodge::GetSingleton()->attempt_dodge(refr, &dodge_directions_dmco_all);
						break;
					}
				}
				continue;
			}
		}
	}
}

void dodge::react_to_bash_sprint(RE::Actor* a_attacker, float attack_range, float mov_speed)
{
	if (!settings::bDodgeAI_Reactive_enable) {
		return;
	}
	auto combatGroup = a_attacker->GetCombatGroup();
	if (combatGroup) {
		for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
			if (it->targetHandle && it->targetHandle.get().get()) {
				RE::Actor* refr = it->targetHandle.get().get();
				if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) <= attack_range + 500.0f) {
					
					if (refr->IsPlayerRef() || refr->IsDead() || !refr->IsInCombat()) {
						continue;
					}
					if (!Utils::Actor::isHumanoid(refr)) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (!(refr->HasKeywordString("ActorTypeNPC") || refr->HasKeywordString("DLC2ActorTypeMiraak"))) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (refr->HasKeywordString("UND_ExcludeDodge")) {
						continue;
					}
					auto CTarget = refr->GetActorRuntimeData().currentCombatTarget.get().get();
					if (!CTarget) {
						continue;
					}
					// if (!ValhallaUtils::is_adversary(refr, a_attacker)) {
					// 	continue;
					// }

					RE::BGSAttackData* attackdata = Utils::get_attackData(a_attacker);
					auto angle = Utils::get_angle_he_me(refr, a_attacker, attackdata);

					float attackAngle = attackdata ? attackdata->data.strikeAngle : 35.0f;

					if (abs(angle) > attackAngle) {
						continue;
					}

					auto bUND_Update_bashsprint = false;

					if (refr->GetGraphVariableBool("bUND_Update_bashsprint", bUND_Update_bashsprint) && bUND_Update_bashsprint) {
						continue;
					}

					auto distance = refr->GetPosition().GetDistance(a_attacker->GetPosition()); ;
					auto time = distance/mov_speed;
					auto time_needed = dodge::round_to(time, 10.0, 1.0);

					if (time_needed <= 0.1f) {
						dodge::GetSingleton()->BashSprint_attempt_dodge(refr, &dodge_directions_tk_horizontal, mov_speed);

					} else {
						refr->SetGraphVariableFloat("fUND_Update_time_required_bashsprint", time_needed - 0.1f);
						refr->SetGraphVariableFloat("fUND_Update_time_counter_bashsprint", 0.0f);
						refr->SetGraphVariableFloat("fUND_Update_attackSpeed_bashsprint", mov_speed);
						refr->SetGraphVariableBool("bUND_Update_bashsprint", true);
					}
				}
				continue;
			}
		}
	}
}

void dodge::react_to_ranged(RE::Actor* a_attacker, float attack_range, float attack_speed)
{
	if (!settings::bDodgeAI_Reactive_enable) {
		return;
	}
	auto combatGroup = a_attacker->GetCombatGroup();
	if (combatGroup) {
		for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
			if (it->targetHandle && it->targetHandle.get().get()) {
				RE::Actor* refr = it->targetHandle.get().get();
				if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) <= attack_range) {
					
					if (refr->IsPlayerRef() || refr->IsDead() || !refr->IsInCombat()) {
						continue;
					}
					if (!Utils::Actor::isHumanoid(refr)) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (!(refr->HasKeywordString("ActorTypeNPC") || refr->HasKeywordString("DLC2ActorTypeMiraak"))) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (refr->HasKeywordString("UND_ExcludeDodge")) {
						continue;
					}
					auto CTarget = refr->GetActorRuntimeData().currentCombatTarget.get().get();
					if (!CTarget) {
						continue;
					}
					// if (!ValhallaUtils::is_adversary(refr, a_attacker)) {
					// 	continue;
					// }

					bool hasLOS = false;
					if (refr->HasLineOfSight(a_attacker, hasLOS) && !hasLOS) {
						continue;
					}

					RE::BGSAttackData* attackdata = Utils::get_attackData(a_attacker);
					auto angle = Utils::get_angle_he_me(refr, a_attacker, attackdata);

					float attackAngle = attackdata ? attackdata->data.strikeAngle : 7.0f;

					if (abs(angle) > attackAngle) {
						continue;
					}

					if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) > 512.0f 
					&& IsMeleeOnly(refr) && refr->AsActorState()->IsSprinting() && refr->AsActorState()->actorState1.movingForward 
					&& !is_adequate_threat(refr, a_attacker)){
						continue;
					}
					

					switch (settings::iDodgeAI_Framework) {
					case 0:
						dodge::GetSingleton()->attempt_dodge(refr, &dodge_directions_tk_horizontal, attack_speed);
						break;
					case 1:
						// dodge::GetSingleton()->attempt_dodge(refr, &dodge_directions_dmco_reactive);
						break;
					}
				}
				continue;
			}
		}
	}
}

void dodge::react_to_shouts_spells(RE::Actor* a_attacker, float attack_range, float attack_speed)
{
	if (!settings::bDodgeAI_Reactive_enable) {
		return;
	}
	auto combatGroup = a_attacker->GetCombatGroup();
	if (combatGroup) {
		for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
			if (it->targetHandle && it->targetHandle.get().get()) {
				RE::Actor* refr = it->targetHandle.get().get();
				if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) <= attack_range) {
					
					if (refr->IsPlayerRef() || refr->IsDead() || !refr->IsInCombat()) {
						continue;
					}
					if (!Utils::Actor::isHumanoid(refr)) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (!(refr->HasKeywordString("ActorTypeNPC") || refr->HasKeywordString("DLC2ActorTypeMiraak"))) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (refr->HasKeywordString("UND_ExcludeDodge")) {
						continue;
					}
					auto CTarget = refr->GetActorRuntimeData().currentCombatTarget.get().get();
					if (!CTarget) {
						continue;
					}
					// if (!ValhallaUtils::is_adversary(refr, a_attacker)) {
					// 	continue;
					// }

					bool hasLOS = false;
					if (refr->HasLineOfSight(a_attacker, hasLOS) && !hasLOS) {
						continue;
					}

					RE::BGSAttackData* attackdata = Utils::get_attackData(a_attacker);
					auto angle = Utils::get_angle_he_me(refr, a_attacker, attackdata);

					float attackAngle = attackdata ? attackdata->data.strikeAngle : 35.0f;

					if (abs(angle) > attackAngle) {
						continue;
					}

					if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) > 512.0f 
					&& IsMeleeOnly(refr) && refr->AsActorState()->IsSprinting() && refr->AsActorState()->actorState1.movingForward 
					&& !is_adequate_threat(refr, a_attacker)){
						continue;
					}

					auto bUND_Update_spell = false;

					if (refr->GetGraphVariableBool("bUND_Update_spell", bUND_Update_spell) && bUND_Update_spell) {
						continue;
					}
					auto distance = refr->GetPosition().GetDistance(a_attacker->GetPosition());
					auto time = distance/attack_speed;
					auto time_needed = dodge::round_to(time, 10.0, 1.0);

					if (time_needed <= 0.1f) {
						time_needed <= 0.0f ? dodge::GetSingleton()->Shouts_Spells_attempt_dodge(refr, &dodge_directions_tk_reactive, attack_speed) 
						: dodge::GetSingleton()->Shouts_Spells_attempt_dodge(refr, &dodge_directions_tk_horizontal, attack_speed);

					} else {
						refr->SetGraphVariableFloat("fUND_Update_time_required_spell", time_needed - 0.1f);
						refr->SetGraphVariableFloat("fUND_Update_time_counter_spell", 0.0f);
						refr->SetGraphVariableFloat("fUND_Update_attackSpeed_spell", attack_speed);
						refr->SetGraphVariableBool("bUND_Update_spell", true);
					}
				}
				continue;
			}
		}
	}
}

void dodge::react_to_shouts_spells_fast(RE::Actor* a_attacker, float attack_range, bool lefthand)
{
	if (!settings::bDodgeAI_Reactive_enable) {
		return;
	}
	auto combatGroup = a_attacker->GetCombatGroup();
	if (combatGroup) {
		for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
			if (it->targetHandle && it->targetHandle.get().get()) {
				RE::Actor* refr = it->targetHandle.get().get();
				if (refr->GetPosition().GetDistance(a_attacker->GetPosition()) <= attack_range) {
					
					if (refr->IsPlayerRef() || refr->IsDead() || !refr->IsInCombat()) {
						continue;
					}
					if (!Utils::Actor::isHumanoid(refr)) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (!(refr->HasKeywordString("ActorTypeNPC") || refr->HasKeywordString("DLC2ActorTypeMiraak"))) {
						if(!(refr->HasKeywordString("VLS_Serana_Key") || refr->HasKeywordString("VLS_Valerica_Key"))){
                            continue;
						}
					}
					if (refr->HasKeywordString("UND_ExcludeDodge")) {
						continue;
					}
					auto CTarget = refr->GetActorRuntimeData().currentCombatTarget.get().get();
					if (!CTarget) {
						continue;
					}
					// if (!ValhallaUtils::is_adversary(refr, a_attacker)) {
					// 	continue;
					// }

					bool hasLOS = false;
					if (refr->HasLineOfSight(a_attacker, hasLOS) && !hasLOS) {
						continue;
					}

					RE::BGSAttackData* attackdata = Utils::get_attackData(a_attacker);
					auto angle = Utils::get_angle_he_me(refr, a_attacker, attackdata);

					float attackAngle = attackdata ? attackdata->data.strikeAngle : 35.0f;

					if (abs(angle) > attackAngle) {
						continue;
					}

					switch (settings::iDodgeAI_Framework) {
					case 0:
						dodge::GetSingleton()->attempt_dodge(refr, &dodge_directions_tk_horizontal);
						break;
					case 1:
						// dodge::GetSingleton()->attempt_dodge(refr, &dodge_directions_dmco_reactive);
						break;
					}
				}
				continue;
			}
		}
	}
}

float dodge::round_to(double value, double value2, double divding_num, double precision)
{
	double result = value / (value2 * divding_num);
	auto answer =  static_cast<float>(std::round(result / precision) * precision);

	return isinf(answer) ? 0.0f : answer;
}

//void dodge::set_dodge_phase(RE::Actor* a_dodger, bool a_isDodging)
//{
//	auto handle = a_dodger->GetHandle();
//	if (handle) {
//		writeLock l (dodging_actors_lock);
//		if (a_isDodging) {
//			if (!dodging_actors.contains(handle)) {
//				dodging_actors.insert(handle);
//			}
//		} else {
//			dodging_actors.erase(handle);
//		}
//	}
//}
//
//bool dodge::get_is_dodging(RE::Actor* a_actor)
//{
//	auto handle = a_actor->GetHandle();
//	if (handle) {
//		readLock l(dodging_actors_lock);
//		return dodging_actors.contains(handle);
//	}
//	return false;
//	
//}

void dodge::Update(RE::Actor* a_actor, [[maybe_unused]] float a_delta)
{
	if (a_actor->GetActorRuntimeData().currentProcess && a_actor->GetActorRuntimeData().currentProcess->InHighProcess() && a_actor->Is3DLoaded()) {
		auto bUND_Update_spell = false;

		if (a_actor->GetGraphVariableBool("bUND_Update_spell", bUND_Update_spell) && bUND_Update_spell) {
			float fUND_Update_time_required_spell = 0.0f;
			float fUND_Update_time_counter_spell = 0.0f;
			float fUND_Update_attackSpeed_spell = 0.0f;
			a_actor->GetGraphVariableFloat("fUND_Update_time_required_spell", fUND_Update_time_required_spell);
			a_actor->GetGraphVariableFloat("fUND_Update_time_counter_spell", fUND_Update_time_counter_spell);
			a_actor->GetGraphVariableFloat("fUND_Update_attackSpeed_spell", fUND_Update_attackSpeed_spell);

			if (settings::bCombatlogging_enable){
				logger::info("Name {} timerequired {}"sv, a_actor->GetName(), fUND_Update_time_required_spell);
			}
			auto counter = fUND_Update_time_counter_spell += g_deltaTime;
			a_actor->SetGraphVariableFloat("fUND_Update_time_counter_spell", counter);
			if (settings::bCombatlogging_enable) {
				logger::info("Name {} timecounter {}"sv, a_actor->GetName(), fUND_Update_time_counter_spell);
			}
			if (counter >= fUND_Update_time_required_spell) {
				a_actor->SetGraphVariableBool("bUND_Update_spell", false);
				if (settings::bCombatlogging_enable) {
					logger::info("Name {} attackspeed {}"sv, a_actor->GetName(), fUND_Update_attackSpeed_spell);
				}
				dodge::GetSingleton()->Shouts_Spells_attempt_dodge(a_actor, &dodge_directions_tk_horizontal, fUND_Update_attackSpeed_spell);
			}
		}

		auto bUND_Update_bashsprint = false;

		if (a_actor->GetGraphVariableBool("bUND_Update_bashsprint", bUND_Update_bashsprint) && bUND_Update_bashsprint) {
			float fUND_Update_time_required_bashsprint = 0.0f;
			float fUND_Update_time_counter_bashsprint = 0.0f;
			float fUND_Update_attackSpeed_bashsprint = 0.0f;
			a_actor->GetGraphVariableFloat("fUND_Update_time_required_bashsprint", fUND_Update_time_required_bashsprint);
			a_actor->GetGraphVariableFloat("fUND_Update_time_counter_bashsprint", fUND_Update_time_counter_bashsprint);
			a_actor->GetGraphVariableFloat("fUND_Update_attackSpeed_bashsprint", fUND_Update_attackSpeed_bashsprint);

			if (settings::bCombatlogging_enable) {
				logger::info("Name {} timerequired {}"sv, a_actor->GetName(), fUND_Update_time_required_bashsprint);
			}
			auto counter = fUND_Update_time_counter_bashsprint += g_deltaTime;
			a_actor->SetGraphVariableFloat("fUND_Update_time_counter_bashsprint", counter);
			if (settings::bCombatlogging_enable) {
				logger::info("Name {} timecounter {}"sv, a_actor->GetName(), fUND_Update_time_counter_bashsprint);
			}
			if (counter >= fUND_Update_time_required_bashsprint) {
				a_actor->SetGraphVariableBool("bUND_Update_bashsprint", false);
				if (settings::bCombatlogging_enable) {
					logger::info("Name {} attackspeed {}"sv, a_actor->GetName(), fUND_Update_attackSpeed_bashsprint);
				}
				dodge::GetSingleton()->BashSprint_attempt_dodge(a_actor, &dodge_directions_tk_horizontal, fUND_Update_attackSpeed_bashsprint);
			}
		}
	}
}

/*Check if the actor is able to dodge.*/
bool dodge::able_dodge(RE::Actor* a_actor)
{
	auto attackState = a_actor->AsActorState()->GetAttackState();
	//auto CombatTarget = a_actor->GetActorRuntimeData().currentCombatTarget.get().get();
	auto ATMagicTarget = a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kParalysis);
	auto magicTarget = a_actor->AsMagicTarget();
	bool IsShouting = false;
	auto DS = dodge::GetSingleton();
	const float SideStep_staminacost = DS->get_stamina_basecost(a_actor, DS->Staminaa);
	bool bUND_InCombatFoundEnemy = false;

	if (getrace_VLserana(a_actor)) {
		if (!a_actor->IsInKillMove() && a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagicka) >= 25 && !(attackState == RE::ATTACK_STATE_ENUM::kHit)) {
			return true;
		}else{
			return false;
		}
	}

	if (settings::bZUPA_mod_Check) {
		const auto magicEffect = RE::TESForm::LookupByEditorID("zxlice_cooldownEffect")->As<RE::EffectSetting>();

		if (!a_actor->IsInKillMove() && (a_actor->GetGraphVariableBool("bUND_InCombatFoundEnemy", bUND_InCombatFoundEnemy) && bUND_InCombatFoundEnemy) && (a_actor->GetGraphVariableBool("IsShouting", IsShouting) && !IsShouting) && ATMagicTarget == 0.0 && !magicTarget->HasEffectWithArchetype(RE::EffectArchetypes::ArchetypeID::kDemoralize)
		&& a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) >= SideStep_staminacost 
		&& !(attackState == RE::ATTACK_STATE_ENUM::kSwing || attackState == RE::ATTACK_STATE_ENUM::kHit  || attackState == RE::ATTACK_STATE_ENUM::kFollowThrough || attackState == RE::ATTACK_STATE_ENUM::kBash 
		|| attackState == RE::ATTACK_STATE_ENUM::kBowDrawn || attackState == RE::ATTACK_STATE_ENUM::kBowReleasing || attackState == RE::ATTACK_STATE_ENUM::kBowFollowThrough) && !magicTarget->HasMagicEffect(magicEffect)) {
			return true;
		}
	} else if (settings::bUAPNG_mod_Check){
		bool IUBusy = false;
		if (!a_actor->IsInKillMove() && (a_actor->GetGraphVariableBool("bUND_InCombatFoundEnemy", bUND_InCombatFoundEnemy) && bUND_InCombatFoundEnemy) && (a_actor->GetGraphVariableBool("IsShouting", IsShouting) && !IsShouting) && ATMagicTarget == 0.0 && !magicTarget->HasEffectWithArchetype(RE::EffectArchetypes::ArchetypeID::kDemoralize)
		&& (a_actor->GetGraphVariableBool("IUBusy", IUBusy) && !IUBusy) && a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) >= SideStep_staminacost 
		&& !(attackState == RE::ATTACK_STATE_ENUM::kSwing || attackState == RE::ATTACK_STATE_ENUM::kHit  || attackState == RE::ATTACK_STATE_ENUM::kFollowThrough || attackState == RE::ATTACK_STATE_ENUM::kBash 
		|| attackState == RE::ATTACK_STATE_ENUM::kBowDrawn || attackState == RE::ATTACK_STATE_ENUM::kBowReleasing || attackState == RE::ATTACK_STATE_ENUM::kBowFollowThrough)) {
			return true;
		}
	} else{
		if (!a_actor->IsInKillMove() && (a_actor->GetGraphVariableBool("bUND_InCombatFoundEnemy", bUND_InCombatFoundEnemy) && bUND_InCombatFoundEnemy) && (a_actor->GetGraphVariableBool("IsShouting", IsShouting) && !IsShouting) && ATMagicTarget == 0.0 && !magicTarget->HasEffectWithArchetype(RE::EffectArchetypes::ArchetypeID::kDemoralize)
		&& a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) >= SideStep_staminacost 
		&& !(attackState == RE::ATTACK_STATE_ENUM::kSwing || attackState == RE::ATTACK_STATE_ENUM::kHit  || attackState == RE::ATTACK_STATE_ENUM::kFollowThrough || attackState == RE::ATTACK_STATE_ENUM::kBash 
		|| attackState == RE::ATTACK_STATE_ENUM::kBowDrawn || attackState == RE::ATTACK_STATE_ENUM::kBowReleasing || attackState == RE::ATTACK_STATE_ENUM::kBowFollowThrough)) {
			return true;
		}
	}
	return false;
}

/*Attempt to dodge an incoming threat, choosing one of the directions from A_DIRECTIONS.*/
void dodge::attempt_dodge(RE::Actor* a_actor, const dodge_dir_set* a_directions, float attack_speed, bool a_forceDodge)
{
	
    auto DS = dodge::GetSingleton();
	const float dodge_chance = a_forceDodge ? 1.0f : get_dodge_chance(a_actor, DS->Armourr, DS->Protagnist_Reflexess, DS->CStylee);

	std::mt19937 gen(rd());

	if (attack_speed > 0.0f) {
		if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > (dodge::round_to(dodge_chance, attack_speed, 2.0))) {
			return;
		}
	} else {
		if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > dodge_chance) {
			return;
		}
	}

	/* Make a copy and shuffle directions. */
	dodge_dir_set directions_shuffled = *a_directions;
	std::shuffle(directions_shuffled.begin(), directions_shuffled.end(), gen);

	for (dodge_direction direction : directions_shuffled) {
		RE::NiPoint3 dodge_dest = Utils::get_abs_pos(a_actor, get_dodge_vector(direction));
		if (can_goto(a_actor, dodge_dest) && able_dodge(a_actor)) {
			if (getrace_VLserana(a_actor)) {
				do_dodge_VLSerana(a_actor, direction);
				if (settings::bCombatlogging_enable) {
					logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge::round_to(dodge_chance, attack_speed, 2.0)));
					// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
					
				}
			}else{
				bool bIsDodging = false;
				if (a_actor->GetGraphVariableBool("bIsDodging", bIsDodging) && !bIsDodging) {
					do_dodge(a_actor, direction);
					if (settings::bCombatlogging_enable) {
						logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge::round_to(dodge_chance, attack_speed, 2.0)));
						// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
						
					}
				}
			}
			break;
		}
	}
}

void dodge::Powerattack_attempt_dodge(RE::Actor* a_actor, const dodge_dir_set* a_directions, bool a_forceDodge)
{
    auto DS = dodge::GetSingleton();
	const float dodge_chance = a_forceDodge ? 1.0f : get_dodge_chance(a_actor, DS->Armourr, DS->Protagnist_Reflexess, DS->CStylee);

	

	std::mt19937 gen(rd());
	
	if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > dodge_chance) {
		return;
	}

	/* Make a copy and shuffle directions. */
	dodge_dir_set directions_shuffled = *a_directions;
	std::shuffle(directions_shuffled.begin(), directions_shuffled.end(), gen);

	for (dodge_direction direction : directions_shuffled) {
		RE::NiPoint3 dodge_dest = Utils::get_abs_pos(a_actor, get_dodge_vector(direction));
		if (can_goto(a_actor, dodge_dest) && able_dodge(a_actor)) {
			if (getrace_VLserana(a_actor)) {
				do_dodge_VLSerana(a_actor, direction);
				if (settings::bCombatlogging_enable) {
					logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge_chance));
					// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
					
				}
			}else{
				bool bIsDodging = false;
				if (a_actor->GetGraphVariableBool("bIsDodging", bIsDodging) && !bIsDodging) {
					do_dodge(a_actor, direction);
					if (settings::bCombatlogging_enable) {
						logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge_chance));
						// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
						
					}
				}
			}
			break;
		}
	}
}

void dodge::NormalAttack_attempt_dodge(RE::Actor* a_actor, const dodge_dir_set* a_directions, float attack_speed, bool a_forceDodge)
{
    auto DS = dodge::GetSingleton();
	const float dodge_chance = a_forceDodge ? 1.0f : get_dodge_chance(a_actor, DS->Armourr, DS->Protagnist_Reflexess, DS->CStylee);

	

	std::mt19937 gen(rd());

	if (attack_speed > 0.0f) {
		if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > (dodge::round_to(dodge_chance, attack_speed, 2.0))) {
			return;
		}
	} else {
		if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > dodge_chance) {
			return;
		}
	}

	/* Make a copy and shuffle directions. */
	dodge_dir_set directions_shuffled = *a_directions;
	std::shuffle(directions_shuffled.begin(), directions_shuffled.end(), gen);

	for (dodge_direction direction : directions_shuffled) {
		RE::NiPoint3 dodge_dest = Utils::get_abs_pos(a_actor, get_dodge_vector(direction));
		if (can_goto(a_actor, dodge_dest) && able_dodge(a_actor)) {
			if (getrace_VLserana(a_actor)) {
				do_dodge_VLSerana(a_actor, direction);
				if (settings::bCombatlogging_enable){
					logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge::round_to(dodge_chance, attack_speed, 2.0)));
					// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
					
				}
				
			}else{
				bool bIsDodging = false;
				if (a_actor->GetGraphVariableBool("bIsDodging", bIsDodging) && !bIsDodging) {
					do_dodge(a_actor, direction);
					if (settings::bCombatlogging_enable) {
						logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge::round_to(dodge_chance, attack_speed, 2.0)));
						// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
						
					}
				}
			}
			break;
		}
	}
}

void dodge::Shouts_Spells_attempt_dodge(RE::Actor* a_actor, const dodge_dir_set* a_directions, float attack_speed, bool a_forceDodge)
{
	a_actor->SetGraphVariableInt("iUND_dodge_type", 0);

	auto DS = dodge::GetSingleton();
	const float dodge_chance = a_forceDodge ? 1.0f : get_dodge_chance(a_actor, DS->Armourr, DS->Protagnist_Reflexess, DS->CStylee);

	

	std::mt19937 gen(rd());

	if (attack_speed > 0.0f){
		if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > (dodge::round_to(dodge_chance, attack_speed, 2.0))) {
			return;
		}
	}else{
		if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > dodge_chance) {
			return;
		}
	}

	/* Make a copy and shuffle directions. */
	dodge_dir_set directions_shuffled = *a_directions;
	std::shuffle(directions_shuffled.begin(), directions_shuffled.end(), gen);

	for (dodge_direction direction : directions_shuffled) {
		RE::NiPoint3 dodge_dest = Utils::get_abs_pos(a_actor, get_dodge_vector(direction));
		if (can_goto(a_actor, dodge_dest) && able_dodge(a_actor)) {
			if (getrace_VLserana(a_actor)) {
				do_dodge_VLSerana(a_actor, direction);
				if (settings::bCombatlogging_enable) {
					logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge::round_to(dodge_chance, attack_speed, 2.0)));
					// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
					
				}
			}else{
				bool bIsDodging = false;
				if (a_actor->GetGraphVariableBool("bIsDodging", bIsDodging) && !bIsDodging) {
					do_dodge(a_actor, direction);
					if (settings::bCombatlogging_enable) {
						logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge::round_to(dodge_chance, attack_speed, 2.0)));
						// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
						
					}
				}
			}
			break;
		}
	}
}

void dodge::Bash_attempt_dodge(RE::Actor* a_actor, const dodge_dir_set* a_directions, bool a_forceDodge)
{
    auto DS = dodge::GetSingleton();
	const float dodge_chance = a_forceDodge ? 1.0f : get_dodge_chance(a_actor, DS->Armourr, DS->Protagnist_Reflexess, DS->CStylee);

	

	std::mt19937 gen(rd());
	
	if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > dodge_chance) {
		return;
	}

	/* Make a copy and shuffle directions. */
	dodge_dir_set directions_shuffled = *a_directions;
	std::shuffle(directions_shuffled.begin(), directions_shuffled.end(), gen);

	for (dodge_direction direction : directions_shuffled) {
		RE::NiPoint3 dodge_dest = Utils::get_abs_pos(a_actor, get_dodge_vector(direction));
		if (can_goto(a_actor, dodge_dest) && able_dodge(a_actor)) {
			if (getrace_VLserana(a_actor)) {
				do_dodge_VLSerana(a_actor, direction);
				if (settings::bCombatlogging_enable) {
					logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge_chance));
					// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
					
				}
			}else{
				bool bIsDodging = false;
				if (a_actor->GetGraphVariableBool("bIsDodging", bIsDodging) && !bIsDodging) {
					do_dodge(a_actor, direction);
					if (settings::bCombatlogging_enable) {
						logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge_chance));
						// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
						
					}
				}
			}
			break;
		}
	}
}

void dodge::BashSprint_attempt_dodge(RE::Actor* a_actor, const dodge_dir_set* a_directions, float mov_speed, bool a_forceDodge)
{
	a_actor->SetGraphVariableInt("iUND_dodge_type", 0);

	auto DS = dodge::GetSingleton();
	const float dodge_chance = a_forceDodge ? 1.0f : get_dodge_chance(a_actor, DS->Armourr, DS->Protagnist_Reflexess, DS->CStylee);

	std::mt19937 gen(rd());

	if (mov_speed > 0.0f) {
		if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > (dodge::round_to(dodge_chance, mov_speed, 2.0))) {
			return;
		}
	} else {
		if (dodge::GetSingleton()->GenerateRandomFloat(0.0f, 1.0f) > dodge_chance) {
			return;
		}
	}

	/* Make a copy and shuffle directions. */
	dodge_dir_set directions_shuffled = *a_directions;
	std::shuffle(directions_shuffled.begin(), directions_shuffled.end(), gen);

	for (dodge_direction direction : directions_shuffled) {
		RE::NiPoint3 dodge_dest = Utils::get_abs_pos(a_actor, get_dodge_vector(direction));
		if (can_goto(a_actor, dodge_dest) && able_dodge(a_actor)) {
			if (getrace_VLserana(a_actor)) {
				do_dodge_VLSerana(a_actor, direction);
				if (settings::bCombatlogging_enable) {
					logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge::round_to(dodge_chance, mov_speed, 2.0)));
					// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
					
				}
			}else{
				bool bIsDodging = false;
				if (a_actor->GetGraphVariableBool("bIsDodging", bIsDodging) && !bIsDodging) {
					do_dodge(a_actor, direction);
					if (settings::bCombatlogging_enable) {
						logger::info("Name {} ajusted_reflexScore {}"sv, a_actor->GetName(), (dodge::round_to(dodge_chance, mov_speed, 2.0)));
						// logger::info("Name {} Reach {}"sv, a_actor->GetName(), Actor_GetReach(a_actor));
						
					}
				}
			}
			break;
		}
	}
}




#define MAX_DIST_DIFFERENCE 50
bool dodge::can_goto(RE::Actor* a_actor, RE::NiPoint3 a_dest)
{
	bool canNavigate = false;
	bool noObstacle = true;
	RE::NiPoint3 raycast_dest = a_dest;
	RE::NiPoint3 nav_dest = a_dest;

	bool gotoNavdest = false;
	/*float expect_dist = a_actor->GetPosition().GetDistance(a_dest);*/
	
	// /*Use Skyrim's internal check*/
	// if (a_actor->UpdateNavPos(a_actor->GetPosition(), nav_dest, 4.0f, a_actor->GetBoundRadius()) 
	// 	&& abs(nav_dest.GetDistance(a_actor->GetPosition()) - expect_dist) < MAX_DIST_DIFFERENCE) {
	// 	RE::NiPoint3 nav_dest_raycast = nav_dest;                   // make a copy of nav_dest to use for raycast
	// 	if (DtryUtils::rayCast::object_exists(nav_dest_raycast)) {//check if the actor can stand on the nav dest
	// 		gotoNavdest = true;
	// 		canNavigate = true;
	// 	}
	// }
	
	/*Use our own pathing check, if skyrim's check fails.*/
	if (!gotoNavdest) {
		canNavigate = DtryUtils::rayCast::object_exists(raycast_dest, 75.f);//bigger range to account for slopes
	}
	
	if (canNavigate) {
		RE::NiPoint3 dest = gotoNavdest ? nav_dest : raycast_dest;
		//if (settings::bDodgeAI_DebugDraw_Enable) {
		//	if (API::TrueHUD_API_acquired) {
		//		API::_TrueHud_API->DrawLine(a_actor->GetPosition(), dest, 1.f, 0xff00ff);  //green line
		//	}
		//}
		
		/*Cast 3 rays from the actor, parallel to the dodging path to check for any obstacles.*/
		float obstacleDist = 0; /*Distance to the obstacle, if any*/
		dest.z += a_actor->GetHeight() * 1 / 4;
		noObstacle &= DtryUtils::rayCast::cast_ray(a_actor, dest, 0.25f, &obstacleDist) == nullptr || obstacleDist >= settings::fDodgeAI_DodgeDist_Permissible;
		dest.z += a_actor->GetHeight() * 1 / 4;
		noObstacle &= DtryUtils::rayCast::cast_ray(a_actor, dest, 0.5f, &obstacleDist) == nullptr || obstacleDist >= settings::fDodgeAI_DodgeDist_Permissible;
		dest.z += a_actor->GetHeight() * 1 / 4;
		noObstacle &= DtryUtils::rayCast::cast_ray(a_actor, dest, 0.75f, &obstacleDist) == nullptr || obstacleDist >= settings::fDodgeAI_DodgeDist_Permissible;
	}
	

	return canNavigate && noObstacle;
	
}


/*Get the direction the actor should dodge in.*/
//dodge_direction dodge::get_dodge_direction(RE::Actor* a_actor, RE::Actor* a_attacker)
//{
//	return dodge_direction::kForward; /*defaults to backward dodging for now*/
//}

int dodge::GenerateRandomInt(int value_a, int value_b) {

	std::mt19937 generator(rd());
	std::uniform_int_distribution<int> dist(value_a, value_b);
	return dist(generator);
}

float dodge::GenerateRandomFloat(float value_a, float value_b)
{
	std::mt19937 generator(rd());
	std::uniform_real_distribution<float> dist(value_a, value_b);
	return dist(generator);
}

static const char* GVI_dodge_dir = "Dodge_Direction";
// static const char* AE_dodge = "Dodge";
void dmco_dodge(RE::Actor* a_actor, dodge_direction a_direction, const char* a_event) {
	auto task = SKSE::GetTaskInterface();
	if (!task) {
		return;
	}
	task->AddTask([a_actor, a_direction, a_event]() {
		a_actor->SetGraphVariableInt(GVI_dodge_dir, a_direction);
		interruptattack(a_actor);
		a_actor->NotifyAnimationGraph(a_event);
	});
}

void dodge::TRKE_dodge(RE::Actor* actor, const char* a_event, bool backingoff)
{
	auto DS = dodge::GetSingleton();
	const float DodgeRoll_staminacost = DS->get_stamina_basecost(actor, DS->Staminaa, true);
	actor->NotifyAnimationGraph("InterruptCast");
	actor->InterruptCast(false);

	if (backingoff) {
		actor->SetGraphVariableInt("iStep", 2);
		actor->SetGraphVariableBool("bUND_IsDodgeRoll", false);
		actor->NotifyAnimationGraph(a_event);
		return;
	}

	if (settings::bHasSilentRollperk_enable == 1) {
		auto data = RE::TESDataHandler::GetSingleton();
		auto bSilentRoll = actor->HasPerk(data->LookupForm<RE::BGSPerk>(0x105F23, "Skyrim.esm"));
		if (dodge::GetSingleton()->GenerateRandomInt(0, 10) <= settings::iDodgeRoll_ActorScaled_Chance && bSilentRoll && actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) >= DodgeRoll_staminacost) {
			actor->SetGraphVariableInt("iStep", 0);
			actor->SetGraphVariableBool("bUND_IsDodgeRoll", true);
		} else {
			actor->SetGraphVariableInt("iStep", 2);
			actor->SetGraphVariableBool("bUND_IsDodgeRoll", false);
		}
		actor->NotifyAnimationGraph(a_event);
		return;

	} else {
		if (dodge::GetSingleton()->GenerateRandomInt(0, 10) <= settings::iDodgeRoll_ActorScaled_Chance && actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) >= DodgeRoll_staminacost) {
			actor->SetGraphVariableInt("iStep", 0);
			actor->SetGraphVariableBool("bUND_IsDodgeRoll", true);	
		} else {
			actor->SetGraphVariableInt("iStep", 2);
			actor->SetGraphVariableBool("bUND_IsDodgeRoll", false);
		}
		actor->NotifyAnimationGraph(a_event);

	}
}

void dodge::do_dodge(RE::Actor* a_actor, dodge_direction a_direction)
{
	switch (a_direction) {
	case dodge_direction::kForward:
		switch (settings::iDodgeAI_Framework) {
		case 0:

			TRKE_dodge(a_actor, "TKDodgeForward");
			break;
		case 1:
			dmco_dodge(a_actor, a_direction, "Dodge");
			break;
		}
		break;
	case dodge_direction::kBackward:
		switch (settings::iDodgeAI_Framework) {
		case 0:

			TRKE_dodge(a_actor, "TKDodgeBack");
			break;
		case 1:
			dmco_dodge(a_actor, a_direction, "Dodge");
			break;
		}
		break;
	case dodge_direction::kLeft:
		switch (settings::iDodgeAI_Framework) {
		case 0:

			TRKE_dodge(a_actor, "TKDodgeLeft");
			break;
		case 1:
			dmco_dodge(a_actor, a_direction, "Dodge");
			break;
		}
		break;
	case dodge_direction::kRight:
		switch (settings::iDodgeAI_Framework) {
		case 0:

			TRKE_dodge(a_actor, "TKDodgeRight");
			break;
		case 1:
			dmco_dodge(a_actor, a_direction, "Dodge");
			break;
		}
		break;
	/*Only possible for DMCO*/
	case dodge_direction::kLeftBackward:
		switch (settings::iDodgeAI_Framework) {
		case 1:
			dmco_dodge(a_actor, a_direction, "Dodge");
			break;
		}
		break;
	case dodge_direction::kLeftForward:
		switch (settings::iDodgeAI_Framework) {
		case 1:
			dmco_dodge(a_actor, a_direction, "Dodge");
			break;
		}
		break;
	case dodge_direction::kRightBackward:
		switch (settings::iDodgeAI_Framework) {
		case 1:
			dmco_dodge(a_actor, a_direction, "Dodge");
			break;
		}
		break;
	case dodge_direction::kRightForward:
		switch (settings::iDodgeAI_Framework) {
		case 1:
			dmco_dodge(a_actor, a_direction, "Dodge");
			break;
		}
		break;
	}
}
/*Get relative dodge vector*/
RE::NiPoint3 dodge::get_dodge_vector(dodge_direction a_direction)
{
	RE::NiPoint3 ret;
	ret.z = 0;
	switch (a_direction) {
	case kForward:
		ret.x = 0;
		ret.y = settings::fDodgeAI_DodgeDist;
		break;
	case kBackward:
		ret.x = 0;
		ret.y = -settings::fDodgeAI_DodgeDist;
		break;
	case kLeft:
		ret.x = -settings::fDodgeAI_DodgeDist;
		ret.y = 0;
		break;
	case kRight:
		ret.x = settings::fDodgeAI_DodgeDist;
		ret.y = 0;
		break;
	case kLeftBackward:
		ret.x = -settings::fDodgeAI_DodgeDist2;
		ret.y = -settings::fDodgeAI_DodgeDist2;
		break;
	case kLeftForward:
		ret.x = -settings::fDodgeAI_DodgeDist2;
		ret.y = settings::fDodgeAI_DodgeDist2;
		break;
	case kRightBackward:
		ret.x = settings::fDodgeAI_DodgeDist2;
		ret.y = -settings::fDodgeAI_DodgeDist2;
		break;
	case kRightForward:
		ret.x = settings::fDodgeAI_DodgeDist2;
		ret.y = settings::fDodgeAI_DodgeDist2;
		break;
	}
	
	return ret;
}

void dodge::do_dodge_VLSerana(RE::Actor* a_actor, dodge_direction a_direction)
{
	auto HdSingle = RE::TESDataHandler::GetSingleton();

	if (a_actor->HasKeywordString("VLS_Serana_Key")) {
		RE::TESGlobal* ZAngle = skyrim_cast<RE::TESGlobal*>(HdSingle->LookupForm(0x804, "VampireLordSerana.esp"));
		if (ZAngle){
			switch (a_direction) {
			case kForward:
				ZAngle->value = GetSingleton()->GenerateRandomFloat(-60.0f, 60.0f);
				break;
			case kBackward:
				ZAngle->value = GetSingleton()->GenerateRandomFloat(GetSingleton()->GenerateRandomFloat(-180.0f, -150.0f), GetSingleton()->GenerateRandomFloat(150.0f, 180.0f));
				break;
			case kLeft:
				ZAngle->value = GetSingleton()->GenerateRandomFloat(-120.0f, -90.0f);
				break;
			case kRight:
				ZAngle->value = GetSingleton()->GenerateRandomFloat(90.0f, 120.0f);
				break;
			}
			send_UNDdodge__event(a_actor);
		}

	} else if (a_actor->HasKeywordString("VLS_Valerica_Key")) {
		RE::TESGlobal* ZAngle = skyrim_cast<RE::TESGlobal*>(HdSingle->LookupForm(0x805, "VampireLordSerana.esp"));
		if (ZAngle) {
			switch (a_direction) {
			case kForward:
				ZAngle->value = GetSingleton()->GenerateRandomFloat(-60.0f, 60.0f);
				break;
			case kBackward:
				ZAngle->value = GetSingleton()->GenerateRandomFloat(GetSingleton()->GenerateRandomFloat(-180.0f, -150.0f), GetSingleton()->GenerateRandomFloat(150.0f, 180.0f));
				break;
			case kLeft:
				ZAngle->value = GetSingleton()->GenerateRandomFloat(-120.0f, -90.0f);
				break;
			case kRight:
				ZAngle->value = GetSingleton()->GenerateRandomFloat(90.0f, 120.0f);
				break;
			}
			send_UNDdodge__event(a_actor);
		}
	}
}
