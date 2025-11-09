#include "mafia/roles/maniac.h"

Role Maniac::role() const {
    return Role::Maniac;
}

Action Maniac::act(const std::map<int, SharedPtr<Player>>& alive_players_map,
                   int player_id,
                   bool is_mafia_round) {
    std::vector<int> kill_targets;

    // Maniac kills anyone except themselves
    for (const auto& [target_id, player_ptr] : alive_players_map) {
        if (target_id != player_id) {
            kill_targets.push_back(target_id);
        }
    }

    selected_target_id = choose_random(kill_targets);
    Logger::get_instance().log_round(
        "Маньяк " + std::to_string(player_id) +
        " выбрал игрока " + std::to_string(selected_target_id) +
        " для убийства.");

    co_return;
}

Action Maniac::vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
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
            "Маньяк " + std::to_string(player_id) +
            " выбрал игрока " + std::to_string(selected_target_id) +
            " на голосовании.");
    } else {
        Logger::get_instance().log_round(
            "Маньяку " + std::to_string(player_id) + " некуда голосовать.");
    }

    co_return;
}
