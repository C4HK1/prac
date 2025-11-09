#include "mafia/roles/mafia.h"

Role Mafia::role() const {
    return is_don_leader ? Role::Don : Role::Mafia;
}

Action Mafia::act(const std::map<int, SharedPtr<Player>>& alive_players_map,
                  int player_id,
                  bool is_mafia_round) {
    if (is_mafia_round) {
        // Regular mafia member chooses target
        std::vector<int> non_mafia_targets;

        for (const auto& [target_id, player_ptr] : alive_players_map) {
            if (player_ptr->player_class() != Class::Mafia) {
                non_mafia_targets.push_back(target_id);
            }
        }

        if (!non_mafia_targets.empty()) {
            selected_target_id = choose_random(non_mafia_targets);
        }

        Logger::get_instance().log_round(
            "Мафия " + std::to_string(player_id) +
            " выбрала игрока " + std::to_string(selected_target_id) +
            " для убийства.");
    } else {
        // Don decides final target based on mafia votes
        std::map<int, int> target_vote_count;

        for (const auto& [mafia_id, player_ptr] : alive_players_map) {
            if (player_ptr->player_class() == Class::Mafia) {
                auto* mafia_member = dynamic_cast<Mafia*>(player_ptr.get());
                if (mafia_member && mafia_member->selected_target_id != -1) {
                    ++target_vote_count[mafia_member->selected_target_id];
                }
            }
        }

        if (!target_vote_count.empty()) {
            int max_votes = 1;
            std::vector<int> most_voted_targets = { this->selected_target_id };

            for (const auto& [target_id, vote_count] : target_vote_count) {
                if (vote_count > max_votes) {
                    max_votes = vote_count;
                    most_voted_targets = { target_id };
                } else if (vote_count == max_votes && vote_count > 1) {
                    most_voted_targets.push_back(target_id);
                }
            }

            selected_target_id = choose_random(most_voted_targets);
            Logger::get_instance().log_round(
                "Дон мафии " + std::to_string(player_id) +
                " выбрал игрока " + std::to_string(selected_target_id) +
                " на основе голосов мафии.");
        }
    }

    co_return;
}

Action Mafia::vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
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
            "Мафия " + std::to_string(player_id) +
            " выбрал игрока " + std::to_string(selected_target_id) +
            " на голосовании.");
    } else {
        Logger::get_instance().log_round(
            "Мафии " + std::to_string(player_id) + " некуда голосовать.");
    }

    co_return;
}
