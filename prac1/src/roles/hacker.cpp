#include "mafia/roles/hacker.h"

Role Hacker::role() const {
    return Role::Hacker;
}

Action Hacker::act(const std::map<int, SharedPtr<Player>>& alive_players_map,
                   int player_id,
                   bool is_mafia_round) {
    std::vector<int> hacking_targets;

    for (const auto& [target_id, player_ptr] : alive_players_map) {
        if (target_id != player_id) {
            hacking_targets.push_back(target_id);
        }
    }

    if (!hacking_targets.empty()) {
        selected_target_id = choose_random(hacking_targets);
        Logger::get_instance().log_round(
            "Хакер " + std::to_string(player_id) +
            " выбрал игрока " + std::to_string(selected_target_id) +
            " для взлома.");
    } else {
        Logger::get_instance().log_round(
            "Хакеру " + std::to_string(player_id) + " некого хакать.");
    }

    co_return;
}

Action Hacker::vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
                    int player_id) {
    std::vector<int> possible_targets;

    for (const auto& [target_id, player_ptr] : alive_players_map) {
        if (target_id != player_id) {
            possible_targets.push_back(target_id);
        }
    }

    if (!possible_targets.empty()) {
        selected_target_id = choose_random(possible_targets);
    }

    Logger::get_instance().log_round(
        "Хакер " + std::to_string(player_id) +
        " выбрал игрока " + std::to_string(selected_target_id) +
        " на голосовании.");

    co_return;
}
