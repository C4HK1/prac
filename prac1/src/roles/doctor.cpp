#include "mafia/roles/doctor.h"

Role Doctor::role() const {
    return Role::Doctor;
}

Action Doctor::act(const std::map<int, SharedPtr<Player>>& alive_players_map,
                   int player_id,
                   bool is_mafia_round) {
    std::vector<int> healing_targets;

    // Doctor cannot heal the same person twice in a row
    for (const auto& [target_id, player_ptr] : alive_players_map) {
        if (target_id != previous_target_id) {
            healing_targets.push_back(target_id);
        }
    }

    if (!healing_targets.empty()) {
        int chosen_patient_id = choose_random(healing_targets);
        selected_target_id = chosen_patient_id;
        previous_target_id = chosen_patient_id;

        Logger::get_instance().log_round(
            "Доктор " + std::to_string(player_id) +
            " лечит игрока " + std::to_string(chosen_patient_id) + ".");
    } else {
        selected_target_id = -1;
        Logger::get_instance().log_round(
            "Доктор не нашел кого лечить и пропускает ход.");
    }

    co_return;
}

Action Doctor::vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
                    int player_id) {
    std::vector<int> possible_targets;

    for (const auto& [target_id, player_ptr] : alive_players_map) {
        if (target_id != player_id) {
            possible_targets.push_back(target_id);
        }
    }

    if (!possible_targets.empty()) {
        selected_target_id = choose_random(possible_targets);
        Logger::get_instance().log_round(
            "Доктору " + std::to_string(player_id) +
            " выбрал игрока " + std::to_string(selected_target_id) +
            " на голосовании.");
    } else {
        Logger::get_instance().log_round(
            "Доктору " + std::to_string(player_id) + " некуда голосовать.");
    }

    co_return;
}

int Doctor::get_prev_target() const {
    return previous_target_id;
}

void Doctor::set_prev_target(int target_id) {
    previous_target_id = target_id;
}