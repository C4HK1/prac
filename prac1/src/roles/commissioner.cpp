#include "mafia/roles/commissioner.h"

Role Commissioner::role() const {
    return Role::Commissioner;
}

Action Commissioner::act(const std::map<int, SharedPtr<Player>>& alive_players_map,
                         int player_id,
                         bool is_mafia_round) {
    if (commissioner_target_id != -1 &&
        alive_players_map.find(commissioner_target_id) != alive_players_map.end()) {
        // Commissioner knows a mafia member, prepare to kill
        kill_target_id = commissioner_target_id;
        Logger::get_instance().log_round(
            "Комиссар знает, что игрок " + std::to_string(kill_target_id) + " мафия.");
    } else {
        // Search for mafia members
        commissioner_target_id = -1;
        std::vector<int> investigation_targets;

        for (const auto& [target_id, player_ptr] : alive_players_map) {
            if (target_id != player_id &&
                !known_civilian_ids.contains(target_id)) {
                investigation_targets.push_back(target_id);
            }
        }

        if (!investigation_targets.empty()) {
            int investigated_player_id = choose_random(investigation_targets);

            if (alive_players_map.at(investigated_player_id)->player_class() == Class::Mafia) {
                commissioner_target_id = investigated_player_id;
                Logger::get_instance().log_round(
                    "Комиссар проверил игрока " + std::to_string(investigated_player_id) +
                    " и обнаружил, что он мафия!!!.");
            } else {
                known_civilian_ids.insert(investigated_player_id);
                Logger::get_instance().log_round(
                    "Комиссар проверил игрока " + std::to_string(investigated_player_id) +
                    ", он не мафия.");
            }
        } else {
            Logger::get_instance().log_round(
                "Комиссару некого проверять, все мафии уже известны.");
        }
    }

    co_return;
}

Action Commissioner::vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
                          int player_id) {
    kill_target_id = -1;

    if (commissioner_target_id != -1 &&
        alive_players_map.find(commissioner_target_id) != alive_players_map.end()) {
        // Vote for known mafia member
        selected_target_id = commissioner_target_id;
        Logger::get_instance().log_round(
            "Комиссар " + std::to_string(player_id) +
            " голосует за игрока " + std::to_string(selected_target_id) +
            " (обнаруженный мафия).");
    } else {
        // Vote randomly
        std::vector<int> possible_targets;

        for (const auto& [target_id, player_ptr] : alive_players_map) {
            if (target_id != player_id) {
                possible_targets.push_back(target_id);
            }
        }

        if (!possible_targets.empty()) {
            selected_target_id = choose_random(possible_targets);
            Logger::get_instance().log_round(
                "Комиссар " + std::to_string(player_id) +
                " выбрал игрока " + std::to_string(selected_target_id) +
                " на голосовании.");
        } else {
            Logger::get_instance().log_round(
                "Комиссару " + std::to_string(player_id) + " некуда голосовать.");
        }
    }

    co_return;
}

void Commissioner::set_kill(int target_id) {
    kill_target_id = target_id;
}