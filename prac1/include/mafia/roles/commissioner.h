#pragma once

#include "../core.h"
#include <set>

class Commissioner : public Player {
public:
    int commissioner_target_id = -1;
    std::set<int> known_civilian_ids;

    Role role() const override;

    Action act(const std::map<int, SharedPtr<Player>>& alive_players_map,
               int player_id,
               bool is_mafia_round) override;

    Action vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
                int player_id) override;

    void set_kill(int target_id);
};