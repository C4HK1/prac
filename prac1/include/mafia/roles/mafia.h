#pragma once

#include "../core.h"

class Mafia : public Player {
public:
    bool is_don_leader = false;

    Role role() const override;

    Action act(const std::map<int, SharedPtr<Player>>& alive_players_map,
               int player_id,
               bool is_mafia_round) override;

    Action vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
                int player_id) override;
};
