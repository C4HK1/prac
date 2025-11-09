#include "mafia/roles/kamikadze.h"

Role Kamikadze::role() const {
    return Role::Kamikadze;
}

Action Kamikadze::act(const std::map<int, SharedPtr<Player>>& alive_players_map,
                      int player_id,
                      bool is_mafia_round) {
    std::vector<int> commissioner_candidates;

    // Look for commissioner among non-mafia players not yet checked
    for (const auto& [target_id, player_ptr] : alive_players_map) {
        if (player_ptr->player_class() != Class::Mafia &&
            !non_commissioner_ids.contains(target_id)) {
            commissioner_candidates.push_back(target_id);
        }
    }

    if (!commissioner_candidates.empty()) {
        int investigated_player_id = choose_random(commissioner_candidates);
        selected_target_id = investigated_player_id;

        found_commissioner = (alive_players_map.at(investigated_player_id)->role() == Role::Commissioner);

        Logger::get_instance().log_round(
            "Камикадзе " + std::to_string(player_id) +
            " выбрал игрока " + std::to_string(selected_target_id) +
            " для проверки." + std::string(" Он ") +
            (found_commissioner ? "ком!!!" : "не ком."));

        if (!found_commissioner) {
            non_commissioner_ids.insert(investigated_player_id);
        }
    } else {
        Logger::get_instance().log_round(
            "Нет коммисара для выбора камикадзе");
    }

    co_return;
}

Action Kamikadze::vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
                       int player_id) {
    std::vector<int> non_mafia_targets;

    for (const auto& [target_id, player_ptr] : alive_players_map) {
        if (player_ptr->player_class() != Class::Mafia) {
            non_mafia_targets.push_back(target_id);
        }
    }

    if (!non_mafia_targets.empty()) {
        selected_target_id = choose_random(non_mafia_targets);
        Logger::get_instance().log_round(
            "Камикадзе " + std::to_string(player_id) +
            " выбрал игрока " + std::to_string(selected_target_id) +
            " на голосовании.");
    } else {
        Logger::get_instance().log_round(
            "Камикадзе " + std::to_string(player_id) + " некуда голосовать.");
    }

    co_return;
}
