#include "mafia/roles/civilian.h"

Role Civilian::role() const {
    return Role::Civilian;
}

Action Civilian::act(const std::map<int, SharedPtr<Player>>& alive_players_map,
                     int player_id,
                     bool is_mafia_round) {
    // Civilians do nothing during night phase
    co_return;
}

Action Civilian::vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
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
            "Мирный " + std::to_string(player_id) +
            " выбрал игрока " + std::to_string(selected_target_id) +
            " на голосовании.");
    } else {
        Logger::get_instance().log_round(
            "Мирному " + std::to_string(player_id) + " некуда голосовать.");
    }

    co_return;
}
