#include "mafia/roles/journalist.h"
#include <algorithm>

Role Journalist::role() const {
    return Role::Journalist;
}

Action Journalist::act(const std::map<int, SharedPtr<Player>>& alive_players_map,
                       int player_id,
                       bool is_mafia_round) {
    std::vector<int> investigation_candidates;

    for (const auto& [target_id, player_ptr] : alive_players_map) {
        investigation_candidates.push_back(target_id);
    }

    if (!investigation_candidates.empty()) {
        int first_target_id = choose_random(investigation_candidates);
        selected_target_id = first_target_id;

        // Remove first target from candidates for second selection
        investigation_candidates.erase(
            std::find(investigation_candidates.begin(),
                      investigation_candidates.end(),
                      first_target_id));

        int second_investigation_target = choose_random(investigation_candidates);
        second_target_id = second_investigation_target;

        Class first_faction = alive_players_map.at(first_target_id)->player_class();
        Class second_faction = alive_players_map.at(second_investigation_target)->player_class();

        are_targets_enemies = (first_faction != second_faction) ||
                              (first_faction == Class::Neutral);

        Logger::get_instance().log_round(
            "Журналист " + std::to_string(player_id) +
            " сравнил игроков " + std::to_string(first_target_id) +
            " и " + std::to_string(second_investigation_target) +
            ". Они " + (are_targets_enemies ? "враги!" : "друзья!"));
    }

    co_return;
}

Action Journalist::vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
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
            "Журналист " + std::to_string(player_id) +
            " выбрал игрока " + std::to_string(selected_target_id) +
            " на голосовании.");
    } else {
        Logger::get_instance().log_round(
            "Журналисту " + std::to_string(player_id) + " некуда голосовать.");
    }

    co_return;
}
