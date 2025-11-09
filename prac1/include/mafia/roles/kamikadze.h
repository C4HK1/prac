#pragma once

#include "../core.h"
#include <set>

class Kamikadze : public Player {
public:
    std::set<int> non_commissioner_ids;
    bool found_commissioner = false;

    Role role() const override;

    Action act(const std::map<int, SharedPtr<Player>>& alive_players_map,
               int player_id,
               bool is_mafia_round) override;

    Action vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
                int player_id) override;
};