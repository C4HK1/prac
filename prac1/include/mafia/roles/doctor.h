#pragma once

#include "../core.h"

class Doctor : public Player {
public:
    Role role() const override;

    Action act(const std::map<int, SharedPtr<Player>>& alive_players_map,
               int player_id,
               bool is_mafia_round) override;

    Action vote(const std::map<int, SharedPtr<Player>>& alive_players_map,
                int player_id) override;

    int get_prev_target() const;
    void set_prev_target(int target_id);
};